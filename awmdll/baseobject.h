#pragma once

#include <Windows.h>
#include "awmdll.h"

class CBaseObject {
public:
	ULONG m_cRef;

    static void* operator new(size_t cb)
    {
        void* p = AWM::HeapImpl::Alloc(cb);
        if (!p) {
            printf("Failed to allocate memory for object creation!");
        }
        return p;
    }

    static void operator delete(void* p)
    {
        AWM::HeapImpl::Free(p);
    }

	CBaseObject();
	ULONG AddRef();
	ULONG Release();
	virtual ~CBaseObject();
};

static_assert(offsetof(CBaseObject, m_cRef) == 0x08, "ERROR: m_cRef not in the correct position!");