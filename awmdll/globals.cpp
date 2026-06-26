#include "globals.h"
#include "math.h"
#include "float.h"

BYTE g_bUpdateWindowTransformOnce = 0;
HRESULT g_hrExpected_EAccessDenied = 0;
HRESULT g_hrExpected_InvalidHandle = 0;
HRESULT g_hrExpected_EInvalidArg = 0;

// ===========================================================================
//  D2DMatrix Functions
// ===========================================================================

D2DMatrix* D2DMatrixIdentity(D2DMatrix* pOut) {
    pOut->m[0][0] = 1.0f;
    pOut->m[0][1] = 0.0f;
    pOut->m[0][2] = 0.0f;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = 0.0f;
    pOut->m[1][1] = 1.0f;
    pOut->m[1][2] = 0.0f;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = 0.0f;
    pOut->m[2][1] = 0.0f;
    pOut->m[2][2] = 1.0f;
    pOut->m[2][3] = 0.0f;
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = 0.0f;
    pOut->m[3][3] = 1.0f;

    return pOut;
}

D2DMatrix* D2DMatrixScaling(D2DMatrix* pOut, float scaleX, float scaleY, float scaleZ) {
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

D2DMatrix* D2DMatrixTranslation(D2DMatrix* pOut, float tx, float ty, float tz) {
    pOut->m[0][0] = 1.0f;
    pOut->m[0][1] = 0.0f;
    pOut->m[0][2] = 0.0f;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = 0.0f;
    pOut->m[1][1] = 1.0f;
    pOut->m[1][2] = 0.0f;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = 0.0f;
    pOut->m[2][1] = 0.0f;
    pOut->m[2][2] = 1.0f;
    pOut->m[2][3] = 0.0f;
    pOut->m[3][0] = tx;
    pOut->m[3][1] = ty;
    pOut->m[3][2] = tz;
    pOut->m[3][3] = 1.0f;

    return pOut;
}

D2DMatrix* D2DMatrixRotationZ(D2DMatrix* pOut, float theta) {
    float s = sinf(theta);
    float c = cosf(theta);

    pOut->m[0][0] = c;
    pOut->m[0][1] = s;
    pOut->m[0][2] = 0.0f;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = -s;
    pOut->m[1][1] = c;
    pOut->m[1][2] = 0.0f;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = 0.0f;
    pOut->m[2][1] = 0.0f;
    pOut->m[2][2] = 1.0f;
    pOut->m[2][3] = 0.0f;
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = 0.0f;
    pOut->m[3][3] = 1.0f;

    return pOut;
}

D2DMatrix* D2DMatrixRotationQuaternion(D2DMatrix* pOut, const D2DQuaternion* pQ) {
    float x = pQ->x;
    float y = pQ->y;
    float z = pQ->z;
    float w = pQ->w;

    float x2 = x + x;  // 2x
    float y2 = y + y;  // 2y
    float z2 = z + z;  // 2z

    float xx = x * x2;  // 2x^2
    float yy = y * y2;  // 2y^2
    float zz = z * z2;  // 2z^2
    float xy = x * y2;  // 2xy
    float xz = x * z2;  // 2xz
    float yz = y * z2;  // 2yz
    float wx = w * x2;  // 2wx
    float wy = w * y2;  // 2wy
    float wz = w * z2;  // 2wz

    pOut->m[0][0] = 1.0f - yy - zz;
    pOut->m[0][1] = xy + wz;
    pOut->m[0][2] = xz - wy;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = xy - wz;
    pOut->m[1][1] = 1.0f - xx - zz;
    pOut->m[1][2] = yz + wx;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = xz + wy;
    pOut->m[2][1] = yz - wx;
    pOut->m[2][2] = 1.0f - xx - yy;
    pOut->m[2][3] = 0.0f;
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = 0.0f;
    pOut->m[3][3] = 1.0f;

    return pOut;
}

D2DMatrix* D2DMatrixRotationYawPitchRoll(D2DMatrix* pOut, float yaw, float pitch, float roll) {
    D2DQuaternion q;
    D3DXQuaternionRotationYawPitchRoll(&q, yaw, pitch, roll);
    D2DMatrixRotationQuaternion(pOut, &q);

    return pOut;
}

D2DMatrix* D2DMatrixPerspectiveRH(D2DMatrix* pOut, float w, float h, float zn, float zf) {
    float twoZn = 2.0f * zn;

    pOut->m[0][0] = twoZn / w;
    pOut->m[0][1] = 0.0f;
    pOut->m[0][2] = 0.0f;
    pOut->m[0][3] = 0.0f;
    pOut->m[1][0] = 0.0f;
    pOut->m[1][1] = twoZn / h;
    pOut->m[1][2] = 0.0f;
    pOut->m[1][3] = 0.0f;
    pOut->m[2][0] = 0.0f;
    pOut->m[2][1] = 0.0f;
    pOut->m[2][2] = zf / (zn - zf);
    pOut->m[2][3] = -1.0f;
    pOut->m[3][0] = 0.0f;
    pOut->m[3][1] = 0.0f;
    pOut->m[3][2] = zn * zf / (zn - zf);
    pOut->m[3][3] = 0.0f;

    return pOut;
}

D2DMatrix* D2DMatrixLookAtRH(D2DMatrix* pOut, const D2DVector3* pEye, const D2DVector3* pAt, const D2DVector3* pUp) {
    D2DVector3 zAxis, xAxis, yAxis;

    // zAxis = normalize(pEye - pAt)
    D3DXVec3Subtract(&zAxis, pEye, pAt);
    D3DXVec3Normalize(&zAxis, &zAxis);

    // xAxis = normalize(cross(pUp, zAxis))
    D3DXVec3Cross(&xAxis, pUp, &zAxis);
    D3DXVec3Normalize(&xAxis, &xAxis);

    // yAxis = cross(zAxis, xAxis)
    D3DXVec3Cross(&yAxis, &zAxis, &xAxis);

    // Fill rotation rows
    pOut->m[0][0] = xAxis.x;
    pOut->m[0][1] = yAxis.x;
    pOut->m[0][2] = zAxis.x;
    pOut->m[0][3] = 0.0f;

    pOut->m[1][0] = xAxis.y;
    pOut->m[1][1] = yAxis.y;
    pOut->m[1][2] = zAxis.y;
    pOut->m[1][3] = 0.0f;

    pOut->m[2][0] = xAxis.z;
    pOut->m[2][1] = yAxis.z;
    pOut->m[2][2] = zAxis.z;
    pOut->m[2][3] = 0.0f;

    // Translation row = negated dot products with eye position
    pOut->m[3][0] = -D3DXVec3Dot(&xAxis, pEye);
    pOut->m[3][1] = -D3DXVec3Dot(&yAxis, pEye);
    pOut->m[3][2] = -D3DXVec3Dot(&zAxis, pEye);
    pOut->m[3][3] = 1.0f;

    return pOut;
}

D2DMatrix* D2DMatrixMultiply(D2DMatrix* pOut, const D2DMatrix* pA, const D2DMatrix* pB) {
    D2DMatrix tmp;
    D2DMatrix* pDst = (pOut == pA || pOut == pB) ? &tmp : pOut;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pDst->m[i][j] = pA->m[i][0] * pB->m[0][j] +
                            pA->m[i][1] * pB->m[1][j] +
                            pA->m[i][2] * pB->m[2][j] +
                            pA->m[i][3] * pB->m[3][j];
        }
    }

    if (pDst != pOut)
    memcpy(pOut, pDst, 64);

    return pOut;
}

D2DMatrix* D2DMatrixInverse(D2DMatrix* pOut, float* pDeterminant, const D2DMatrix* pM) {
    // Cofactors (3*3 sub-determinants), transposed for Cramer's rule.
    // Each is computed as a combination of products of pairs of elements
    // from the 3 rows/columns not including the cofactor's own row/column.

    // -- Pre-compute six 2*2 minors from rows 2 and 3 --
    float A2323 = pM->m[2][2] * pM->m[3][3] - pM->m[2][3] * pM->m[3][2];
    float A1323 = pM->m[2][1] * pM->m[3][3] - pM->m[2][3] * pM->m[3][1];
    float A1223 = pM->m[2][1] * pM->m[3][2] - pM->m[2][2] * pM->m[3][1];
    float A0323 = pM->m[2][0] * pM->m[3][3] - pM->m[2][3] * pM->m[3][0];
    float A0223 = pM->m[2][0] * pM->m[3][2] - pM->m[2][2] * pM->m[3][0];
    float A0123 = pM->m[2][0] * pM->m[3][1] - pM->m[2][1] * pM->m[3][0];

    // -- Pre-compute six 2*2 minors from rows 0 and 1 --
    float A2301 = pM->m[0][2] * pM->m[1][3] - pM->m[0][3] * pM->m[1][2];
    float A1301 = pM->m[0][1] * pM->m[1][3] - pM->m[0][3] * pM->m[1][1];
    float A1201 = pM->m[0][1] * pM->m[1][2] - pM->m[0][2] * pM->m[1][1];
    float A0301 = pM->m[0][0] * pM->m[1][3] - pM->m[0][3] * pM->m[1][0];
    float A0201 = pM->m[0][0] * pM->m[1][2] - pM->m[0][2] * pM->m[1][0];
    float A0101 = pM->m[0][0] * pM->m[1][1] - pM->m[0][1] * pM->m[1][0];

    // -- Compute determinant (expansion along row 0) --
    float det = pM->m[0][0] * (pM->m[1][1] * A2323 - pM->m[1][2] * A1323 + pM->m[1][3] * A1223)
        - pM->m[0][1] * (pM->m[1][0] * A2323 - pM->m[1][2] * A0323 + pM->m[1][3] * A0223)
        + pM->m[0][2] * (pM->m[1][0] * A1323 - pM->m[1][1] * A0323 + pM->m[1][3] * A0123)
        - pM->m[0][3] * (pM->m[1][0] * A1223 - pM->m[1][1] * A0223 + pM->m[1][2] * A0123);

    // -- Singular check --
    if (det == 0.0f) return nullptr;

    float rcp = 1.0f / det;
    // Check finiteness via __imp__finite (operates on double)
    if (!_finite((double)rcp)) return nullptr;

    // -- Write pDeterminant if requested --
    if (pDeterminant) *pDeterminant = det;

    // -- Write transposed cofactors * rcp --
    pOut->m[0][0] =  (pM->m[1][1] * A2323 - pM->m[1][2] * A1323 + pM->m[1][3] * A1223) * rcp;
    pOut->m[0][1] = -(pM->m[0][1] * A2323 - pM->m[0][2] * A1323 + pM->m[0][3] * A1223) * rcp;
    pOut->m[0][2] =  (pM->m[0][1] * A2301 - pM->m[0][2] * A1301 + pM->m[0][3] * A1201) * rcp;
    pOut->m[0][3] = -(pM->m[0][1] * A2301 - pM->m[0][2] * A1301 + pM->m[0][3] * A1201) * rcp;
    pOut->m[1][0] = -(pM->m[1][0] * A2323 - pM->m[1][2] * A0323 + pM->m[1][3] * A0223) * rcp;
    pOut->m[1][1] =  (pM->m[0][0] * A2323 - pM->m[0][2] * A0323 + pM->m[0][3] * A0223) * rcp;
    pOut->m[1][2] = -(pM->m[0][0] * A2301 - pM->m[0][2] * A0301 + pM->m[0][3] * A0201) * rcp;
    pOut->m[1][3] =  (pM->m[0][0] * A2301 - pM->m[0][2] * A0301 + pM->m[0][3] * A0201) * rcp;
    pOut->m[2][0] =  (pM->m[1][0] * A1323 - pM->m[1][1] * A0323 + pM->m[1][3] * A0123) * rcp;
    pOut->m[2][1] = -(pM->m[0][0] * A1323 - pM->m[0][1] * A0323 + pM->m[0][3] * A0123) * rcp;
    pOut->m[2][2] =  (pM->m[0][0] * A1301 - pM->m[0][1] * A0301 + pM->m[0][3] * A0101) * rcp;
    pOut->m[2][3] = -(pM->m[0][0] * A1301 - pM->m[0][1] * A0301 + pM->m[0][3] * A0101) * rcp;
    pOut->m[3][0] = -(pM->m[1][0] * A1223 - pM->m[1][1] * A0223 + pM->m[1][2] * A0123) * rcp;
    pOut->m[3][1] =  (pM->m[0][0] * A1223 - pM->m[0][1] * A0223 + pM->m[0][2] * A0123) * rcp;
    pOut->m[3][2] = -(pM->m[0][0] * A1201 - pM->m[0][1] * A0201 + pM->m[0][2] * A0101) * rcp;
    pOut->m[3][3] =  (pM->m[0][0] * A1201 - pM->m[0][1] * A0201 + pM->m[0][2] * A0101) * rcp;

    return pOut;
}

// ===========================================================================
//  D3DX Functions
// ===========================================================================

D2DVector3* D3DXVec3Subtract(D2DVector3* pOut, const D2DVector3* pA, const D2DVector3* pB) {
    pOut->x = pA->x - pB->x;
    pOut->y = pA->y - pB->y;
    pOut->z = pA->z - pB->z;

    return pOut;
}

float D3DXVec3Dot(const D2DVector3* pA, const D2DVector3* pB) {
    return pA->x * pB->x + pA->y * pB->y + pA->z * pB->z;
}

float D3DXVec3LengthSq(const D2DVector3* pV) {
    return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z;
}

D2DVector3* D3DXVec3Cross(D2DVector3* pOut, const D2DVector3* pA, const D2DVector3* pB) {
    // Compute into temporaries first to handle aliasing
    float x = pA->y * pB->z - pA->z * pB->y;
    float y = pA->z * pB->x - pA->x * pB->z;
    float z = pA->x * pB->y - pA->y * pB->x;
    pOut->x = x;
    pOut->y = y;
    pOut->z = z;

    return pOut;
}

D2DVector3* D3DXVec3Normalize(D2DVector3* pOut, const D2DVector3* pSrc) {
    float lenSq = D3DXVec3LengthSq(pSrc);
    float diff = lenSq - 1.0f;

    if (diff >= -FLT_EPSILON && diff <= FLT_EPSILON)
    {
        // Already unit length (within one ULP of 1.0).
        // If pOut and pSrc are the same object, nothing to do.
        if (pOut != pSrc)
        {
            pOut->x = pSrc->x;
            pOut->y = pSrc->y;
            pOut->z = pSrc->z;
        }
        return pOut;
    }

    // Needs normalisation
    if (lenSq <= FLT_MIN)
    {
        // Degenerate vector — output zero rather than inf/NaN.
        pOut->x = pOut->y = pOut->z = 0.0f;
    }
    else
    {
        float len = sqrtf(lenSq);
        D2DVector3 n = *pSrc / len;
        pOut->x = n.x;
        pOut->y = n.y;
        pOut->z = n.z;
    }

    return pOut;
}

D2DVector3* D3DXVec3BaryCentric(D2DVector3* pOut, const D2DVector3* pV1, const D2DVector3* pV2, const D2DVector3* pV3, float f, float g) {
    pOut->x = pV1->x + f * (pV2->x - pV1->x) + g * (pV3->x - pV1->x);
    pOut->y = pV1->y + f * (pV2->y - pV1->y) + g * (pV3->y - pV1->y);
    pOut->z = pV1->z + f * (pV2->z - pV1->z) + g * (pV3->z - pV1->z);

    return pOut;
}

D2DQuaternion* D3DXQuaternionRotationYawPitchRoll(D2DQuaternion* pOut, float yaw, float pitch, float roll) {
    float hr = roll * 0.5f;
    float hp = pitch * 0.5f;
    float hy = yaw * 0.5f;

    float sr = sinf(hr);
    float cr = cosf(hr);
    float sp = sinf(hp);
    float cp = cosf(hp);
    float sy = sinf(hy);
    float cy = cosf(hy);

    pOut->x = cy * sp * cr + sy * cp * sr;
    pOut->y = sy * cp * cr - cy * sp * sr;
    pOut->z = cy * cp * sr - sy * sp * cr;
    pOut->w = cy * cp * cr + sy * sp * sr;

    return pOut;
}

D2DVector2* D3DXVec2TransformCoord(D2DVector2* pOut, const D2DVector2* pVec, const D2DMatrix* pMat) {
    float x = pVec->x;
    float y = pVec->y;

    float rx = x * pMat->m[0][0] + y * pMat->m[1][0] + pMat->m[3][0];  // row-major
    float ry = x * pMat->m[0][1] + y * pMat->m[1][1] + pMat->m[3][1];
    float w  = x * pMat->m[0][3] + y * pMat->m[1][3] + pMat->m[3][3];

    // Write both components as a single 64-bit store
    pOut->x = rx;
    pOut->y = ry;

    // Perspective divide if W != 1
    float wdiff = w - 1.0f;
    if (wdiff < -FLT_EPSILON || wdiff > FLT_EPSILON)
        *pOut /= w;

    return pOut;
}

D2DVector3* D3DXVec3TransformCoord(D2DVector3* pOut, const D2DVector3* pSrc, const D2DMatrix* pMat) {
    float x = pSrc->x, y = pSrc->y, z = pSrc->z;

    float rx = x * pMat->m[0][0] + y * pMat->m[1][0] + z * pMat->m[2][0] + pMat->m[3][0];
    float ry = x * pMat->m[0][1] + y * pMat->m[1][1] + z * pMat->m[2][1] + pMat->m[3][1];
    float rz = x * pMat->m[0][2] + y * pMat->m[1][2] + z * pMat->m[2][2] + pMat->m[3][2];
    float w  = x * pMat->m[0][3] + y * pMat->m[1][3] + z * pMat->m[2][3] + pMat->m[3][3];

    pOut->x = rx;
    pOut->y = ry;
    pOut->z = rz;

    float wdiff = w - 1.0f;
    if (wdiff < -FLT_EPSILON || wdiff > FLT_EPSILON)
        *pOut /= w;

    return pOut;
}

D2DVector3* D3DXVec3TransformCoordArray(D2DVector3* pOut, UINT outStride, const D2DVector3* pSrc, UINT srcStride, const D2DMatrix* pMat, UINT count) {
    D2DVector3* pResult = pOut;

    // Originally hardcoded as 4. Modified since the function is only called once with count set to 4 anyway.
    for (int i = 0; i < count; i++)
    {
        float x = pSrc->x, y = pSrc->y, z = pSrc->z;

        float rx = x * pMat->m[0][0] + y * pMat->m[1][0] + z * pMat->m[2][0] + pMat->m[3][0];
        float ry = x * pMat->m[0][1] + y * pMat->m[1][1] + z * pMat->m[2][1] + pMat->m[3][1];
        float rz = x * pMat->m[0][2] + y * pMat->m[1][2] + z * pMat->m[2][2] + pMat->m[3][2];
        float w  = x * pMat->m[0][3] + y * pMat->m[1][3] + z * pMat->m[2][3] + pMat->m[3][3];

        pOut->x = rx;
        pOut->y = ry;
        pOut->z = rz;

        float wdiff = w - 1.0f;
        if (wdiff < -FLT_EPSILON || wdiff > FLT_EPSILON)
            *pOut /= w;

        pSrc++;
        pOut++;
    }

    return pResult;
}

D2DVector3* D3DXVec3ProjectArray(
    D2DVector3*           pOut,
    UINT                  outStride,
    const D2DVector3*     pSrc,
    UINT                  srcStride,
    const D3D10_VIEWPORT* pViewport,
    const D2DMatrix*      pProjection,
    const D2DMatrix*      pView,
    const D2DMatrix*      pWorld,
    UINT                  count
) {
    D2DMatrix combined;   // final world*view*projection matrix
    D2DMatrix viewport;   // viewport transform matrix

    // -- Step 1: build combined W*V*P ----------------------------------------
    {
        const D2DMatrix* pWV = nullptr;
        D2DMatrix wv_temp;

        if (pWorld && pView)
        {
            D2DMatrixMultiply(&wv_temp, pWorld, pView);
            pWV = &wv_temp;
        }
        else if (pWorld) { pWV = pWorld; }
        else if (pView) { pWV = pView; }

        if (pProjection)
        {
            if (pWV) D2DMatrixMultiply(&combined, pWV, pProjection);
            else     combined = *pProjection;
        }
        else
        {
            if (pWV) combined = *pWV;
            else     D2DMatrixIdentity(&combined);
        }
    }

    // -- Step 2: build viewport matrix ----------------------------------------
    if (pViewport)
    {
        float w2 = (float)pViewport->Width * 0.5f;
        float h2 = (float)pViewport->Height * 0.5f;
        float tx = (float)pViewport->TopLeftX + w2;
        float ty = (float)pViewport->TopLeftY + h2;
        float minD = pViewport->MinDepth;
        float maxD = pViewport->MaxDepth;

        // Start from zero-filled local, fill non-zero entries:
        memset(&viewport, 0, sizeof(viewport));
        viewport.m[0][0] = w2;             // X scale
        viewport.m[1][1] = -h2;            // Y scale (flipped)
        viewport.m[2][2] = maxD - minD;    // Z scale
        viewport.m[3][0] = tx;             // X translation
        viewport.m[3][1] = ty;             // Y translation
        viewport.m[3][2] = minD;           // Z translation
        viewport.m[3][3] = 1.0f;

        // -- Step 3: multiply combined * viewport -----------------------------
        D2DMatrix result;
        D2DMatrixMultiply(&result, &combined, &viewport);

        // -- Step 4: transform all vectors ------------------------------------
        return D3DXVec3TransformCoordArray(pOut, 12, pSrc, 12, &result, 4);
    }
    else
    {
        // No viewport — transform with combined matrix only
        return D3DXVec3TransformCoordArray(pOut, 12, pSrc, 12, &combined, 4);
    }
}