#pragma once
#include "awmdll.h"
#include "globals.h"
#include "baseobject.h"

#include <Windows.h>
#include <Uxtheme.h>

class CVisual : public CBaseObject {
public:
    // CBaseObject occupies [+0x00..+0x17]:
    //   [+0x00]  void* vtable          - overridden by CVisual's (then CTopLevelWindow's)
    //   [+0x08..+0x17]   CBaseObject fields (ref count etc.; exact layout not reversed)
    BYTE         _pad_cbase[0x10];         // CBaseObject fields after vtable [+0x08..+0x17]

    CVisual* m_pParent;                // +0x18  parent in composition tree;
    //        NULL at construction, set by
    //        VisualCollection::InsertRelative when
    //        this visual is added as a child.
    //        PropagateDirtyChildren walks this chain.

// VisualCollection m_children embedded at [+0x20..+0x4F] (~48 bytes).
// Internal layout not fully reversed; known sub-fields:
//   [+0x20]  void* vtable           - VisualCollection vtable
//   [+0x28]  CVisual* m_pOwner        - back-ptr to the containing CVisual (= this)
//   [+0x30..+0x4F]   doubly-linked child list state (head, tail, count, flags)
    BYTE         _pad_children[0x30];      // VisualCollection internal [+0x20..+0x4F]

    DWORD        m_dwDirtyFlags;           // +0x50  composition dirty-flag bitfield
    BYTE         m_bFlags;                 // +0x54  bit 0: size change also marks scale dirty
    //        (set externally; not written in any reversed fn)
    BYTE         _pad1[0x13];              // +0x55..+0x67  (zero-init)

    POINT        m_ptOffset;               // +0x68  position relative to parent visual {x, y}

    SIZE         m_szSize;                 // +0x70  visual size in pixels {cx, cy}

    _MARGINS     m_rcInsets;               // +0x78  inset from each parent edge
    //        Initialised to INT_MAX in CVisual ctor.

    BYTE         _pad2[0x08];              // +0x88..+0x8F  (zero-init)

    double       m_dScaleX;               // +0x90  horizontal scale factor (init 1.0)
    double       m_dScaleY;               // +0x98  vertical scale factor   (init 1.0)
    double       m_dOpacity;              // +0xA0  opacity 0.0 .. 1.0      (init 1.0)
    //        SetOpacity snaps 0.0 -> 1.0 (zero opacity
    //        is illegal; 0.0 input uses 1.0f fallback)

    DWORD        m_dwReserved_A8;          // +0xA8  initialised to -2 in ctor;
    //        purpose not reversed (frame cookie?)

    DWORD        m_dwVisualFlags;          // +0xAC  visual render-state flags:
    //           bit 4 (0x10): custom interpolation mode set

    DWORD        m_eInterpolationMode;     // +0xB0  MilBitmapInterpolationMode enum value;
    //        only valid when m_dwVisualFlags bit 4 is set

    BYTE         _pad3[0x2C];              // +0xB4..+0xDF  (not reversed)

    BYTE         m_bWindowStateFlags;      // +0xE0  window state flags (see CTopLevelWindow analysis)
    //        Placed here within CVisual's footprint;
    //        managed exclusively by CTopLevelWindow.

    BYTE         _pad4[0x0F];              // +0xE1..+0xEF  (not reversed)
    // sizeof(CVisual) ~ 0xF0 bytes (end of CVisual subobject in CTopLevelWindow)

    enum DirtyFlags : uint8_t {
        kDirtyNone = 0x00000000,

        // Bit 0: set by PropagateDirtyChildren on ancestor nodes to mark 
        // that at least one descendant is dirty
        kDirtyChildPropagated = 0x00000001,

        // Bit 1: set by SetSize, all SetInset* calls
        kDirtyGeometry = 0x00000002,

        // Bit 3: set by SetOffset
        kDirtyPosition = 0x00000008,

        // Bit 4: set by SetScale; also set by SetSize when m_bFlags bit 0 is true
        kDirtyScale = 0x00000010,

        // Bit 5: set by SetOpacity
        kDirtyOpacity = 0x00000020,

        // Bit 6: set by SetInterpolationMode / ClearInterpolationMode
        kDirtyInterpolation = 0x00000040
    };

    // -- Virtual methods -------------------------------------------------------
    virtual HRESULT ValidateVisual();
    virtual HRESULT SetSize(const SIZE* pSize);
    // (further virtual slots not yet fully enumerated)

    // -- Non-virtual methods ---------------------------------------------------
    void    PropagateDirtyChildren();
    void    SetDirtyFlags(ULONG flags);
    void    SetScale(double scaleX, double scaleY);
    void    SetOpacity(double opacity);
    void    SetOffset(const POINT* pOffset);
    void    SetInsetFromParentLeft(INT left);
    void    SetInsetFromParentRight(INT right);
    void    SetInsetFromParentTop(INT top);
    void    SetInsetFromParentBottom(INT bottom);
    void    SetInsetFromParent(const _MARGINS& margins);
    void    SetInterpolationMode(MilBitmapInterpolationMode mode);
    void    ClearInterpolationMode();
    HRESULT AddChild(CVisual* pChild, bool bInsertAbove);
    HRESULT RemoveChild(CVisual* pChild);
    HRESULT Initialize(MIL_CHANNEL* pChannel);
};

static_assert(offsetof(CVisual, m_pParent) == 0x18, "ERROR: m_pParent not in the correct position!");
static_assert(offsetof(CVisual, m_dwDirtyFlags) == 0x50, "ERROR: m_dwDirtyFlags not in the correct position!");
static_assert(offsetof(CVisual, m_ptOffset) == 0x68, "ERROR: m_ptOffset not in the correct position!");
static_assert(offsetof(CVisual, m_szSize) == 0x70, "ERROR: m_szSize not in the correct position!");
static_assert(offsetof(CVisual, m_rcInsets) == 0x78, "ERROR: m_rcInsets not in the correct position!");
static_assert(offsetof(CVisual, m_dScaleX) == 0x90, "ERROR: m_dScaleX not in the correct position!");
static_assert(offsetof(CVisual, m_dScaleY) == 0x98, "ERROR: m_dScaleY not in the correct position!");
static_assert(offsetof(CVisual, m_dOpacity) == 0xA0, "ERROR: m_dOpacity not in the correct position!");
static_assert(offsetof(CVisual, m_dwVisualFlags) == 0xAC, "ERROR: m_dwVisualFlags not in the correct position!");
static_assert(offsetof(CVisual, m_eInterpolationMode) == 0xB0, "ERROR: m_eInterpolationMode not in the correct position!");
static_assert(offsetof(CVisual, m_bWindowStateFlags) == 0xE0, "ERROR: m_bWindowStateFlags not in the correct position!");