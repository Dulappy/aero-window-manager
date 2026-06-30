#include "hookdwm.h"
#include "awmdll.h"
#include "awmerrors.h"
#include "globals.h"

#include <Windows.h>
#include <stdint.h>

HMODULE hModule = NULL;
FILE* stream = NULL;
funchook_t* funchook = NULL;

HMODULE hudwm = NULL;

HANDLE AWM::Init::GetHeap() {
    g_hProcessHeap = GetProcessHeap();
    return g_hProcessHeap;
}
void* AWM::HeapImpl::Alloc(size_t cb) {
    if (cb == 0) cb = 1;
    return HeapAlloc(g_hProcessHeap, 0, cb);
}
void AWM::HeapImpl::Free(void* p) {
    if (p) HeapFree(g_hProcessHeap, 0, p);
}

__declspec(dllexport) DWORD WINAPI main(DWORD* dword) {
    int rv = 0;
    rv = CoInitialize(NULL);
    funchook = funchook_create();

    // Create console window for logging
    FILE* conout;

    stream = stdout;
#if AWM_DEBUG == TRUE
    if (!AllocConsole());
    if (freopen_s(
        &conout,
        "CONOUT$",
        "w",
        stdout)
        );
    fprintf(
        stream,
        "  Aero Window Manager Logs  \n============================\n"
    );
#endif

    // -------------------------------------------------------------------

    if (!AWM::Init::GetHeap()) {
        rv = ERR_FAILEDTOGETHEAP;
        goto cleanup;
    }

    rv = LoadSymbols(&hModule, &hudwm, uDWM_addresses, symNames, stream);
    if (rv != AWM_SUCCESS) {
        goto cleanup;
    }

    rv = HookFunctions();
    if (rv != AWM_SUCCESS) {
        goto cleanup;
    }

    //rv = InitFactories();
    /*if (rv < 0) {
        goto cleanup;
    }*/

cleanup:
    if (rv == AWM_SUCCESS) {
        printf("AWM has initialized successfully.\n");
    }
    else {
        printf("AWM has failed to initialize and will now shut down. Please exit the injector if it is still running.\nReturn value: %d\n", rv);

        funchook_uninstall(funchook, 0);
        funchook_destroy(funchook);
        FreeLibraryAndExitThread(
            hModule,
            rv
        );
    }

    /*HANDLE hEvent = CreateEventW(NULL, true, false, L"awmsettingschanged");
    CreateThread(NULL, 0, ListenForSettingsChange, hEvent, 0, NULL);

    AWM_GetDWMWindowAtlas();*/

    return rv;
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}