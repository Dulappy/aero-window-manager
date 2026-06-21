#pragma once

#include <d2d1.h>
#include <math.h>
#include <Uxtheme.h>

typedef enum _MIOS_D2D1_3DOBJECT_HIGHLIGHT {
    MIOS_D2D1_3DOBJECT_HIGHLIGHT_NONE = 0,
    MIOS_D2D1_3DOBJECT_HIGHLIGHT_BOX = 1

} MIOS_D2D1_3DOBJECT_HIGHLIGHT;

typedef enum _MIOS_D2D1_3DOBJECT_STYLE {
    MIOS_D2D1_3DOBJECT_STYLE_NORMAL = 0,
    MIOS_D2D1_3DOBJECT_STYLE_PRESSED = 1,
    MIOS_D2D1_3DOBJECT_STYLE_INDENTED = 2,
    MIOS_D2D1_3DOBJECT_STYLE_NORMAL_ALT = 3

} MIOS_D2D1_3DOBJECT_STYLE;

class IMiosD2D1RenderTarget : public ID2D1RenderTarget {
public:
    void DrawNineSliceBitmap(
        ID2D1Bitmap* bitmap,
        const D2D1_RECT_F* destinationRectangle,
        FLOAT opacity,
        D2D1_BITMAP_INTERPOLATION_MODE interpolationMode,
        const D2D1_RECT_F* sourceRectangle,
        const MARGINS* margins
    );
    HRESULT Draw3DObject(
        D2D1_RECT_F* rc,                    // [in] Full 3D object rectangle, [out] Face rectangle
        MIOS_D2D1_3DOBJECT_HIGHLIGHT mode,  // [in] Highlight mode
        MIOS_D2D1_3DOBJECT_STYLE style      // [in] 3D object style
    );
};

void IMiosD2D1RenderTarget::DrawNineSliceBitmap(
    ID2D1Bitmap* bitmap,
    const D2D1_RECT_F* destinationRectangle,
    FLOAT opacity,
    D2D1_BITMAP_INTERPOLATION_MODE interpolationMode,
    const D2D1_RECT_F* sourceRectangle,
    const MARGINS* margins
)
{
    // top-left
    D2D1_RECT_F centerrc = *destinationRectangle;
    centerrc.left += margins->cxLeftWidth;
    centerrc.right -= margins->cxRightWidth;
    centerrc.top += margins->cyTopHeight;
    centerrc.bottom -= margins->cyBottomHeight;
    D2D1_RECT_F drawrc;
    drawrc.left = (float)floor(destinationRectangle->left);
    drawrc.top = (float)floor(destinationRectangle->top);
    drawrc.right = (float)floor(centerrc.left);
    drawrc.bottom = (float)floor(centerrc.top);
    D2D1_RECT_F pullrc = {
        sourceRectangle->left,
        sourceRectangle->top,
        sourceRectangle->left + (float)margins->cxLeftWidth,
        sourceRectangle->top + (float)margins->cyTopHeight
    };
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // top-center
    drawrc.left = (float)floor(centerrc.left);
    drawrc.right = (float)floor(centerrc.right);
    pullrc.left += (float)margins->cxLeftWidth;
    pullrc.right = (float)(sourceRectangle->right - margins->cxRightWidth);
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // top-right
    drawrc.left = (float)floor(centerrc.right);
    drawrc.right = (float)floor(centerrc.right) + (float)margins->cxRightWidth;
    pullrc.left = (float)(sourceRectangle->right - margins->cxRightWidth);
    pullrc.right = sourceRectangle->right;
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // center-left
    drawrc.left = (float)floor(centerrc.left) - (float)margins->cxLeftWidth;
    drawrc.right = (float)floor(centerrc.left);
    drawrc.top = (float)floor(centerrc.top);
    drawrc.bottom = (float)floor(centerrc.bottom);
    pullrc.left = sourceRectangle->left;
    pullrc.right = sourceRectangle->left + (float)margins->cxLeftWidth;
    pullrc.top = sourceRectangle->top + (float)margins->cyTopHeight;
    pullrc.bottom = (float)(sourceRectangle->bottom - margins->cyBottomHeight);
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // center
    drawrc.left = (float)floor(centerrc.left);
    drawrc.right = (float)floor(centerrc.right);
    pullrc.left += (float)margins->cxLeftWidth;
    pullrc.right = (float)(sourceRectangle->right - margins->cxRightWidth);
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // center-right
    drawrc.left = (float)floor(centerrc.right);
    drawrc.right = (float)floor(centerrc.right) + (float)margins->cxRightWidth;
    pullrc.left = (float)(sourceRectangle->right - margins->cxRightWidth);
    pullrc.right = sourceRectangle->right;
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // bottom-left
    drawrc.left = (float)floor(centerrc.left) - margins->cxLeftWidth;
    drawrc.top = (float)floor(centerrc.bottom);
    drawrc.right = (float)floor(centerrc.left);
    drawrc.bottom = (float)floor(centerrc.bottom) + margins->cyBottomHeight;
    pullrc.left = sourceRectangle->left;
    pullrc.top = (float)(sourceRectangle->bottom - margins->cyBottomHeight);
    pullrc.right = sourceRectangle->left + (float)margins->cxLeftWidth;
    pullrc.bottom = sourceRectangle->bottom;
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // bottom-center
    drawrc.left = (float)floor(centerrc.left);
    drawrc.right = (float)floor(centerrc.right);
    pullrc.left += (float)margins->cxLeftWidth;
    pullrc.right = (float)(sourceRectangle->right - margins->cxRightWidth);
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

    // bottom-right
    drawrc.left = (float)floor(centerrc.right);
    drawrc.right = (float)floor(centerrc.right) + (float)margins->cxRightWidth;
    pullrc.left = (float)(sourceRectangle->right - margins->cxRightWidth);
    pullrc.right = sourceRectangle->right;
    this->DrawBitmap(bitmap, &drawrc, opacity, interpolationMode, &pullrc);

}

HRESULT IMiosD2D1RenderTarget::Draw3DObject(D2D1_RECT_F* rc, MIOS_D2D1_3DOBJECT_HIGHLIGHT mode, MIOS_D2D1_3DOBJECT_STYLE style) {
    HRESULT hr = 0;
    ID2D1SolidColorBrush* background{};
    ID2D1SolidColorBrush* brushBRO{};
    ID2D1SolidColorBrush* brushTLO{};
    ID2D1SolidColorBrush* brushBRI{};
    ID2D1SolidColorBrush* brushTLI{};
    ID2D1SolidColorBrush* face{};
    COLORREF ref;
    D2D1_COLOR_F clrbkgd;
    D2D1_COLOR_F clrBRO;
    D2D1_COLOR_F clrTLO;
    D2D1_COLOR_F clrBRI;
    D2D1_COLOR_F clrTLI;
    D2D1_COLOR_F clrface;

    switch (style) {
    case MIOS_D2D1_3DOBJECT_STYLE_NORMAL:
        ref = GetThemeSysColor(NULL, COLOR_3DDKSHADOW);
        clrBRO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DHILIGHT);
        clrTLO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrBRI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DLIGHT);
        clrTLI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        break;

    case MIOS_D2D1_3DOBJECT_STYLE_PRESSED:
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrBRO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrTLO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DFACE);
        clrBRI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DFACE);
        clrTLI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        break;

    case MIOS_D2D1_3DOBJECT_STYLE_INDENTED:
        ref = GetThemeSysColor(NULL, COLOR_3DHILIGHT);
        clrBRO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DDKSHADOW);
        clrTLO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DLIGHT);
        clrBRI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrTLI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        break;

    case MIOS_D2D1_3DOBJECT_STYLE_NORMAL_ALT:
        ref = GetThemeSysColor(NULL, COLOR_3DDKSHADOW);
        clrBRO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DLIGHT);
        clrTLO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrBRI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DHILIGHT);
        clrTLI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        break;

    default:
        ref = GetThemeSysColor(NULL, COLOR_3DDKSHADOW);
        clrBRO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DHILIGHT);
        clrTLO = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DSHADOW);
        clrBRI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        ref = GetThemeSysColor(NULL, COLOR_3DLIGHT);
        clrTLI = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
    }

    if (mode == MIOS_D2D1_3DOBJECT_HIGHLIGHT_BOX) {
        ref = GetThemeSysColor(NULL, COLOR_WINDOWFRAME);
        clrbkgd = D2D1::ColorF(
            (float)GetRValue(ref) / 255.0f,
            (float)GetGValue(ref) / 255.0f,
            (float)GetBValue(ref) / 255.0f,
            1.0f
        );
        hr = this->CreateSolidColorBrush(clrbkgd, &background);
        if (hr < 0)
            goto release;
        this->FillRectangle(rc, background);
        rc->left += 1;
        rc->top += 1;
        rc->right -= 1;
        rc->bottom -= 1;
    
    }

    hr = this->CreateSolidColorBrush(clrBRO, &brushBRO);
    if (hr < 0)
        goto release;
    this->FillRectangle(rc, brushBRO);

    rc->right -= 1;
    rc->bottom -= 1;
    hr = this->CreateSolidColorBrush(clrTLO, &brushTLO);
    if (hr < 0)
        goto release;
    this->FillRectangle(rc, brushTLO);

    rc->left += 1;
    rc->top += 1;
    hr = this->CreateSolidColorBrush(clrBRI, &brushBRI);
    if (hr < 0)
        goto release;
    this->FillRectangle(rc, brushBRI);

    rc->right -= 1;
    rc->bottom -= 1;
    hr = this->CreateSolidColorBrush(clrTLI, &brushTLI);
    if (hr < 0)
        goto release;
    this->FillRectangle(rc, brushTLI);

    ref = GetThemeSysColor(NULL, COLOR_3DFACE);
    clrface = D2D1::ColorF(
        (float)GetRValue(ref) / 255.0f,
        (float)GetGValue(ref) / 255.0f,
        (float)GetBValue(ref) / 255.0f,
        1.0f
    );
    rc->left += 1;
    rc->top += 1;
    hr = this->CreateSolidColorBrush(clrface, &face);
    if (hr < 0)
        goto release;
    this->FillRectangle(rc, face);

release:
    if (background)
        background->Release();
    if (brushBRO)
        brushBRO->Release();
    if (brushTLO)
        brushTLO->Release();
    if (brushBRI)
        brushBRI->Release();
    if (brushTLI)
        brushTLI->Release();
    if (face)
        face->Release();
    return hr;
}