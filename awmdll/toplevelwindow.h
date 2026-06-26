#pragma once

#include <Windows.h>
#include <Uxtheme.h>
#include "windowdata.h"

/*#define WINDOWSTATE_HASDEFAULTSCALE       0x01u
#define WINDOWSTATE_RGNRECTUNCHANGED      0x02u
#define WINDOWSTATE_MAXIMIZED             0x04u
#define WINDOWSTATE_HASNOVISUALCHILDREN   0x08u
#define WINDOWSTATE_ISBEINGCLONED         0x10u
#define WINDOWSTATE_THICKFRAME            0x20u
#define WINDOWSTATE_ICONICANIMATIONACTIVE 0x40u*/

class CTopLevelWindow {
public:
    BYTE         _pad0[0x0E0];
    BYTE         m_bWindowStateFlags;   // +0x0E0
    BYTE         _pad1[0x1AB];
    MARGINS      m_marBorderOutset;     // +0x28C
    MARGINS      m_marBorderOutsetMax;  // +0x29C
    BYTE         _pad2[0x1C];
    CWindowData* m_pWindowData;         // +0x2C8

    enum WindowStateFlags : uint8_t {
        kEmpty                  = 0x00,
        kHasDefaultScale        = 0x01,
        kRgnRectUnchanged       = 0x02,
        kIsMaximized            = 0x04,
        kNoVisualChildren       = 0x08,
        kCloningInProgress      = 0x10,
        kHasThickframe          = 0x20,
        kIconicAnimationActive  = 0x40, 
    };

    void UpdateWindowRegion();
    HRESULT UpdateInputTransform();
};

// Compile-time offset assertions

static_assert(offsetof(CTopLevelWindow, m_bWindowStateFlags) == 0x0E0, "ERROR: m_bWindowStateFlags not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutset) == 0x28C, "ERROR: m_marBorderOutset not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutsetMax) == 0x29C, "ERROR: m_marBorderOutsetMax not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_pWindowData) == 0x2C8, "ERROR: m_pWindowData not in the correct position!");