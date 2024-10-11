#include <valinet/hooking/exeinject.h>
#include <valinet/pdb/pdb.h>
#include <stdio.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <wtsapi32.h>
#include <userenv.h>
#pragma comment(lib, "Shlwapi.lib")

#define SYMBOLS_PATH "\\symbols\\"

#define ERR_GET_TOKEN 1;
#define ERR_REVERT 2;
#define ERR_IMPERSONATION 3;

FILE* stream = NULL;
BOOL firstCrash = TRUE;

DWORD AWM_HandleCrash() {
    return 0;
}

int main() {
    FILE* conout;
    TCHAR szLibPath[_MAX_PATH];
    CHAR szSymPath[_MAX_PATH];

    // Prevent multiple instances (causes DWM crash loop)
    CreateMutex(NULL, TRUE, L"AWM_Injector");
    if (ERROR_ALREADY_EXISTS == GetLastError())
        return 1;

    stream = stdout;
    if (!AllocConsole());
    if (freopen_s(
        &conout,
        "CONOUT$",
        "w",
        stdout)
        );
    fprintf(
        stream,
        "Aero Window Manager\n===================\n"
    );
    GetModuleFileName(
        GetModuleHandle(NULL),
        szLibPath,
        _MAX_PATH
    );

    PathRemoveFileSpec(szLibPath);
    wcscat_s(
        szLibPath,
        _MAX_PATH,
        (TCHAR*)L"\\awmdll.dll"
    );

    /*BOOL bResult = FALSE;
    DWORD sessionID = WTSGetActiveConsoleSessionId();

    HANDLE hToken = NULL;
    HKEY hKey = NULL;
    bResult = WTSQueryUserToken(sessionID, &hToken);
    if (!bResult) {
        fprintf(stream, "token error");
        return ERR_GET_TOKEN;
    }
    bResult = ImpersonateLoggedOnUser(hToken);
    if (!bResult) {
        fprintf(stream, "impersonation error");
        return ERR_IMPERSONATION;
    }

    // i think you can't pass HKEYs into other processes...
    if (RegOpenCurrentUser(KEY_ALL_ACCESS, &hKey)) {
        fprintf(stream, "error opening key");
    }
    fprintf(stream, "%i\n", hKey);
    DWORD dwords[3] = {1, 2, 3};


    bResult = RevertToSelf();
    if (!bResult) {
        fprintf(stream, "revert error");
        return ERR_REVERT;
    }*/

    return VnInjectAndMonitorProcess(
        szLibPath,
        sizeof(szLibPath),
        (char*)"main",
        (TCHAR*)L"dwm.exe",
        (TCHAR*)L"AWM",
        (LPTHREAD_START_ROUTINE)AWM_HandleCrash,
        GetModuleHandle(NULL),
        stream,
        1000,
        NULL,
        FALSE,
        1000,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        NULL,
        NULL,
        TRUE
    );
}