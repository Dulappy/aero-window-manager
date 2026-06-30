#pragma once

#include <Windows.h>
#include <Uxtheme.h>

class CWindowData {
public:
    BYTE    _pad0[0x28];
    HWND    m_pHwnd;               // +0x028
    BYTE    _pad1[0x10];
    MARGINS m_marWindowFrame;      // +0x040
    BYTE    _pad2[0x14];
    DWORD   m_dwStyle;             // +0x064
    DWORD   m_dwExStyle;           // +0x068
    BYTE    _pad3[0x20];
    float   m_fScaleX;             // +0x08C
    float   m_fScaleY;             // +0x090
    float   m_fScaleX_copy;        // +0x094
    float   m_fScaleY_copy;        // +0x098
    RECT    m_rcWindowBounds;      // +0x09C
};

// Compile-time offset assertions

static_assert(offsetof(CWindowData, m_pHwnd) == 0x028, "ERROR: m_pHwnd not in the correct position!");
static_assert(offsetof(CWindowData, m_marWindowFrame) == 0x040, "ERROR: m_marWindowFrame not in the correct position!");
static_assert(offsetof(CWindowData, m_dwStyle) == 0x064, "ERROR: m_dwStyle not in the correct position!");
static_assert(offsetof(CWindowData, m_dwExStyle) == 0x068, "ERROR: m_dwExStyle not in the correct position!");
static_assert(offsetof(CWindowData, m_fScaleX) == 0x08C, "ERROR: m_fScaleX not in the correct position!"); // This catches the other three below it too.
static_assert(offsetof(CWindowData, m_rcWindowBounds) == 0x09C, "ERROR: m_rcWindowBounds not in the correct position!");