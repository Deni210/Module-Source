#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#include "Offsets/Offsets.hpp"

inline uintptr_t bitmap = *(uintptr_t*)Offsets::BitMap;

void SetFunction(const std::string& funcName);
std::string GetFunction();

__forceinline bool CheckMemory(uintptr_t address) {
    if (address < 0x10000 || address > 0x7FFFFFFFFFFF) {
        return false;
    }

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(reinterpret_cast<void*>(address), &mbi, sizeof(mbi)) == 0) {
        return false;
    }

    if (mbi.Protect & PAGE_NOACCESS || mbi.State != MEM_COMMIT) {
        return false;
    }

    return true;
}

#define PatchCFG(page) (*reinterpret_cast<uint8_t*>((bitmap) + ((page) >> 0x13)) |= (1 << (((page) >> 16) & 7)))

/*
__forceinline void PatchCFG(uintptr_t page) {
    uintptr_t byteOffset = (page >> 0x13);
    uintptr_t bitOffset = (page >> 16) & 7;

    uint8_t* cfgEntry = (uint8_t*)(bitmap + byteOffset);

    DWORD oldProtect;
    VirtualProtect(cfgEntry, 1, PAGE_READWRITE, &oldProtect);

    *cfgEntry |= (1 << bitOffset);

    VirtualProtect(cfgEntry, 1, oldProtect, &oldProtect);
}*/