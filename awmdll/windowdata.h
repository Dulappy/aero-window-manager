#pragma once

#include "Windows.h"

class CWindowData {
public:
    BYTE    _pad0[0x28];
    HWND    m_pHwnd;               // +0x028
    BYTE    _pad1[0x5C];
    float   m_fScaleX;             // +0x08C
    float   m_fScaleY;             // +0x090
    float   m_fScaleX_copy;        // +0x094
    float   m_fScaleY_copy;        // +0x098
    RECT    m_rcWindowBounds;      // +0x09C
};

// Compile-time offset assertions

static_assert(offsetof(CWindowData, m_pHwnd) == 0x028, "ERROR: m_pHwnd not in the correct position!");
static_assert(offsetof(CWindowData, m_fScaleX) == 0x08C, "ERROR: m_fScaleX not in the correct position!"); // This catches the other three below it too.
static_assert(offsetof(CWindowData, m_rcWindowBounds) == 0x09C, "ERROR: m_rcWindowBounds not in the correct position!");