#pragma once

/*
* VALID BUILDS:
* 19041 (2004-22H2)
* 18362 (1903-1909)
* 17763 (1809)
*/
#define TARGETBUILD     17763

#define VER_MAJOR 0
#define VER_MINOR 1
#define VER_PATCH 0 // these will be moved when i get around to adding proper versioning
// ---------------------------------------------------------------------------
#define ID_CLOSE 3
#define ID_MAXRES 2
#define ID_MIN 1

#if TARGETBUILD == 19041 || TARGETBUILD == 18362
#define STATE_MAXIMIZED  0x04
#define STYLE_THICKFRAME 0x20
#elif TARGETBUILD == 17763
#define STATE_MAXIMIZED  0x08
#define STYLE_THICKFRAME 0x40
#endif

#define TYPE_TOOL       2
#define TYPE_LONEBTN    0xB00
// ---------------------------------------------------------------------------

// CVisual definitions (possibly build-independent)
// Some of these may be CWindowData-specific
#define CVis_SomeFlags  84

#define CVis_Hidden         88
#define CVis_BorderWidth    96

#define CVis_Width      120
#define CVis_Height     124
#define CVis_Size       CVis_Width

#define CVis_InsetLeft          128
#define CVis_InsetRight         132
#define CVis_InsetTop           136
#define CVis_InsetBottom        140
#define CVis_InsetFromParent    CVis_InsetLeft

#define CVis_DPI                324 // This might fit CWindowData more.

#define CVIS_FLAG_UPDATEVISIBILITY 0x100
#define CVIS_FLAG_RTL                  1

// CGlassColorizationResources definitions
#define CGCR_ColorRed       16
#define CGCR_ColorGreen     20
#define CGCR_ColorBlue      24
#define CGCR_ColorAlpha     28
#define CGCR_Color CGCR_ColorRed

// CWindowData definitions
#define CWD_HWND        40

#define CWD_WindowRectAlt		48
#define CWD_WindowRectLeft		CWD_WindowRect
#define CWD_WindowRectTop		CWD_WindowRect + 4
#define CWD_WindowRectRight		CWD_WindowRect + 8
#define CWD_WindowRectBottom	CWD_WindowRect + 12

#define CWD_BorderLeft      64
#define CWD_BorderRight     68
#define CWD_BorderTop       72
#define CWD_BorderBottom    76
#define CWD_BorderMargins   64

#define CWD_WinStyle    100
#define CWD_WinStyleEx  104
#define CWD_DPIFloat    304		// Actually contains the scaling amount of the client area.

#define CWD_AccentPolicy    152
#define CWD_WindowRect      180

// CText definitions
#define CTxt_Flags          80
#define CTxt_Flags2         280
#define CTxt_String         288
#define CTxt_Font           296

#define CTxt_TransformProxy 392
#define CTxt_LimRight       400
#define CTxt_LimBottom      404
#define CTxt_Color          408
#define CTxt_ColorBg        412

#define CTXT_FLAG_UPDATE         0x1000
#define CTXT_FLAG2_TEXTCHANGED        1
#define CTXT_FLAG2_RTL                2
// AWM Text Extension
#define CTxt_Ex			CTxt_LimRight
#define CTxt_ExFlag		CTxt_Color + 3 // +3 was picked because it's always 0.

// Extra definitions
#define CDM_Compositor          40
#define CDM_TextCache           264
#define CDM_ImagingFactory      312
#define CDM_CaptionFont         352
#define CDM_Win10BorderSize     480

#define CDM_DWMWindowThemeData		608
#define CDM_ThemeLibrary			616
#define CDM_DWMTouchThemeData       624
#define CDM_DWMPenThemeData         632
#define CDM_AnimationsThemeData     640
#define CDM_TimingFunctionThemeData	648

#define CTxtC_HDC	104

// ---------------------------------------------------------------------------

// Build-dependent definitions
#if TARGETBUILD == 19041
#define wState1     240
#define wState2     592

#define CTLW_ClientArea     480
#define CTLW_ClientVisual   544
#define clientInsetLeft     596
#define clientInsetRight    600
#define clientInsetTop      604
#define clientInsetBottom   608

#define borderInsetLeft     612
#define borderInsetRight    616
#define borderInsetTop      620
#define borderInsetBottom   624

#define CTLW_BorderOutsetLeft    628
#define CTLW_BorderOutsetRight   632
#define CTLW_BorderOutsetTop     636
#define CTLW_BorderOutsetBottom  640

#define CTLW_BorderOutsetLeftMax    644
#define CTLW_BorderOutsetRightMax   648
#define CTLW_BorderOutsetTopMax     652
#define CTLW_BorderOutsetBottomMax  656

#define CTLW_ClientBlur     296
#define CTLW_WindowVisual   728

#define CTLW_BorderSizesMax     644 // DEPRECATED
#define CTLW_BorderSizesNormal  628 // DEPRECATED
#define CTLW_ButtonVisuals      488
#define CTLW_TextVisual         520
#define CTLW_IconVisual         528

#define CTLW_GlassColorBorder   576     // these are CGlassColorizationResources objects
#define CTLW_GlassColorTB       584

#define CTLW_BorderColor        692
#define CTLW_TitlebarColor      708

#define CTLW_BorderBrushProxy       752
#define CTLW_TitlebarBrushProxy     760
#elif TARGETBUILD == 17763 || TARGETBUILD == 18362
#define wState1     240
#define wState2     584

#define CTLW_ClientArea     472
#define CTLW_ClientVisual   536
#define clientInsetLeft     588
#define clientInsetRight    592
#define clientInsetTop      596
#define clientInsetBottom   600

#define borderInsetLeft     604
#define borderInsetRight    608
#define borderInsetTop      612
#define borderInsetBottom   616

#define CTLW_BorderOutsetLeft    620
#define CTLW_BorderOutsetRight   624
#define CTLW_BorderOutsetTop     628
#define CTLW_BorderOutsetBottom  632

#define CTLW_BorderOutsetLeftMax    636
#define CTLW_BorderOutsetRightMax   640
#define CTLW_BorderOutsetTopMax     644
#define CTLW_BorderOutsetBottomMax  648

#define CTLW_ClientBlur     288
#define CTLW_WindowVisual   720

#define CTLW_BorderSizesMax     636
#define CTLW_BorderSizesNormal  620
#define CTLW_ButtonVisuals      480
#define CTLW_TextVisual         512
#define CTLW_IconVisual         520

#define CTLW_GlassColorBorder   568     // these are CGlassColorizationResources objects
#define CTLW_GlassColorTB       576

#define CTLW_BorderColor        684
#define CTLW_TitlebarColor      700

#define CTLW_BorderBrushProxy       744
#define CTLW_TitlebarBrushProxy     752
#endif

// 1809-exclusive definitions
#if TARGETBUILD == 17763
#define CTxt_Resource		16
#define CResource_Channel	16
#endif

#define CTLW_BorderMargins          borderInsetLeft
#define CTLW_BorderMarginsOuter     CTLW_BorderOutsetLeft
#define CTLW_BorderMarginsOuterMax  CTLW_BorderOutsetLeftMax
#define CTLW_ClientMargins          clientInsetLeft
#define CTLW_WindowData             CTLW_WindowVisual
