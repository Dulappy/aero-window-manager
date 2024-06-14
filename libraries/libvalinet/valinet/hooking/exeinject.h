#ifndef LIBVALINET_HOOKING_EXEINJECT_H_
#define LIBVALINET_HOOKING_EXEINJECT_H_
#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <winbase.h>
#include <processthreadsapi.h>

#define MODULE_ARRAY_INITIAL_SIZE           100
#define WM_APP_CRASHED                      WM_USER + 100
#define ERROR_LOAD_LIBRARY                  0x2
#define ERROR_NO_MAIN_IN_INJECTION_LIB      0x21
#define ERROR_GETMODULEHANDLE_KERNEL32      0x3
#define ERROR_GETPROCADDRESS_LOADLIBRARYW   0x31
#define ERROR_GETPROCADDRESS_FREELIBRARY    0x32
#define ERROR_APP_NOT_RUNNING               0x4
#define ERROR_APP_OPENPROCESS               0x41
#define ERROR_APP_VIRTUALALLOC              0x5
#define ERROR_APP_WRITEPROCESSMEMORY        0x51
#define ERROR_FAILED_TO_INJECT              0x6
#define ERROR_FAILED_TO_RUN_ENTRY_POINT     0x61
#define ERROR_CANNOT_FIND_LIBRARY_IN_APP    0x8
#define ERROR_MODULE_ARRAY_ALLOC            0x81
#define ERROR_APP_MODULE_ENUM               0x82
#define ERROR_MODULE_ARRAY_REALLOC          0x83
#define ERROR_CANNOT_GET_ADDRESS_MODULE     0x84
#define ERROR_CANNOT_RUN_INJECTION_MAIN     0x91
#define ERROR_CANNOT_DETERMINE_STATUS_DLL   0x93
#define ERROR_CREATE_MESSAGE_WINDOW         0x10
#define ERROR_REGISTER_APP_WATCH            0x11
#define ERROR_REGISTER_EXIT_HANDLER         0x12
#define ERROR_MESSAGE_QUEUE                 0x13
#define ERROR_APP_CRASHED                   0x222
#define ERROR_FAILED_TO_CALL_FREELIBRARY    0x500
#define ERROR_FREELIBRARY_FAILED            0x501
#define HOOK_CLASSIC_LOOKUP                 0x0
#define HOOK_BY_SHELLCODE                   0x1
#define HOOK_METHOD                         HOOK_CLASSIC_LOOKUP

LRESULT CALLBACK VnWindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_APP_CRASHED:
        PostMessage(
            hWnd, 
            WM_QUIT, 
            ERROR_APP_CRASHED, 
            0
        );
        break;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID CALLBACK libvalinet_hooking_exeinject_WaitForDWMToCrash(
    _In_ PVOID   lpParameter,
    _In_ BOOLEAN TimerOrWaitFired
)
{
    SendMessage(
        (HWND)(lpParameter), 
        WM_APP_CRASHED, 
        0, 
        0
    );
}

DWORD libvalinet_hooking_exeinject_ExitHandler(
    HANDLE hProcess,
    HMODULE hMod,
    uintptr_t hInjection,
    FILE* stream,
    LPTHREAD_START_ROUTINE lpCustomExitHandler
)
{
    HANDLE hThread = NULL;
    DWORD dwThreadExitCode = 0;

    hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)((uintptr_t)hMod + (uintptr_t)hInjection),
        NULL,
        0,
        NULL
    );
    WaitForSingleObject(
        hThread,
        INFINITE
    );
    GetExitCodeThread(
        hThread,
        &dwThreadExitCode
    );
    if (dwThreadExitCode)
    {
        if (stream)
        {
            fprintf(
                stream, 
                "E. Error while unhooking application (%d).\n", 
                dwThreadExitCode
            );
        }
    }

    if (stream)
    {
        fprintf(
            stream, 
            "E. Successfully unhooked application.\n"
        );
    }

    if (lpCustomExitHandler)
    {
        lpCustomExitHandler(NULL);
    }

    return ERROR_SUCCESS;
}

DWORD libvalinet_hooking_exeinject_FreeRemoteLibrary(
    HANDLE hProcess,
    HANDLE hMod,
    FARPROC hAddrFreeLibrary,
    FILE* stream
)
{
    DWORD dwThreadExitCode = 0;
    HANDLE hThread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)hAddrFreeLibrary,
        hMod,
        0,
        NULL
    );
    if (hThread == NULL)
    {

        if (stream)
        {
            fprintf(
                stream, 
                "ERROR: Unable to call FreeLibrary.\n"
            );
        }

        TerminateProcess(hProcess, 0);
        return ERROR_FAILED_TO_CALL_FREELIBRARY;
    }
    WaitForSingleObject(
        hThread,
        INFINITE
    );
    BOOL bResult = GetExitCodeThread(
        hThread,
        &dwThreadExitCode
    );
    if (!dwThreadExitCode || !bResult)
    {

        if (stream)
        {
            fprintf(
                stream, 
                "ERROR: FreeLibrary failed.\n"
            );
        }

        return ERROR_FREELIBRARY_FAILED;
    }

    if (stream)
    {
        fprintf(
            stream, 
            "FreeLibrary OK.\n"
        );
    }

    return ERROR_SUCCESS;
}

int VnInjectAndMonitorProcess(
    TCHAR* szLibPath,
    DWORD dwLibPathSize,
    char* entryPoint,
    const TCHAR* szProcessName,
    const TCHAR* szClassName,
    LPTHREAD_START_ROUTINE lpCrashOrFailureCallback,
    HINSTANCE hInstance,
    FILE* stream,
    DWORD dwRestartDelay,
    LRESULT (*lpWindowProc)(HWND, UINT, WPARAM, LPARAM),
    BOOL bWaitForProcess,
    DWORD dwCheckDelay,
    DWORD dwStartupDelay,
    LPTHREAD_START_ROUTINE lpCustomExitHandler,
    LPHANDLE lphProcess,
    HMODULE* lphMod,
    LPVOID* lphInjection,
    int* messages,
    DWORD messagesSize,
    HWND* lphwnd,
    LPVOID* lpParam,
    bool onlyActiveSID
)
{
    SIZE_T i = 0;
    BOOL bRet = FALSE;
    DWORD dwRet = 0;
    BOOL bErr = FALSE;
    HANDLE hProcess = NULL;
    HMODULE hMod = NULL;
    uintptr_t hInjection = NULL;
    HMODULE hKernel32 = NULL;
    FARPROC hAdrLoadLibraryW = NULL;
    FARPROC hAdrFreeLibrary = NULL;
    void* pLibRemote = NULL;
    void* pShellCode = NULL;
    wchar_t szTmpLibPath[_MAX_PATH];
    BOOL bResult = FALSE;
    HANDLE hSnapshot = NULL;
    PROCESSENTRY32 stProcessEntry = { 0 };
    DWORD dwProcessId = 0;
    HMODULE* hMods = NULL;
    DWORD hModuleArrayInitialBytesInitial =
        MODULE_ARRAY_INITIAL_SIZE * sizeof(HMODULE);
    DWORD hModuleArrayInitialBytes = hModuleArrayInitialBytesInitial;
    DWORD hModuleArrayBytesNeeded = 0;
    HMODULE hInjectionDll = NULL;
    FARPROC hInjectionMainFunc = NULL;
    WNDCLASS wc = { 0 };
    HWND hWnd = NULL;
    MSG msg = { 0 };
    HANDLE hWaitObject = NULL;
    HANDLE hThread = NULL;
    DWORD dwThreadExitCode = 0;
    SIZE_T dwBytesRead = 0;
    DWORD dwSessionId = 0;
    BYTE shellCode[] =
    {
        0x53, 0x48, 0x89, 0xE3, 0x48, 0x83, 0xEC, 0x20, 0x66, 0x83,
        0xE4, 0xC0, 0x48, 0xB9, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        0x41, 0x41, 0x48, 0xBA, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
        0x41, 0x41, 0xFF, 0xD2, 0x48, 0xBA, 0x41, 0x41, 0x41, 0x41,
        0x41, 0x41, 0x41, 0x41, 0x48, 0x89, 0x02, 0x48, 0x89, 0xDC,
        0x5B, 0xC3
    };
    // shell code injection technique on x64 courtesy of:
    // https://clymb3r.wordpress.com/2013/05/26/implementing-remote-loadlibrary-and-remote-getprocaddress-using-powershell-and-assembly/
    /* x64 assembly is:
    [SECTION .text]
    global _start
    _start:
        ; Save rsp and setup stack for function call
        push rbx
        mov rbx, rsp
        sub rsp, 0x20
        and sp, 0xffc0

        ; Call LoadLibraryA
        mov rcx, 0x4141414141414141 ; Ptr to string of library, set by injector
        mov rdx, 0x4141414141414141 ; Address of LoadLibrary, set by injector
        call rdx

        mov rdx, 0x4141414141414141 ; Ptr to save result, set by injector
        mov [rdx], rax

        ; Fix stack
        mov rsp, rbx
        pop rbx
        ret
     */

    // Step 1: Print library path
    wprintf(
        L"1. Hook Library Path: %s\n",
        szLibPath
    );

    // Step 2: Get DLL entry point address
    hInjectionDll = LoadLibrary(szLibPath);
    if (hInjectionDll == NULL)
    {
        if (stream)
        {
            fprintf(
                stream,
                "2. ERROR: Cannot load injection library.\n"
            );
        }
        return ERROR_LOAD_LIBRARY;
    }
    hInjectionMainFunc = GetProcAddress(
        hInjectionDll,
        entryPoint
    );
    if (hInjectionMainFunc == NULL)
    {
        if (stream)
        {
            fprintf(
                stream,
                "2. ERROR: Injection library lacks entry point.\n"
            );
        }
        return ERROR_NO_MAIN_IN_INJECTION_LIB;
    }
    hInjection = (uintptr_t)(hInjectionMainFunc) - (uintptr_t)(hInjectionDll);
    if (lphInjection)
    {
        *lphInjection = (LPVOID)hInjection;
    }
    FreeLibrary(hInjectionDll);
    wprintf(
        L"2. Hook Library Entry Point: 0x%x\n",
        hInjection
    );

    // Step 3: Get address of LoadLibraryW & FreeLibrary
    hKernel32 = GetModuleHandle(L"Kernel32");
    if (hKernel32 == NULL)
    {
        if (stream)
        {
            fprintf(
                stream,
                "3. ERROR: Cannot find address of Kernel32.\n"
            );
        }
        return ERROR_GETMODULEHANDLE_KERNEL32;
    }
    hAdrLoadLibraryW = GetProcAddress(
        hKernel32,
        "LoadLibraryW"
    );
    if (hAdrLoadLibraryW == NULL)
    {
        if (stream)
        {
            fprintf(
                stream,
                "3. ERROR: Cannot find address of LoadLibraryW.\n"
            );
        }
        return ERROR_GETPROCADDRESS_LOADLIBRARYW;
    }
    wprintf(
        L"3. LoadLibraryW address: %d\n",
        hAdrLoadLibraryW
    );
    hAdrFreeLibrary = GetProcAddress(
        hKernel32,
        "FreeLibrary"
    );
    if (hAdrFreeLibrary == NULL)
    {

        if (stream)
        {
            fprintf(
                stream, 
                "3. ERROR: Cannot find address of FreeLibrary.\n"
            );
        }

        return ERROR_GETPROCADDRESS_FREELIBRARY;
    }
    wprintf(
        L"3. FreeLibrary address: %d\n",
        hAdrFreeLibrary
    );
    // Repeatedly inject application
    while (TRUE)
    {
        // Step 4: Find application.exe
        do
        {
            stProcessEntry.dwSize = sizeof(PROCESSENTRY32);
            hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
            if (Process32First(hSnapshot, &stProcessEntry) == TRUE)
            {
                while (Process32Next(hSnapshot, &stProcessEntry) == TRUE)
                {
                    if (!wcscmp(stProcessEntry.szExeFile, szProcessName))
                    {
                        dwProcessId = stProcessEntry.th32ProcessID;
                        bResult = ProcessIdToSessionId(dwProcessId, &dwSessionId);
                        if (!bResult)
                        {
                            if (stream)
                            {
                                fprintf(
                                    stream,
                                    "4. ERROR: Cannot get session ID from process ID.\n"
                                );
                            }
                            return ERROR_APP_OPENPROCESS;
                        }
                        if (dwSessionId == WTSGetActiveConsoleSessionId() || !onlyActiveSID) {
                            hProcess = OpenProcess(
                                PROCESS_ALL_ACCESS,
                                FALSE,
                                dwProcessId
                            );
                            if (lphProcess)
                            {
                                *lphProcess = hProcess;
                            }
                            if (hProcess == NULL)
                            {
                                if (stream)
                                {
                                    fprintf(
                                        stream,
                                        "4. ERROR: Cannot get handle to application.\n"
                                    );
                                }
                                return ERROR_APP_OPENPROCESS;
                            }
                            if (stream)
                            {
                                fprintf(
                                    stream,
                                    "4. Found application, PID: %d\n",
                                    dwProcessId
                                );
                            }
                            break;
                        }
                    }
                }
            }
            CloseHandle(hSnapshot);
            if (bWaitForProcess)
            {
                if (dwProcessId == 0)
                {
                    if (stream)
                    {
                        fprintf(stream, "4. Waiting for application to start...\n");
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                if (dwProcessId == 0)
                {
                    if (stream)
                    {
                        fprintf(
                            stream,
                            "4. ERROR: Application is not running.\n"
                        );
                    }
                    return ERROR_APP_NOT_RUNNING;
                }
            }
            Sleep(dwCheckDelay);
        }
        while (dwProcessId == 0);
        Sleep(dwStartupDelay);

        // Step 5: Write path to library in application's memory
        pLibRemote = VirtualAllocEx(
            hProcess,
            NULL,
            dwLibPathSize,
            MEM_COMMIT,
            PAGE_READWRITE
        );
        if (pLibRemote == NULL)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "5. ERROR: Cannot alloc memory in application.\n"
                );
            }
            return ERROR_APP_VIRTUALALLOC;
        }
        bResult = WriteProcessMemory(
            hProcess,
            pLibRemote,
            (void*)szLibPath,
            dwLibPathSize,
            NULL
        );
        if (!bResult)
        {

            if (stream)
            {
                fprintf(
                    stream,
                    "5. ERROR: Cannot write memory in application.\n"
                );
            }

            VirtualFreeEx(
                hProcess,
                (LPVOID)pLibRemote,
                0,
                MEM_RELEASE
            );
            return ERROR_APP_WRITEPROCESSMEMORY;
        }
        if (stream)
        {
            fprintf(
                stream,
                "5. Wrote library path in application's memory.\n"
            );
        }

#if HOOK_METHOD == HOOK_BY_SHELLCODE && _WIN64
        // Step 6: Write shell code to application's memory
        pShellCode = VirtualAllocEx(
            hProcess,
            NULL,
            dwLibPathSize,
            MEM_COMMIT,
            PAGE_EXECUTE_READWRITE
        );
        if (pShellCode == NULL)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "6. ERROR: Cannot alloc memory in application.\n"
                );
            }
            VirtualFreeEx(
                hProcess,
                (LPVOID)pLibRemote,
                0,
                MEM_RELEASE
            );
            return ERROR_APP_VIRTUALALLOC;
        }
        // Address of string containing path of module to load
        *((uintptr_t*)(shellCode + 14)) = (uintptr_t)pLibRemote;
        // Address of function to call (LoadLibraryW)
        *((uintptr_t*)(shellCode + 24)) = (uintptr_t)hAdrLoadLibraryW;
        // Address to write return value to
        // Writing on top of the path in order to spare some calls,
        // since the path is not required after calling LoadLibraryW
        *((uintptr_t*)(shellCode + 36)) = (uintptr_t)pLibRemote;
        bResult = WriteProcessMemory(
            hProcess,
            pShellCode,
            (void*)shellCode,
            sizeof(shellCode),
            NULL
        );
        if (!bResult)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "6. ERROR: Cannot write memory in application.\n"
                );
            }
            VirtualFreeEx(
                hProcess,
                (LPVOID)pLibRemote,
                0,
                MEM_RELEASE
            );
            VirtualFreeEx(
                hProcess,
                (LPVOID)pShellCode,
                0,
                MEM_RELEASE
            );
            return ERROR_APP_WRITEPROCESSMEMORY;
        }
        if (stream)
        {
            fprintf(
                stream, 
                "6. Wrote shell code in application's memory.\n"
            );
        }

        // Step 7: Call shell code
        hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)pShellCode,
            NULL,
            0,
            NULL
        );
        if (hThread == NULL)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "7. ERROR: Failed to inject library into application.\n"
                );
            }
            VirtualFreeEx(
                hProcess,
                (LPVOID)pLibRemote,
                0,
                MEM_RELEASE
            );
            VirtualFreeEx(
                hProcess,
                (LPVOID)pShellCode,
                0,
                MEM_RELEASE
            );
            return ERROR_FAILED_TO_INJECT;
        }
        WaitForSingleObject(
            hThread,
            INFINITE
        );
        bResult = GetExitCodeThread(
            hThread,
            &dwThreadExitCode
        );
        if (!dwThreadExitCode || !bResult)
        {
            if (stream)
            {
                fprintf(
                    stream, 
                    "7. ERROR: Failed to run lib entry point in application.\n"
                );
            }
            bErr = TRUE;
        }
        else
        {
            if (stream)
            {
                fprintf(
                    stream, 
                    "7. Successfully injected library into application.\n"
                );
            }
        }


        // Step 8: Check result and cleanup
        bResult = ReadProcessMemory(
            hProcess,
            pLibRemote,
            &hMod,
            sizeof(HMODULE),
            &dwBytesRead
        );
        if (!bResult || dwBytesRead != sizeof(HMODULE))
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "8. ERROR: Cannot get address of loaded module.\n"
                );
            }
            VirtualFreeEx(
                hProcess,
                (LPVOID)pLibRemote,
                0,
                MEM_RELEASE
            );
            VirtualFreeEx(
                hProcess,
                (LPVOID)pShellCode,
                0,
                MEM_RELEASE
            );
            TerminateProcess(hProcess, 0);
            return ERROR_CANNOT_GET_ADDRESS_MODULE;
        }
        if (lphMod)
        {
            *lphMod = hMod;
        }
        VirtualFreeEx(
            hProcess,
            (LPVOID)pLibRemote,
            0,
            MEM_RELEASE
        );
        VirtualFreeEx(
            hProcess,
            (LPVOID)pShellCode,
            0,
            MEM_RELEASE
        );
        if (bErr)
        {
            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess, 
                hMod, 
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_FAILED_TO_RUN_ENTRY_POINT;
        }
#else
        // Step 6: Load library in application
        hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)hAdrLoadLibraryW,
            pLibRemote,
            0,
            NULL
        );
        if (hThread == NULL)
        {
            if (stream)
            {
                fprintf(
                    stream, 
                    "6. ERROR: Failed to inject library into application.\n"
                );
            }
            return ERROR_FAILED_TO_INJECT;
        }
        WaitForSingleObject(
            hThread,
            INFINITE
        );
        bResult = GetExitCodeThread(
            hThread,
            &dwThreadExitCode
        );
        if (!dwThreadExitCode || !bResult)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "6. ERROR: Failed to run library entry point in "
                    "application.\n"
                );
            }
            return ERROR_FAILED_TO_RUN_ENTRY_POINT;
        }
        hMod = (HMODULE)dwThreadExitCode;
        if (lphMod)
        {
            *lphMod = hMod;
        }
        if (stream)
        {
            fprintf(
                stream,
                "6. Successfully injected library into application.\n"
            );
        }

        // Step 7: Free path from application's memory
        VirtualFreeEx(
            hProcess,
            (LPVOID)pLibRemote,
            0,
            MEM_RELEASE
        );
        if (stream)
        {
            fprintf(
                stream,
                "7. Freed path from application's memory.\n"
            );
        }

        // Step 8: Get address of library in application's memory
        // This is actually optional, but application is not tested without
        hModuleArrayInitialBytes = hModuleArrayInitialBytesInitial;
        hMods = (HMODULE*)calloc(
            hModuleArrayInitialBytes,
            1
        );
        if (hMods == NULL)
        {

            if (stream)
            {
                fprintf(
                    stream,
                    "8. ERROR: Cannot allocate module array.\n"
                );
            }

            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess, 
                hMod, 
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_MODULE_ARRAY_ALLOC;
        }
        bResult = EnumProcessModulesEx(
            hProcess,
            hMods,
            hModuleArrayInitialBytes,
            &hModuleArrayBytesNeeded,
            LIST_MODULES_ALL
        );
        if (!bResult)
        {

            if (stream)
            {
                fprintf(
                    stream,
                    "8. ERROR: Unable to enum modules in application.\n"
                );
            }

            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess, 
                hMod, 
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_APP_MODULE_ENUM;
        }
        if (hModuleArrayInitialBytes < hModuleArrayBytesNeeded)
        {
            hMods = (HMODULE*)realloc(
                hMods,
                hModuleArrayBytesNeeded
            );
            if (hMods == NULL)
            {

                if (stream)
                {
                    fprintf(
                        stream,
                        "8. ERROR: Cannot reallocate module array.\n"
                    );
                }

                if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                    hProcess, 
                    hMod, 
                    hAdrFreeLibrary,
                    stream
                ))
                {
                    TerminateProcess(hProcess, 0);
                    return dwRet;
                }
                return ERROR_MODULE_ARRAY_REALLOC;
            }
            hModuleArrayInitialBytes = hModuleArrayBytesNeeded;
            bResult = EnumProcessModulesEx(
                hProcess,
                hMods,
                hModuleArrayInitialBytes,
                &hModuleArrayBytesNeeded,
                LIST_MODULES_ALL
            );
            if (!bResult)
            {

                if (stream)
                {
                    fprintf(
                        stream,
                        "8. ERROR: Unable to enum modules in application.\n"
                    );
                }

                if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                    hProcess,
                    hMod, 
                    hAdrFreeLibrary,
                    stream
                ))
                {
                    TerminateProcess(hProcess, 0);
                    return dwRet;
                }
                return ERROR_APP_MODULE_ENUM;
            }
        }
        CharLower(szLibPath);
        if (hModuleArrayBytesNeeded / sizeof(HMODULE) == 0)
        {
            i = -1;
        }
        else
        {
            for (i = 0; i <= hModuleArrayBytesNeeded / sizeof(HMODULE); ++i)
            {
                if (i == hModuleArrayBytesNeeded / sizeof(HMODULE))
                {
                    i = -1;
                    break;
                }
                bResult = GetModuleFileNameEx(
                    hProcess,
                    hMods[i],
                    szTmpLibPath,
                    _MAX_PATH
                );
                if (bResult)
                {
                    CharLower(szTmpLibPath);
                    if (!wcscmp(szTmpLibPath, szLibPath))
                    {
                        break;
                    }
                }
            }
        }
        if (i == -1)
        {

            if (stream)
            {
                fprintf(
                    stream,
                    "8. ERROR: Cannot find library in application's memory.\n"
                );
            }

            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess, 
                hMod, 
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_CANNOT_FIND_LIBRARY_IN_APP;
        }

        wprintf(
            L"8. Found library in application's memory (%d/%d).\n",
            i,
            hModuleArrayBytesNeeded / sizeof(HMODULE)
        );

        hMod = hMods[i];
        if (lphMod)
        {
            *lphMod = hMod;
        }
        free(hMods);
#endif

        // Step 9: Register and create window
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = lpWindowProc ? lpWindowProc : VnWindowProc;
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wc.hInstance = hInstance;
        wc.lpszClassName = szClassName;
        wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
        RegisterClass(&wc);
        hWnd = CreateWindowEx(
            0,                      // Optional window styles
            szClassName,             // Window class
            TEXT(""),               // Window text
            WS_OVERLAPPEDWINDOW,    // Window style
            // Size and position
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            NULL,                   // Parent window    
            NULL,                   // Menu
            hInstance,              // Instance handle
            NULL                    // Additional application data
        );
        if (!hWnd)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "10. Failed to create message window (%d).\n",
                    GetLastError()
                );
            }
            libvalinet_hooking_exeinject_ExitHandler(
                hProcess, 
                hMod, 
                hInjection,
                stream,
                lpCustomExitHandler
            );
            return ERROR_CREATE_MESSAGE_WINDOW;
        }
        if (stream)
        {
            fprintf(
                stream,
                "10. Successfully created message window (%d).\n",
                hWnd
            );
        }
        for (i = 0; i < messagesSize; ++i)
        {
            ChangeWindowMessageFilter(
                messages[i], 
                MSGFLT_ADD
            );
        }
        if (lphwnd)
        {
            *lphwnd = hWnd;
        }

        // Step 10: Run DLL's entry point
        hThread = CreateRemoteThread(
            hProcess,
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)((uintptr_t)hMod + (uintptr_t)hInjection),
            lpParam ? *lpParam : NULL,
            0,
            NULL
        );
        if (hThread == NULL)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "9. ERROR: Cannot execute injection entry point.\n"
                );
            }
            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess,
                hMod,
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_CANNOT_RUN_INJECTION_MAIN;
        }
        WaitForSingleObject(
            hThread,
            INFINITE
        );
        bResult = GetExitCodeThread(
            hThread,
            &dwThreadExitCode
        );
        if (!bResult)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "9. ERROR: Cannot determine status of injected DLL.\n"
                );
            }
            if (dwRet = libvalinet_hooking_exeinject_FreeRemoteLibrary(
                hProcess,
                hMod,
                hAdrFreeLibrary,
                stream
            ))
            {
                TerminateProcess(hProcess, 0);
                return dwRet;
            }
            return ERROR_CANNOT_DETERMINE_STATUS_DLL;
        }
        if (dwThreadExitCode)
        {
            if (lpCrashOrFailureCallback != NULL)
            {
                if (dwRet =
                    lpCrashOrFailureCallback((LPVOID)dwThreadExitCode))
                {
                    return dwRet;
                }
            }
            else
            {
                return dwThreadExitCode;
            }
        }
        if (stream)
        {
            fprintf(
                stream,
                "9. Successfully hooked application.\n"
            );
        }

        // Step 11: Listen for application crashes
        RegisterWaitForSingleObject(
            &hWaitObject,
            hProcess,
            (WAITORTIMERCALLBACK)
                (libvalinet_hooking_exeinject_WaitForDWMToCrash),
            (PVOID)(hWnd),
            INFINITE,
            WT_EXECUTEONLYONCE
        );
        if (!hWaitObject)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "11. Unable to register for watching application.\n"
                );
            }
            libvalinet_hooking_exeinject_ExitHandler(
                hProcess, 
                hMod, 
                hInjection,
                stream,
                lpCustomExitHandler
            );
            return ERROR_REGISTER_APP_WATCH;
        }
        if (stream)
        {
            fprintf(
                stream,
                "11. Registered for watching application.\n"
            );
        }

        // Step 13: Listen for messages
        if (stream)
        {
            fprintf(
                stream,
                "Listening for messages...\n"
            );
        }
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (bRet == -1)
            {
                libvalinet_hooking_exeinject_ExitHandler(
                    hProcess, 
                    hMod, 
                    hInjection,
                    stream,
                    lpCustomExitHandler
                );
                return ERROR_MESSAGE_QUEUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        if (msg.wParam != ERROR_APP_CRASHED)
        {
            if (stream)
            {
                fprintf(
                    stream,
                    "Shutting down application...\n"
                );
            }
            libvalinet_hooking_exeinject_ExitHandler(
                hProcess, 
                hMod, 
                hInjection,
                stream,
                lpCustomExitHandler
            );
            return ERROR_SUCCESS;
        }
        // not required; in fact, it will post a WM_QUIT 
        // for the next window we spawn; really stupid idea
        //DestroyWindow(hWnd);
        UnregisterClass(szClassName, hInstance);

        if (stream)
        {
            fprintf(
                stream,
                "Application was restarted, rehooking...\n"
            );
        }

        // wait a bit for application to respawn
        Sleep(dwRestartDelay);
    }
    return ERROR_SUCCESS;
}

#endif