#ifndef LIBVALINET_HOOKING_IATPATCH_H_
#define LIBVALINET_HOOKING_IATPATCH_H_
#include <Windows.h>
#include <DbgHelp.h>
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
#include <stdio.h>
#include <conio.h>
#endif
// https://blog.neteril.org/blog/2016/12/23/diverting-functions-windows-iat-patching/
inline BOOL VnPatchIAT(HMODULE hMod, PSTR libName, PSTR funcName, uintptr_t hookAddr)
{
    // Increment module reference count to prevent other threads from unloading it while we're working with it
    HMODULE module;

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hMod, &module)) return FALSE;

    // Get a reference to the import table to locate the kernel32 entry
    ULONG size;
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToDataEx(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size, NULL);

    // In the import table find the entry that corresponds to kernel32
    BOOL found = FALSE;
    while (importDescriptor->Characteristics && importDescriptor->Name) {
        PSTR importName = (PSTR)((PBYTE)module + importDescriptor->Name);
        if (_stricmp(importName, libName) == 0) {
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
            printf("[PatchIAT] Found %s in IAT.\n", libName);
#endif
            found = TRUE;
            break;
        }
        importDescriptor++;
    }
    if (!found) {
        FreeLibrary(module);
        return FALSE;
    }

    // From the kernel32 import descriptor, go over its IAT thunks to
    // find the one used by the rest of the code to call GetProcAddress
    PIMAGE_THUNK_DATA oldthunk = (PIMAGE_THUNK_DATA)((PBYTE)module + importDescriptor->OriginalFirstThunk);
    PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((PBYTE)module + importDescriptor->FirstThunk);
    while (thunk->u1.Function) {
        PROC* funcStorage = (PROC*)&thunk->u1.Function;

        BOOL bFound = FALSE;
        if (oldthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
        {
            bFound = (!(*((WORD*)&(funcName)+1)) && IMAGE_ORDINAL32(oldthunk->u1.Ordinal) == (DWORD)funcName);
        }
        else
        {
            PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)module + oldthunk->u1.AddressOfData);
            bFound = ((*((WORD*)&(funcName)+1)) && !_stricmp((char*)byName->Name, funcName));
        }

        // Found it, now let's patch it
        if (bFound) {
            // Get the memory page where the info is stored
            MEMORY_BASIC_INFORMATION mbi;
            VirtualQuery(funcStorage, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

            // Try to change the page to be writable if it's not already
            if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect)) {
                FreeLibrary(module);
                return FALSE;
            }

            // Store our hook
            *funcStorage = (PROC)hookAddr;
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
            if ((*((WORD*)&(funcName)+1)))
            {
                printf("[PatchIAT] Patched %s in %s to 0x%p.\n", funcName, libName, hookAddr);
            }
            else
            {
                printf("[PatchIAT] Patched 0x%x in %s to 0x%p.\n", funcName, libName, hookAddr);
            }
#endif

            // Restore the old flag on the page
            DWORD dwOldProtect;
            VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);

            // Profit
            FreeLibrary(module);
            return TRUE;
        }

        thunk++;
        oldthunk++;
    }

    FreeLibrary(module);
    return FALSE;
}

// https://stackoverflow.com/questions/50973053/how-to-hook-delay-imports
inline BOOL VnPatchDelayIAT(HMODULE hMod, PSTR libName, PSTR funcName, uintptr_t hookAddr)
{
    // Increment module reference count to prevent other threads from unloading it while we're working with it
    HMODULE lib;

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hMod, &lib)) return FALSE;

    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)lib;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((uintptr_t)lib + dos->e_lfanew);
    PIMAGE_DELAYLOAD_DESCRIPTOR dload = (PIMAGE_DELAYLOAD_DESCRIPTOR)((uintptr_t)lib +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT].VirtualAddress);
    while (dload->DllNameRVA)
    {
        char* dll = (char*)((uintptr_t)lib + dload->DllNameRVA);
        if (!_stricmp(dll, libName)) {
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
            printf("[PatchDelayIAT] Found %s in IAT.\n", libName);
#endif

            PIMAGE_THUNK_DATA firstthunk = (PIMAGE_THUNK_DATA)((uintptr_t)lib + dload->ImportNameTableRVA);
            PIMAGE_THUNK_DATA functhunk = (PIMAGE_THUNK_DATA)((uintptr_t)lib + dload->ImportAddressTableRVA);
            while (firstthunk->u1.AddressOfData)
            {
                if (firstthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                {
                    if (!(*((WORD*)&(funcName)+1)) && IMAGE_ORDINAL32(firstthunk->u1.Ordinal) == (DWORD)funcName)
                    {
                        DWORD oldProtect;
                        if (VirtualProtect(&functhunk->u1.Function, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) 
                        {
                            functhunk->u1.Function = (uintptr_t)hookAddr;
                            VirtualProtect(&functhunk->u1.Function, sizeof(uintptr_t), oldProtect, &oldProtect);
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
                            printf("[PatchDelayIAT] Patched 0x%x in %s to 0x%p.\n", funcName, libName, hookAddr);
#endif
                            FreeLibrary(lib);
                            return TRUE;
                        }
                        FreeLibrary(lib);
                        return FALSE;
                    }
                }
                else
                {
                    PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)lib + firstthunk->u1.AddressOfData);
                    if ((*((WORD*)&(funcName)+1)) && !_stricmp((char*)byName->Name, funcName))
                    {
                        DWORD oldProtect;
                        if (VirtualProtect(&functhunk->u1.Function, sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect))
                        {
                            functhunk->u1.Function = (uintptr_t)hookAddr;
                            VirtualProtect(&functhunk->u1.Function, sizeof(uintptr_t), oldProtect, &oldProtect);
#ifdef _LIBVALINET_DEBUG_HOOKING_IATPATCH
                            printf("[PatchDelayIAT] Patched %s in %s to 0x%p.\n", funcName, libName, hookAddr);
#endif
                            FreeLibrary(lib);
                            return TRUE;
                        }
                        FreeLibrary(lib);
                        return FALSE;
                    }
                }
                functhunk++;
                firstthunk++;
            }
        }
        dload++;
    }
    FreeLibrary(lib);
    return FALSE;
}
#endif