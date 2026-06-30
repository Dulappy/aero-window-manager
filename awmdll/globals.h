#pragma once
#include <Windows.h>
#include <d3d10.h>
#include <stdint.h>
#include <stdio.h>

#define E_INVALIDWINDOWHANDLE _HRESULT_TYPEDEF_(0x80070578)

extern HANDLE g_hProcessHeap;

extern BYTE g_bUpdateWindowTransformOnce;
extern HRESULT g_hrExpected_EAccessDenied;
extern HRESULT g_hrExpected_InvalidHandle;
extern HRESULT g_hrExpected_EInvalidArg;

void MilInstrumentationCheckHR(int32_t, int32_t*, int64_t, int32_t, uint32_t);

enum MilBitmapInterpolationMode
{
    MilBitmapInterpolationMode_NearestNeighbor = 0,
    MilBitmapInterpolationMode_Linear = 1,
    // (further values not observed)
};

typedef struct MIL_CHANNEL__* MIL_CHANNEL; // Dummy. Points to a CMilChannel object in dwmcore.dll.

struct D2DMatrix
{
    float m[4][4];
}; 

struct D2DVector3
{
    float x, y, z;

    D2DVector3& operator/=(float divisor) {
        float rcp = 1.0f / divisor;
        x *= rcp;
        y *= rcp;
        z *= rcp;
        return *this;
    }
    D2DVector3 operator/(float divisor) const {
        float rcp = 1.0f / divisor;
        return { this->x * rcp, this->y * rcp, this->z * rcp };
    }
};

struct D2DVector2
{
    float x, y; 
    
    D2DVector2& operator/=(float divisor) {
        float rcp = 1.0f / divisor;
        this->x *= rcp;
        this->y *= rcp;
        return *this;
    }
};

// D2DQuaternion: { x, y, z, w }
struct D2DQuaternion
{
    float x, y, z, w;
};

// Builds the 4×4 identity matrix.
D2DMatrix* D2DMatrixIdentity(D2DMatrix* pOut);

// Builds a pure axis-aligned scaling matrix.
D2DMatrix* D2DMatrixScaling(D2DMatrix* pOut, float scaleX, float scaleY, float scaleZ);

// Builds a pure translation matrix.
D2DMatrix* D2DMatrixTranslation(D2DMatrix* pOut, float tx, float ty, float tz);

// Builds a rotation matrix around the Z-axis by angle theta (radians).
D2DMatrix* D2DMatrixRotationZ(D2DMatrix* pOut, float theta);

// Converts a unit quaternion (x, y, z, w) to a 3×3 rotation embedded in a 4×4 matrix (bottom row and right column are 0,0,0,1).
D2DMatrix* D2DMatrixRotationQuaternion(D2DMatrix* pOut, const D2DQuaternion* pQ);

// Converts (yaw, pitch, roll) Euler angles to a rotation matrix via an intermediate quaternion.
D2DMatrix* D2DMatrixRotationYawPitchRoll(D2DMatrix* pOut, float yaw, float pitch, float roll);

// Right-handed perspective projection matrix.
D2DMatrix* D2DMatrixPerspectiveRH(D2DMatrix* pOut, float w, float h, float zn, float zf);

// Right-handed look-at view matrix.
D2DMatrix* D2DMatrixLookAtRH(D2DMatrix* pOut, const D2DVector3* pEye, const D2DVector3* pAt, const D2DVector3* pUp);

// Matrix multiplication: pOut = pA * pB
D2DMatrix* D2DMatrixMultiply(D2DMatrix* pOut, const D2DMatrix* pA, const D2DMatrix* pB);

// Computes the inverse of a 4×4 matrix using cofactor expansion (Cramer's rule).
D2DMatrix* D2DMatrixInverse(D2DMatrix* pOut, float* pDeterminant, const D2DMatrix* pM);


// Component-wise subtraction.
D2DVector3* D3DXVec3Subtract(D2DVector3* pOut, const D2DVector3* pA, const D2DVector3* pB);

// Returns the dot product.
float D3DXVec3Dot(const D2DVector3* pA, const D2DVector3* pB);

// Returns squared length (dot product with itself).
float D3DXVec3LengthSq(const D2DVector3* pV);

// Computes pA * pB.
D2DVector3* D3DXVec3Cross(D2DVector3* pOut, const D2DVector3* pA, const D2DVector3* pB);

// Normalize vector.
D2DVector3* D3DXVec3Normalize(D2DVector3* pOut, const D2DVector3* pSrc);

// Standard barycentric interpolation.
D2DVector3* D3DXVec3BaryCentric(D2DVector3* pOut, const D2DVector3* pV1, const D2DVector3* pV2, const D2DVector3* pV3, float f, float g);

// Converts Euler angles to a unit quaternion.
D2DQuaternion* D3DXQuaternionRotationYawPitchRoll(D2DQuaternion* pOut, float yaw, float pitch, float roll);

// Transforms a 2D point by a 4×4 matrix, treating the input as (x, y, 0, 1), then performs a perspective divide by W if W deviates from 1.0.
D2DVector2* D3DXVec2TransformCoord(D2DVector2* pOut, const D2DVector2* pVec, const D2DMatrix* pMat);

// Transforms a 3D point by a 4×4 matrix treating input as (x, y, z, 1), then divides by W if W != 1. Single-vector version of TransformCoordArray.
D2DVector3* D3DXVec3TransformCoord(D2DVector3* pOut, const D2DVector3* pSrc, const D2DMatrix* pMat);

// Applies D3DXVec3TransformCoord to N vectors, advancing both src and dst pointers by 12 bytes (sizeof D2DVector3) per iteration.
D2DVector3* D3DXVec3TransformCoordArray(D2DVector3* pOut, UINT outStride, const D2DVector3* pSrc, UINT srcStride, const D2DMatrix* pMat, UINT count);

// Projects an array of 3D world-space points to screen space by combining up to three transformation matrices and a viewport transform.
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
);