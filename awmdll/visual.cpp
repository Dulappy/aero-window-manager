#include "visual.h"

#include <math.h>
#include <float.h>

void CVisual::PropagateDirtyChildren() {
    CVisual* pAncestor = this->m_pParent;

    while (pAncestor != nullptr)
    {
        if (pAncestor->m_dwDirtyFlags & kDirtyChildPropagated)
            return;

        pAncestor->m_dwDirtyFlags |= kDirtyChildPropagated;
        pAncestor = pAncestor->m_pParent;
    }
}

void CVisual::SetDirtyFlags(ULONG flags) {
    DWORD current = this->m_dwDirtyFlags;

    if ((current & flags) == flags)
        return;

    this->m_dwDirtyFlags = current | flags;
    this->PropagateDirtyChildren();
}

void CVisual::SetScale(double scaleX, double scaleY) {
    float fScaleX;
    float baseX;
    float currentX;
    float relDiffX;

    float fScaleY;
    float baseY;
    float currentY;
    float relDiffY;

    fScaleX = (float)scaleX;
    baseX = (fScaleX == 0.0f) ? 1.0f : fScaleX;

    currentX = (float)this->m_dScaleX;
    relDiffX = fabsf((currentX - fScaleX) / baseX);
    if (relDiffX >= FLT_EPSILON)
        goto do_update;

    fScaleY = (float)scaleY;
    baseY = (fScaleY == 0.0f) ? 1.0f : fScaleY;

    currentY = (float)this->m_dScaleY;
    relDiffY = fabsf((currentY - fScaleY) / baseY);
    if (relDiffY >= FLT_EPSILON)
        goto do_update;

    return;

do_update:
    this->m_dScaleX = scaleX;
    this->m_dScaleY = scaleY;
    this->SetDirtyFlags(kDirtyScale);
}

void CVisual::SetInterpolationMode(MilBitmapInterpolationMode mode) {
    if ((this->m_dwVisualFlags & kDirtyScale) &&
        this->m_eInterpolationMode == (DWORD)mode)
        return;

    this->m_dwVisualFlags |= kDirtyScale;
    this->m_eInterpolationMode = (DWORD)mode;
    this->SetDirtyFlags(kDirtyInterpolation);
}

void CVisual::ClearInterpolationMode() {
    if (!(this->m_dwVisualFlags & kDirtyScale)) return;
    this->m_dwVisualFlags &= ~kDirtyScale;
    this->SetDirtyFlags(kDirtyInterpolation);
}

void CVisual::SetOpacity(double opacity) {
    float f = (float)opacity;
    if (f == 0.0f) { opacity = 1.0; f = 1.0f; }

    float currentF = (float)this->m_dOpacity;
    if (fabsf((currentF - f) / f) < FLT_EPSILON)
        return;

    this->m_dOpacity = opacity;
    this->SetDirtyFlags(kDirtyOpacity);
}

void CVisual::SetOffset(const POINT* pOffset) {
    if (this->m_ptOffset.x == pOffset->x &&
        this->m_ptOffset.y == pOffset->y)
        return;

    this->m_ptOffset.x = pOffset->x;
    this->m_ptOffset.y = pOffset->y;
    this->SetDirtyFlags(kDirtyPosition);
}

void CVisual::SetInsetFromParentLeft(INT left) {
    if (this->m_rcInsets.cxLeftWidth == left) return;
    this->m_rcInsets.cxLeftWidth = left;
    this->SetDirtyFlags(kDirtyGeometry);
}

void CVisual::SetInsetFromParentRight(INT right) {
    if (this->m_rcInsets.cxRightWidth == right) return;
    this->m_rcInsets.cxRightWidth = right;
    this->SetDirtyFlags(kDirtyGeometry);
}

void CVisual::SetInsetFromParentTop(INT top) {
    if (this->m_rcInsets.cyTopHeight == top) return;
    this->m_rcInsets.cyTopHeight = top;
    this->SetDirtyFlags(kDirtyGeometry);
}

void CVisual::SetInsetFromParentBottom(INT bottom) {
    if (this->m_rcInsets.cyBottomHeight == bottom) return;
    this->m_rcInsets.cyBottomHeight = bottom;
    this->SetDirtyFlags(kDirtyGeometry);
}

void CVisual::SetInsetFromParent(const _MARGINS& margins) {
    if (this->m_rcInsets.cyTopHeight == margins.cyTopHeight &&
        this->m_rcInsets.cxLeftWidth == margins.cxLeftWidth &&
        this->m_rcInsets.cxRightWidth == margins.cxRightWidth &&
        this->m_rcInsets.cyBottomHeight == margins.cyBottomHeight)
        return;

    this->m_rcInsets.cyTopHeight = margins.cyTopHeight;
    this->m_rcInsets.cxLeftWidth = margins.cxLeftWidth;
    this->m_rcInsets.cxRightWidth = margins.cxRightWidth;
    this->m_rcInsets.cyBottomHeight = margins.cyBottomHeight;
    this->SetDirtyFlags(kDirtyGeometry);
}
