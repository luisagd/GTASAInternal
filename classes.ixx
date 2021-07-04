export module classes;
#include "stdafx.h"
//import vec;

export struct RwV3d
{
    float x;   /**< X value */
    float y;   /**< Y value */
    float z;   /**< Z value */
};

export class WeaponSlot
{
public:
    uint32_t Type; //0x0000
    uint32_t State; //0x0004
    uint32_t AmmoInClip; //0x0008
    uint32_t AmmoRemaining; //0x000C
    char pad_0010[12]; //0x0010
}; //Size: 0x001C

export class ViewMatrix
{
public:
    float x1; //0x0000
    float x2; //0x0004
    float x3; //0x0008
    char pad_000C[4]; //0x000C
    float y1; //0x0010
    float y2; //0x0014
    float y3; //0x0018
    char pad_001C[4]; //0x001C
    float z1; //0x0020
    float z2; //0x0024
    float z3; //0x0028
    char pad_002C[4]; //0x002C
    float w1; //0x0030
    float w2; //0x0034
    float w3; //0x0038
    char pad_003C[4]; //0x003C
}; //Size: 0x0040

export class CPed
{
public:
    char pad_0000[20]; //0x0000
    class ViewMatrix* pViewMatrix; //0x0014
    char pad_0018[44]; //0x0018
    float SpeedVectorX; //0x0044
    float SpeedVectorY; //0x0048
    char pad_004C[116]; //0x004C
    void* CarNearest; //0x00C0
    char pad_00C4[936]; //0x00C4
    int8_t N0000037C; //0x046C
    char pad_046D[2]; //0x046D
    uint8_t CrouchState; //0x046F
    char pad_0470[208]; //0x0470
    float Health; //0x0540
    float MaxHealth; //0x0544
    char pad_0548[20]; //0x0548
    float Yaw;
    char pad_05482[64]; //0x0548
    class WeaponSlot WeaponSlot[13]; //0x05A0
    char pad_05A4[88]; //0x05A4
    CPed* CpedFired; //0x0764
    char pad_0768[92]; //0x0768

    void GetBonePosition(RwV3d& outPosition, unsigned int boneId, bool updateSkinBones);
}; //Size: 0x07C4

static_assert(sizeof(CPed) == 0x7c4);

export struct CPedPoolInfo {
    CPed* FirstCPed;
    uint8_t(*CPedsInUse)[140];
    uint32_t MaxCPed;
    uint32_t CurrentCPedNum;
};// Size: 0x10
static_assert(sizeof(CPedPoolInfo) == 0x10);

static constexpr float FLOAT_EPSILON = 0.0001f;

export class CVector4D
{
public:
    float fX = 0.0f;
    float fY = 0.0f;
    float fZ = 0.0f;
    float fW = 0.0f;

    constexpr CVector4D() noexcept = default;

    constexpr CVector4D(const CVector4D&) noexcept = default;

    constexpr CVector4D& operator=(const CVector4D&) noexcept = default;

    constexpr CVector4D(float x, float y, float z, float w) noexcept :
        fX(x),
        fY(y),
        fZ(z),
        fW(w)
    {
    }

    // Warning, this function is returning the wrong value(fW is missing), its kept because nothing uses it, only
    // CLuaVector4DDefs.
    constexpr float DotProduct(const CVector4D& other) const noexcept { return fX * other.fX + fY * other.fY + fZ * other.fZ; }

    float Length() const noexcept { return sqrt(fX * fX + fY * fY + fZ * fZ + fW * fW); }

    // LengthSquared returns Length() without sqrt applied (i.e. returns x*x* + y*y + z*z + w*w).
    // This can be useful if you only want to compare lengths.
    constexpr float LengthSquared() const noexcept { return (fX * fX) + (fY * fY) + (fZ * fZ) + (fW * fW); }

    void Normalize() noexcept
    {
        const float fLength = Length();
        if (fLength > 0.0f)
        {
            fX /= fLength;
            fY /= fLength;
            fZ /= fLength;
            fW /= fLength;
        }
    }

    constexpr CVector4D operator*(const CVector4D& vecRight) const { return CVector4D(fX * vecRight.fX, fY * vecRight.fY, fZ * vecRight.fZ, fW * vecRight.fW); }

    constexpr CVector4D operator*(const float fRight) const noexcept { return CVector4D(fX * fRight, fY * fRight, fZ * fRight, fW * fRight); }


    constexpr CVector4D operator+(const CVector4D& vecRight) const noexcept { return CVector4D(fX + vecRight.fX, fY + vecRight.fY, fZ + vecRight.fZ, fW + vecRight.fW); }

    constexpr CVector4D operator-(const CVector4D& vecRight) const noexcept { return CVector4D(fX - vecRight.fX, fY - vecRight.fY, fZ - vecRight.fZ, fW - vecRight.fW); }


    constexpr CVector4D operator/(const CVector4D& vecRight) const noexcept { return CVector4D(fX / vecRight.fX, fY / vecRight.fY, fZ / vecRight.fZ, fW / vecRight.fW); }

    constexpr CVector4D operator/(const float fRight) const noexcept { return CVector4D(fX / fRight, fY / fRight, fZ / fRight, fW / fRight); }

    constexpr void operator+=(const float fRight) noexcept
    {
        fX += fRight;
        fY += fRight;
        fZ += fRight;
        fW += fRight;
    }

    constexpr void operator+=(const CVector4D& vecRight) noexcept
    {
        fX += vecRight.fX;
        fY += vecRight.fY;
        fZ += vecRight.fZ;
        fW += vecRight.fW;
    }

    constexpr void operator-=(const float fRight) noexcept
    {
        fX -= fRight;
        fY -= fRight;
        fZ -= fRight;
        fW -= fRight;
    }

    constexpr void operator-=(const CVector4D& vecRight) noexcept
    {
        fX -= vecRight.fX;
        fY -= vecRight.fY;
        fZ -= vecRight.fZ;
        fW -= vecRight.fW;
    }

    constexpr void operator*=(const float fRight) noexcept
    {
        fX *= fRight;
        fY *= fRight;
        fZ *= fRight;
        fW *= fRight;
    }


    constexpr void operator/=(const float fRight) noexcept
    {
        fX /= fRight;
        fY /= fRight;
        fZ /= fRight;
        fW /= fRight;
    }

    constexpr void operator/=(const CVector4D& vecRight) noexcept
    {
        fX /= vecRight.fX;
        fY /= vecRight.fY;
        fZ /= vecRight.fZ;
        fW /= vecRight.fW;
    }

    bool operator==(const CVector4D& param) const noexcept
    {
        return ((fabs(fX - param.fX) < FLOAT_EPSILON) && (fabs(fY - param.fY) < FLOAT_EPSILON) && (fabs(fZ - param.fZ) < FLOAT_EPSILON) &&
            (fabs(fW - param.fW) < FLOAT_EPSILON));
    }

    bool operator!=(const CVector4D& param) const noexcept { return !(*this == param); }
};

export class CVector
{
private:
    static constexpr float FLOAT_EPSILON = 0.0001f;
public:
    float fX = 0.0f;
    float fY = 0.0f;
    float fZ = 0.0f;

    constexpr CVector() = default;

    constexpr CVector(float x, float y, float z) :
        fX(x),
        fY(y),
        fZ(z)
    {
    }

    constexpr CVector(const CVector4D& vec) noexcept :
        fX(vec.fX),
        fY(vec.fY),
        fZ(vec.fZ)
    {
    }

    constexpr CVector& operator=(const CVector4D& vec) noexcept
    {
        fX = vec.fX;
        fY = vec.fY;
        fZ = vec.fZ;
        return *this;
    }


    constexpr CVector Clone() const { return *this; }

    // Normalize returns the normalized length of the vector.
    float Normalize()
    {
        const float t = Length();
        if (t > FLOAT_EPSILON)
        {
            fX /= t;
            fY /= t;
            fZ /= t;

            return t;
        }
        else
            return 0;
    }

    inline float Length() const { return sqrt((fX * fX) + (fY * fY) + (fZ * fZ)); }

    // LengthSquared returns Length() without sqrt applied (i.e. returns x*x* + y*y + z*z).
    // This can be useful if you only want to compare lengths.
    inline float LengthSquared() const { return (fX * fX) + (fY * fY) + (fZ * fZ); }

    inline float DotProduct(const CVector* param) const { return fX * param->fX + fY * param->fY + fZ * param->fZ; }

    void CrossProduct(const CVector* param)
    {
        const float _fX = fX, _fY = fY, _fZ = fZ;
        fX = _fY * param->fZ - param->fY * _fZ;
        fY = _fZ * param->fX - param->fZ * _fX;
        fZ = _fX * param->fY - param->fX * _fY;
    }

    // Convert (direction) to rotation
    CVector ToRotation() const
    {
        CVector vecRotation;
        vecRotation.fZ = atan2(fY, fX);
        CVector vecTemp(std::hypotf(fX, fY), fZ, 0);
        vecTemp.Normalize();
        constexpr float pi = 3.14159265358979323846f;
        vecRotation.fY = atan2(vecTemp.fX, vecTemp.fY) - pi / 2;
        return vecRotation;
    }

    // Return a perpendicular direction
    CVector GetOtherAxis() const
    {
        CVector vecResult;
        if (std::abs(fX) > std::abs(fY))
            vecResult = CVector(fZ, 0, -fX);
        else
            vecResult = CVector(0, -fZ, fY);

        vecResult.Normalize();
        return vecResult;
    }

    // Intersections code based on https://github.com/juj/MathGeoLib/blob/master/src/Geometry/Plane.h
    bool IntesectsLinePlane(const CVector& vecRay, const CVector& vecNormal, const CVector& vecPosition, float* fOutDist) const noexcept
    {
        const float fDenom = vecNormal.DotProduct(&vecRay);

        if (fabs(fDenom) > 1e-4f)
        {
            *fOutDist = (vecPosition.Length() - vecNormal.DotProduct(this)) / fDenom;
            return true;
        }

        if (fDenom != 0.0f)
        {
            *fOutDist = (vecPosition.Length() - vecNormal.DotProduct(this)) / fDenom;
            return fabs(*fOutDist) < 1e-4f;
        }

        *fOutDist = 0.0f;
        return fabs(vecNormal.DotProduct(this) - vecPosition.Length()) < 1e-3f;;
    }

    bool IntersectsSegmentPlane(const CVector& vecSegment, const CVector& vecNormal, const CVector& vecPosition, CVector* outVec) const noexcept
    {
        float fDist;
        CVector vecRay = vecSegment;
        vecRay.Normalize();
        bool bIntersects = IntesectsLinePlane(vecRay, vecNormal, vecPosition, &fDist);

        const float fSegLength = vecSegment.Length();

        *outVec = *this + vecRay * fDist;
        return bIntersects && fDist >= 0 && (fDist <= fSegLength);
    }

    // https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    bool IntersectsSegmentTriangle(const CVector& vecSegment, const CVector& vecVert1, const CVector& vecVert2, const CVector& vecVert3, CVector* outVec) const noexcept
    {
        constexpr float fEpsilon = 1e-6f;

        CVector vecEdge1, vecEdge2, h, s;
        float a, f, u, v;

        CVector vecRay = vecSegment;
        vecRay.Normalize();
        h = vecRay;

        vecEdge1 = vecVert2 - vecVert1;
        vecEdge2 = vecVert3 - vecVert1;

        h.CrossProduct(&vecEdge2);
        a = vecEdge1.DotProduct(&h);

        if (a > -fEpsilon && a < fEpsilon)
        {
            return false;
        }

        f = 1.0f / a;
        s = *this - vecVert1;
        u = f * s.DotProduct(&h);
        if (u < 0.0f || u > 1.0f)
        {
            return false;
        }

        CVector sCrossE1 = s;
        sCrossE1.CrossProduct(&vecEdge1);
        v = f * vecRay.DotProduct(&sCrossE1);
        if (v < 0.0f || u + v > 1.0f)
        {
            return false;
        }

        float t = f * vecEdge2.DotProduct(&sCrossE1);
        if (t > fEpsilon && t <= vecSegment.Length())
        {
            *outVec = *this + vecRay * t;
            return true;
        }
        return false;
    }

    constexpr CVector operator+(const CVector& vecRight) const noexcept { return CVector(fX + vecRight.fX, fY + vecRight.fY, fZ + vecRight.fZ); }


    constexpr CVector operator-(const CVector& vecRight) const noexcept { return CVector(fX - vecRight.fX, fY - vecRight.fY, fZ - vecRight.fZ); }

    constexpr CVector operator-() const noexcept { return CVector(-fX, -fY, -fZ); }


    constexpr CVector operator*(const CVector& vecRight) const noexcept { return CVector(fX * vecRight.fX, fY * vecRight.fY, fZ * vecRight.fZ); }

    constexpr CVector operator*(const float fRight) const noexcept { return CVector(fX * fRight, fY * fRight, fZ * fRight); }


    constexpr CVector operator/(const CVector& vecRight) const noexcept { return CVector(fX / vecRight.fX, fY / vecRight.fY, fZ / vecRight.fZ); }

    constexpr CVector operator/(const float fRight) const noexcept { return CVector(fX / fRight, fY / fRight, fZ / fRight); }

    constexpr void operator+=(const float fRight) noexcept
    {
        fX += fRight;
        fY += fRight;
        fZ += fRight;
    }

    constexpr void operator+=(const CVector& vecRight) noexcept
    {
        fX += vecRight.fX;
        fY += vecRight.fY;
        fZ += vecRight.fZ;
    }

    constexpr void operator-=(const float fRight) noexcept
    {
        fX -= fRight;
        fY -= fRight;
        fZ -= fRight;
    }

    constexpr void operator-=(const CVector& vecRight) noexcept
    {
        fX -= vecRight.fX;
        fY -= vecRight.fY;
        fZ -= vecRight.fZ;
    }

    constexpr void operator*=(const float fRight) noexcept
    {
        fX *= fRight;
        fY *= fRight;
        fZ *= fRight;
    }

    constexpr void operator*=(const CVector& vecRight) noexcept
    {
        fX *= vecRight.fX;
        fY *= vecRight.fY;
        fZ *= vecRight.fZ;
    }

    constexpr void operator/=(const float fRight) noexcept
    {
        fX /= fRight;
        fY /= fRight;
        fZ /= fRight;
    }

    constexpr void operator/=(const CVector& vecRight) noexcept
    {
        fX /= vecRight.fX;
        fY /= vecRight.fY;
        fZ /= vecRight.fZ;
    }


    inline bool operator==(const CVector& param) const noexcept
    {
        return ((fabs(fX - param.fX) < FLOAT_EPSILON) && (fabs(fY - param.fY) < FLOAT_EPSILON) && (fabs(fZ - param.fZ) < FLOAT_EPSILON));
    }

    inline bool operator!=(const CVector& param) const noexcept { return !(*this == param); }
};

export enum ePedBones
{
    BONE_PELVIS1 = 1,
    BONE_PELVIS = 2,
    BONE_SPINE1 = 3,
    BONE_UPPERTORSO = 4,
    BONE_NECK = 5,
    BONE_HEAD2 = 6,
    BONE_HEAD1 = 7,
    BONE_HEAD = 8,
    BONE_RIGHTUPPERTORSO = 21,
    BONE_RIGHTSHOULDER = 22,
    BONE_RIGHTELBOW = 23,
    BONE_RIGHTWRIST = 24,
    BONE_RIGHTHAND = 25,
    BONE_RIGHTTHUMB = 26,
    BONE_LEFTUPPERTORSO = 31,
    BONE_LEFTSHOULDER = 32,
    BONE_LEFTELBOW = 33,
    BONE_LEFTWRIST = 34,
    BONE_LEFTHAND = 35,
    BONE_LEFTTHUMB = 36,
    BONE_LEFTHIP = 41,
    BONE_LEFTKNEE = 42,
    BONE_LEFTANKLE = 43,
    BONE_LEFTFOOT = 44,
    BONE_RIGHTHIP = 51,
    BONE_RIGHTKNEE = 52,
    BONE_RIGHTANKLE = 53,
    BONE_RIGHTFOOT = 54,
};

