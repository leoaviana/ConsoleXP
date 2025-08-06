#include "pch.h"
#include "MemoryUtils.h" 

void* MemoryUtils::ReadPointer(void* startAddress, const std::vector<int>& pointerOffsets, int staticOffset) {
    void* currentAddress = startAddress;

    __try {
        for (const auto& pointerOffset : pointerOffsets) {
            int pointer = *reinterpret_cast<int*>(currentAddress) + pointerOffset;
            currentAddress = reinterpret_cast<void*>(pointer);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
    return static_cast<char*>(currentAddress) + staticOffset;
}

void MemoryUtils::WriteProt(void* target, const void* data, size_t dataSize) {
    DWORD oldProtect;
    VirtualProtect(target, dataSize, PAGE_EXECUTE_READWRITE, &oldProtect);
    std::memcpy(target, data, dataSize);
    VirtualProtect(target, dataSize, oldProtect, &oldProtect);
}

// https://www.unknowncheats.me/forum/c-/322655-unmanaged-code-caves-detours.html

std::vector<uint8_t> MemoryUtils::CalculateJumpBytes(int jmpAddress, int targetInstruction, int targetInstructionLength) {
    if (targetInstructionLength < 5) {
        throw std::runtime_error("targetInstructionLength cannot be less than 5.");
    }

    // Gets the bytes of the JMP instruction
    std::vector<uint8_t> jmpBytes = { 0xE9 };

    // Calculates the address that gets jumped to
    int jmpOperand = jmpAddress - targetInstruction - targetInstructionLength;

    // Adds the target jump address to the jump instruction and returns the completed instruction bytes
    std::vector<uint8_t> bytes(jmpBytes);
    std::vector<uint8_t> operandBytes(reinterpret_cast<uint8_t*>(&jmpOperand), reinterpret_cast<uint8_t*>(&jmpOperand) + sizeof(jmpOperand));
    bytes.insert(bytes.end(), operandBytes.begin(), operandBytes.end());

    // NOPs out unwanted Opcodes
    while (bytes.size() < targetInstructionLength) {
        bytes.push_back(0x90);
    }

    if (bytes.size() < targetInstructionLength) {
        throw std::runtime_error("targetInstructionLength is too short");
    }
    return bytes;
}

MemoryUtils::DetourAddress MemoryUtils::CreateDetour(void* targetInstruction, int targetInstructionLength, std::vector<uint8_t> injectedInstructions, bool jumpBack, bool overwriteInstructions) {
    if (sizeof(void*) == 8) {
        throw std::runtime_error("Detouring x64 functions is not currently supported");
    }

    if (targetInstructionLength < 5) {
        throw std::runtime_error("targetInstructionLength cannot be less than 5");
    }

    void* allocation = nullptr;
    int allocationSize = 0;

    if (!injectedInstructions.empty() || jumpBack) {
        if (!injectedInstructions.empty()) {
            allocationSize += injectedInstructions.size();
        }

        if (jumpBack) {
            allocationSize += 5;
        }

        if (!overwriteInstructions) {
            allocationSize += targetInstructionLength;
            std::vector<uint8_t> instructions(targetInstructionLength);
            std::memcpy(instructions.data(), targetInstruction, targetInstructionLength);
            if (injectedInstructions.empty()) {
                injectedInstructions = std::move(instructions);
            }
            else {
                // Adds the bytes of the overwritten instruction to the bottom 
                injectedInstructions.insert(injectedInstructions.end(), instructions.begin(), instructions.end());
            }
        }
    }
    else {
        throw std::runtime_error("No memory being injected");
    }

    // allocates new memory for the code cave to be written to
    allocation = VirtualAlloc(nullptr, allocationSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (allocation == nullptr)
    {
        throw std::runtime_error("memory allocation failed");
    }

    if (jumpBack) {
        // calculates the bytes for a jmp instruction back out of the code cave
        std::vector<uint8_t> jmpBytes = CalculateJumpBytes(reinterpret_cast<uintptr_t>(targetInstruction) + targetInstructionLength, reinterpret_cast<uintptr_t>(allocation) + allocationSize - 5, 5);
        if (injectedInstructions.empty()) {
            injectedInstructions = std::move(jmpBytes);
        }
        else {
            injectedInstructions.insert(injectedInstructions.end(), jmpBytes.begin(), jmpBytes.end());
        }
    }

    // writes the instructions to the codecave
    std::memcpy(allocation, injectedInstructions.data(), injectedInstructions.size());

    // calculates the bytes for a jmp into the code cave
    std::vector<uint8_t> jmpIntoCodeCave = CalculateJumpBytes(reinterpret_cast<uintptr_t>(allocation) + targetInstructionLength - 5, reinterpret_cast<uintptr_t>(targetInstruction), targetInstructionLength);

    // stores the oldInstructions to easily free the detour later.
    std::vector<uint8_t> oldInstructions(targetInstructionLength);
    std::memcpy(oldInstructions.data(), targetInstruction, targetInstructionLength);

    // overwrites the target instruction with a jmp to the code cave
    WriteProt(targetInstruction, jmpIntoCodeCave.data(), jmpIntoCodeCave.size());

    DWORD oldProtection;
    VirtualProtect(allocation, allocationSize, PAGE_EXECUTE_READ, &oldProtection);

    return DetourAddress(allocation, allocationSize, targetInstruction, oldInstructions);
}