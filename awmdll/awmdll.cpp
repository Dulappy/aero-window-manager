#define AWM_DEBUG TRUE

#include <d2d1_1.h>
#include <valinet/hooking/iatpatch.h>
#include <awmclrconv.h>
#include <stdio.h>
#include <cstdint>
#include <stdbool.h>
#include <math.h>

#include <Windows.h>
#include <wingdi.h>
#include <shlwapi.h>
#include <sddl.h>
#include <uxtheme.h>
#include <D3D9Types.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <winbase.h>
#include <wtsapi32.h>
#include <winuser.h>
#include <wincodec.h>

#include "funchook.h"
#pragma comment(lib, "Psapi.lib")

#include "symbols.h"
#include <awmtypes.h>
#include <awmerrors.h>
#include "definitions.h"
#include "miosd2d1.h"
#include "miosuxthemehelper.h"
// ---------------------------------------------------------------

// symbol address list
DWORD addresses[SYMBOLS_AMOUNT] = { 0 };
DWORD addresses_user32[SYMBOLS_AMOUNT_USER32] = { 0 };
DWORD addresses_dwmcore[SYMBOLS_AMOUNT_DWMCORE] = { 0 };

// symbol name list
/*char* symNames[SYMBOLS_AMOUNT] = {
    (char*)CTLW_TreatAsActiveWindow_Name,
    (char*)CTLW_UpdateNCAreaPositionsAndSizes_Name,
    (char*)CVis_SetDirtyFlags_Name,
    (char*)CTLW_UpdatePinnedParts_Name,
    (char*)CTLW_UpdateColorizationColor_Name,
    (char*)CTLW_UpdateNCAreaBackground_Name,
    (char*)CSCLMBP_Update_Name,
    (char*)CTLW_OnAccentPolicyUpdated_Name,
    (char*)CTLW_UpdateMarginsDependentOnStyle_Name,
    (char*)CTLW_ValidateVisual_Name,
    (char*)CTLW_s_marMinInflationThickness_Name
};*/

LPCWSTR symNames[SYMBOLS_AMOUNT] = {
    CTLW_TreatAsActiveWindow_Name,
    CTLW_UpdateNCAreaPositionsAndSizes_Name,
    CVis_SetDirtyFlags_Name,
    CTLW_UpdatePinnedParts_Name,
    CTLW_UpdateColorizationColor_Name,
    CTLW_UpdateNCAreaBackground_Name,
    CSCLMBP_Update_Name,
    CTLW_OnAccentPolicyUpdated_Name,
    CTLW_UpdateMarginsDependentOnStyle_Name,
    CTLW_ValidateVisual_Name,
    CTLW_s_marMinInflationThickness_Name,
    CDM_s_pDesktopManagerInstance_Name,
    CText_ValidateResources_Name,
    CBmpSrc_Create_WICBitmap_Name,
    CDII_Create_Name,
    CRDV_AddInstruction_Name,
    CText_ReleaseResources_Name,
    CBaseObject_Release_Name,
    CCompositor_CreateMatrixTransformProxy_Name,
    CPushTransformInstruction_Create_Name,
    CPopInstruction_Create_Name,
    CText_CText_Name,
    CText_SetColor_Name,
    CText_SetBackgroundColor_Name,
    CTLW_UpdateWindowVisuals_Name,
    CText_InitializeVisualTreeClone_Name,
    CText_Destroy_Name,
    CDesktopManager_LoadTheme_Name,
    CDesktopManager_UnloadTheme_Name,
    CButton_SetVisualStates_Name,
    CText_SetText_Name,
    CText_SetFont_Name
};

#if TARGETBUILD == 19041 || TARGETBUILD == 18362
LPCWSTR symNames_dwmcore[SYMBOLS_AMOUNT_DWMCORE] = {
    CRD_DrawSolidColorRectangle_Name
};
#elif TARGETBUILD == 17763
LPCWSTR symNames_dwmcore[SYMBOLS_AMOUNT_DWMCORE] = {
    CChSolidColorLegacyMilBrushUpdate_Name
};
#endif

char* symNames_user32[SYMBOLS_AMOUNT_USER32] = {
    (char*)SetWindowRgnEx_Name
};

// ---------------------------------------------------------------------------

HMODULE hModule = NULL;
FILE* stream = NULL;
funchook_t* funchook = NULL;

HMODULE hudwm = NULL;
HMODULE huser32 = NULL;
HMODULE hdwmcore = NULL;

ID2D1Factory* d2dfactory;
IDWriteFactory* dwritefactory;

// PLACEHOLDER FUNCTION, JUST SETTING ARBITRARY VALUES FOR NOW. (Settings Key will be HKCU\Software\AWM)
int LoadSettingsFromRegistry() {
    /*BOOL bResult = FALSE;
    DWORD cbData = 4;
    LSTATUS test = 0;
    fprintf(stream, "%i\n", hKey);
    //return 0;

    HKEY hKeyAWM = NULL;

    test = RegOpenKeyW(hKey, L"SOFTWARE\\AWM", &hKeyAWM);
    fprintf(stream, "%i\n", test);

    test = RegQueryValueExW(hKeyAWM, L"Window_CornerRadiusX", NULL, NULL, (BYTE*)&awmsettings.cornerRadiusX, &cbData);
    fprintf(stream, "%i\n", test);
    cbData = 4;
    RegQueryValueExW(hKeyAWM, L"Window_CornerRadiusY", NULL, NULL, (BYTE*)&awmsettings.cornerRadiusY, &cbData);*/

    DWORD cbData = 4;
    DWORD data = NULL;
    HKEY awmkey = NULL;
    int rv = NULL;
    RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\AWM", 0, KEY_QUERY_VALUE, &awmkey);
    rv = RegQueryValueEx(awmkey, L"Window_CornerRadiusX", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.cornerRadiusX = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_CornerRadiusY", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.cornerRadiusY = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_ButtonAlign", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.tbBtnAlign = (AWM_BUTTON_ALIGN)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_ButtonAlignPal", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.tbBtnAlignPalette = (AWM_BUTTON_ALIGN)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_TargetHeight", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.targetTBHeight = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_TargetHeightPal", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.targetTBHeightPalette = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnHeight", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnHeight = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnHeightLone", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnHeightLone = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnHeightPal", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnHeightPalette = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_MidBtnHeight", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.midBtnHeight = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_EdgeBtnHeight", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.edgeBtnHeight = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnWidth", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnWidth = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnWidthLone", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnWidthLone = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnWidthPal", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnWidthPalette = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_MidBtnWidth", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.midBtnWidth = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_EdgeBtnWidth", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.edgeBtnWidth = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnInsTop", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnInsetTopNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnInsTopLone", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnInsetTopLoneNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_MidBtnInsTop", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.midBtnInsetTopNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_EdgeBtnInsTop", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.edgeBtnInsetTopNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnInsTopMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnInsetTopMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnInsTopLoneMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnInsetTopLoneMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_MidBtnInsTopMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.midBtnInsetTopMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_EdgeBtnInsTopMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.edgeBtnInsetTopMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnOffset", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnOffsetNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnOffsetMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnOffsetMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnOffsetPal", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnOffsetPalette = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_XBtnAfter", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.xBtnInsetAfter = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_MidBtnAfter", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.midBtnInsetAfter = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_EdgeBtnAfter", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.edgeBtnInsetAfter = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_InsetLeftAdd", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.insetLeftAddNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_InsetLeftMul", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.insetLeftMulNormal = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_InsetLeftAddMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.insetLeftAddMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_InsetLeftMulMax", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.insetLeftMulMaximized = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_TextInset", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textInset = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_BtnInactiveOpacity", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.tbBtnInactiveOpacity = (float)data / 100;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"UseTransparency", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.useTransparency = (BYTE)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"AccentPolicy", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.accent = (ACCENT)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorRActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorActiveR = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorGActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorActiveG = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorBActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorActiveB = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorRInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorInactiveR = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorGInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorInactiveG = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorBInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorInactiveB = data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorBalanceActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorBalanceActive = (float)data / 100.0;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Window_ColorBalanceInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorBalanceInactive = (float)data / 100.0;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorRActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextActiveR = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorGActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextActiveG = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorBActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextActiveB = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorAActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextActiveA = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorRInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextInactiveR = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorGInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextInactiveG = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorBInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextInactiveB = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ColorAInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextInactiveA = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorRActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowActiveR = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorGActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowActiveG = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorBActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowActiveB = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorAActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowActiveA = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorRInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowInactiveR = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorGInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowInactiveG = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorBInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowInactiveB = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowColorAInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.colorTextShadowInactiveA = (float)data / 255.0f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowOffsetX", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textShadowOffsetX = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_ShadowOffsetY", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textShadowOffsetY = (float)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_Alignment", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textAlignment = (AWM_TEXT_ALIGNMENT)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"TB_IconTextVertAlign", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.iconTextVerticalAlign = (AWM_TEXT_VERTICAL_ALIGNMENT)data;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_GlowOpacityActive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textGlowOpacityActive = (float)data / 100.f;
    }
    cbData = 4;
    data = NULL;
    rv = RegQueryValueEx(awmkey, L"Text_GlowOpacityInactive", NULL, NULL, (LPBYTE)&data, &cbData);
    if (!rv) {
        awmsettings.textGlowOpacityInactive = (float)data / 100.f;
    }

    RegCloseKey(awmkey);

    return 0;
}

// ===========================================================================
//  HOOKED FUNCTION TYPE DEFINITIONS
// ===========================================================================

MARGINS* CTLW_marMinInflationThickness;
__int64* CDM_pDesktopManagerInstance = nullptr;

typedef bool (*CTLW_TreatAsActiveWindow_t)(void* pThis);
CTLW_TreatAsActiveWindow_t CTLW_TreatAsActiveWindow_orig;

typedef long (*CTLW_UpdateNCAreaPositionsAndSizes_t)(void* pThis);
CTLW_UpdateNCAreaPositionsAndSizes_t CTLW_UpdateNCAreaPositionsAndSizes_orig;

typedef void (*CVis_SetDirtyFlags_t)(void* pThis, ULONG flag);
CVis_SetDirtyFlags_t CVisual_SetDirtyFlags;

typedef __int64 (*CTLW_UpdatePinnedParts_t)(void* pThis);
CTLW_UpdatePinnedParts_t CTopLevelWindow_UpdatePinnedParts;

typedef __int64 (*CTLW_UpdateColorizationColor_t)(void* pThis);
CTLW_UpdateColorizationColor_t CTLW_UpdateColorizationColor_orig;

typedef __int64 (*CTLW_UpdateNCAreaBackground_t)(void* pThis);
CTLW_UpdateNCAreaBackground_t CTLW_UpdateNCAreaBackground_orig;

typedef __int64 (*CSCLMBP_Update_t)(void* pThis, double alpha, D3DCOLORVALUE* color);
CSCLMBP_Update_t CSolidColorMilBrushProxy_Update;

typedef void (*CTLW_OnAccentPolicyUpdated_t)(void* pThis);
CTLW_OnAccentPolicyUpdated_t CTopLevelWindow_OnAccentPolicyUpdated;

typedef bool (*CTLW_UpdateMarginsDependentOnStyle_t)(void* pThis);
CTLW_UpdateMarginsDependentOnStyle_t CTLW_UpdateMarginsDependentOnStyle_orig;

typedef __int64 (*CTLW_ValidateVisual_t)(void* pThis);
CTLW_ValidateVisual_t CTLW_ValidateVisual_orig;

typedef int (*SetWindowRgnEx_t)(HWND hWnd, HRGN hRgn, BOOL bRedraw);
SetWindowRgnEx_t SetWindowRgnEx;

typedef __int64 (*CRD_DrawSolidColorRectangle_t)(void* pThis, void* pDrawingContext, void* pDrawListEntryBuilder, char unk, MilRectF* rect, D3DCOLORVALUE* color);
CRD_DrawSolidColorRectangle_t CRD_DrawSolidColorRectangle_orig;

typedef int (*CText_ValidateResources_t)(CText* pThis);
CText_ValidateResources_t CText_ValidateResources_orig;
#if TARGETBUILD == 19041 || TARGETBUILD == 18362
typedef int (*CBitmapSource_Create_t)(IWICBitmap* bitmap, MARGINS* margins, CBitmapSource** ppBitmapSource);
#elif TARGETBUILD == 17763
typedef int (*CBitmapSource_Create_t)(IWICBitmap* bitmap, MARGINS* margins, IDwmChannel* channel, CBitmapSource** ppBitmapSource);
#endif
CBitmapSource_Create_t CBitmapSource_Create;

typedef int (*CDrawImageInstruction_Create_t)(CBitmapSource* pBitmapSource, RECT* rect, CDrawImageInstruction** ppDrawImageInstruction);
CDrawImageInstruction_Create_t CDrawImageInstruction_Create;

typedef int (*CRenderDataVisual_AddInstruction_t)(CRenderDataVisual* pThis, CRenderDataInstruction* pInstruction);
CRenderDataVisual_AddInstruction_t CRenderDataVisual_AddInstruction;

typedef int (*CText_ReleaseResources_t)(CText* pThis);
CText_ReleaseResources_t CText_ReleaseResources;

typedef int (*CBaseObject_Release_t)(CBaseObject* pThis);
CBaseObject_Release_t CBaseObject_Release;

typedef int (*CCompositor_CreateMatrixTransformProxy_t)(CCompositor* pThis, CMatrixTransformProxy** proxy);
CCompositor_CreateMatrixTransformProxy_t CCompositor_CreateMatrixTransformProxy;

typedef int (*CPushTransformInstruction_Create_t)(CBaseTransformProxy* proxy, CPushTransformInstruction** pThis);
CPushTransformInstruction_Create_t CPushTransformInstruction_Create;

typedef int (*CPopInstruction_Create_t)(CPopInstruction** pThis);
CPopInstruction_Create_t CPopInstruction_Create;

typedef int (*CCh_SolidColorLegacyMilBrushUpdate_t)(CChannel* pThis, int, double opacity, D3DCOLORVALUE*, UINT, UINT, UINT);
CCh_SolidColorLegacyMilBrushUpdate_t CChannel_SolidColorLegacyMilBrushUpdate;

typedef CText* (*CText_CText_t)(CText* pThis);
CText_CText_t CText_CText_orig;

typedef void (*CText_SetColor_t)(CText* pThis, COLORREF color);
CText_SetColor_t CText_SetColor_orig;

typedef void (*CText_SetBackgroundColor_t)(CText* pThis, COLORREF color);
CText_SetBackgroundColor_t CText_SetBackgroundColor_orig;

typedef int (*CTLW_UpdateWindowVisuals_t)(CTopLevelWindow* pThis);
CTLW_UpdateWindowVisuals_t CTLW_UpdateWindowVisuals_orig;

typedef int (*CText_InitializeVisualTreeClone_t)(CText* pThis, CText* pNew, UINT options);
CText_InitializeVisualTreeClone_t CText_InitializeVisualTreeClone_orig;

typedef CText* (*CText_Destroy_t)(CText* pThis, UINT someFlags);
CText_Destroy_t CText_Destroy_orig;

typedef long (*CDesktopManager_LoadTheme_t)(CDesktopManager* pThis);
CDesktopManager_LoadTheme_t CDesktopManager_LoadTheme_orig;

typedef void (*CDesktopManager_UnloadTheme_t)(CDesktopManager* pThis);
CDesktopManager_UnloadTheme_t CDesktopManager_UnloadTheme_orig;

typedef long (*CButton_SetVisualStates_t)(CButton* pThis, CBitmapSourceArray* array1, CBitmapSourceArray* array2, CBitmapSource* bmpsrc, float opacity);
CButton_SetVisualStates_t CButton_SetVisualStates_orig;

typedef long (*CText_SetText_t)(CText* pThis, wchar_t* str);
CText_SetText_t CText_SetText_orig;

typedef void (*CText_SetFont_t)(CText* pThis, LOGFONTW* font);
CText_SetFont_t CText_SetFont_orig;

// ===========================================================================
//  FUNCTIONS
// ===========================================================================

IWICBitmap* DWMAtlas{};
HRESULT AWM_GetDWMWindowAtlas() {
    HRESULT hr = 0;
    IWICBitmapSource* bmpsource;
    IWICImagingFactory* imagingfactory = *(IWICImagingFactory**)(*CDM_pDesktopManagerInstance + CDM_ImagingFactory);

    HTHEME hTheme = *(HTHEME*)(*CDM_pDesktopManagerInstance + CDM_DWMWindowThemeData);
    HINSTANCE hInstance = *(HINSTANCE*)(*CDM_pDesktopManagerInstance + CDM_ThemeLibrary);
    hr = GetMiosThemeAtlas(hTheme, hInstance, imagingfactory, &bmpsource);

    hr = imagingfactory->CreateBitmapFromSource(
        bmpsource,
        WICBitmapCacheOnDemand,
        &DWMAtlas
    );

    if (bmpsource)
        bmpsource->Release();

    return hr;
}

void AWM_DestroyDWMWindowAtlas() {
    if (DWMAtlas)
        DWMAtlas->Release();
}

void CVisual_SetInsetFromParentLeft(BYTE* pThis, int inset) {
    if (*(DWORD*)(pThis + CVis_InsetLeft) != inset) {
        *(DWORD*)(pThis + CVis_InsetLeft) = inset;
        CVisual_SetDirtyFlags(pThis, 2);
    }
}
void CVisual_SetInsetFromParentRight(BYTE* pThis, int inset) {
    if (*(DWORD*)(pThis + CVis_InsetRight) != inset) {
        *(DWORD*)(pThis + CVis_InsetRight) = inset;
        CVisual_SetDirtyFlags(pThis, 2);
    }
}
void CVisual_SetInsetFromParentTop(BYTE* pThis, int inset) {
    if (*(DWORD*)(pThis + CVis_InsetTop) != inset) {
        *(DWORD*)(pThis + CVis_InsetTop) = inset;
        CVisual_SetDirtyFlags(pThis, 2);
    }
}
void CVisual_SetInsetFromParentBottom(BYTE* pThis, int inset) {
    if (*(DWORD*)(pThis + CVis_InsetBottom) != inset) {
        *(DWORD*)(pThis + CVis_InsetBottom) = inset;
        CVisual_SetDirtyFlags(pThis, 2);
    }
}
void CVisual_SetInsetFromParent(BYTE* pThis, _MARGINS* inset) {
    MARGINS* insetFromParent = (MARGINS*)(pThis + CVis_InsetFromParent);
    if (insetFromParent->cxLeftWidth != inset->cxLeftWidth
     || insetFromParent->cxRightWidth != inset->cxRightWidth
     || insetFromParent->cyTopHeight != inset->cyTopHeight
     || insetFromParent->cyBottomHeight != inset->cyBottomHeight) 
    {
        *(MARGINS*)(pThis + CVis_InsetFromParent) = *inset;
        CVisual_SetDirtyFlags(pThis, 2);
    }
}

void CVisual_SetSize(BYTE* pThis, tagSIZE* size) {
    SIZE* currentSize = (SIZE*)(pThis + CVis_Size);
    if (currentSize->cx != size->cx
      || currentSize->cy != size->cy) 
    {
        *(SIZE*)(pThis + CVis_Size) = *size;
        if (*(pThis + CVis_SomeFlags) & 1) {
            CVisual_SetDirtyFlags(pThis, 16);
        }
        CVisual_SetDirtyFlags(pThis, 2);
    }
}

void CVisual_Hide(BYTE* pThis) {
    int* hidden = (int*)(pThis + CVis_Hidden);
    if (!*hidden) {
        *hidden = 1;
        CVisual_SetDirtyFlags(pThis, CVIS_FLAG_UPDATEVISIBILITY);
    }
}
void CVisual_Unhide(BYTE* pThis) {
    int* hidden = (int*)(pThis + CVis_Hidden);
    if (*hidden) {
        *hidden = 0;
        CVisual_SetDirtyFlags(pThis, CVIS_FLAG_UPDATEVISIBILITY);
    }
}

bool CTopLevelWindow_IsSheetOfGlass(BYTE* pThis) {
    BYTE* windowVisual = *(BYTE**)(pThis + CTLW_WindowVisual);
    return *(int*)(windowVisual + 80) == 0x7fffffff 
        && *(int*)(windowVisual + 84) == 0x7fffffff
        && *(int*)(windowVisual + 88) == 0x7fffffff
        && *(int*)(windowVisual + 92) == 0x7fffffff; // not sure what these offsets are meant to represent (88 even overlaps with CVis_Hidden)
}

void CText_CreateTextLayout(BYTE* pThis) {
    HRESULT hr = 0;
    LPCWSTR string = *(LPCWSTR*)(pThis + CTxt_String);
    TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);
    IDWriteTextLayout* textlayout = NULL;
    IDWriteTextFormat* textformat = textex->textFormat;
    LOGFONT font = *(LOGFONT*)(pThis + CTxt_Font);
    if (string) {
        int stringlength = wcslen(string);
        DWRITE_TEXT_RANGE range = { 0, stringlength }; 
        hr = dwritefactory->CreateTextLayout(string, stringlength, textformat, 0, 0, &textlayout);
        if (hr < 0)
            goto release;
        hr = textlayout->SetStrikethrough(font.lfStrikeOut, range);
        if (hr < 0)
            goto release;
        hr = textlayout->SetUnderline(font.lfUnderline, range);
        if (hr < 0)
            goto release;
    }
release:
    if (hr >= 0) {
        if (textex->textLayout)
            textex->textLayout->Release();
        textex->textLayout = textlayout;
    }
}

void CText_CreateTextFormat(BYTE* pThis, LOGFONTW* font) {
    HRESULT hr = 0;
    IDWriteTextFormat* textformat;
    IDWriteInlineObject* trimmingsign;
    DWRITE_TRIMMING trimming;
    TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);

    DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
    if (font->lfItalic)
        style = DWRITE_FONT_STYLE_ITALIC;
    float height = (float)-font->lfHeight;
    if (height < 0) {
        height = -height;
    }
    else if (height == 0) {
        height = -(float)((*(LOGFONT*)(*CDM_pDesktopManagerInstance + CDM_CaptionFont)).lfHeight);
    }
    hr = dwritefactory->CreateTextFormat(
        font->lfFaceName,
        NULL,
        (DWRITE_FONT_WEIGHT)font->lfWeight,
        style,
        DWRITE_FONT_STRETCH_NORMAL,
        height,
        L"en-us",
        &textformat
    );
    if (hr < 0)
        goto release;

    textformat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    hr = dwritefactory->CreateEllipsisTrimmingSign(textformat, &trimmingsign);
    if (hr < 0)
        goto release;
    trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, NULL, 0 };
    hr = textformat->SetTrimming(&trimming, trimmingsign);
    if (hr < 0)
        goto release;
    /*if (CVisualFlags & CVIS_FLAG_RTL) {
        hr = textformat->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
        if (hr < 0)
            goto release;
    }

    if (awmsettings.textAlignment == AWM_TEXT_CENTER_ICONBUTTONS) {
        textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else if (awmsettings.textAlignment == AWM_TEXT_RIGHT) {
        textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
    }*/
release:
    if (hr >= 0) {
        if (textex->textFormat)
            textex->textFormat->Release();
        textex->textFormat = textformat;
        CText_CreateTextLayout(pThis);
    }
}

long CText_SetText_Hook(BYTE* pThis, wchar_t* str) {
    long hr = CText_SetText_orig(pThis, str);
    TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);
    if (*(pThis + CTxt_ExFlag)) {
        textex->render = true;
        CText_CreateTextLayout(pThis);
    }
    return hr;
}

void CText_SetFont_Hook(BYTE* pThis, LOGFONTW* font) {
    TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);
    if (memcmp((pThis + CTxt_Font), font, sizeof(LOGFONTW))) {
        if (*(pThis + CTxt_ExFlag)) {
            textex->render = true;
            CText_CreateTextFormat(pThis, font);
        }
    }
    CText_SetFont_orig(pThis, font);
}

long CDesktopManager_LoadTheme_Hook(BYTE* pThis) {
    long rv = CDesktopManager_LoadTheme_orig(pThis);
    rv = AWM_GetDWMWindowAtlas();

    return rv;
}

void CDesktopManager_UnloadTheme_Hook(BYTE* pThis) {
    CDesktopManager_UnloadTheme_orig(pThis);
    AWM_DestroyDWMWindowAtlas();
}

long CButton_SetVisualStates_Hook(void* pThis, CBitmapSourceArray* array1, CBitmapSourceArray* array2, CBitmapSource* bmpsrc, float opacity) {
    if (opacity != 1.f)
        opacity = awmsettings.tbBtnInactiveOpacity;
    return CButton_SetVisualStates_orig(pThis, array1, array2, bmpsrc, opacity);
}

bool CTLW_UpdateMarginsDependentOnStyle_Hook(BYTE* pThis) {
    //MARGINS test = {-5, -5, -5, -5};
    //MARGINS test2 = { 100, 100, 100, 100 };
    /*BYTE* atlasedRectsVisual = *(BYTE**)(pThis + 280);
    *(MARGINS*)(atlasedRectsVisual + 272) = test;
    CVisual_SetDirtyFlags(atlasedRectsVisual, 0x2000);*/
    //*(RECT*)(windowVisual + 48) = { -5, -5, 500, 500 };
    //*(MARGINS*)(pThis + 628) = test; // outer border visual outset
    //*(MARGINS*)(pThis + 644) = test2; // outer border visual outset
    //CTLW_UpdateMarginsDependentOnStyle_orig(pThis);

    BYTE* windowData = *(BYTE**)(pThis + CTLW_WindowData);
    int DPIValue = *(int*)(windowData + CVis_DPI);

    bool isSomething = *(pThis + wState2) & 6; // COMPATIBILITY ISSUE: UPDATE NUMBER 6 WHEN POSSIBLE FOR 1809 COMPATIBILITY

    DWORD winStyle = *(DWORD*)(windowData + CWD_WinStyle);
    DWORD winStyleEx = *(DWORD*)(windowData + CWD_WinStyleEx);
    MARGINS borMargins = *(MARGINS*)(windowData + CWD_BorderMargins);

    if (isSomething
     && borMargins.cxLeftWidth >= CTLW_marMinInflationThickness->cxLeftWidth
     && borMargins.cxRightWidth >= CTLW_marMinInflationThickness->cxRightWidth
     && borMargins.cyTopHeight >= CTLW_marMinInflationThickness->cyTopHeight
     && borMargins.cyBottomHeight >= CTLW_marMinInflationThickness->cyBottomHeight
     && !(winStyleEx & 0x800000)
     && GetSystemMetricsForDpi(SM_CXPADDEDBORDER, DPIValue) > 0) 
    {
        RECT rect = { 0, 0, 0, 0 };
        AdjustWindowRectExForDpi(&rect, winStyle, 0, winStyleEx, DPIValue);

        MARGINS targetmargins;
        int left = -rect.left;
        if (borMargins.cxLeftWidth > left)
            left = borMargins.cxLeftWidth;
        targetmargins.cxLeftWidth = borMargins.cxLeftWidth - left;

        int right = rect.right;
        if (borMargins.cxRightWidth > right)
            right = borMargins.cxRightWidth;
        targetmargins.cxRightWidth = borMargins.cxRightWidth - right;

        int top = -rect.top;
        if (borMargins.cyTopHeight > top)
            top = borMargins.cyTopHeight;
        targetmargins.cyTopHeight = borMargins.cyTopHeight - top;

        int bottom = rect.bottom;
        if (borMargins.cyBottomHeight > bottom)
            bottom = borMargins.cyBottomHeight;
        targetmargins.cyBottomHeight = borMargins.cyBottomHeight - bottom;

        *(MARGINS*)(pThis + CTLW_BorderMarginsOuter) = targetmargins; // TEMPORARY
    }

    return true;
}

__int64 CTLW_ValidateVisual_Hook(BYTE* pThis) { // TO BE OVERHAULED ONCE/IF VALIDATEVISUAL HAS BEEN REWRITTEN
    __int64 rv = CTLW_ValidateVisual_orig(pThis);

#if TARGETBUILD >= 18362
    MARGINS borderOutset = *(MARGINS*)(pThis + CTLW_BorderMarginsOuter);
    if (*(pThis + wState1) & STATE_MAXIMIZED) 
        borderOutset = *(MARGINS*)(pThis + CTLW_BorderMarginsOuterMax);

    MARGINS clientMargins = *(MARGINS*)(pThis + CTLW_ClientMargins);

    bool hasClient = clientMargins.cxLeftWidth != 0     // could have some edge cases but it's the best i can do
                  || clientMargins.cxRightWidth != 0    // without rewriting ValidateVisual
                  || clientMargins.cyTopHeight != 0
                  || clientMargins.cyBottomHeight != 0;
    BYTE* windowData = *(BYTE**)(pThis + CTLW_WindowData);

    if (!rv && /**(HWND*)(windowData + CWD_HWND) && (*(pThis + wState1) & STYLE_THICKFRAME) == 0 &&*/ hasClient) {
        //bool isMaximized = *(pThis + wState1) & STATE_MAXIMIZED;
        RECT rect = *(RECT*)(windowData + CWD_WindowRect);
        float DPI = *(float*)(windowData + CWD_DPIFloat);

        if (true == true) { // placeholder if statement
            if (borderOutset.cxLeftWidth < 0)
                rect.left += (int)floor((float)((float)borderOutset.cxLeftWidth / DPI) + 0.5);
            if (borderOutset.cxRightWidth < 0)
                rect.right -= (int)floor((float)((float)borderOutset.cxRightWidth / DPI) + 0.5);
            if (borderOutset.cyTopHeight < 0)
                rect.top += (int)floor((float)((float)borderOutset.cyTopHeight / DPI) + 0.5);
            if (borderOutset.cyBottomHeight < 0)
                rect.bottom -= (int)floor((float)((float)borderOutset.cyBottomHeight / DPI) + 0.5);
        }

        bool isEqual = EqualRect(&rect, (RECT*)(windowData + CWD_WindowRect));
        if ((*(pThis + wState1) & 2) == 0 || !isEqual) { // COMPATIBILITY: REMEMBER TO UPDATE THE 2 HERE TO BE COMPATIBLE WITH EVERY VERSION
            HRGN rgn = CreateRectRgnIndirect(&rect);
            fprintf(stream, "i have set the region\n");
            if (!SetWindowRgnEx(*(HWND*)(windowData + CWD_HWND), rgn, 1)) {     // I NEED TO GET SYMBOLS FOR USER32.DLL TO USE SETWINDOWRGNEX (and hope it works)
                if (rgn) {
                    DeleteObject(rgn);
                }
            }
            *(pThis + wState1) &= ~2;   // COMPATIBILITY
            *(pThis + wState1) |= 2 * isEqual; // COMPATIBILITY
        }
    }
#endif
    return rv;
}

__int64 CTLW_UpdateColorizationColor_Hook(BYTE* pThis) {
    __int64 rv = CTLW_UpdateColorizationColor_orig(pThis);
    int balanceColor = 224;
    int balClrPremulActive = balanceColor * (1.0 - awmsettings.colorBalanceActive);
    int balClrPremulInactive = balanceColor * (1.0 - awmsettings.colorBalanceInactive);
    BYTE* titlebarGlassColor = *(BYTE**)(pThis + CTLW_GlassColorTB);
    BYTE* borderGlassColor = *(BYTE**)(pThis + CTLW_GlassColorBorder);
    D3DCOLORVALUE* tbGlass3DColor = (D3DCOLORVALUE*)(titlebarGlassColor + CGCR_Color);
    D3DCOLORVALUE* borGlass3DColor = (D3DCOLORVALUE*)(borderGlassColor + CGCR_Color);
    D3DCOLORVALUE colorization = { 0 };

    bool isActive = CTLW_TreatAsActiveWindow_orig(pThis);
    if (awmsettings.useTransparency) {
        if (isActive) {
            colorization.r = AWM_Convert_sRGB_to_scRGB[awmsettings.colorActiveR] / 255;
            colorization.g = AWM_Convert_sRGB_to_scRGB[awmsettings.colorActiveG] / 255;
            colorization.b = AWM_Convert_sRGB_to_scRGB[awmsettings.colorActiveB] / 255;
            colorization.a = awmsettings.colorBalanceActive;
        }
        else {
            colorization.r = AWM_Convert_sRGB_to_scRGB[awmsettings.colorInactiveR] / 255;
            colorization.g = AWM_Convert_sRGB_to_scRGB[awmsettings.colorInactiveG] / 255;
            colorization.b = AWM_Convert_sRGB_to_scRGB[awmsettings.colorInactiveB] / 255;
            colorization.a = awmsettings.colorBalanceInactive;
        }
    }
    else {
        if (isActive) {
            colorization.r = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorActiveR * awmsettings.colorBalanceActive + balClrPremulActive)] / 255;
            colorization.g = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorActiveG * awmsettings.colorBalanceActive + balClrPremulActive)] / 255;
            colorization.b = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorActiveB * awmsettings.colorBalanceActive + balClrPremulActive)] / 255;
            colorization.a = 1.0;
        }
        else {
            colorization.r = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorInactiveR * awmsettings.colorBalanceInactive + balClrPremulInactive)] / 255;
            colorization.g = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorInactiveG * awmsettings.colorBalanceInactive + balClrPremulInactive)] / 255;
            colorization.b = AWM_Convert_sRGB_to_scRGB[(int)floor(awmsettings.colorInactiveB * awmsettings.colorBalanceInactive + balClrPremulInactive)] / 255;
            colorization.a = 1.0;
        }
    }
    *tbGlass3DColor = colorization;
    *borGlass3DColor = colorization;
    /*fprintf(stream, "%f\n", colorization.r);
    fprintf(stream, "%f\n", colorization.g);
    fprintf(stream, "%f\n", colorization.b);
    fprintf(stream, "%f\n", colorization.a);*/
    return rv;
}

__int64 CTLW_UpdateNCAreaBackground_Hook(BYTE* pThis) {
    int rv = CTLW_UpdateNCAreaBackground_orig(pThis);
    D3DCOLORVALUE color = { 0 };
    BYTE* borderBrushProxy = *(BYTE**)(pThis + CTLW_BorderBrushProxy);
    BYTE* titlebarBrushProxy = *(BYTE**)(pThis + CTLW_TitlebarBrushProxy);
    BYTE* windowData  = *(BYTE**)(pThis + CTLW_WindowData);

    if (borderBrushProxy) {
        BYTE* colorizationResources = NULL;
        if (CTopLevelWindow_IsSheetOfGlass(pThis)) {
            colorizationResources = *(BYTE**)(pThis + CTLW_GlassColorTB);
        }
        else {
            colorizationResources = *(BYTE**)(pThis + CTLW_GlassColorBorder);
        }
        D3DCOLORVALUE colorResource = *(D3DCOLORVALUE*)(colorizationResources + CGCR_Color);
        D3DCOLORVALUE* borClr = (D3DCOLORVALUE*)(pThis + CTLW_BorderColor);
        color.r = colorResource.r;
        color.g = colorResource.g;
        color.b = colorResource.b;
        color.a = colorResource.a;
        //if (borClr->r != color.r || borClr->g != color.g || borClr->b != color.b || borClr->a != color.a) {
#if TARGETBUILD == 19041 || TARGETBUILD == 18362
        CSolidColorMilBrushProxy_Update(borderBrushProxy, 1.0, &color);
#elif TARGETBUILD == 17763
        CChannel_SolidColorLegacyMilBrushUpdate(
            *(CChannel**)(borderBrushProxy + 16),
            *(int*)(borderBrushProxy + 24),
            1.0,
            &color,
            0,
            0,
            0
        );
#endif
        *borClr = color;
        //}

        // This method will be replaced eventually, due to the slowdowns it causes
        // Disabling transparency effects removes the slowdowns
        ACCENT_POLICY* pAccentPolicy = (ACCENT_POLICY*)(windowData + CWD_AccentPolicy);
        if (pAccentPolicy->nAccentState != awmsettings.accent) {
            pAccentPolicy->nAccentState = awmsettings.accent;
            //pAccentPolicy->nFlags = ACCENT_USEGEOMETRY;
            pAccentPolicy->nColor = 0;
            if (pAccentPolicy->nAccentState == ACCENT_ENABLE_ACRYLICBLURBEHIND)
                pAccentPolicy->nColor = 0x01000000;
            CTopLevelWindow_OnAccentPolicyUpdated(pThis);
        }

    }
    if (titlebarBrushProxy) {
        BYTE* colorizationResources = *(BYTE**)(pThis + CTLW_GlassColorTB);
        D3DCOLORVALUE colorResource = *(D3DCOLORVALUE*)(colorizationResources + CGCR_Color);
        D3DCOLORVALUE* tbClr = (D3DCOLORVALUE*)(pThis + CTLW_TitlebarColor);
        color.r = colorResource.r;
        color.g = colorResource.g;
        color.b = colorResource.b;
        color.a = colorResource.a;
        //if (tbClr->r != color.r || tbClr->g != color.g || tbClr->b != color.b || tbClr->a != color.a) {
#if TARGETBUILD == 19041 || TARGETBUILD == 18362
        CSolidColorMilBrushProxy_Update(titlebarBrushProxy, 1.0, &color);
#elif TARGETBUILD == 17763
        CChannel_SolidColorLegacyMilBrushUpdate(
            *(CChannel**)(titlebarBrushProxy + 16), 
            *(int*)(titlebarBrushProxy + 24),
            1.0,
            &color,
            0,
            0,
            0
        );
#endif
        *tbClr = color;
        //}
        // move some of these into their own function, same with above
    }

    return rv;
}

long CTopLevelWindow_UpdateNCAreaButton(BYTE* pThis, int buttonId, int insetTop, int* insetRight, int DPIValue) {

    if (!*(long long*)(pThis + CTLW_ButtonVisuals + 8 * buttonId)) { // if this button doesn't exist on the window, return.
        return 0;
    }
    BYTE* buttonVisual = *(BYTE**)(pThis + CTLW_ButtonVisuals + 8 * buttonId);
    BYTE* windowVisual = *(BYTE**)(pThis + CTLW_WindowVisual);
    MARGINS borderMargins = *(MARGINS*)(pThis + CTLW_BorderMargins);
    bool isMaximized = *(pThis + wState1) & STATE_MAXIMIZED;
    bool isTool = *(pThis + wState2) & TYPE_TOOL;

    int TBHeight;
    int width;
    int height;
    if (isTool) {
        TBHeight = GetSystemMetricsForDpi((int)53, (UINT)DPIValue);
    }
    else {
        TBHeight = GetSystemMetricsForDpi((int)31, (UINT)DPIValue);
    }

    int fullTBHeight = TBHeight + *(DWORD*)(windowVisual + CVis_BorderWidth) + 1;

    CVisual_SetInsetFromParentRight(buttonVisual, *insetRight);

    if (isTool) {
        height = floor(TBHeight * (double)(awmsettings.xBtnHeightPalette / awmsettings.targetTBHeightPalette) + 0.5);
        width = floor(TBHeight * (double)(awmsettings.xBtnWidthPalette / awmsettings.targetTBHeightPalette) + 0.5);

        if (awmsettings.tbBtnAlignPalette == AWM_BTN_ALIGN_FULL) {
            BYTE* windowVisual = *(BYTE**)(pThis + CTLW_WindowVisual);
            height = fullTBHeight;
        }
        *insetRight += width;
    }
    else {
        if (buttonId == ID_CLOSE) {
            if ((*(DWORD*)(pThis + wState2) & TYPE_LONEBTN) == 0) {
                width = floor(TBHeight * (double)(awmsettings.xBtnWidthLone / awmsettings.targetTBHeight) + 0.5);
                height = floor(TBHeight * (double)(awmsettings.xBtnHeightLone / awmsettings.targetTBHeight) + 0.5);

                int insetAdd;

                if (isMaximized) {
                    insetAdd = awmsettings.xBtnInsetTopLoneMaximized;
                }
                else {
                    insetAdd = awmsettings.xBtnInsetTopLoneNormal;
                }

                switch (awmsettings.tbBtnAlign) {
                case AWM_BTN_ALIGN_TOP:
                case AWM_BTN_ALIGN_FULL:
                    insetTop += insetAdd;
                    break;
                case AWM_BTN_ALIGN_CENTER:
                    insetTop += (fullTBHeight - height - insetTop) / 2;
                    break;
                case AWM_BTN_ALIGN_TBCENTER:
                    insetTop = fullTBHeight - TBHeight - 1;
                    insetTop += (TBHeight - height) / 2;
                    break;
                case AWM_BTN_ALIGN_BOTTOM:
                    insetTop = fullTBHeight - height - insetAdd;
                    break;
                }
            }
            else {
                width = floor(TBHeight * (double)(awmsettings.xBtnWidth / awmsettings.targetTBHeight) + 0.5);
                height = floor(TBHeight * (double)(awmsettings.xBtnHeight / awmsettings.targetTBHeight) + 0.5);

                int insetAdd;

                if (isMaximized) {
                    insetAdd = awmsettings.xBtnInsetTopMaximized;
                }
                else {
                    insetAdd = awmsettings.xBtnInsetTopNormal;
                }

                switch (awmsettings.tbBtnAlign) {
                case AWM_BTN_ALIGN_TOP:
                case AWM_BTN_ALIGN_FULL:
                    insetTop += insetAdd;
                    break;
                case AWM_BTN_ALIGN_CENTER:
                    insetTop += (fullTBHeight - height - insetTop) / 2;
                    break;
                case AWM_BTN_ALIGN_TBCENTER:
                    insetTop = fullTBHeight - TBHeight - 1;
                    insetTop += (TBHeight - height) / 2;
                    break;
                case AWM_BTN_ALIGN_BOTTOM:
                    insetTop = fullTBHeight - height - insetAdd;
                    break;
                }
            }
            *insetRight += width + awmsettings.xBtnInsetAfter;
        }
        else if ((buttonId != ID_MIN || *(long long*)(pThis + CTLW_ButtonVisuals)) && buttonId) {
            width = floor(TBHeight * (double)(awmsettings.midBtnWidth / awmsettings.targetTBHeight) + 0.5);
            height = floor(TBHeight * (double)(awmsettings.midBtnHeight / awmsettings.targetTBHeight) + 0.5);

            int insetAdd;

            if (isMaximized) {
                insetAdd = awmsettings.midBtnInsetTopMaximized;
            }
            else {
                insetAdd = awmsettings.midBtnInsetTopNormal;
            }

            switch (awmsettings.tbBtnAlign) {
            case AWM_BTN_ALIGN_TOP:
            case AWM_BTN_ALIGN_FULL:
                insetTop += insetAdd;
                break;
            case AWM_BTN_ALIGN_CENTER:
                insetTop += (fullTBHeight - height - insetTop) / 2;
                break;
            case AWM_BTN_ALIGN_TBCENTER:
                insetTop = fullTBHeight - TBHeight - 1;
                insetTop += (TBHeight - height) / 2;
                break;
            case AWM_BTN_ALIGN_BOTTOM:
                insetTop = fullTBHeight - height - insetAdd;
                break;
            }

            *insetRight += width + awmsettings.midBtnInsetAfter;
        }
        else {
            width = floor(TBHeight * (double)(awmsettings.edgeBtnWidth / awmsettings.targetTBHeight) + 0.5);
            height = floor(TBHeight * (double)(awmsettings.edgeBtnHeight / awmsettings.targetTBHeight) + 0.5);

            int insetAdd;

            if (isMaximized) {
                insetAdd = awmsettings.edgeBtnInsetTopMaximized;
            }
            else {
                insetAdd = awmsettings.edgeBtnInsetTopNormal;
            }

            switch (awmsettings.tbBtnAlign) {
            case AWM_BTN_ALIGN_TOP:
            case AWM_BTN_ALIGN_FULL:
                insetTop += insetAdd;
                break;
            case AWM_BTN_ALIGN_CENTER:
                insetTop += (fullTBHeight - height - insetTop) / 2;
                break;
            case AWM_BTN_ALIGN_TBCENTER:
                insetTop = fullTBHeight - TBHeight;
                insetTop += (TBHeight - height - 0.5f) / 2;
                break;
            case AWM_BTN_ALIGN_BOTTOM:
                insetTop = fullTBHeight - height - insetAdd;
                break;
            }

            *insetRight += width + awmsettings.edgeBtnInsetAfter;
        }

        // This could be reworked to use fullTBHeight instead, at the cost of losing Windows 10 parity.
        if (awmsettings.tbBtnAlign == AWM_BTN_ALIGN_FULL) {
            height = TBHeight + *(DWORD*)(windowVisual + CVis_BorderWidth);
            if (height > borderMargins.cyTopHeight - insetTop) {
                height -= insetTop;
            }
        }
    }
    CVisual_SetInsetFromParentTop(buttonVisual, insetTop);

    SIZE size = { width, height };
    CVisual_SetSize(buttonVisual, &size);
    return 0;
}

long CTLW_UpdateNCAreaPositionsAndSizes_Hook(BYTE* pThis) {
    int XBOffsetNor = awmsettings.xBtnOffsetNormal;
    int XBOffsetMax = awmsettings.xBtnOffsetMaximized;
    MARGINS clientMargins = *(MARGINS*)(pThis + CTLW_ClientMargins);

    // Here as a tiny test, plus a reference on how to use CDM_pDesktopManagerInstance
    //*(double*)(*CDM_pDesktopManagerInstance + CDM_Win10BorderSize) = (double)(3 + GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXPADDEDBORDER));

    bool isMaximized = *(pThis + wState1) & STATE_MAXIMIZED;
    bool isTool = *(pThis + wState2) & TYPE_TOOL;

    BYTE* windowVisual = *(BYTE**)(pThis + CTLW_WindowVisual);
    int DPIValue = *(int*)(windowVisual + CVis_DPI);

    // For getting the caption buttons back in windows without a client area.
    /*if (DPIValue == 0)
        DPIValue = 96;*/

    if (*(long long *)(pThis + CTLW_ClientArea)) {
        BYTE* clientVisual = *(BYTE **)(pThis + CTLW_ClientVisual);
        CVisual_SetInsetFromParentLeft(clientVisual, clientMargins.cxLeftWidth);
        CVisual_SetInsetFromParentRight(clientVisual, clientMargins.cxRightWidth);
        CVisual_SetInsetFromParentTop(clientVisual, clientMargins.cyTopHeight);
        CVisual_SetInsetFromParentBottom(clientVisual, clientMargins.cyBottomHeight);

        // seems unused, included here anyway just in case
        BYTE* clientBlur = *(BYTE **)(pThis + CTLW_ClientBlur);
        if (clientBlur) {
            CVisual_SetInsetFromParent(clientBlur, (struct _MARGINS*)(clientVisual + CVis_InsetFromParent));
        }
    }

    MARGINS* borderSizes;
    if (isMaximized) {
        borderSizes = (MARGINS*)(pThis + CTLW_BorderSizesMax);
    }
    else {
        borderSizes = (MARGINS*)(pThis + CTLW_BorderSizesNormal);
    }

    // left inset calculations for icon and text
    int insetLeft = awmsettings.insetLeftAddNormal + awmsettings.insetLeftMulNormal * clientMargins.cxLeftWidth;
    if (isMaximized) {
        insetLeft = awmsettings.insetLeftAddMaximized + awmsettings.insetLeftMulMaximized * clientMargins.cxLeftWidth;
    }

    // right inset calculations for buttons
    // might add some more options here later
    int insetRight = clientMargins.cxRightWidth;
    if (insetRight <= 0) {
        insetRight = *(int*)(windowVisual + CVis_BorderWidth);
    }

    /*if (insetRight + XBOffsetNor <= borderSizes->cxRightWidth + XBOffsetMax) {
        insetRight = borderSizes->cxRightWidth + XBOffsetMax;
    }
    else {
        insetRight += XBOffsetNor;
    }*/
    if (isMaximized) {
        insetRight = borderSizes->cxRightWidth + XBOffsetMax;
    }
    else {
        insetRight += XBOffsetNor;
    }

    int insetTop = borderSizes->cyTopHeight;

    if (isTool) {
        int TBHeight = GetSystemMetricsForDpi(53, DPIValue);
        int fullTBHeight = TBHeight + *(DWORD*)(windowVisual + CVis_BorderWidth) + 1;
        int btnHeight = floor(TBHeight * (double)(awmsettings.xBtnHeightPalette / awmsettings.targetTBHeightPalette) + 0.5);

        switch (awmsettings.tbBtnAlignPalette) {
        case AWM_BTN_ALIGN_TOP:
            insetTop += awmsettings.xBtnInsetDirectionalPalette;
            break;
        case AWM_BTN_ALIGN_CENTER:
            insetTop += (fullTBHeight - btnHeight - insetTop) / 2;
            break;
        case AWM_BTN_ALIGN_TBCENTER:
            insetTop = fullTBHeight - TBHeight - 1;
            insetTop += (TBHeight - btnHeight)/2;
            break;
        case AWM_BTN_ALIGN_BOTTOM:
            insetTop = fullTBHeight - btnHeight - awmsettings.xBtnInsetDirectionalPalette;
            break;
        }
        insetRight+=awmsettings.xBtnOffsetPalette;
    }

    CTopLevelWindow_UpdateNCAreaButton(pThis, 3, insetTop, &insetRight, DPIValue);
    CTopLevelWindow_UpdateNCAreaButton(pThis, 2, insetTop, &insetRight, DPIValue);
    CTopLevelWindow_UpdateNCAreaButton(pThis, 1, insetTop, &insetRight, DPIValue);
    CTopLevelWindow_UpdateNCAreaButton(pThis, 0, insetTop, &insetRight, DPIValue);

    BYTE* iconVisual = *(BYTE**)(pThis + CTLW_IconVisual);
    if (iconVisual) {
        tagSIZE size = { 0 };
        if (*(int*)(windowVisual + CVis_InsetTop) || *(int*)(windowVisual + CVis_InsetBottom) || (*(DWORD*)(pThis + wState2) & 0x10000) == 0) {
            size.cx = GetSystemMetricsForDpi(SM_CXSMICON, DPIValue);
            size.cy = GetSystemMetricsForDpi(SM_CYSMICON, DPIValue);
        }
        CVisual_SetSize(iconVisual, &size);

        if (awmsettings.iconTextVerticalAlign == AWM_TEXT_VALIGN_CENTER) {
            insetTop = borderSizes->cyTopHeight + (clientMargins.cyTopHeight - size.cx - borderSizes->cyTopHeight) / 2;
        }
        else {
            int borderSize = *(DWORD*)(windowVisual + CVis_BorderWidth);
            insetTop = borderSize + (clientMargins.cyTopHeight - size.cx - borderSize - 1) / 2;
        }
        CVisual_SetInsetFromParentLeft(iconVisual, insetLeft);
        CVisual_SetInsetFromParentTop(iconVisual, insetTop);

        if (size.cx > 0) {
            insetLeft += size.cx + awmsettings.textInset;
        }
    }

    BYTE* textVisual = *(BYTE**)(pThis + CTLW_TextVisual);
    if (textVisual) {
        int borderSize;
        if (awmsettings.iconTextVerticalAlign == AWM_TEXT_VALIGN_CENTER)
            borderSize = borderSizes->cyTopHeight;
        else
            borderSize = *(DWORD*)(windowVisual + CVis_BorderWidth);

        tagSIZE size = {0, clientMargins.cyTopHeight - borderSize - 2};

        CVisual_SetInsetFromParentTop(textVisual, borderSize + 1);
        //CVisual_SetInsetFromParentLeft(textVisual, insetLeft);
        
        // Changed from above to support my hacky method of handling text glow
        CVisual_SetInsetFromParentLeft(textVisual, 1 + borderSizes->cxLeftWidth);
        CVisual_SetInsetFromParentRight(textVisual, 1);
        CVisual_SetSize(textVisual, &size);

        // place this somewhere it will update on window resize.
        TEXTEX* textex = *(TEXTEX**)(textVisual + CTxt_Ex);
        //RECT winrc = *(RECT*)(windowVisual + CWD_WindowRect);
        //textex->tbWidth = winrc.right - winrc.left;
        textex->textInset = { 0 };
        textex->textInset.cxLeftWidth = insetLeft - 1 - borderSizes->cxLeftWidth;
        textex->textInset.cxRightWidth = insetRight - 1;


        // SOME TESTING CODE. IT CAN BE SAFELY REMOVED AND IS NOT SUPPOSED TO BE HERE.

        /*RECT rc = {winrc.left + borderSizes->cxLeftWidth,
            winrc.top + borderSizes->cyTopHeight,
            winrc.right + borderSizes->cxRightWidth,
            winrc.bottom + borderSizes->cyBottomHeight,
        };

        float DPI = *(float*)(windowVisual + CWD_DPIFloat);
        *(float*)(windowVisual + CWD_DPIFloat) = 1.f;
        fprintf(stream, "%f\n", DPI);
        fprintf(stream, "%i\n", *(int*)(windowVisual + CWD_WindowRectRight));
        HRGN rgn = CreateRectRgnIndirect(&winrc);
        SetWindowRgnEx(*(HWND*)(windowVisual + CWD_HWND), rgn, 1);
        DeleteObject(rgn);*/
    }

    CTopLevelWindow_UpdatePinnedParts(pThis); // Handles the atlas borders. A rewrite would be able to restore 7 behavior fully (except thickening of small kernel frames).

    return 0;
}

CText* CText_CText_Hook(BYTE* pThis) {
    CText* textobj = CText_CText_orig(pThis);
    TEXTEX* textex = (TEXTEX*)malloc(sizeof(TEXTEX));
    ZeroMemory(textex, sizeof(TEXTEX));
    *(TEXTEX**)((BYTE*)textobj + CTxt_Ex) = textex;
    // This can be checked so that the object is created within various text-related functions.
    *((BYTE*)textobj + CTxt_ExFlag) = true;
    textex->render = true;
    return textobj;
}

CText* CText_Destroy_Hook(BYTE* pThis, UINT someFlags) {
    if (*((BYTE*)pThis + CTxt_ExFlag)) {
        TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);
        if (textex->textFormat)
            textex->textFormat->Release();
        if (textex->textLayout)
            textex->textLayout->Release();
        free(*(TEXTEX**)(pThis + CTxt_Ex));
    }
    CText* textobj = CText_Destroy_orig(pThis, someFlags);
    return textobj;
}

int CTLW_UpdateWindowVisuals_Hook(BYTE* pThis) {
    int rv = CTLW_UpdateWindowVisuals_orig(pThis);
    D2D1_COLOR_F color;
    D2D1_COLOR_F colorShadow;
    float glowOpacity = awmsettings.textGlowOpacityActive;
    BYTE* windowdata = *(BYTE**)(pThis + CTLW_WindowData);
    RECT winrc = *(RECT*)(windowdata + CWD_WindowRect);
    BYTE* textobj = *(BYTE**)(pThis + CTLW_TextVisual);
    if (textobj) {
        if (*(textobj + CTxt_ExFlag) == false) {
            TEXTEX* textex = (TEXTEX*)malloc(sizeof(TEXTEX));
            ZeroMemory(textex, sizeof(TEXTEX));
            *(TEXTEX**)(textobj + CTxt_Ex) = textex;
            *(textobj + CTxt_ExFlag) = true;
            textex->render = true;
        }
        TEXTEX* textex = *(TEXTEX**)(textobj + CTxt_Ex);
        if (CTLW_TreatAsActiveWindow_orig(pThis)) {
            color.r = awmsettings.colorTextActiveR;
            color.g = awmsettings.colorTextActiveG;
            color.b = awmsettings.colorTextActiveB;
            color.a = awmsettings.colorTextActiveA;
            colorShadow.r = awmsettings.colorTextShadowActiveR;
            colorShadow.g = awmsettings.colorTextShadowActiveG;
            colorShadow.b = awmsettings.colorTextShadowActiveB;
            colorShadow.a = awmsettings.colorTextShadowActiveA;
            glowOpacity = awmsettings.textGlowOpacityActive;
            if (!textex->isActive) {
                textex->render = true;
                textex->isActive = true;
            }
        }
        else {
            color.r = awmsettings.colorTextInactiveR;
            color.g = awmsettings.colorTextInactiveG;
            color.b = awmsettings.colorTextInactiveB;
            color.a = awmsettings.colorTextInactiveA;
            colorShadow.r = awmsettings.colorTextShadowInactiveR;
            colorShadow.g = awmsettings.colorTextShadowInactiveG;
            colorShadow.b = awmsettings.colorTextShadowInactiveB;
            colorShadow.a = awmsettings.colorTextShadowInactiveA;
            glowOpacity = awmsettings.textGlowOpacityInactive;
            if (textex->isActive) {
                textex->render = true;
                textex->isActive = false;
            }
        }
        BYTE* textFlags2 = (textobj + CTxt_Flags2);
        *textFlags2 &= ~CTXT_FLAG2_TEXTCHANGED;
        textex->color = color;
        textex->shadowcolor = colorShadow;
        textex->glowopacity = glowOpacity;
    }

    return rv;
}

int CText_InitializeVisualTreeClone_Hook(BYTE* pThis, BYTE* pNew, UINT options) {
    // The pointer to the TEXTEX struct is saved, as the function overwrites the pointer.
    long long pSaved = *(long long*)(pNew + CTxt_Ex);
    int rv = CText_InitializeVisualTreeClone_orig(pThis, pNew, options);
    *(long long*)(pNew + CTxt_Ex) = pSaved; // reload the TEXTEX struct pointer into its position

    if (*(BYTE*)(pThis + CTxt_ExFlag) == false) {
        TEXTEX* textex = (TEXTEX*)malloc(sizeof(TEXTEX));
        ZeroMemory(textex, sizeof(TEXTEX));
        *(TEXTEX**)(pThis + CTxt_Ex) = textex;
        *(pThis + CTxt_ExFlag) = true;
        textex->render = true;
    }
    TEXTEX* textex1 = *(TEXTEX**)(pThis + CTxt_Ex);
    TEXTEX* textex2 = *(TEXTEX**)(pNew + CTxt_Ex);
    textex2->color = textex1->color;
    textex2->shadowcolor = textex1->shadowcolor;
    textex2->tbWidth = textex1->tbWidth;
    textex2->glowopacity = textex1->glowopacity;
    textex2->textInset = textex1->textInset;
    //CopyMemory(textex2, textex1, sizeof(TEXTEX));
    //textex2->textFormat = textex1->textFormat;
    //textex2->textLayout = NULL;
    //textex2->render = true;

    return rv;
}

void CText_SetColor_Hook(BYTE* pThis, COLORREF color) {
}

void CText_SetBackgroundColor_Hook(BYTE* pThis, COLORREF color) {
}

int CText_ValidateResources_Hook(BYTE* pThis) {
    int hr = 0;
    DWORD* textFlags = (DWORD*)(pThis + CTxt_Flags);
    BYTE* textFlags2 = (pThis + CTxt_Flags2);
    DWORD CVisualFlags = *(pThis + CVis_SomeFlags);
    if (*textFlags & CTXT_FLAG_UPDATE) {
        LOGFONT font = *(LOGFONT*)(pThis + CTxt_Font);
        RECT fillbox = { 0 };
        D2D1_SIZE_F size = { 0 };
        IWICImagingFactory* imagingfactory = *(IWICImagingFactory**)(*CDM_pDesktopManagerInstance + CDM_ImagingFactory);
        IWICBitmap* bitmap{};
        ID2D1RenderTarget* target{};
        IMiosD2D1RenderTarget* miostarget{};
        IDWriteTextFormat* textformat{};
        IDWriteTextLayout* textlayout{};
        ID2D1SolidColorBrush* textbrush{};
        ID2D1SolidColorBrush* shadowbrush{};
        IDWriteInlineObject* trimmingsign{};
        ID2D1Bitmap* atlasbitmap{};
        DWRITE_TEXT_METRICS metrics;
        CCompositor* compositor = *(CCompositor**)(*CDM_pDesktopManagerInstance + CDM_Compositor);
        CBaseTransformProxy* transformProxy = nullptr;
        CBitmapSource* bmpsrc = nullptr;
        CDrawImageInstruction* imageinstruction = nullptr;
        CPushTransformInstruction* transforminstruction = nullptr;
        CPopInstruction* popinstruction = nullptr;
        HTHEME hTheme = *(HTHEME*)(*CDM_pDesktopManagerInstance + CDM_DWMWindowThemeData);
        RECT atlasrect = { 0 };
        MARGINS sizingmargins = { 0 };
        MARGINS contentmargins = { 0 };
        D2D1_RECT_F glowdrawrect = { 0 };
        D2D1_RECT_F glowsrcrect = { 0 };
        D2D1_SIZE_F glowscale = { 1, 1 };
        bool glowsizechanged = false;

        LPCWSTR string = *(LPCWSTR*)(pThis + CTxt_String);
        TEXTEX* textex = *(TEXTEX**)(pThis + CTxt_Ex);
        if (*(BYTE*)(pThis + CTxt_ExFlag) == false) {
            TEXTEX* textex = (TEXTEX*)malloc(sizeof(TEXTEX));
            ZeroMemory(textex, sizeof(TEXTEX));
            *(TEXTEX**)(pThis + CTxt_Ex) = textex;
            *(pThis + CTxt_ExFlag) = true;
            textex->render = true;
        }
        fillbox.right = *(int*)(pThis + CVis_Width);
        fillbox.bottom = *(int*)(pThis + CVis_Height);

        if (fillbox.left != textex->fillboxSize.left ||
            fillbox.top != textex->fillboxSize.top ||
            fillbox.right != textex->fillboxSize.right ||
            fillbox.bottom != textex->fillboxSize.bottom)
            textex->render = true;
        if (textex->render) {
            CText_ReleaseResources(pThis);
            if (fillbox.right > 0 && fillbox.bottom > 0) {
                textex->fillboxSize = fillbox;

                if (CVisualFlags & CVIS_FLAG_RTL) {
                    //OffsetRect(&fillbox, -(fillbox.right), 0);
                    //OffsetRect(&fillbox, -(textex->textInset.cxLeftWidth), 0);
                    //textex->textInset.cxLeftWidth = 0;
                }

                if (string) {
                    //int stringlength = wcslen(string);
                    //DWRITE_TEXT_RANGE range = { 0, stringlength };

                    hr = imagingfactory->CreateBitmap(
                        fillbox.right - fillbox.left,
                        fillbox.bottom - fillbox.top,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapCacheOnDemand,
                        &bitmap
                    );
                    if (hr < 0)
                        goto release;

                    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT);
                    hr = d2dfactory->CreateWicBitmapRenderTarget(bitmap, props, &target);
                    if (hr < 0)
                        goto release;

                    size = target->GetSize();
                    size.width -= (textex->textInset.cxLeftWidth + textex->textInset.cxRightWidth);
                    if (size.width > 0) {

                        /*D2D1_COLOR_F colorText = {
                            awmsettings.colorTextActiveR,
                            awmsettings.colorTextActiveG,
                            awmsettings.colorTextActiveB,
                            1.0f
                        };*/
                        D2D1_COLOR_F colorText = textex->color;
                        hr = target->CreateSolidColorBrush(colorText, &textbrush);
                        if (hr < 0)
                            goto release;

                        /*D2D1_COLOR_F colorShadow = {
                            awmsettings.colorTextShadowActiveR,
                            awmsettings.colorTextShadowActiveG,
                            awmsettings.colorTextShadowActiveB,
                            awmsettings.colorTextShadowActiveA
                        };*/
                        D2D1_COLOR_F colorShadow = textex->shadowcolor;
                        hr = target->CreateSolidColorBrush(colorShadow, &shadowbrush);
                        if (hr < 0)
                            goto release;

                        /*DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
                        if (font.lfItalic)
                            style = DWRITE_FONT_STYLE_ITALIC;
                        float height = (float)-font.lfHeight;
                        if (height < 0) {
                            height = -height;
                        }
                        else if (height == 0) {
                            height = -(float)((*(LOGFONT*)(*CDM_pDesktopManagerInstance + CDM_CaptionFont)).lfHeight);
                        }
                        hr = dwritefactory->CreateTextFormat(
                            font.lfFaceName,
                            NULL,
                            (DWRITE_FONT_WEIGHT)font.lfWeight,
                            style,
                            DWRITE_FONT_STRETCH_NORMAL,
                            height,
                            L"en-us",
                            &textformat
                        );
                        if (hr < 0)
                            goto release;

                        textformat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                        hr = dwritefactory->CreateEllipsisTrimmingSign(textformat, &trimmingsign);
                        if (hr < 0)
                            goto release;
                        DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, NULL, 0 };
                        hr = textformat->SetTrimming(&trimming, trimmingsign);
                        if (hr < 0)
                            goto release;
                        if (CVisualFlags & CVIS_FLAG_RTL) {
                            hr = textformat->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
                            if (hr < 0)
                                goto release;
                        }

                        if (awmsettings.textAlignment == AWM_TEXT_CENTER_ICONBUTTONS) {
                            textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                        }
                        else if (awmsettings.textAlignment == AWM_TEXT_RIGHT) {
                            textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                        }*/
                        if (!textex->textFormat) {
                            CText_CreateTextFormat(pThis, &font);
                        }
                        textformat = textex->textFormat;
                        textlayout = textex->textLayout;
                        if (!textlayout)
                            goto release;
                        /*hr = dwritefactory->CreateTextLayout(string, stringlength, textformat, size.width, size.height, &textlayout);
                        if (hr < 0)
                            goto release;
                        hr = textlayout->SetStrikethrough(font.lfStrikeOut, range);
                        if (hr < 0)
                            goto release;
                        hr = textlayout->SetUnderline(font.lfUnderline, range);
                        if (hr < 0)
                            goto release;*/
                        hr = textlayout->SetMaxWidth(size.width);
                        if (hr < 0)
                            goto release;
                        hr = textlayout->SetMaxHeight(size.height);
                        if (hr < 0)
                            goto release;
                        if (CVisualFlags & CVIS_FLAG_RTL) {
                            hr = textlayout->SetReadingDirection(DWRITE_READING_DIRECTION_RIGHT_TO_LEFT);
                            if (hr < 0)
                                goto release;
                        }

                        if (awmsettings.textAlignment == AWM_TEXT_CENTER_ICONBUTTONS) {
                            textlayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                        }
                        else if (awmsettings.textAlignment == AWM_TEXT_RIGHT) {
                            textlayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                        }
                        hr = textlayout->GetMetrics(&metrics);
                        if (hr < 0)
                            goto release;

                        MARGINS insetFromParent = *(MARGINS*)(pThis + CVis_InsetFromParent);
                        DWORD fullTBWidth = fillbox.right;

                        // This fixes the bug described below:
                        if (insetFromParent.cxLeftWidth == 0x7fffffff || insetFromParent.cxRightWidth == 0x7fffffff)
                            fullTBWidth = textex->tbWidth;
                        textex->tbWidth = fullTBWidth;

                        float xoffset = 0.0f;
                        float yoffset = (size.height - metrics.height - 1.5f) / 2.0f;
                        if (awmsettings.textAlignment == AWM_TEXT_CENTER_W8) {
                            xoffset = (float)floor((fullTBWidth - metrics.width) / 2.0f + 0.5f);

                            // BUG! For some reason the inset is set to 0x7fffffff while minimizing. Make fullTBWidth 
                            // be a property in the extra CText object that will be created, to avoid this issue, as 
                            // it will be taken directly from the window itself (in UpdateNCAreaPositionsAndSizes)

                            //xoffset -= (float)insetFromParent.cxLeftWidth;
                            xoffset -= 1;
                            xoffset -= textex->textInset.cxLeftWidth;
                            if (xoffset < 0)
                                xoffset = 0;
                            if (xoffset + metrics.width >= size.width) {
                                xoffset = (size.width - metrics.width) / 2.0f;

                                textlayout->SetMaxWidth(fullTBWidth);
                                DWRITE_TEXT_METRICS fullmetrics;
                                textlayout->GetMetrics(&fullmetrics);
                                if (size.width < fullmetrics.width) {
                                    xoffset = 0.0f;
                                }
                                textlayout->SetMaxWidth(size.width);
                            }
                        }
                        else if (awmsettings.textAlignment == AWM_TEXT_CENTER_DULAPPY) {
                            xoffset = ((float)fullTBWidth - metrics.width) / 2.0f;
                            //xoffset -= (float)insetFromParent.cxLeftWidth;
                            xoffset -= 1;
                            xoffset -= textex->textInset.cxLeftWidth;
                            if (xoffset + metrics.width >= size.width)
                                xoffset = size.width - metrics.width;
                            if (xoffset < 0)
                                xoffset = 0;
                        }

                        // Ensure that the update flag will be set every time the window is resized.
                        *textFlags2 &= ~CTXT_FLAG2_TEXTCHANGED;
                        // These two are now being used to store the CTextEx object's address.
                        //*(DWORD*)(pThis + CTxt_LimRight) = 0;
                        //*(DWORD*)(pThis + CTxt_LimBottom) = 0;

                        hr = GetMiosThemeBitmapProps(
                            hTheme,
                            45,
                            0,
                            &atlasrect,
                            &sizingmargins,
                            &contentmargins,
                            0,
                            true
                        );
                        /*if (hr < 0)
                            goto release;*/
                        hr = target->CreateBitmapFromWicBitmap(DWMAtlas, &atlasbitmap);
                        if (hr < 0)
                            goto release;

                        if (CVisualFlags & CVIS_FLAG_RTL) {
                            xoffset = -xoffset;
                            xoffset -= textex->textInset.cxLeftWidth;
                        }
                        else if (metrics.left < 0) {
                            xoffset -= metrics.left;
                        }
                        glowsrcrect.left = atlasrect.left;
                        glowsrcrect.top = atlasrect.top;
                        glowsrcrect.right = atlasrect.right;
                        glowsrcrect.bottom = atlasrect.bottom;

                        glowdrawrect.left = xoffset - contentmargins.cxLeftWidth;
                        glowdrawrect.right = xoffset + metrics.width + contentmargins.cxRightWidth;
                        glowdrawrect.top = (int)(yoffset + 0.75f - (float)contentmargins.cyTopHeight);
                        glowdrawrect.bottom = yoffset + metrics.height + contentmargins.cyBottomHeight;
                        if (metrics.left > 0) {
                            glowdrawrect.left += metrics.left;
                            glowdrawrect.right += metrics.left;
                        }
                        if (glowdrawrect.bottom - glowdrawrect.top < sizingmargins.cyBottomHeight + sizingmargins.cyTopHeight) {
                            float heightnew = sizingmargins.cyBottomHeight + sizingmargins.cyTopHeight;
                            glowscale.height = (float)(glowdrawrect.bottom - glowdrawrect.top) / heightnew;
                            glowdrawrect.bottom = glowdrawrect.top + heightnew;
                            glowsizechanged = true;
                        }
                        if (glowdrawrect.right - glowdrawrect.left < sizingmargins.cxRightWidth + sizingmargins.cxLeftWidth) {
                            float widthnew = sizingmargins.cxRightWidth + sizingmargins.cxLeftWidth;
                            glowscale.width = (float)(glowdrawrect.right - glowdrawrect.left) / widthnew;
                            glowdrawrect.right = glowdrawrect.left + widthnew;
                            glowsizechanged = true;
                        }

                        target->BeginDraw();
                        //fwprintf(stream, L"%i\n", insetFromParent.cxLeftWidth);
                        D2D1_POINT_2F start = { xoffset , yoffset };
                        D2D1_POINT_2F startshadow = start;
                        startshadow.x += awmsettings.textShadowOffsetX;
                        startshadow.y += awmsettings.textShadowOffsetY;

                        D2D1_MATRIX_3X2_F trnsfrmmatrix = D2D1::Matrix3x2F::Translation(textex->textInset.cxLeftWidth, 0);
                        target->SetTransform(trnsfrmmatrix);

                        if (glowsizechanged) {
                            D2D1_POINT_2F point = { glowdrawrect.left, glowdrawrect.top };
                            D2D1_MATRIX_3X2_F sizematrix = D2D1::Matrix3x2F::Scale(glowscale, point);
                            target->SetTransform(trnsfrmmatrix * sizematrix);
                        }

                        miostarget = (IMiosD2D1RenderTarget*)target;
                        miostarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                        miostarget->DrawNineSliceBitmap(
                            atlasbitmap,
                            &glowdrawrect,
                            textex->glowopacity,
                            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                            &glowsrcrect,
                            &sizingmargins
                        );
                        miostarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

                        target->SetTransform(D2D1::Matrix3x2F::Scale(1, 1) * trnsfrmmatrix);

                        miostarget->PushAxisAlignedClip({ 0, 0, size.width, size.height }, D2D1_ANTIALIAS_MODE_ALIASED);
                        target->DrawTextLayout(startshadow, textlayout, shadowbrush, D2D1_DRAW_TEXT_OPTIONS_NONE);
                        target->DrawTextLayout(start, textlayout, textbrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
                        miostarget->PopAxisAlignedClip();

                        hr = target->EndDraw();
                        if (hr < 0)
                            goto release;

                        /*transformProxy = *(CBaseTransformProxy**)(pThis + CTxt_TransformProxy);
                        hr = CCompositor_CreateMatrixTransformProxy(compositor, &transformProxy);
                        if (hr < 0)
                            goto release;
                        hr = CPushTransformInstruction_Create(transformProxy, &transforminstruction);
                        if (hr < 0)
                            goto release;
                        hr = CRenderDataVisual_AddInstruction(pThis, transforminstruction);
                        if (hr < 0)
                            goto release;*/

    #if TARGETBUILD == 19041 || TARGETBUILD == 18362
                        hr = CBitmapSource_Create(bitmap, 0, &bmpsrc);
    #elif TARGETBUILD == 17763
                        BYTE* resource = *(BYTE**)(pThis + CTxt_Resource);
                        hr = CBitmapSource_Create(bitmap, 0, *(IDwmChannel**)(resource + CResource_Channel), &bmpsrc);
    #endif
                        if (hr < 0)
                            goto release;
                        hr = CDrawImageInstruction_Create(bmpsrc, &fillbox, &imageinstruction);
                        if (hr < 0)
                            goto release;
                        hr = CRenderDataVisual_AddInstruction(pThis, imageinstruction);
                        if (hr < 0)
                            goto release;

                        /*hr = CPopInstruction_Create(&popinstruction);
                        if (hr < 0)
                            goto release;
                        hr = CRenderDataVisual_AddInstruction(pThis, popinstruction);*/
                    }
                    textex->render = false;
                }
            }
        }
    release:
        if (bitmap)
            bitmap->Release();
        if (target)
            target->Release();
        if (textbrush)
            textbrush->Release();
        if (shadowbrush)
            shadowbrush->Release();
        //if (textformat)
        //    textformat->Release();
        //if (textlayout)
        //    textlayout->Release();
        if (trimmingsign)
            trimmingsign->Release();
        if (atlasbitmap)
            atlasbitmap->Release();
        if (bmpsrc)
            CBaseObject_Release(bmpsrc);
        //if (transforminstruction)
        //    CBaseObject_Release(transforminstruction);
        if (imageinstruction)
            CBaseObject_Release(imageinstruction);
        //if (popinstruction)
        //    CBaseObject_Release(popinstruction);
    }
    *textFlags &= ~CTXT_FLAG_UPDATE;
    return hr;
}

float red = 0.0;
float green = 0.0;
float blue = 0.0;
__int64 CRD_DrawSolidColorRectangle_Hook(BYTE* pThis, BYTE* pDrawingContext, BYTE* pDrawListEntryBuilder, char unk, MilRectF* rect, D3DCOLORVALUE* color) {
    /*if (red == 1.0 && green != 1.0) {
        if (green < 1.0)
            green += 0.1;
        if (blue > 0.0)
            blue -= 0.1;
    }
    if (green == 1.0 && blue != 1.0) {
        if (blue < 1.0)
            blue += 0.1;
        if (red > 0.0)
            red -= 0.1;
    }
    if (blue == 1.0 && red != 1.0) {
        if (red < 1.0)
            red += 0.1;
        if (green > 0.0)
            green -= 0.1;
    }*/
    if (red >= 1.0F) {
        red = 0.0F;
    }
    else
        red += (1.0F/60.0F);
    color->r = red;
    color->g = green;
    color->b = blue;
    return 0;
    return CRD_DrawSolidColorRectangle_orig(pThis, pDrawingContext, pDrawListEntryBuilder, unk, rect, color);
}
// ===========================================================================
HRGN AWM_CreateRoundRectRgn(int x1, int x2, int y1, int y2, int w, int h) {
    w = awmsettings.cornerRadiusX * 2;
    h = awmsettings.cornerRadiusY * 2;
    return CreateRoundRectRgn(x1, x2, y1, y2, w, h);
}

int HookFunctions() {

    SetWindowRgnEx = (SetWindowRgnEx_t)GetProcAddress(huser32, "SetWindowRgnEx");

#if TARGETBUILD == 19041 || TARGETBUILD == 18362
    CRD_DrawSolidColorRectangle_orig = (CRD_DrawSolidColorRectangle_t)(
        (uintptr_t)hdwmcore +
        (uintptr_t)addresses_dwmcore[0]
        );
#elif TARGETBUILD == 17763
    CChannel_SolidColorLegacyMilBrushUpdate = (CCh_SolidColorLegacyMilBrushUpdate_t)(
        (uintptr_t)hdwmcore +
        (uintptr_t)addresses_dwmcore[0]
        );
#endif

    CTLW_marMinInflationThickness = (MARGINS*)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[10]
        );
    CDM_pDesktopManagerInstance = (__int64*)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[11]
        );

    CTLW_TreatAsActiveWindow_orig = (CTLW_TreatAsActiveWindow_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[0]
        );

    CTLW_UpdateNCAreaPositionsAndSizes_orig = (CTLW_UpdateNCAreaPositionsAndSizes_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[1]
        );

    CVisual_SetDirtyFlags = (CVis_SetDirtyFlags_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[2]
        );

    CTopLevelWindow_UpdatePinnedParts = (CTLW_UpdatePinnedParts_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[3]
        );

    CTLW_UpdateColorizationColor_orig = (CTLW_UpdateColorizationColor_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[4]
        );

    CTLW_UpdateNCAreaBackground_orig = (CTLW_UpdateNCAreaBackground_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[5]
        );

    CSolidColorMilBrushProxy_Update = (CSCLMBP_Update_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[6]
        );

    CTopLevelWindow_OnAccentPolicyUpdated = (CTLW_OnAccentPolicyUpdated_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[7]
        );

    CTLW_UpdateMarginsDependentOnStyle_orig = (CTLW_UpdateMarginsDependentOnStyle_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[8]
        );

    CTLW_ValidateVisual_orig = (CTLW_ValidateVisual_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[9]
        );

    CText_ValidateResources_orig = (CText_ValidateResources_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[12]
        );

    CBitmapSource_Create = (CBitmapSource_Create_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[13]
        );

    CDrawImageInstruction_Create = (CDrawImageInstruction_Create_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[14]
        );

    CRenderDataVisual_AddInstruction = (CRenderDataVisual_AddInstruction_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[15]
        );

    CText_ReleaseResources = (CText_ReleaseResources_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[16]
        );

    CBaseObject_Release = (CBaseObject_Release_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[17]
        );

    CCompositor_CreateMatrixTransformProxy = (CCompositor_CreateMatrixTransformProxy_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[18]
        );

    CPushTransformInstruction_Create = (CPushTransformInstruction_Create_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[19]
        );

    CPopInstruction_Create = (CPopInstruction_Create_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[20]
        );

    CText_CText_orig = (CText_CText_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[21]
        );

    CText_SetColor_orig = (CText_SetColor_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[22]
        );

    CText_SetBackgroundColor_orig = (CText_SetBackgroundColor_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[23]
        );

    CTLW_UpdateWindowVisuals_orig = (CTLW_UpdateWindowVisuals_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[24]
        );

    CText_InitializeVisualTreeClone_orig = (CText_InitializeVisualTreeClone_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[25]
        );

    CText_Destroy_orig = (CText_Destroy_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[26]
        );

    CDesktopManager_LoadTheme_orig = (CDesktopManager_LoadTheme_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[27]
        );

    CDesktopManager_UnloadTheme_orig = (CDesktopManager_UnloadTheme_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[28]
        );
    CButton_SetVisualStates_orig = (CButton_SetVisualStates_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[29]
        );
    CText_SetText_orig = (CText_SetText_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[30]
        );
    CText_SetFont_orig = (CText_SetFont_t)(
        (uintptr_t)hudwm +
        (uintptr_t)addresses[31]
        );

    // Funchook stuff
    int rv = 0;
    rv = funchook_prepare(funchook, (void**)&CTLW_UpdateNCAreaPositionsAndSizes_orig, CTLW_UpdateNCAreaPositionsAndSizes_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CTLW_UpdateColorizationColor_orig, CTLW_UpdateColorizationColor_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CTLW_UpdateNCAreaBackground_orig, CTLW_UpdateNCAreaBackground_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_ValidateResources_orig, CText_ValidateResources_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_CText_orig, CText_CText_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_SetColor_orig, CText_SetColor_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_SetBackgroundColor_orig, CText_SetBackgroundColor_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CTLW_UpdateWindowVisuals_orig, CTLW_UpdateWindowVisuals_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_InitializeVisualTreeClone_orig, CText_InitializeVisualTreeClone_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_Destroy_orig, CText_Destroy_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CDesktopManager_LoadTheme_orig, CDesktopManager_LoadTheme_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CDesktopManager_UnloadTheme_orig, CDesktopManager_UnloadTheme_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CButton_SetVisualStates_orig, CButton_SetVisualStates_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_SetText_orig, CText_SetText_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CText_SetFont_orig, CText_SetFont_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    /*rv = funchook_prepare(funchook, (void**)&CRD_DrawSolidColorRectangle_orig, CRD_DrawSolidColorRectangle_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }*/

    // I GIVE UP WITH THESE TWO FOR NOW, WILL RESUME DEVELOPMENT AT A FUTURE TIME
    /*rv = funchook_prepare(funchook, (void**)&CTLW_UpdateMarginsDependentOnStyle_orig, CTLW_UpdateMarginsDependentOnStyle_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }
    rv = funchook_prepare(funchook, (void**)&CTLW_ValidateVisual_orig, CTLW_ValidateVisual_Hook);
    if (rv) {
        return ERR_FH_INIT;
    }*/

    rv = funchook_install(funchook, 0);
    if (rv) {
        return ERR_FH_HOOK;
    }

    return 0;
}

int InitFactories() {
    int hr = 0;
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dfactory);
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&dwritefactory);
    return hr;
}

DWORD WINAPI ListenForSettingsChange(HANDLE hEvent) {

    while (true) {
        DWORD rv = WaitForSingleObject(hEvent, -1);
        if (rv == WAIT_OBJECT_0) {
            LoadSettingsFromRegistry();
            ResetEvent(hEvent);
        }
        fwprintf(stream, L"stopped waiting\n");
    }

    return 0;
}

__declspec(dllexport) DWORD WINAPI main(DWORD* dword) {
    funchook = funchook_create();

    // Create console window for logging
    FILE* conout;

    stream = stdout;
#if AWM_DEBUG == TRUE
    if (!AllocConsole());
        if (freopen_s(
            &conout,
            "CONOUT$",
            "w",
            stdout)
            );
            fprintf(
                stream,
                "Aero Window Manager Logs\n========================\n"
            );
#endif

    // -------------------------------------------------------------------

    int rv = LoadSettingsFromRegistry();
    //int rv = 0;
    if (rv) {
        funchook_destroy(funchook);
        FreeLibraryAndExitThread(
            hModule,
            rv
        );
        return rv;
    }

    // -------------------------------------------------------------------

    LoadSymbols(&hModule, &hudwm, &hdwmcore, &huser32, addresses, symNames, addresses_dwmcore, symNames_dwmcore, stream);
    if (hudwm) {
        BOOL patched = VnPatchIAT(hudwm, (PSTR)"gdi32.dll", (PSTR)"CreateRoundRectRgn", (uintptr_t)AWM_CreateRoundRectRgn);
    }

    rv = HookFunctions();
    if (rv) {
        funchook_uninstall(funchook, 0);
        funchook_destroy(funchook);
        FreeLibraryAndExitThread(
            hModule,
            rv
        );
    }

    rv = InitFactories();
    if (rv < 0) {
        funchook_uninstall(funchook, 0);
        funchook_destroy(funchook);
        FreeLibraryAndExitThread(
            hModule,
            rv
        );
    }

    HANDLE hEvent = CreateEventW(NULL, true, false, L"awmsettingschanged");
    CreateThread(NULL, 0, ListenForSettingsChange, hEvent, 0, NULL);

    AWM_GetDWMWindowAtlas();

    return 0;
}

/*WNDPROC wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_WTSSESSION_CHANGE) {
        ExitThread(0);
    }
    fprintf(stream, "I WAS CALLED");
    DefWindowProc(hWnd, msg, wParam, lParam);
}*/

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
        hModule = hinstDLL;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}