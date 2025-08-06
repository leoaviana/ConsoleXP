#include "pch.h"
#include "ActionTarget.h"
#include "Log.h"


constexpr auto PLAYER_ACTIONTARGET_CHANGED = "PLAYER_ACTIONTARGET_CHANGED";
guid_t lastTarget = 0;

static guid_t getTokenGuid()
{
    return lastTarget;
}

static bool getTokenId(guid_t guid)
{
    if (!guid) return false; 
    return true;
}

static void onUpdateCallback()
{
    if (!IsInWorld()) return;

    if (lastTarget != Hooks::actionTargetGUID)
    { 
		if (lastTarget != 0)
		{ 
            Game::ClearHighlight(reinterpret_cast<void*>(Game::GetObjectPointer(lastTarget)), 2);
		}

        lastTarget = Hooks::actionTargetGUID;

        if (lastTarget != 0)
        {
            Game::ApplyHighlight(reinterpret_cast<void*>(Game::GetObjectPointer(lastTarget)), 2);
        }

        FrameScript::FireEvent(PLAYER_ACTIONTARGET_CHANGED, "");
    }
}



void ActionTarget::Initialize()
{
    Log::Write("Initializing ActionTarget event and token registration.");
    Hooks::FrameXML::registerEvent(PLAYER_ACTIONTARGET_CHANGED); 
    Hooks::FrameScript::registerToken("actiontarget", getTokenGuid, getTokenId);
    Hooks::FrameScript::registerOnUpdate(onUpdateCallback);
}