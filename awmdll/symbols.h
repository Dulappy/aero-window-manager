#pragma once

#include <Windows.h>
//#include <d3d9types.h>
//#include <uxtheme.h>
#include <stdio.h>
#include "definitions.h"
//#include <awmtypes.h>

#define SYMBOLS_AMOUNT 32
#define SYMBOLS_PATH "\\symbols\\"
#define SYMBOLS_PATH_WIDE L"\\symbols\\"
#define CTLW_TreatAsActiveWindow_Name L"private: bool __cdecl CTopLevelWindow::TreatAsActiveWindow(void)"
#define CTLW_UpdateNCAreaPositionsAndSizes_Name L"private: long __cdecl CTopLevelWindow::UpdateNCAreaPositionsAndSizes(void)"
#define CVis_SetDirtyFlags_Name L"public: virtual void __cdecl CVisual::SetDirtyFlags(unsigned long)"
#define CTLW_UpdatePinnedParts_Name L"private: long __cdecl CTopLevelWindow::UpdatePinnedParts(void)"
#define CTLW_UpdateColorizationColor_Name L"private: long __cdecl CTopLevelWindow::UpdateColorizationColor(void)"
#define CTLW_UpdateNCAreaBackground_Name L"private: long __cdecl CTopLevelWindow::UpdateNCAreaBackground(void)"
#define CSCLMBP_Update_Name L"public: long __cdecl CSolidColorLegacyMilBrushProxy::Update(double,struct _D3DCOLORVALUE const &)"
#define CTLW_OnAccentPolicyUpdated_Name L"public: void __cdecl CTopLevelWindow::OnAccentPolicyUpdated(void)"
#define CTLW_UpdateMarginsDependentOnStyle_Name L"private: bool __cdecl CTopLevelWindow::UpdateMarginsDependentOnStyle(void)"
#define CTLW_ValidateVisual_Name L"public: virtual long __cdecl CTopLevelWindow::ValidateVisual(void)"
#define CTLW_s_marMinInflationThickness_Name L"private: static struct _MARGINS CTopLevelWindow::s_marMinInflationThickness"
#define CDM_s_pDesktopManagerInstance_Name L"private: static class CDesktopManager * CDesktopManager::s_pDesktopManagerInstance"
#define CText_ValidateResources_Name L"private: long __cdecl CText::ValidateResources(void)"
#if TARGETBUILD == 19041 || TARGETBUILD == 18362
#define CBmpSrc_Create_WICBitmap_Name L"public: static long __cdecl CBitmapSource::Create(struct IWICBitmap *,struct _MARGINS const *,class CBitmapSource * *)"
#elif TARGETBUILD == 17763
#define CBmpSrc_Create_WICBitmap_Name L"public: static long __cdecl CBitmapSource::Create(struct IWICBitmap *,struct _MARGINS const *,struct IDwmChannel *,class CBitmapSource * *)"
#endif
#define CDII_Create_Name L"public: static long __cdecl CDrawImageInstruction::Create(class CBitmapSource *,struct tagRECT const *,class CDrawImageInstruction * *)"
#define CRDV_AddInstruction_Name L"public: long __cdecl CRenderDataVisual::AddInstruction(class CRenderDataInstruction *)"
#define CText_ReleaseResources_Name L"private: void __cdecl CText::ReleaseResources(void)"
#define CBaseObject_Release_Name L"public: unsigned long __cdecl CBaseObject::Release(void)"
#define CCompositor_CreateMatrixTransformProxy_Name L"protected: long __cdecl CCompositor::CreateProxy<class CMatrixTransformProxy>(class CMatrixTransformProxy * *)"
#define CPushTransformInstruction_Create_Name L"public: static long __cdecl CPushTransformInstruction::Create(class CBaseTransformProxy *,class CPushTransformInstruction * *)"
#define CPopInstruction_Create_Name L"public: static long __cdecl CPopInstruction::Create(class CPopInstruction * *)"
#define CText_CText_Name L"private: __cdecl CText::CText(void)"
#define CText_SetColor_Name L"public: void __cdecl CText::SetColor(unsigned long)"
#define CText_SetBackgroundColor_Name L"public: void __cdecl CText::SetBackgroundColor(unsigned long)"
#define CTLW_UpdateWindowVisuals_Name L"private: long __cdecl CTopLevelWindow::UpdateWindowVisuals(void)"
#define CText_InitializeVisualTreeClone_Name L"protected: long __cdecl CText::InitializeVisualTreeClone(class CText *,enum CloneOptions)"
#define CText_Destroy_Name L"private: virtual void* __cdecl CText::`scalar deleting dtor'(unsigned int)"
#define CDesktopManager_LoadTheme_Name L"private: long __cdecl CDesktopManager::LoadTheme(void)"
#define CDesktopManager_UnloadTheme_Name L"private: void __cdecl CDesktopManager::UnloadTheme(void)"
#define CButton_SetVisualStates_Name L"public: long __cdecl CButton::SetVisualStates(class CBitmapSourceArray const &,class CBitmapSourceArray const &,class CBitmapSource *,float)"
#define CText_SetText_Name L"public: long __cdecl CText::SetText(unsigned short const *)"
#define CText_SetFont_Name L"public: void __cdecl CText::SetFont(struct tagLOGFONTW const &)"

#define SYMBOLS_AMOUNT_USER32 1
#define SetWindowRgnEx_Name "SetWindowRgnEx"

#if TARGETBUILD == 19041 || TARGETBUILD == 18362
#define SYMBOLS_AMOUNT_DWMCORE 1
#define CRD_DrawSolidColorRectangle_Name L"private: long __cdecl CRenderData::DrawSolidColorRectangle(class CDrawingContext *,class CDrawListEntryBuilder *,bool,struct MilRectF const &,struct _D3DCOLORVALUE const &)"
#elif TARGETBUILD == 17763
#define SYMBOLS_AMOUNT_DWMCORE 1
#define CChSolidColorLegacyMilBrushUpdate_Name L"public: virtual long __cdecl CChannel::SolidColorLegacyMilBrushUpdate(unsigned int,double,struct _D3DCOLORVALUE const &,unsigned int,unsigned int,unsigned int)"
#endif

// ---------------------------------------------------------------------------

int LoadSymbols(HMODULE* phModule, HMODULE* phudwm, HMODULE* phdwmcore, HMODULE* phuser32, DWORD addresses[], LPCWSTR symNames[], DWORD addresses_dwmcore[], LPCWSTR symNames_dwmcore[], FILE* stream);