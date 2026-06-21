#pragma once

#include "Windows.h"

class CWindowData {
public:
    BYTE    _pad0[0x28];
    HWND    m_pHwnd;               // +0x028
    BYTE    _pad1[0x5C];
    float   m_fScaleX;             // +0x08C
    float   m_fScaleY;             // +0x090
    BYTE    _pad2[0x08];
    RECT    m_rcWindowBounds;      // +0x09C
};

// Compile-time offset assertions

static_assert(offsetof(CWindowData, m_pHwnd) == 0x028, "ERROR: m_pHwnd not in the correct position!");
static_assert(offsetof(CWindowData, m_fScaleX) == 0x08C, "ERROR: m_fScaleX not in the correct position!");
static_assert(offsetof(CWindowData, m_fScaleY) == 0x090, "ERROR: m_fScaleY not in the correct position!");
static_assert(offsetof(CWindowData, m_rcWindowBounds) == 0x09C, "ERROR: m_rcWindowBounds not in the correct position!");