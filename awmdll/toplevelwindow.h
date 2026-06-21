#pragma once

#include <Windows.h>
#include <Uxtheme.h>
#include "windowdata.h"

#define WINDOWSTATE_MAXIMIZED 0x04

class CTopLevelWindow {
public:
    BYTE         _pad0[0x0E0];
    BYTE         m_bWindowStateFlags;   // +0x0E0
    BYTE         _pad1[0x1AB];
    MARGINS      m_marBorderOutset;     // +0x28C
    MARGINS      m_marBorderOutsetMax;  // +0x29C
    BYTE         _pad2[0x1C];
    CWindowData* m_pWindowData;         // +0x2C8

    void UpdateWindowRegion();
};

// Compile-time offset assertions

static_assert(offsetof(CTopLevelWindow, m_bWindowStateFlags) == 0x0E0, "ERROR: m_bWindowStateFlags not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutset) == 0x28C, "ERROR: m_marBorderOutset not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutsetMax) == 0x29C, "ERROR: m_marBorderOutsetMax not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_pWindowData) == 0x2C8, "ERROR: m_bWindowStateFlags not in the correct position!");