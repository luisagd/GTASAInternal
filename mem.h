#pragma once
#include "stdafx.h"
namespace mem
{
	void Patch(uint8_t* dst, BYTE* src, uint32_t size)noexcept;
	void Patch(void* dst, void* src, uint32_t size) noexcept;
	void Nop(uint8_t* dst, uint32_t size) noexcept;
	uintptr_t FindDMAAddy(uintptr_t ptr, std::vector<uint32_t> offsets);
	//void* FindDMAAddy(uintptr_t ptr, std::vector<uint32_t> offsets);
	template<typename T>
	T FindDMAAddy2(T ptr, std::vector<uint32_t> offsets);
	bool Detour32(char* src, char* dst, const uintptr_t len);
	char* TrampHook32(char* src, char* dst, const uintptr_t len);
};



