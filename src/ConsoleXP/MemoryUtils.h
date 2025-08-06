#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstring> 
#include <stdexcept>
#include <memoryapi.h>
#include <windows.h>

namespace MemoryUtils
{
    class DetourAddress {
    public:
        void* Pointer;
        int Size;

    private:
        void* targetInstruction;
        std::vector<uint8_t> oldInstructions;

    public:
        DetourAddress() : Pointer(nullptr), Size(0), targetInstruction(nullptr), oldInstructions({}) {}

        DetourAddress(void* ptr, int sz, void* ins, const std::vector<uint8_t>& olins)
            : Pointer(ptr), Size(sz), targetInstruction(ins), oldInstructions(olins) {
        }

        void Free() {
            // NYI
        }
    };

    void* ReadPointer(void* startAddress, const std::vector<int>& pointerOffsets, int staticOffset = 0);
    void WriteProt(void* target, const void* data, size_t dataSize);
    std::vector<uint8_t> CalculateJumpBytes(int jmpAddress, int targetInstruction, int targetInstructionLength);
    DetourAddress CreateDetour(void* targetInstruction, int targetInstructionLength, std::vector<uint8_t> injectedInstructions = {}, bool jumpBack = true, bool overwriteInstructions = true);
}

