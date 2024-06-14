#include <stdio.h>
#include <string>
#include <Windows.h>
#include <winbase.h>
#include <shlobj_core.h>
#include <stdint.h>
#include <combaseapi.h>
#include <ole2.h>
#include <diasdk/include/dia2.h>
#pragma comment(lib, __FILE__"\\..\\diasdk\\lib\\amd64\\diaguids.lib")
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define MIOSSUCCESS 0
#define MIOSERR_FAILEDTOCREATEINSTANCE -1

class MiosSymbol {
	public:
		IDiaDataSource* MiosDiaSource = NULL;
		IDiaSession* MiosDiaSession = NULL;
		IDiaSymbol* MiosGlobalScope = NULL;
		IDiaEnumSymbols* MiosEnumSymbols = NULL;

		MiosSymbol(LPCWSTR pdbPath);
		~MiosSymbol();
		int GetSymbolAddresses(DWORD* addresses, LPCWSTR* symNames, int symAmount);
};

MiosSymbol::MiosSymbol(LPCWSTR pdbPath) {
	int hr = CoCreateInstance(CLSID_DiaSource,
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(IDiaDataSource),
		(void**)&MiosDiaSource);
	if (hr == S_OK) {
		MiosDiaSource->loadDataFromPdb(pdbPath);
		MiosDiaSource->openSession(&MiosDiaSession);
		MiosDiaSession->get_globalScope(&MiosGlobalScope);
		MiosGlobalScope->findChildren(SymTagNull, nullptr, nsNone, &MiosEnumSymbols);
	}
}

MiosSymbol::~MiosSymbol() {
	if (MiosEnumSymbols)
		MiosEnumSymbols->Release();
	if (MiosGlobalScope)
		MiosGlobalScope->Release();
	if (MiosDiaSession)
		MiosDiaSession->Release();
	if (MiosDiaSource)
		MiosDiaSource->Release();
}

int MiosSymbol::GetSymbolAddresses(DWORD* addresses, LPCWSTR* symNames, int symAmount) {
	IDiaSymbol* MiosCurrentSymbol = NULL;
	ULONG count = 0;
	BSTR MiosCurrentSymbolName;
	while (MiosEnumSymbols->Next(1, &MiosCurrentSymbol, &count) == S_OK) {
		for (int i = 0; i < symAmount; i++) {
			MiosCurrentSymbol->get_undecoratedName(&MiosCurrentSymbolName);
			std::wstring MiosCurrentSymbolString(MiosCurrentSymbolName, SysStringLen(MiosCurrentSymbolName));
			if (_wcsicmp(symNames[i], MiosCurrentSymbolString.c_str()) == 0) {
				MiosCurrentSymbol->get_relativeVirtualAddress(&(addresses[i]));
				break;
			}
		}
	}

	return 0;
};

