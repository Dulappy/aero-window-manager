#pragma once
#include "hookdwm.h"
#include "awmdll.h"

#include "valinet/ini/ini.h"
#include "valinet/pdb/pdb.h"
#include "miossymbols.h"
#include "awmerrors.h"
#include "funchook.h"

#include "baseobject.h"
#include "toplevelwindow.h"
#include "windowdata.h"

#include <Psapi.h>
#include <atlbase.h>
#include <atlconv.h>
#include <windows.h>
#include <pathcch.h>
#include <stdio.h>

// ===========================================================================
//  LOAD SYMBOLS
// ===========================================================================

LPCWSTR symNames[] = {
    CTLW_UpdateWindowRegion_Name,
    CTLW_UpdateInputTransform_Name,
    CTLW_UpdateWindowScale_Name,
};
void* hookedFns[] = {
    GetMemberFnPtr(&CTopLevelWindow::UpdateWindowRegion),
    GetMemberFnPtr(&CTopLevelWindow::UpdateInputTransform),
    GetMemberFnPtr(&CTopLevelWindow::UpdateWindowScale),
};

const uint32_t symAmount = sizeof(symNames) / sizeof(symNames[0]);

// symbol address list
DWORD uDWM_addresses[symAmount] = { 0 };

int LoadSymbols(HMODULE* phModule, HMODULE* phudwm, DWORD addresses[], LPCWSTR symNames[], FILE* stream)
{
    int rv;
    CHAR szSymPath[_MAX_PATH];
    ZeroMemory(szSymPath, sizeof(szSymPath));
    GetModuleFileNameA(*phModule, szSymPath, _MAX_PATH);
    PathRemoveFileSpecA(szSymPath);
    strcat_s(szSymPath, _MAX_PATH, SYMBOLS_PATH);
    CHAR windir[MAX_PATH];
    GetEnvironmentVariableA("windir", windir, sizeof(windir));

    // UDWM.DLL
    CHAR uDWMPath[MAX_PATH];
    ZeroMemory(uDWMPath, sizeof(uDWMPath));
    strcpy_s(uDWMPath, sizeof(uDWMPath), windir);
    strcat_s(uDWMPath, sizeof(uDWMPath), "\\system32\\uDWM.dll");

    rv = VnDownloadSymbols(NULL, uDWMPath, szSymPath, _MAX_PATH, stream);
    if (rv != 0) {
        return ERR_SYMBOLFAILURE;
    }

    // WIDE STRING ADDED FOR MIOSSYMBOLS. OLD SYMPATH TO BE REMOVED ONCE LIBVALINET IS PHASED OUT OF THE SYMBOLS
    LPCWSTR szSymPathW = ATL::CA2W(szSymPath);

    MiosSymbol* uDWMSyms = new MiosSymbol(szSymPathW);
    if (GetLastError() != MIOSSUCCESS) {
        return ERR_SYMBOLFAILURE;
    }
    uDWMSyms->GetSymbolAddresses(addresses, symNames, symAmount);
    delete uDWMSyms;

    // https://www.youtube.com/watch?v=4aOcjAkkJNc
    ZeroMemory(szSymPath, sizeof(szSymPath));
    GetModuleFileNameA(*phModule, szSymPath, _MAX_PATH);
    PathRemoveFileSpecA(szSymPath);
    strcat_s(szSymPath, _MAX_PATH, SYMBOLS_PATH);

    *phudwm = GetModuleHandleW(L"udwm.dll");
    if (!*phudwm) {
        return ERR_MODULE;
    }

    return AWM_SUCCESS;
}

// ===========================================================================
//  HELPER FUNCTIONS
// ===========================================================================

template <typename Fn>
void* GetMemberFnPtr(Fn pmf)
{
    union { Fn pmf; void* ptr; } u;
    u.pmf = pmf;
    return u.ptr;
}

// ===========================================================================
//  HOOK FUNCTIONS
// ===========================================================================

// For this project, since I don't plan on calling any of the original functions, keeping their addresses is pointless.
int HookFunctions() {
    int rv = 0;

    for (int i = 0; i < symAmount; i++) {
        void* addr = (void*)((uintptr_t)hudwm + (uintptr_t)uDWM_addresses[i]);
        rv = funchook_prepare(funchook, (void**)&addr, hookedFns[i]);
        if (rv) {
            return ERR_FH_INIT;
        }
    }

    rv = funchook_install(funchook, 0);
    if (rv) {
        return ERR_FH_HOOK;
    }

    return AWM_SUCCESS;
}