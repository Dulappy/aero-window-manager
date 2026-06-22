#pragma once
#include <Windows.h>

#define E_INVALIDWINDOWHANDLE _HRESULT_TYPEDEF_(0x80070578)

extern BYTE g_bUpdateWindowTransformOnce;
extern HRESULT g_hrExpected_EAccessDenied;
extern HRESULT g_hrExpected_InvalidHandle;
extern HRESULT g_hrExpected_EInvalidArg;

struct D2DMatrix
{
    float m[4][4];
};

D2DMatrix* D2DMatrixScaling(D2DMatrix* pOut, float scaleX, float scaleY, float scaleZ);