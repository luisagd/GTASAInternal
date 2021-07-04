#include "stdafx.h"
#include "mem.h"

void mem::Patch(uint8_t* dst, uint8_t* src, uint32_t size) noexcept{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}
void mem::Patch(void* dst, void* src, uint32_t size) noexcept{
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}
void mem::Nop(uint8_t* dst, uint32_t size) noexcept {
	DWORD oldprotect;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	memset(dst, 0x90, size);
	VirtualProtect(dst, size, oldprotect, &oldprotect);
}

uintptr_t mem::FindDMAAddy(uintptr_t ptr, std::vector<uint32_t> offsets) {
uintptr_t addr = ptr;
for (auto& offset : offsets) {
		addr = *(uintptr_t*)addr;
		addr += offset;
	}
	return addr;
};

template<typename T>
T mem::FindDMAAddy2(T ptr, std::vector<uint32_t> offsets) {
	uintptr_t addr = (uintptr_t)ptr;
	for (auto& offset : offsets) {
		addr = *(uintptr_t*)addr;
		addr += offset;
	}
	return static_cast<T>(addr);
};

bool mem::Detour32(char* src, char* dst, const uintptr_t len)
{
	if (len < 5) return false;

	DWORD  curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	intptr_t  relativeAddress = (intptr_t)(dst - (intptr_t)src) - 5;

	*src = (char)'\xE9';
	*(intptr_t*)((intptr_t)src + 1) = relativeAddress;

	VirtualProtect(src, len, curProtection, &curProtection);
	return true;
}

char* mem::TrampHook32(char* src, char* dst, const uintptr_t len){
	// Make sure the length is greater than 5
	if (len < 5) return 0;

	// Create the gateway (len + 5 for the overwritten bytes + the jmp)
	void* gateway = VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	//Write the stolen bytes into the gateway
	memcpy(gateway, src, len);

	// Get the gateway to destination addy
	intptr_t  gatewayRelativeAddr = ((intptr_t)src - (intptr_t)gateway) - 5;

	// Add the jmp opcode to the end of the gateway
	*(char*)((intptr_t)gateway + len) = 0xE9;

	// Add the address to the jmp
	*(intptr_t*)((intptr_t)gateway + len + 1) = gatewayRelativeAddr;

	// Perform the detour
	Detour32(src, dst, len);

	return (char*)gateway;
}