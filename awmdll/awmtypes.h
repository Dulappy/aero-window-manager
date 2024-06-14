#pragma once

typedef D2D1_RECT_F MilRectF;

typedef struct _TEXTEX {
    D2D1_COLOR_F color;
    D2D1_COLOR_F shadowcolor;
    float glowopacity;
    DWORD tbWidth;
    MARGINS textInset;
    bool render;
    IDWriteTextFormat* textFormat = NULL;
    IDWriteTextLayout* textLayout = NULL;
    RECT fillboxSize;
    bool isActive; // this is used mainly to check for re-rendering when window state changes
} TEXTEX;

typedef enum _ACCENT {							// Values passed to SetWindowCompositionAttribute determining the appearance of a window

    ACCENT_DISABLED = 0,                        // Use no accent.
    ACCENT_ENABLE_GRADIENT = 1,					// Use a solid color specified by nColor. This mode ignores the alpha value and is fully opaque.
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,		// Use a tinted transparent overlay. nColor is the tint color.
    ACCENT_ENABLE_BLURBEHIND = 3,				// Use a tinted blurry overlay. nColor is the tint color.
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,		// Use an aspect similar to Fluent design. nColor is tint color. This mode bugs if the alpha value is 0.

    //ACCENT_NORMAL = 150							// (Fake value) Emulate regular taskbar appearance
} ACCENT;

typedef struct _ACCENT_POLICY				// Determines how a window's transparent region will be painted
{
    ACCENT   nAccentState;			    // Appearance
    int  nFlags;				        // Nobody knows how this value works
    unsigned int nColor;				// A color in the hex format AABBGGRR
    int  nAnimationId;			        // Nobody knows how this value works
} ACCENT_POLICY;

#define ACCENT_USECOLOR     0x02
#define ACCENT_USEGEOMETRY  0x10

typedef void CBitmapSource;
typedef void CRenderDataInstruction;
typedef void CBaseTransformProxy;
typedef void CCompositor;
typedef void CVisual;
typedef void CBaseObject;
typedef void CTextCache;
typedef void CChannel;
typedef void IDwmChannel;
typedef void CTopLevelWindow;
typedef void CDesktopManager;
typedef void CBitmapSourceArray;
typedef void CButton;

typedef CVisual CRenderDataVisual;
typedef CVisual CWindowData;
typedef CVisual CText;

typedef CRenderDataInstruction CDrawImageInstruction;
typedef CRenderDataInstruction CPushTransformInstruction;
typedef CRenderDataInstruction CPopInstruction;

typedef CBaseTransformProxy CMatrixTransformProxy;

typedef enum _AWM_TEXT_ALIGNMENT {
    AWM_TEXT_LEFT = 0,
    AWM_TEXT_CENTER_W8 = 1,
    AWM_TEXT_CENTER_ICONBUTTONS = 2,
    AWM_TEXT_CENTER_DULAPPY = 3,
    AWM_TEXT_RIGHT = 4
}AWM_TEXT_ALIGNMENT;

typedef enum _AWM_BUTTON_ALIGN {
    AWM_BTN_ALIGN_TOP = 0,
    AWM_BTN_ALIGN_CENTER = 1,
    AWM_BTN_ALIGN_TBCENTER = 2,
    AWM_BTN_ALIGN_BOTTOM = 3,
    AWM_BTN_ALIGN_FULL = 4
}AWM_BUTTON_ALIGN;

typedef enum _AWM_TEXT_VERTICAL_ALIGNMENT {
    AWM_TEXT_VALIGN_CENTER = 0,
    AWM_TEXT_VALIGN_TBCENTER = 1
}AWM_TEXT_VERTICAL_ALIGNMENT;

struct _awmsettings {
    // BORDER CORNERS
    int cornerRadiusX = 6;
    int cornerRadiusY = 6;

    // NC AREA BUTTONS
    //bool btnUseFullTBHeight = false;
    //bool btnUseFullTBHeightPalette = false;
    AWM_BUTTON_ALIGN tbBtnAlign = AWM_BTN_ALIGN_TBCENTER;
    AWM_BUTTON_ALIGN tbBtnAlignPalette = AWM_BTN_ALIGN_CENTER;
    float targetTBHeight = 21;
    float targetTBHeightPalette = 17;

    float xBtnHeight = 20;
    float xBtnHeightLone = 20;
    float xBtnHeightPalette = 17;
    float midBtnHeight = 20;
    float edgeBtnHeight = 20;

    float xBtnWidth = 49;
    float xBtnWidthLone = 49;
    float xBtnWidthPalette = 17;
    float midBtnWidth = 27;
    float edgeBtnWidth = 29;

    int xBtnInsetTopNormal = 1;
    int xBtnInsetTopLoneNormal = 1;
    int midBtnInsetTopNormal = 1;
    int edgeBtnInsetTopNormal = 1;

    int xBtnInsetDirectionalPalette = 0;

    int xBtnInsetTopMaximized = -1;
    int xBtnInsetTopLoneMaximized = -1;
    int midBtnInsetTopMaximized = -1;
    int edgeBtnInsetTopMaximized = -1;
    // ADD EXTRA OPTIONS HERE FOR HOW MANY TIMES THE BORDER WILL BE TAKEN INTO ACCOUNT (LIKE FOR THE ICON)

    int xBtnOffsetNormal = -2;
    int xBtnOffsetMaximized = 2;
    int xBtnOffsetPalette = 1;

    int xBtnInsetAfter = 0;
    int midBtnInsetAfter = 0;
    int edgeBtnInsetAfter = 0;  // doesn't really do much apart from making the space for the text smaller

    // NC AREA ICON
    int insetLeftAddNormal = 0;       // a static number added to the inset
    int insetLeftMulNormal = 1;       // how many times the border size will be taken into account
    int insetLeftAddMaximized = 0;
    int insetLeftMulMaximized = 1;

    // NC AREA TEXT
    int textInset = 5;       // a static number added to the inset

    AWM_TEXT_VERTICAL_ALIGNMENT iconTextVerticalAlign = AWM_TEXT_VALIGN_TBCENTER;

    // INACTIVE TITLEBAR FADING
    float tbBtnInactiveOpacity = 1.0f;

    // COLORIZATION SETTINGS
    bool useTransparency = true;
    float colorBalanceActive = 1.0;
    float colorBalanceInactive = 0.4;
    int colorActiveR = 3;
    int colorActiveG = 48;
    int colorActiveB = 124;
    int colorInactiveR = 3;
    int colorInactiveG = 48;
    int colorInactiveB = 124;
    // maybe settings to modify the border should be added
    // as well as more settings for tweaking the custom shader that will be added eventually

    float colorTextActiveR = 1.0f;
    float colorTextActiveG = 1.0f;
    float colorTextActiveB = 1.0f;
    float colorTextActiveA = 1.0f;
    float colorTextInactiveR = 1.0f;
    float colorTextInactiveG = 1.0f;
    float colorTextInactiveB = 1.0f;
    float colorTextInactiveA = 1.0f;

    float colorTextShadowActiveR = 0.0f;
    float colorTextShadowActiveG = 0.0f;
    float colorTextShadowActiveB = 0.0f;
    float colorTextShadowActiveA = 1.0f;
    float colorTextShadowInactiveR = 0.0f;
    float colorTextShadowInactiveG = 0.0f;
    float colorTextShadowInactiveB = 0.0f;
    float colorTextShadowInactiveA = 1.0f;

    float textShadowOffsetX = 1.0f;
    float textShadowOffsetY = 1.0f;

    float textGlowOpacityActive = 1.0f;
    float textGlowOpacityInactive = 0.4f;

    AWM_TEXT_ALIGNMENT textAlignment = AWM_TEXT_LEFT;

    ACCENT accent = ACCENT_DISABLED;

} awmsettings;
