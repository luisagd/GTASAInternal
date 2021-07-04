export module vec;
#include "stdafx.h"
import classes;
//#include "Classes.h"


template<typename T>
T* ptr_add(T* cped, uint32_t n) {
	char* ptr = (char*)cped;
	ptr += n;
	return (T*)ptr;
};

export struct vec3
{
	constexpr vec3() :x(0.0f), y(0.0f), z(0.0f) {};
	constexpr vec3(float _x, float _y, float _z) :x(_x), y(_y), z(_z) {};
	union
	{
		struct { float x, y, z; };
		float v[3];
	};
	//float v[3];
	//float& operator[] (int i) { return v[i]; }
	//float& x() { return v[0]; }
	//float& y() { return v[1]; }
	//float& z() { return v[2]; }
	vec3 operator + (const vec3& rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	vec3 operator - (const vec3& rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	vec3 operator * (const float& rhs) const { return vec3(x * rhs, y * rhs, z * rhs); }
	vec3 operator / (const float& rhs) const { return vec3(x / rhs, y / rhs, z / rhs); }
	vec3& operator += (const vec3& rhs) { return *this = *this + rhs; }
	vec3& operator -= (const vec3& rhs) { return *this = *this - rhs; }
	vec3& operator *= (const float& rhs) { return *this = *this * rhs; }
	vec3& operator /= (const float& rhs) { return *this = *this / rhs; }
	operator D3DXVECTOR3() {
		return D3DXVECTOR3(x, y, z);
	}
	operator CVector() {
		return CVector(x, y, z);
	}
	float dot() const { return x * x + y * y + z * z; }
	float Length() const { return sqrtf(dot()); }
	vec3 Normalize() const { return *this * (1 / Length()); }
	float Distance(const vec3& rhs) const { return (*this - rhs).Length(); }

};

export struct vec2
{
	constexpr vec2() :x(0.0f), y(0.0f) {};
	constexpr vec2(float _x, float _y) :x(_x), y(_y) {};
	union
	{
		struct { float x, y; };
		float v[2];
	};

	vec2 operator + (const vec2& rhs) const { return vec2(x + rhs.x, y + rhs.y); }
	vec2 operator - (const vec2& rhs) const { return vec2(x - rhs.x, y - rhs.y); }
	vec2 operator * (const float& rhs) const { return vec2(x * rhs, y * rhs); }
	vec2 operator / (const float& rhs) const { return vec2(x / rhs, y / rhs); }
	vec2& operator += (const vec2& rhs) { return *this = *this + rhs; }
	vec2& operator -= (const vec2& rhs) { return *this = *this - rhs; }
	vec2& operator *= (const float& rhs) { return *this = *this * rhs; }
	vec2& operator /= (const float& rhs) { return *this = *this / rhs; }
};

export inline vec3 RadToDeg(const vec3& radians)
{
	vec3 degrees;
	degrees.x = radians.x * (180 / PI);
	degrees.y = radians.y * (180 / PI);
	degrees.z = radians.z * (180 / PI);
	return degrees;
}

export inline vec3 DegToRad(const vec3& degrees)
{
	vec3 radians;
	radians.x = degrees.x * (PI / 180);
	radians.y = degrees.y * (PI / 180);
	radians.z = degrees.z * (PI / 180);
	return radians;
}

export inline bool IsValidCPed(CPed* ped) {
	return (ped && ped->pViewMatrix && ped->pViewMatrix->w1 != 0.0f && ped->Health > 0);
}

export vec3 GetBonePosition(CPed* ped, unsigned int boneId, bool updateSkinBones)
{
	RwV3d outPosition;
	if (IsValidCPed(ped)) {
		((void(__thiscall*)(CPed*, RwV3d&, unsigned int, bool))0x5E4280)(ped, outPosition, boneId, updateSkinBones);
		return vec3(outPosition.x, outPosition.y, outPosition.z);
	}
	else
		return vec3(0, 0, 0);
}