#include "awmdll.h"
#include "globals.h"
#include "toplevelwindow.h"
#include "windowdata.h"

#include <stdio.h>
#include <math.h>
#include <float.h>

MARGINS CTopLevelWindow::s_marMinInflationThickness = { 0 };
int CTopLevelWindow::s_iDpiIndex = 0;

void CTopLevelWindow::UpdateWindowRegion() {
    CWindowData* pData = m_pWindowData;

    if (!pData->m_pHwnd)
        return;

    RECT localRect = pData->m_rcWindowBounds;

    const MARGINS pMargins = (m_bWindowStateFlags & kIsMaximized)
        ? m_marBorderOutsetMax
        : m_marBorderOutset;

    float scaleX = pData->m_fScaleX;
    if (fabsf(scaleX) >= FLT_EPSILON)
    {
        if (pMargins.cxLeftWidth < 0)
            localRect.left += (int)floor((double)pMargins.cxLeftWidth / scaleX + 0.5);
        if (pMargins.cxRightWidth < 0)
            localRect.right -= (int)floor((double)pMargins.cxRightWidth / scaleX + 0.5);
    }

    float scaleY = pData->m_fScaleY;
    if (fabsf(scaleY) >= FLT_EPSILON)
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

    float scaleX = pData->m_fScaleX;
    float scaleY = pData->m_fScaleY;

    bool bIsIdentity = (fabsf(scaleX - 1.0f) < FLT_EPSILON) &&
        (fabsf(scaleY - 1.0f) < FLT_EPSILON);

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

void CTopLevelWindow::UpdateWindowScale() {
    CVisual* pFrame = this->m_pFrameVisual;
    CWindowData* pData = this->m_pWindowData;

    float fScaleX = pData->m_fScaleX_copy;
    float fScaleY = pData->m_fScaleY_copy;

    if (pFrame != nullptr) {

        pFrame->SetScale((double)fScaleX, (double)fScaleY);

        // Determine interpolation mode based on whether scale = identity
        float diffX = fabsf(pData->m_fScaleX_copy - 1.0f);
        if (diffX >= FLT_EPSILON)
        {
            pFrame->ClearInterpolationMode();
            goto check_client_blur;
        }

        float diffY = fabsf(pData->m_fScaleY_copy - 1.0f);
        if (diffY >= FLT_EPSILON)
        {
            pFrame->ClearInterpolationMode();
            goto check_client_blur;
        }

        // Both scales are identity, use nearest-neighbor (no filtering needed)
        pFrame->SetInterpolationMode(MilBitmapInterpolationMode_NearestNeighbor);
    }

check_client_blur:
    CVisual* pNCBG = this->m_pClientBlurVisual;
    if (pNCBG != nullptr)
        pNCBG->SetScale((double)fScaleX, (double)fScaleY);
}

void CTopLevelWindow::CalculateOutsideMargins(CWindowData* pData, UINT dwFlags, _MARGINS* pOutMargins) {
    // ---- Always zero the output first ----------------------------------------
    memset(pOutMargins, 0, sizeof(_MARGINS));   // 16 bytes
    
    // ---- Gate 1: caller must request this via flags bits 1 or 2 -------------
    if ((dwFlags & 0x06) == 0)
        return;

    // ---- Gates 2-5: requested inflation must meet the system minimum --------
    if (pData->m_marWindowFrame.cxLeftWidth < s_marMinInflationThickness.cxLeftWidth ||
        pData->m_marWindowFrame.cyTopHeight < s_marMinInflationThickness.cyTopHeight ||
        pData->m_marWindowFrame.cxRightWidth < s_marMinInflationThickness.cxRightWidth ||
        pData->m_marWindowFrame.cyBottomHeight < s_marMinInflationThickness.cyBottomHeight)
        return;

    // ---- Gate 6: undocumented ex-style bit 23 (0x00800000) must be clear -----
    // No public WS_EX_* constant maps to this bit; it is a reserved/internal
    // extended-style flag used by the window manager.
    if (pData->m_dwExStyle & 0x00800000)
        return;

    // ---- Gate 7: system must add padded-border thickness to frames -----------
    if (GetSystemMetrics(SM_CXPADDEDBORDER) <= 0)
        return;

    // ---- Compute Windows' natural frame thickness via AdjustWindowRectEx ----
    DWORD dwStyle = pData->m_dwStyle;
    DWORD dwExStyle = pData->m_dwExStyle;
    dwStyle |= WS_THICKFRAME;

    RECT rcAdjust = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&rcAdjust, dwStyle, FALSE, dwExStyle);

    INT naturalLeft = -rcAdjust.left;
    INT naturalTop = -rcAdjust.top;
    INT naturalRight = rcAdjust.right;
    INT naturalBottom = rcAdjust.bottom;

    INT reqLeft = pData->m_marWindowFrame.cxLeftWidth;
    pOutMargins->cxLeftWidth = reqLeft - max(reqLeft, naturalLeft);

    INT reqTop = pData->m_marWindowFrame.cyTopHeight;
    pOutMargins->cyTopHeight = reqTop - max(reqTop, naturalTop);

    INT reqRight = pData->m_marWindowFrame.cxRightWidth;
    pOutMargins->cxRightWidth = reqRight - max(reqRight, naturalRight);

    INT reqBottom = pData->m_marWindowFrame.cyBottomHeight;
    pOutMargins->cyBottomHeight = reqBottom - max(reqBottom, naturalBottom);
}

HRESULT CTopLevelWindow::ReadSystemMetrics()
{
    // ---- Ensure window-frame state is ready before querying metrics ----------
    HRESULT hr = EnsureWindowFrames();
    if (FAILED(hr))
    {
        MilInstrumentationCheckHR(5, nullptr, 0, 0x146 /* line 326 */, hr);
        return hr;
    }

    INT cxSizeFrame = GetSystemMetrics(SM_CXSIZEFRAME);
    INT cySizeFrame = GetSystemMetrics(SM_CYSIZEFRAME);

    s_iDpiIndex = -1;

    // ---- Populate s_marMinInflationThickness -----------------------------------
    // Horizontal thickness (cxSizeFrame) is applied to BOTH left and top;
    // vertical thickness (cySizeFrame) is applied to BOTH right and bottom.
    s_marMinInflationThickness.cxLeftWidth = cxSizeFrame;
    s_marMinInflationThickness.cyTopHeight = cxSizeFrame;
    s_marMinInflationThickness.cxRightWidth = cySizeFrame;
    s_marMinInflationThickness.cyBottomHeight = cySizeFrame;

    return hr;
}

HRESULT CTopLevelWindow::EnsureWindowFrames()
{
    /*HRESULT hr = S_OK;

    // ---- Fast path: already initialised --------------------------------------
    // If the frame array already has at least one element, there is nothing to
    // do.  s_rgpwfWindowFrames.m_cCount is the gate.  (The compiler reads it
    // via a direct global load at 0x7FF72FE9138, which is &s_rgpwfWindowFrames
    // + 0x18.)
    if (s_rgpwfWindowFrames.m_cCount != 0)
        return S_OK;

    // ---- Build the eight per-DPI window frames -------------------------------
    // Loop runs exactly 8 times (edi = 0..7).  Each iteration allocates one
    // WindowFrame, constructs it, and appends it to s_rgpwfWindowFrames.
    for (UINT i = 0; i < 8; i++)
    {
        void* pRaw = AWM::HeapImpl::Alloc(0x4F0);

        WindowFrame* pFrame;
        if (pRaw == nullptr)
        {
            // Allocation failed: report E_OUTOFMEMORY and abort the whole
            // function (this is a hard failure, unlike the per-append error
            // path below which continues the loop).
            pFrame = nullptr;
            hr = E_OUTOFMEMORY;
            MilInstrumentationCheckHR(5, nullptr, 0, 0x177 /*line 375, hr);
            return hr;
        }

        // Placement-construct the WindowFrame in the raw block.
        pFrame = new (pRaw) WindowFrame();   // call WindowFrame::WindowFrame()

        // ---- Append pFrame to s_rgpwfWindowFrames ----------------------------
        // Inlined DynArray fast-append: if there is spare capacity, write
        // directly; otherwise fall back to AddMultipleAndSet to grow.
        UINT  cCount = s_rgpwfWindowFrames.m_cCount;       // [0x...9138]
        UINT  cNewCount = cCount + 1;

        // Overflow guard: cNewCount must not wrap below cCount.
        if (cNewCount < cCount)
        {
            // Arithmetic overflow on the index (count + 1 wrapped to 0).
            hr = (HRESULT)0x80070216;   // HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW)
            MilInstrumentationCheckHR(5, nullptr, 0, 0xB0 /*line 176, hr);
            // NOTE: this path does NOT return; it falls through to the loop
            // increment and continues to the next iteration (jmp loc_7FF72FB7857).
        }
        else if (cNewCount > s_rgpwfWindowFrames.m_cCapacity)   // [0x...9134]
        {
            // Not enough capacity: grow via AddMultipleAndSet.
            //   rcx = this = &s_rgpwfWindowFrames
            //   rdx = 8        (first param -- target/hint element count)
            //   r8d = 1        (second param = 8 - 7, elements to add)
            //   r9  = &pFrame  (source pointer to copy the one element from)
            HRESULT hrAdd = s_rgpwfWindowFrames.AddMultipleAndSet(
                8,            // a   (rdx)
                1,            // b   (r8 = 8 - 7)
                &pFrame);     // pSrc (r9)
            if (hrAdd < 0)
            {
                // Grow failed: report its HRESULT and continue the loop.
                hr = hrAdd;
                MilInstrumentationCheckHR(5, nullptr, 0, 0xBB /*line 187, hr);
            }
            // success -> fall through to loop increment
        }
        else
        {
            // Fast path: capacity available, write directly into the array.
            //   s_rgpwfWindowFrames.m_pData[cCount] = pFrame;
            //   s_rgpwfWindowFrames.m_cCount       = cNewCount;
            s_rgpwfWindowFrames.m_pData[cCount] = pFrame;   // [base + cCount*8]
            s_rgpwfWindowFrames.m_cCount = cNewCount;
        }
        // loop: inc edi; cmp edi, 8; jb ...
    }

    return hr;*/
}
