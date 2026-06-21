#include "awmdll.h"
#include "toplevelwindow.h"
#include "windowdata.h"

#include <stdio.h>
#include <math.h>

void CTopLevelWindow::UpdateWindowRegion() {
    CWindowData* pData = m_pWindowData;

    if (!pData->m_pHwnd)
        return;

    RECT localRect = pData->m_rcWindowBounds;

    const MARGINS pMargins = (m_bWindowStateFlags & WINDOWSTATE_MAXIMIZED)
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

    if ((m_bWindowStateFlags & 0x02) && bUnchanged)
        return;

    HRGN hRgn = bUnchanged
        ? nullptr
        : CreateRectRgnIndirect(&localRect);

    BOOL bSuccess = SetWindowRgnEx(pData->m_pHwnd, hRgn, TRUE);

    if (!bSuccess && hRgn)
        DeleteObject(hRgn);

    m_bWindowStateFlags &= ~0x02u;
    if (bUnchanged)
        m_bWindowStateFlags |= 0x02u;
}