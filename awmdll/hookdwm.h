#pragma once

#include <Windows.h>
#include <stdio.h>

#define SYMBOLS_PATH "\\symbols\\"
#define SYMBOLS_PATH_WIDE L"\\symbols\\"
#define CTLW_UpdateWindowRegion_Name L"private: void __cdecl CTopLevelWindow::UpdateWindowRegion(void)"
#define CTLW_UpdateInputTransform_Name L"private: long __cdecl CTopLevelWindow::UpdateInputTransform(void)"
#define CTLW_UpdateWindowScale_Name L"private: void __cdecl CTopLevelWindow::UpdateWindowScale(void)"

// ---------------------------------------------------------------------------

extern LPCWSTR symNames[];
extern DWORD uDWM_addresses[];

int LoadSymbols(HMODULE* phModule, HMODULE* phudwm, DWORD addresses[], LPCWSTR symNames[], FILE* stream);
int HookFunctions();