#pragma once
#include <symbols.h>
#include <valinet/ini/ini.h>
#include <valinet/pdb/pdb.h>
#include <miossymbols.h>
#include <atlbase.h>
#include <atlconv.h>
#include <awmerrors.h>

// ===========================================================================
//  LOAD SYMBOLS
// ===========================================================================

int LoadSymbols(HMODULE* phModule, HMODULE* phudwm, HMODULE* phdwmcore, HMODULE* phuser32, DWORD addresses[], LPCWSTR symNames[], DWORD addresses_dwmcore[], LPCWSTR symNames_dwmcore[], FILE* stream)
{
    int rv;
    CHAR szSymPath[_MAX_PATH];
    ZeroMemory(
        szSymPath,
        (_MAX_PATH) * sizeof(char)
    );
    GetModuleFileNameA(
        *phModule,
        szSymPath,
        _MAX_PATH
    );
    PathRemoveFileSpecA(szSymPath);
    strcat_s(
        szSymPath,
        _MAX_PATH,
        SYMBOLS_PATH
    );

    rv = VnDownloadSymbols(NULL, (char*)"C:\\Windows\\system32\\uDWM.dll", szSymPath, _MAX_PATH, stream);
    if (stream != NULL) {
        fprintf(
            stream,
            "ERROR %i\n", rv
        );
    }

    // WIDE STRING ADDED FOR MIOSSYMBOLS. OLD SYMPATH TO BE REMOVED ONCE LIBVALINET IS PHASED OUT OF THE SYMBOLS
    LPCWSTR szSymPathW = ATL::CA2W(szSymPath);

    MiosSymbol* uDWMSyms = new MiosSymbol(szSymPathW);
    uDWMSyms->GetSymbolAddresses(addresses, symNames, SYMBOLS_AMOUNT);
    delete uDWMSyms;

    /*VnGetSymbols(szSymPath, addresses, symNames, SYMBOLS_AMOUNT, stream);
    fprintf(
        stream,
        "%i\n", addresses[0]
    );*/

    // reset the symbol path
    /*ZeroMemory(
        szSymPath,
        (_MAX_PATH) * sizeof(char)
    );
    GetModuleFileNameA(
        hModule,
        szSymPath,
        _MAX_PATH
    );
    PathRemoveFileSpecA(szSymPath);
    strcat_s(
        szSymPath,
        _MAX_PATH,
        SYMBOLS_PATH
    );

    rv = VnDownloadSymbols(NULL, (char*)"C:\\Windows\\system32\\user32.dll", szSymPath, _MAX_PATH, stream);
    fprintf(
        stream,
        "ERROR %i\n", rv
    );

    VnGetSymbols(szSymPath, addresses_user32, symNames_user32, SYMBOLS_AMOUNT_USER32, stream);
    fprintf(
        stream,
        "%i\n", addresses[0]
    );*/

    // reset the symbol path
    ZeroMemory(
        szSymPath,
        (_MAX_PATH) * sizeof(char)
    );
    GetModuleFileNameA(
        *phModule,
        szSymPath,
        _MAX_PATH
    );
    PathRemoveFileSpecA(szSymPath);
    strcat_s(
        szSymPath,
        _MAX_PATH,
        SYMBOLS_PATH
    );

    rv = VnDownloadSymbols(NULL, (char*)"C:\\Windows\\system32\\dwmcore.dll", szSymPath, _MAX_PATH, stream);
    if (stream != NULL) {
        fprintf(
            stream,
            "ERROR %i\n", rv
        );
    }

    szSymPathW = ATL::CA2W(szSymPath);

    MiosSymbol* dwmcoreSyms = new MiosSymbol(szSymPathW);
    dwmcoreSyms->GetSymbolAddresses(addresses_dwmcore, symNames_dwmcore, SYMBOLS_AMOUNT_DWMCORE);
    delete dwmcoreSyms;
    //if (rv) {
    //    return rv;
    //}

    //VnGetSymbols(szSymPath, addresses_dwmcore, symNames_dwmcore, SYMBOLS_AMOUNT_DWMCORE, stream);

    *phudwm = GetModuleHandleW(L"udwm.dll");
    *phuser32 = GetModuleHandleW(L"user32.dll");
    *phdwmcore = GetModuleHandleW(L"dwmcore.dll");
    if (!*phudwm || !*phuser32 || !*phdwmcore) {
        return ERR_MODULE;
    }
	return 0;
}
