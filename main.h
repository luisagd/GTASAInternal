#pragma once

#include "Classes.h"
#include "mem.h"

#ifndef MAIN_H
#define MAIN_H

template<typename T>
T* ptr_add(T* cped, uint32_t n) {
	char* ptr = (char*)cped;
	ptr += n;
	return (T*)ptr;
};

struct vec3
{
	constexpr vec3():x(0.0f), y(0.0f), z(0.0f) {};
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
		return D3DXVECTOR3(x,y,z);
	}
	operator CVector() {
		return CVector(x, y, z);
	}
	float dot() const { return x * x + y * y + z * z; }
	float Length() const { return sqrtf(dot()); }
	vec3 Normalize() const { return *this * (1 / Length()); }
	float Distance(const vec3& rhs) const { return (*this - rhs).Length(); }

};

//std::ostream& operator<<(std::ostream& os, vec3 const& m);
vec3 RadToDeg(vec3& radians);

vec3 DegToRad(const vec3& degrees);

float RadToDeg(float radian);

float DegToRad(float degree);

float Magnitude(vec3 vec);

float Distance(vec3 src, vec3 dst);

struct vec2
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

bool GetD3D9Device(void** pTable, size_t Size);
HRESULT _stdcall EndSceneHook(IDirect3DDevice9* pDevice);
HRESULT _stdcall BeginSceneHook(IDirect3DDevice9* pDevice);

CPed* GetBestTarget(struct CPedPoolInfo* CPedPoolInfo, const vec3& src, const vec3& angles);
inline bool IsValidCPed(CPed* ped);

#endif