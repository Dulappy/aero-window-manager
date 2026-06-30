#include "baseobject.h"

CBaseObject::CBaseObject() {
    this->m_cRef = 1;
}

ULONG CBaseObject::AddRef() {
    ++this->m_cRef;
    return this->m_cRef;
}

ULONG CBaseObject::Release() {
    if (--this->m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return this->m_cRef;
}

CBaseObject::~CBaseObject() {
}