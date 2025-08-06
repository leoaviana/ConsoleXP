#include "pch.h"
#include "Highlight.h"
#include "Targeting.h"

uint64_t Highlight::lastCandidateInteract = 0;
uint64_t Highlight::lastCandidateMouseOver = 0;

uint32_t Highlight::HighlightInteract(bool enableAura, uint32_t spellId)
{
    uint32_t candidate = Targeting::CollectInteractKeyTarget();

    if (candidate == -1 || candidate == 0)
    {
        if (Highlight::lastCandidateInteract != 0)
        {
            uint32_t lastCandidatePtr = Game::GetObjectPointer(Highlight::lastCandidateInteract);

            if (lastCandidatePtr)
            {
                Game::ClearHighlight(reinterpret_cast<void*>(lastCandidatePtr), 2); //idk, 2 seems to not get removed by mouseover.
                if (enableAura)
                {
                    // If there is no candidate near, we remove the aura from the last candidate 
                    Game::ClearEffect(reinterpret_cast<void*>(lastCandidatePtr), spellId, EffectKitSlot::StateKit);
                }
            }

            Highlight::lastCandidateInteract = 0;
        }
        return 0;
    }

    uint64_t candidateGUID = *reinterpret_cast<uint64_t*>(candidate + 0x30);
    int candidatePointer = Game::GetObjectPointer(candidateGUID);
    uint32_t candidateType = *reinterpret_cast<uint32_t*>(candidatePointer + 0x14);

    if (Highlight::lastCandidateInteract == candidateGUID) return 0;

    uint32_t lastCandidatePtr = Game::GetObjectPointer(Highlight::lastCandidateInteract);

    if (lastCandidatePtr)
    {

        Game::ClearHighlight(reinterpret_cast<void*>(lastCandidatePtr), 2); //idk, 2 seems to not get removed by mouseover.
        if (enableAura)
        {
            // If the last closest object is available, we remove it's yellow aura in order to apply to the new closest candidate
            // for the default spellId 54273: PS Engineering - Arena Aura Gold we get it's visualkitid from it's StateKit

            Game::ClearEffect(reinterpret_cast<void*>(lastCandidatePtr), spellId, EffectKitSlot::StateKit);
        }
    }
    
    if (candidateType == ObjectType::UNIT)
    {
        if (enableAura)
        {
            Game::PlayVisual(reinterpret_cast<void*>(candidate), Game::GetVisualKitIdFromSpellId(spellId, SpellVisualKitOffset::StateKit));
        }
    } 


    Game::ApplyHighlight(reinterpret_cast<void*>(candidate), 2); //idk, 2 seems to not get removed by mouseover.


    Highlight::lastCandidateInteract = candidateGUID;

    return 1;
}

uint32_t Highlight::HighlightMouseOver(bool enableAura, uint32_t spellId)
{
    uint32_t candidate = Targeting::CollectInteractMouseOverTarget();

    if (candidate == -1 || candidate == 0)
    {
        if (Highlight::lastCandidateMouseOver != 0)
        {
            uint32_t lastCandidatePtr = Game::GetObjectPointer(Highlight::lastCandidateMouseOver);

            if (lastCandidatePtr)
            {
                Game::ClearHighlight(reinterpret_cast<void*>(lastCandidatePtr), 2); //idk, 2 seems to not get removed by mouseover.
                if (enableAura)
                {
                    // If there is no candidate near, we remove the aura from the last candidate 
                    Game::ClearEffect(reinterpret_cast<void*>(lastCandidatePtr), spellId, EffectKitSlot::StateKit);
                }
            }

            Highlight::lastCandidateMouseOver = 0;
        }
        return 0;
    }

    uint64_t candidateGUID = *reinterpret_cast<uint64_t*>(candidate + 0x30);
    int candidatePointer = Game::GetObjectPointer(candidateGUID);
    uint32_t candidateType = *reinterpret_cast<uint32_t*>(candidatePointer + 0x14);

    if (Highlight::lastCandidateMouseOver == candidateGUID) return 0;

    uint32_t lastCandidatePtr = Game::GetObjectPointer(Highlight::lastCandidateMouseOver);

    if (lastCandidatePtr)
    {

        Game::ClearHighlight(reinterpret_cast<void*>(lastCandidatePtr), 2); //idk, 2 seems to not get removed by mouseover.
        if (enableAura)
        {
            // If the last closest object is available, we remove it's yellow aura in order to apply to the new closest candidate
            // for the default spellId 54273: PS Engineering - Arena Aura Gold we get it's visualkitid from it's StateKit

            Game::ClearEffect(reinterpret_cast<void*>(lastCandidatePtr), spellId, EffectKitSlot::StateKit);
        }
    }

    if (candidateType == ObjectType::UNIT)
    {
        if (enableAura)
        {
            Game::PlayVisual(reinterpret_cast<void*>(candidate), Game::GetVisualKitIdFromSpellId(spellId, SpellVisualKitOffset::StateKit));
        }
    }


    Game::ApplyHighlight(reinterpret_cast<void*>(candidate), 2); //idk, 2 seems to not get removed by mouseover.


    Highlight::lastCandidateMouseOver = candidateGUID;

    return 1;
}