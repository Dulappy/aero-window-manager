#pragma once

#include <windows.h>
#include <wincodec.h>
#include <vssym32.h>
#include <Uxtheme.h>

#define MIOS_TMT_GLYPHDIBDATA96 3
#define MIOS_TMT_GLYPHDIBDATA120 4
#define MIOS_TMT_GLYPHDIBDATA144 5
#define MIOS_TMT_GLYPHDIBDATA192 6
#define MIOS_TMT_GLYPHDIBDATA240 7
#define MIOS_TMT_GLYPHDIBDATA288 8
#define MIOS_TMT_GLYPHDIBDATA384 9

HRESULT GetMiosThemeAtlas(
    HTHEME hTheme, 
    HINSTANCE hInstance, 
    IWICImagingFactory* pImagingFactory,
    IWICBitmapSource** ppBitmapSource
) 
{
    HRESULT hr = 0;

    void* themestream = NULL;
    DWORD streamlen;
    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* decodedframe = NULL;
    IStream* stream = NULL;

    hr = GetThemeStream(hTheme, 0, 0, TMT_DISKSTREAM, &themestream, &streamlen, hInstance);
    if (hr < 0)
        goto release;
    hr = CreateStreamOnHGlobal(NULL, true, &stream);
    if (hr < 0)
        goto release;

    ULARGE_INTEGER largeint;
    largeint.QuadPart = streamlen;
    hr = stream->SetSize(largeint);
    if (hr < 0)
        goto release;
    hr = stream->Write(themestream, streamlen, NULL);
    if (hr < 0)
        goto release;

    hr = pImagingFactory->CreateDecoderFromStream(stream, NULL, WICDecodeMetadataCacheOnLoad, &decoder);
    if (hr < 0)
        goto release;
    hr = decoder->GetFrame(0, &decodedframe);
    if (hr < 0)
        goto release;

    hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, decodedframe, ppBitmapSource);

release:
    if (decoder)
        decoder->Release();
    if (decodedframe)
        decodedframe->Release();
    if (stream)
        stream->Release();

    return hr;
}

HRESULT GetMiosThemeBitmapProps(
    HTHEME hTheme,
    int iPartId,
    int iStateId,
    RECT* rc,
    MARGINS* sizingmargins,
    MARGINS* contentmargins,
    int img,   // Set to -1 to ignore this
    bool isAtlas
)
{
    HRESULT hr = 0;
    int imagecount = 0;
    if (rc && isAtlas) {
        hr = GetThemeRect(hTheme, iPartId, iStateId, TMT_ATLASRECT, rc);
    }
    if (sizingmargins) {
        hr = GetThemeMargins(hTheme, NULL, iPartId, iStateId, TMT_SIZINGMARGINS, NULL, sizingmargins);
    }
    if (contentmargins) {
        hr = GetThemeMargins(hTheme, NULL, iPartId, iStateId, TMT_CONTENTMARGINS, NULL, contentmargins);
    }
    hr = GetThemeInt(hTheme, iPartId, iStateId, TMT_IMAGECOUNT, &imagecount);
    if (imagecount > 0 && img < imagecount && img >= 0) {
        int imagelayout;
        SIZE size = { rc->right - rc->left, rc->bottom - rc->top };
        hr = GetThemeEnumValue(hTheme, iPartId, iStateId, TMT_IMAGELAYOUT, &imagelayout);
        if (imagelayout == IL_HORIZONTAL && !isAtlas) {
            rc->right = rc->left + size.cx / imagecount;
            OffsetRect(rc, img * size.cx / imagecount, 0);
        }
        else {
            rc->bottom = rc->top + size.cy / imagecount;
            OffsetRect(rc, 0, img * size.cy / imagecount);
        }
    }

    if (isAtlas)
        InflateRect(rc, -1, -1);

    return hr;
}

HRESULT GetMiosThemeWICBitmap(
    HTHEME hTheme,
    int iPartId,
    int iStateId,
    int iPropId,
    IWICImagingFactory* pImagingFactory,
    IWICBitmap** ppIBitmap
) 
{
    HRESULT hr = 0;
    HBITMAP hbmp = NULL;
    hr = GetThemeBitmap(
        hTheme,
        iPartId,
        iStateId,
        iPropId,
        GBF_DIRECT,
        &hbmp
    );
    if (hr >= 0) {
        pImagingFactory->CreateBitmapFromHBITMAP(hbmp, NULL, WICBitmapUsePremultipliedAlpha, ppIBitmap);
    }

    return hr;
}