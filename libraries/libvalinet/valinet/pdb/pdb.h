#ifndef LIBVALINET_PDB_PDB_H_
#define LIBVALINET_PDB_PDB_H_
//
// pdb includes:
// * pdbdump - Small tool to list and query symbols in PDB files.
//   original source code: https://gist.github.com/mridgers/2968595
// * PDBDownloader
//   original source code: https://github.com/rajkumar-rangaraj/PDB-Downloader

#include <valinet/internet/get.h>
#include <stdio.h>
#include <Windows.h>
#include <winbase.h>
#include <shlobj_core.h>
#include <stdint.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define ASSERT(x, m, ...)                   if (!(x)) { fprintf(stderr, m, __VA_ARGS__);    \
                                                exit(-1); }
#define VN_PDB_ONE_MB                       (10240 * 10240)
#define VN_PDB_ADDRESS_OFFSET               0x400000
#define VN_PDB_SYMBOL_HOSTNAME              "msdl.microsoft.com"
#define VN_PDB_SYMBOL_WEB                   "/download/symbols/"
#define VN_PDB_USER_AGENT                   "Microsoft-Symbol-Server/10.0.10036.206"
#define VN_PDB_FORM_HEADERS                 "Content-Type: application/octet-stream;\r\n"
#define VN_PDB_DOWNLOAD_FILE_BUFFER_SIZE    4096

// https://deplinenoise.wordpress.com/2013/06/14/getting-your-pdb-name-from-a-running-executable-windows/
typedef struct _PdbInfo
{
    DWORD     Signature;
    GUID      Guid;
    DWORD     Age;
    char      PdbFileName[1];
};
typedef struct _PdbInfo PdbInfo;

//------------------------------------------------------------------------------
// https://stackoverflow.com/questions/3828835/how-can-we-check-if-a-file-exists-or-not-using-win32-program
int fileExists(char* file)
{
    WIN32_FIND_DATAA FindFileData;
    HANDLE handle = FindFirstFileA(file, &FindFileData);
    int found = handle != INVALID_HANDLE_VALUE;
    if (found)
    {
        FindClose(handle);
    }
    return found;
}

enum e_mode
{
    e_mode_resolve_stdin,
    e_mode_enum_symbols,
};

enum e_enum_type
{
    e_enum_type_symbols,
    e_enum_type_types
};

struct _sym_info
{
    DWORD64     addr;
    int         size;
    char* name;
    char* file;
    int         tag : 8;
    int         line : 24;
};
typedef struct _sym_info sym_info_t;

struct _pool
{
    char* base;
    int     committed;
    int     size;
    int     used;
};
typedef struct _pool pool_t;

typedef int (sort_func_t)(const sym_info_t*, const sym_info_t*);

int                 g_page_size = 0;
HANDLE              g_handle = (HANDLE)0x493;
int                 g_csv_output = 0;
int                 g_sym_count = 0;
enum e_mode         g_mode = e_mode_enum_symbols;
enum e_enum_type    g_enum_type = e_enum_type_symbols;
pool_t              g_symbol_pool;
pool_t              g_string_pool;
extern const char* g_sym_tag_names[];      /* ...at end of file */

void pool_create(pool_t* pool, int size)
{
    pool->base = (char*)VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_READWRITE);
    pool->size = size;
    pool->committed = 0;
    pool->used = 0;
}

void pool_destroy(pool_t* pool)
{
    VirtualFree(pool->base, 0, MEM_RELEASE);
}

void pool_clear(pool_t* pool)
{
    pool->used = 0;
}

void* pool_alloc(pool_t* pool, int size)
{
    int i;
    char* addr;

    ASSERT(size < g_page_size, "Allocation too large!");

    i = pool->used + size;
    if (i >= pool->committed)
    {
        ASSERT(i < pool->size, "Memory pool exhausted.");
        VirtualAlloc((void*)(pool->base + pool->committed), g_page_size,
            MEM_COMMIT, PAGE_READWRITE
        );
        pool->committed += g_page_size;
    }

    addr = pool->base + pool->used;
    pool->used += size;
    return addr;
}

void dbghelp_to_sym_info(SYMBOL_INFO* info, sym_info_t* sym_info)
{
    BOOL ok;
    DWORD disp;
    IMAGEHLP_LINE64 line;

    // General properties
    sym_info->addr = info->Address;
    sym_info->size = info->Size;
    sym_info->tag = info->Tag;

    // Symbol name
    sym_info->name = (char*)pool_alloc(&g_string_pool, info->NameLen + 1);
    memcpy(sym_info->name, info->Name, info->NameLen);

    // Get file and line number info.
    line.SizeOfStruct = sizeof(line);
    ok = SymGetLineFromAddr64(g_handle, info->Address, &disp, &line);
    if ((ok != FALSE) && line.FileName)
    {
        sym_info->line = line.LineNumber;
        sym_info->file = (char*)pool_alloc(&g_string_pool, strlen(line.FileName) + 1);
        memcpy(sym_info->file, line.FileName, strlen(line.FileName));
    }
    else
    {
        sym_info->line = 0;
        sym_info->file = (char*)"?";
    }
}

BOOL CALLBACK enum_proc(SYMBOL_INFO* info, ULONG size, void* param)
{
    sym_info_t* sym_info;

    sym_info = (sym_info_t*)pool_alloc(&g_symbol_pool, sizeof(sym_info_t));
    dbghelp_to_sym_info(info, sym_info);

    ++g_sym_count;

    return TRUE;
}

int create_pools(uintptr_t base_addr)
{
    BOOL ok;
    FILE* in;
    int size, i;
    const char* guide;

    // Fetch PDB file for the module.
    IMAGEHLP_MODULE64 module = { sizeof(module) };
    ok = SymGetModuleInfo64(g_handle, base_addr, &module);
    if (!ok)
    {
        return 0;
    }

    guide = module.LoadedPdbName;

    // An .exe with no symbols available?
    if (!guide || guide[0] == '\0')
    {
        return 0;
    }

    // Get file size.
    fopen_s(&in, guide, "rb");
    ASSERT(in != NULL, "Failed to open pool-size guide file.");

    fseek(in, 0, SEEK_END);
    size = ftell(in);
    fclose(in);

    // Use anecdotal evidence to guess at suitable pool sizes :).
    i = size / 4;
    pool_create(&g_string_pool, (i < VN_PDB_ONE_MB) ? VN_PDB_ONE_MB : i);

    i = size / 25;
    pool_create(&g_symbol_pool, (i < VN_PDB_ONE_MB) ? VN_PDB_ONE_MB : i);

    return 1;
}

uintptr_t load_module(const char* pdb_file)
{
    uintptr_t base_addr = VN_PDB_ADDRESS_OFFSET;

    base_addr = (size_t)SymLoadModuleEx(g_handle, NULL, pdb_file, NULL,
        base_addr, 0x7fffffff, NULL, 0
    );

    return base_addr;
}

INT VnGetSymbols(
    const char* pdb_file,
    DWORD* addresses,
    char** symbols,
    DWORD numOfSymbols,
    FILE* stream
)
{
    DWORD options;
    SYSTEM_INFO sys_info;
    int i;
    uintptr_t base_addr;
    DWORD ok;

    // Get page size.
    GetSystemInfo(&sys_info);
    g_page_size = sys_info.dwPageSize;

    // Initialise DbgHelp
    options = SymGetOptions();
    options &= ~SYMOPT_DEFERRED_LOADS;
    options |= SYMOPT_LOAD_LINES;
    options |= SYMOPT_IGNORE_NT_SYMPATH;
#if ENABLE_DEBUG_OUTPUT
    options |= SYMOPT_DEBUG;
#endif
    options |= SYMOPT_UNDNAME;
    SymSetOptions(options);

    ok = SymInitialize(g_handle, NULL, FALSE);
    if (!ok)
    {
        return -1;
    }

    // Load module.
    base_addr = load_module(pdb_file);
    if (!base_addr)
    {
        SymCleanup(g_handle);
        return -2;
    }

    if (!create_pools(base_addr))
    {
        SymCleanup(g_handle);
        return -3;
    }

    g_sym_count = 0;
    for (i = 0; i < numOfSymbols; ++i)
    {
        SymEnumSymbols(g_handle, base_addr, symbols[i], enum_proc, NULL);
        if (g_sym_count != i + 1)
        {
            SymCleanup(g_handle);
            return -4;
        }
    }

    for (i = 0; i < g_sym_count; ++i)
    {
        sym_info_t* sym_info = ((sym_info_t*)g_symbol_pool.base) + i;
        addresses[i] = sym_info->addr - VN_PDB_ADDRESS_OFFSET;
        fprintf(
            stream,
            "%s\n", sym_info->name
        );
    }

    // Done.
    ok = SymUnloadModule64(g_handle, (DWORD64)base_addr);
    if (!ok)
    {
        SymCleanup(g_handle);
        return -5;
    }

    pool_destroy(&g_string_pool);
    pool_destroy(&g_symbol_pool);

    SymCleanup(g_handle);

    return 0;
}

// adapted from: https://github.com/rajkumar-rangaraj/PDB-Downloader
INT VnDownloadSymbols(
    HMODULE hModule,
    char* dllName,
    char* szLibPath,
    UINT sizeLibPath,
    FILE* stream
)
{
    HANDLE hFile;
    HANDLE hFileMapping;
    LPVOID lpFileBase;
    PBYTE baseImage;
    PIMAGE_DOS_HEADER dosHeader;
#ifdef _WIN64
    PIMAGE_NT_HEADERS64 ntHeader;
#else
    PIMAGE_NT_HEADERS32 ntHeader;
#endif
    PIMAGE_SECTION_HEADER sectionHeader;
    DWORD ptr;
    UINT nSectionCount;
    UINT i;
    uintptr_t offset;
    UINT cbDebug = 0;
    PIMAGE_DEBUG_DIRECTORY imageDebugDirectory;
    PdbInfo* pdb_info = NULL;
    char url[_MAX_PATH];
    ZeroMemory(url, _MAX_PATH * sizeof(char));
    strcat_s(url, _MAX_PATH, VN_PDB_SYMBOL_WEB);

    char fileAddr[_MAX_PATH];
    ZeroMemory(fileAddr, _MAX_PATH * sizeof(char));

    hFile = CreateFileA(
        dllName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 1;
    }

    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == 0)
    {
        CloseHandle(hFile);
        return 2;
    }

    lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (lpFileBase == 0)
    {
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 3;
    }

    baseImage = (PBYTE)lpFileBase;
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 4;
    }

#ifdef _WIN64
    ntHeader = (PIMAGE_NT_HEADERS64)((u_char*)dosHeader + dosHeader->e_lfanew);
#else
    ntHeader = (PIMAGE_NT_HEADERS32)((u_char*)dosHeader + dosHeader->e_lfanew);
#endif
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 5;
    }
    if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress == 0)
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 6;
    }
    cbDebug = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
    ptr = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
    sectionHeader = IMAGE_FIRST_SECTION(ntHeader);
    nSectionCount = ntHeader->FileHeader.NumberOfSections;
    for (i = 0; i <= nSectionCount; ++i, ++sectionHeader)
    {
        if ((sectionHeader->VirtualAddress) > ptr)
        {
            sectionHeader--;
            break;
        }
    }
    if (i > nSectionCount)
    {
        sectionHeader = IMAGE_FIRST_SECTION(ntHeader);
        UINT nSectionCount = ntHeader->FileHeader.NumberOfSections;
        for (i = 0; i < nSectionCount - 1; ++i, ++sectionHeader);
    }
    offset = (uintptr_t)baseImage + ptr + (uintptr_t)sectionHeader->PointerToRawData - (uintptr_t)sectionHeader->VirtualAddress;
    while (cbDebug >= sizeof(IMAGE_DEBUG_DIRECTORY))
    {
        imageDebugDirectory = (PIMAGE_DEBUG_DIRECTORY)(offset);
        offset += sizeof(IMAGE_DEBUG_DIRECTORY);
        if (imageDebugDirectory->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
        {
            pdb_info = (PdbInfo*)((uintptr_t)baseImage + imageDebugDirectory->PointerToRawData);
            if (0 == memcmp(&pdb_info->Signature, "RSDS", 4))
            {
                strcat_s(url, _MAX_PATH, pdb_info->PdbFileName);
                strcat_s(url, _MAX_PATH, "/");
                // https://stackoverflow.com/questions/1672677/print-a-guid-variable
                sprintf_s(
                    url + strlen(url),
                    33,
                    "%08lX%04hX%04hX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                    pdb_info->Guid.Data1,
                    pdb_info->Guid.Data2,
                    pdb_info->Guid.Data3,
                    pdb_info->Guid.Data4[0],
                    pdb_info->Guid.Data4[1],
                    pdb_info->Guid.Data4[2],
                    pdb_info->Guid.Data4[3],
                    pdb_info->Guid.Data4[4],
                    pdb_info->Guid.Data4[5],
                    pdb_info->Guid.Data4[6],
                    pdb_info->Guid.Data4[7]
                );
                sprintf_s(
                    url + strlen(url),
                    4,
                    "%x/",
                    pdb_info->Age
                );

                // Do the same thing but with backslashes for the file address.
                strcat_s(fileAddr, _MAX_PATH, pdb_info->PdbFileName);
                strcat_s(fileAddr, _MAX_PATH, "\\");
                sprintf_s(
                    fileAddr + strlen(fileAddr),
                    33,
                    "%08lX%04hX%04hX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                    pdb_info->Guid.Data1,
                    pdb_info->Guid.Data2,
                    pdb_info->Guid.Data3,
                    pdb_info->Guid.Data4[0],
                    pdb_info->Guid.Data4[1],
                    pdb_info->Guid.Data4[2],
                    pdb_info->Guid.Data4[3],
                    pdb_info->Guid.Data4[4],
                    pdb_info->Guid.Data4[5],
                    pdb_info->Guid.Data4[6],
                    pdb_info->Guid.Data4[7]
                );
                sprintf_s(
                    fileAddr + strlen(fileAddr),
                    4,
                    "%x\\",
                    pdb_info->Age
                );

                strcat_s(url, _MAX_PATH, pdb_info->PdbFileName);

                //strncpy_s(fileAddr, _MAX_PATH - 1, url + sizeof(VN_PDB_SYMBOL_WEB) - 1, _MAX_PATH - sizeof(VN_PDB_SYMBOL_WEB) - 2);

                //fileAddr[_MAX_PATH - 1] = 0;

                break;
            }
        }
        cbDebug -= (UINT)sizeof(IMAGE_DEBUG_DIRECTORY);
    }
    if (pdb_info == NULL)
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 7;
    }
    PathRemoveFileSpecA(szLibPath);
    strcat_s(
        szLibPath,
        sizeLibPath,
        "\\"
    );
    strcat_s(
        szLibPath,  
        sizeLibPath,
        fileAddr
    );

    LSTATUS dir = SHCreateDirectoryExA(NULL, szLibPath, NULL);

    if ( dir != ERROR_SUCCESS && dir != ERROR_FILE_EXISTS && dir != ERROR_ALREADY_EXISTS && dir != ERROR_CANCELLED) {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 8;
    }

    strcat_s(
        szLibPath,
        sizeLibPath,
        pdb_info->PdbFileName
    );

    fprintf(
        stream,
        "%s\n", szLibPath
    );

    UnmapViewOfFile(lpFileBase);
    CloseHandle(hFileMapping);
    CloseHandle(hFile);
    if (fileExists(szLibPath))
    {
        UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
        return 0;
        //DeleteFileA(szLibPath);
    }
    return VnDownloadFile(
        szLibPath,
        (char*)VN_PDB_SYMBOL_HOSTNAME,
        url,
        (char*)VN_PDB_USER_AGENT,
        INTERNET_DEFAULT_HTTP_PORT,
        INTERNET_SERVICE_HTTP,
        NULL,
        (char*)VN_PDB_FORM_HEADERS,
        VN_PDB_DOWNLOAD_FILE_BUFFER_SIZE
    );
}
#endif
