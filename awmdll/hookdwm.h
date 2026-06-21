#pragma once

#include <Windows.h>
#include <stdio.h>

#define SYMBOLS_PATH "\\symbols\\"
#define SYMBOLS_PATH_WIDE L"\\symbols\\"
#define CTLW_UpdateWindowRegion_Name L"private: void __cdecl CTopLevelWindow::UpdateWindowRegion(void)"

// ---------------------------------------------------------------------------

extern LPCWSTR symNames[];
extern DWORD uDWM_addresses[];

int LoadSymbols(HMODULE* phModule, HMODULE* phudwm, DWORD addresses[], LPCWSTR symNames[], FILE* stream);
int HookFunctions();