#pragma once

#define AWM_DEBUG true
#define TARGET_BUILD 7601

#include <windows.h>
#include <pathcch.h>
#include <stdio.h>
#include <stdint.h>

#include "globals.h"
#include "funchook.h"
#pragma comment(lib, "user32_undoc.lib")

extern HMODULE hModule;
extern FILE* stream;
extern funchook_t* funchook;

extern HMODULE hudwm;

extern "C" BOOL WINAPI SetWindowRgnEx(HWND hWnd, HRGN hRgn, BOOL bRedraw);
extern "C" BOOL WINAPI UpdateWindowTransform(HWND hWnd, D2DMatrix* pMatrix, BOOL bSomething);

namespace AWM {
    namespace Init {
        HANDLE GetHeap();
    }
    namespace HeapImpl {
        void* Alloc(size_t cb);
        void Free(void* p);
    }
}