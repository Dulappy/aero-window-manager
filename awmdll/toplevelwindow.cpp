#include "awmdll.h"
#include "globals.h"
#include "toplevelwindow.h"
#include "windowdata.h"

#include <stdio.h>
#include <math.h>

void CTopLevelWindow::UpdateWindowRegion() {
    CWindowData* pData = m_pWindowData;

    if (!pData->m_pHwnd)
        return;

    RECT localRect = pData->m_rcWindowBounds;

    const MARGINS pMargins = (m_bWindowStateFlags & kIsMaximized)
        ? m_marBorderOutsetMax
        : m_marBorderOutset;

    // Zero but with a margin of error.
    constexpr float kScaleEpsilon = 1.192e-6f;  // 0x35A00000 = 1.25 * 2^-20

    float scaleX = pData->m_fScaleX;
    if (fabsf(scaleX) >= kScaleEpsilon)
    {
        if (pMargins.cxLeftWidth < 0)
            localRect.left += (int)floor((double)pMargins.cxLeftWidth / scaleX + 0.5);
        if (pMargins.cxRightWidth < 0)
            localRect.right -= (int)floor((double)pMargins.cxRightWidth / scaleX + 0.5);
    }

    float scaleY = pData->m_fScaleY;
    if (fabsf(scaleY) >= kScaleEpsilon)
    {
        if (pMargins.cyTopHeight < 0)
            localRect.top += (int)floor((double)pMargins.cyTopHeight / scaleY + 0.5);
        if (pMargins.cyBottomHeight < 0)
            localRect.bottom -= (int)floor((double)pMargins.cyBottomHeight / scaleY + 0.5);
    }

    bool bUnchanged = EqualRect(&localRect, &pData->m_rcWindowBounds) != 0;

    if ((m_bWindowStateFlags & kRgnRectUnchanged) && bUnchanged)
        return;

    HRGN hRgn = bUnchanged
        ? nullptr
        : CreateRectRgnIndirect(&localRect);

    BOOL bSuccess = SetWindowRgnEx(pData->m_pHwnd, hRgn, TRUE);

    if (!bSuccess && hRgn)
        DeleteObject(hRgn);

    m_bWindowStateFlags &= ~kRgnRectUnchanged;
    if (bUnchanged)
        m_bWindowStateFlags |= kRgnRectUnchanged;
}

HRESULT CTopLevelWindow::UpdateInputTransform() {
    CWindowData* pData = this->m_pWindowData;

    if (pData->m_pHwnd == nullptr)
        return S_OK;

    constexpr float kOne = 1.0f;
    constexpr float kEpsilon = 1.192e-6f;

    float scaleX = pData->m_fScaleX;
    float scaleY = pData->m_fScaleY;

    bool bIsIdentity = (fabsf(scaleX - kOne) < kEpsilon) &&
        (fabsf(scaleY - kOne) < kEpsilon);

    bool bWasIdentity = (this->m_bWindowStateFlags & kHasDefaultScale) != 0;
    if (bWasIdentity && bIsIdentity)
        return S_OK;

    D2DMatrix localMatrix;
    D2DMatrixScaling(&localMatrix, scaleX, scaleY, 1.0f);

    // Kind of pointless (these are referenced nowhere else), but still here just for the sake of it.
    if (!(g_bUpdateWindowTransformOnce & 1))
    {
        g_hrExpected_EAccessDenied = 0x80070005;
        g_hrExpected_InvalidHandle = 0x80070578;
        g_bUpdateWindowTransformOnce |= 1;
        g_hrExpected_EInvalidArg = 0x80070057;
    }

    SetLastError(0); // clear any stale error before the call

    // NOTE: UpdateWindowTransform has been removed from Windows 10.
    BOOL   bSuccess = UpdateWindowTransform(pData->m_pHwnd, &localMatrix, TRUE);
    HRESULT hr = S_OK;

    if (bSuccess)
    {
        this->m_bWindowStateFlags &= ~kHasDefaultScale;                         // clear kHasDefaultScale
        this->m_bWindowStateFlags |= (bIsIdentity ? kHasDefaultScale : kEmpty); // write new state
    }
    else
    {
        DWORD dwErr = GetLastError();

        if (dwErr == 0)
        {
            hr = 0x88980003; // DWM-internal error code
        }
        else
        {
            hr = HRESULT_FROM_WIN32(dwErr & 0xFFFF);
            if (hr >= 0)
                hr = 0x88980003; // if HRESULT_FROM_WIN32 was positive, override
        }

        MilInstrumentationCheckHR(5, nullptr, 0, 0x1403 /*line 5123*/, hr); // Who the fuck cares?
    }

    if (hr == E_INVALIDWINDOWHANDLE || hr == E_INVALIDARG || hr == E_ACCESSDENIED) {
        return S_OK;
    }

    return hr;
}
