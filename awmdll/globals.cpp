#include "globals.h"

BYTE g_bUpdateWindowTransformOnce = 0;
HRESULT g_hrExpected_EAccessDenied = 0;
HRESULT g_hrExpected_InvalidHandle = 0;
HRESULT g_hrExpected_EInvalidArg = 0;

D2DMatrix* D2DMatrixScaling(D2DMatrix* pOut,
    float      scaleX,
    float      scaleY,
    float      scaleZ)
{
    pOut->m[0][0] = scaleX;
    pOut->m[0][1] = 0.0f;
    pOut->m[0][2] = 0.0f;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = 0.0f;
    pOut->m[1][1] = scaleY;
    pOut->m[1][2] = 0.0f;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = 0.0f;
    pOut->m[2][1] = 0.0f;
    pOut->m[2][2] = scaleZ;
    pOut->m[2][3] = 0.0f;
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = 0.0f;
    pOut->m[3][3] = 1.0f;

    return pOut;
}
