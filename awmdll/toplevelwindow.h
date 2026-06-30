#pragma once

#include <Windows.h>
#include <Uxtheme.h>

#include "windowdata.h"
#include "visual.h"
#include "button.h"

class CTopLevelWindow : public CVisual {
public:
    BYTE         _pad0[0x18];          
    CVisual*     m_pClientBlurVisual;   // +0x108
    BYTE         _pad1[0xE0];
    CVisual*     m_pContentVisual;      // +0x1f0
    CButton*     m_pButtonHelp;         // +0x1f8
    CButton*     m_pButtonMinimize;     // +0x200
    CButton*     m_pButtonMaximize;     // +0x208
    CButton*     m_pButtonClose;        // +0x210
    BYTE         _pad2[0x18];
    CVisual*     m_pFrameVisual;        // +0x230 (Seems to be the grandparent of m_pContentVisual, although there don't seem to be many differences in what they control)
    BYTE         _pad3[0x54];
    MARGINS      m_marBorderOutset;     // +0x28C
    MARGINS      m_marBorderOutsetMax;  // +0x29C
    BYTE         _pad4[0x1C];
    CWindowData* m_pWindowData;         // +0x2C8

    // -------- STATIC VARIABLES --------
    static MARGINS s_marMinInflationThickness;
    static int s_iDpiIndex;

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

    enum ButtonType {
        ButtonType_Help = 0,
        ButtonType_Minimize = 1,
        ButtonType_Maximize = 2,
        ButtonType_Close = 3,
    };

    void UpdateWindowRegion();
    HRESULT UpdateInputTransform();
    void UpdateWindowScale();
    HRESULT UpdateNCAreaButton(ButtonType eType, INT cyButton, INT cyTop, INT* pcxButtonRow);
    HRESULT UpdatePinnedParts();

    static void CalculateOutsideMargins(CWindowData* pData, UINT dwFlags, _MARGINS* pOutMargins);
    static HRESULT ReadSystemMetrics();
    static HRESULT EnsureWindowFrames();
};

// Compile-time offset assertions

static_assert(offsetof(CTopLevelWindow, m_pClientBlurVisual) == 0x108, "ERROR: m_pClientBlurVisual not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_pButtonHelp) == 0x1f8, "ERROR: m_pButtonHelp not in the correct position!"); // This catches the other three below it too.
static_assert(offsetof(CTopLevelWindow, m_pFrameVisual) == 0x230, "ERROR: m_pFrameVisual not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutset) == 0x28C, "ERROR: m_marBorderOutset not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_marBorderOutsetMax) == 0x29C, "ERROR: m_marBorderOutsetMax not in the correct position!");
static_assert(offsetof(CTopLevelWindow, m_pWindowData) == 0x2C8, "ERROR: m_pWindowData not in the correct position!");