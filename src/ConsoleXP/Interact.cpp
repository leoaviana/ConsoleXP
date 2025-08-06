#include "pch.h"
#include "Interact.h"
#include "Targeting.h"

uint32_t Interact::InteractKey() 
{
    uint32_t candidate = Targeting::CollectInteractKeyTarget();

    if (candidate == -1 || candidate == 0) return 0;

    uint64_t candidateGUID = *reinterpret_cast<uint64_t*>(candidate + 0x30);
    int candidatePointer = Game::GetObjectPointer(candidateGUID);
    uint32_t candidateType = *reinterpret_cast<uint32_t*>(candidatePointer + 0x14);

    if (candidateType == ObjectType::UNIT)
    {
        Game::SetTargetInteract(candidate, candidateGUID);

    }
    else
    {
        Game::Interact(candidatePointer, 1, Offsets::FUN_RIGHT_CLICK_OBJECT);
    }

    return 1;
}

uint32_t Interact::InteractMouseOver()
{
    uint32_t bestTarget = Targeting::CollectInteractMouseOverTarget();

    if (bestTarget == -1 || bestTarget == 0) return 0;

    if (bestTarget)
    {
        uint64_t guid = *reinterpret_cast<uint64_t*>(bestTarget + 0x30);
        uint32_t type = *reinterpret_cast<uint32_t*>(bestTarget + 0x14);

        float descriptorScale = *(float*)(bestTarget + 0x9C);
        float baseScale = *(float*)(bestTarget + 0x98);
        float trueScale = descriptorScale * baseScale;

        //printf("Object: %x | Radius: %.2f | Height: %.2f | Scale: %.2f\n", bestTarget, radius, height, trueScale);

        if (type == ObjectType::UNIT)
            Game::SetTargetInteract(bestTarget, guid);
        else
            Game::Interact(bestTarget, 1, Offsets::FUN_RIGHT_CLICK_OBJECT);

        return 1;
    }

    return 0;
}