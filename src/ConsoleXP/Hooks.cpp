#include "pch.h"
#include "Hooks.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "Targeting.h"
#include "Highlight.h"
#include "Interact.h"
#include "Camera.h" 
#include "Log.h"
#include "CVarHandler.h"
#ifdef _DEBUG
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")
#endif

HMODULE Hooks::hModule = (HMODULE)NULL;
bool Hooks::Detached = false; 
bool Hooks::enableActionTarget = false; 
bool Hooks::enableHighlightAura = false;

bool Hooks::enableHighlightInteract = true;
bool Hooks::enableHighlightMouseOver = true;

bool Hooks::ignoreCameraYaw = false;

uint32_t Hooks::highlightSpellID = 54273;
float Hooks::actionTargetingCone = 0.30f; 

uint64_t Hooks::actionTargetGUID = 0;
Camera* Hooks::Cam = new Camera();

struct CVarArgs {
    Console::CVar** dst;
    const char* name;
    const char* desc;
    Console::CVarFlags flags;
    const char* initialValue;
    Console::CVar::Handler_t func;
};

static std::vector<CVarArgs> s_customCVars;
void Hooks::FrameXML::registerCVar(Console::CVar** dst, const char* str, const char* desc, Console::CVarFlags flags, const char* initialValue, Console::CVar::Handler_t func)
{
    s_customCVars.push_back({ dst, str, desc, flags, initialValue, func });
}

typedef void(__cdecl* CVars_Initialize_Fn)();
CVars_Initialize_Fn Original_CVars_Initialize = nullptr;
CVars_Initialize_Fn CVars_Initialize_Ptr = reinterpret_cast<CVars_Initialize_Fn>(0x0051D9B0);

static std::vector<lua_CFunction> s_customLuaLibs;
void Hooks::FrameXML::registerLuaLib(lua_CFunction func) { s_customLuaLibs.push_back(func); }

typedef void(__cdecl* Lua_OpenFrameXMLApiFn)();
Lua_OpenFrameXMLApiFn oLua_OpenFrameXMLApi = nullptr;

void hkLua_OpenFrameXMLApi()
{
    lua_State* L = GetLuaState();
    for (auto& func : s_customLuaLibs)
        func(L);
}

// Hooked function
void __cdecl Hooked_CVars_Initialize()
{
    Original_CVars_Initialize();
    for (const auto& [dst, name, desc, flags, initialValue, func] : s_customCVars) {
        Console::CVar* cvar = Console::RegisterCVar(name, desc, flags, initialValue, func, 0, 0, 0, 0);
        if (dst) *dst = cvar;
    }

    hkLua_OpenFrameXMLApi(); 
}

static std::vector<const char*> s_customEvents;
void Hooks::FrameXML::registerEvent(const char* str) { s_customEvents.push_back(str); }

typedef void(__cdecl* FrameScript_FillEvents_Fn)(const char** list, size_t count);
FrameScript_FillEvents_Fn Original_FrameScript_FillEvents = nullptr;
FrameScript_FillEvents_Fn FrameScript_FillEvents_Ptr = reinterpret_cast<FrameScript_FillEvents_Fn>(0x0081B5F0);

void __cdecl Hooked_FrameScript_FillEvents(const char** list, size_t count) {
    std::vector<const char*> events;
    events.reserve(count + s_customEvents.size());
    events.insert(events.end(), &list[0], &list[count]);
    events.insert(events.end(), s_customEvents.begin(), s_customEvents.end());
    Original_FrameScript_FillEvents(events.data(), events.size());
}

struct CustomTokenDetails {
    CustomTokenDetails() { memset(this, NULL, sizeof(*this)); }
    CustomTokenDetails(Hooks::FrameScript::TokenGuidGetter* getGuid, Hooks::FrameScript::TokenIdGetter* getId)
        : hasN(false), getGuid(getGuid), getId(getId)
    {
    }
    CustomTokenDetails(Hooks::FrameScript::TokenNGuidGetter* getGuid, Hooks::FrameScript::TokenIdNGetter* getId)
        : hasN(true), getGuidN(getGuid), getIdN(getId)
    {
    }

    bool hasN;
    union {
        Hooks::FrameScript::TokenGuidGetter* getGuid;
        Hooks::FrameScript::TokenNGuidGetter* getGuidN;
    };
    union {
        Hooks::FrameScript::TokenIdGetter* getId;
        Hooks::FrameScript::TokenIdNGetter* getIdN;
    };
};
static std::unordered_map<std::string, CustomTokenDetails> s_customTokens;
void Hooks::FrameScript::registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId) { s_customTokens[token] = { getGuid, getId }; }
void Hooks::FrameScript::registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId) { s_customTokens[token] = { getGuid, getId }; }


typedef int(__cdecl* t_Script_GetGUIDFromToken)(char* token, uint64_t* outGuid, char trailing);
t_Script_GetGUIDFromToken oScript_GetGUIDFromToken = nullptr;

int __cdecl hkScript_GetGUIDFromToken(char* token, uint64_t* outGuid, char trailing)
{
    if (!token || *token == '\0') {
        if (trailing == '\0')
            return 0;

        *outGuid = *(uint64_t*)0x00bd07b0; // fallback default
        return 1;
    }

    for (const auto& [prefix, handler] : s_customTokens) {
        if (strncmp(token, prefix.c_str(), prefix.length()) == 0) {
            token += prefix.length();

            if (handler.hasN) {
                int index = atoi(token);
                *outGuid = (index > 0) ? handler.getGuidN(index - 1) : 0;
            }
            else {
                *outGuid = handler.getGuid();
            }
            return 1;
        }
    }

    // If no match, fallback to original
    return oScript_GetGUIDFromToken(token, outGuid, trailing);
}


/*
* The following code is part of the Action Targeting system, it should behave almost identical to retail wow, except in cases you call
* Auto Attack if it's already auto attacking another target, it will set the target to the action target but it will not auto attack it, 
* you must click twice...
*/

bool isInActionTargetMode = true;
static guid_t lastTarget = 0;

static void UpdateActionTargetLogic()
{
    uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
    uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
    uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
    uint32_t player = Game::GetObjectPointer(playerGUID);

    guid_t currentTarget = Game::UnitTargetGuid(player);

    if (currentTarget == 0 && lastTarget != 0)
    {
        isInActionTargetMode = true;
    }

    lastTarget = currentTarget;
}

typedef void(__cdecl* Target_t)(guid_t guid);
Target_t oTarget = nullptr;

void __cdecl hkTarget(guid_t guid)
{
    isInActionTargetMode = false;
    oTarget(guid);
}

typedef uint32_t(__thiscall* OnAttackIconPressed_t)(void* thisPtr, guid_t guid, uint32_t p3);
OnAttackIconPressed_t oOnAttackIconPressed = nullptr; 

uint32_t __fastcall hkOnAttackIconPressed(void* thisPtr, void* /*unused*/, guid_t guid, uint32_t p3)
{
    guid_t currentTarget = Game::UnitTargetGuid((uint32_t)thisPtr);

    if (Hooks::enableActionTarget && isInActionTargetMode)
    {
        if (currentTarget != Hooks::actionTargetGUID)
        {
            guid = Hooks::actionTargetGUID;
        }

        uint32_t result;
        result = oOnAttackIconPressed(thisPtr, guid, 0);

        isInActionTargetMode = true;  
        return result;
    } 

    return oOnAttackIconPressed(thisPtr, guid, 0);
}

typedef void(__cdecl* CastSpell_t)(int spellID, int itemAddr, guid_t guid, char isTrade);
CastSpell_t oCastSpell = nullptr;

static int IsHarmfulSpell(void* row, int spellID)
{ 
    uint32_t currentCastingSpell = *(uint32_t*)0x00D397D0;
    uint32_t currentAutoRepeatSpell = *(uint32_t*)0x00D397CC;

	if (spellID == currentCastingSpell || spellID == currentAutoRepeatSpell)
	{
		return 1; // Harmful spell maybe?
	}

	switch (spellID)
	{
	case 6603: // Auto Attack
	case 75:   // Auto Shot
	case 5019: // Shoot 
		return 1; // Harmful spell, since it will attack the target
	default: 
        break;
	}


    typedef int(__cdecl* IsHarmfulSpellFn)(const void* localizedRow);
    IsHarmfulSpellFn IsHarmfulSpell = reinterpret_cast<IsHarmfulSpellFn>(0x007fe1b0);
    if (IsHarmfulSpell(row) == 2) // 2 means harmful spell
    {
        return 1;
    }

    return 0;
}

void __cdecl hkCastSpell(int spellID, int itemAddr, guid_t targetID, char isTrade)
{
    // This pointer for GetLocalizedRow's Spell DB
    const void* localizedRowThis = reinterpret_cast<void*>(0x00AD49D0);

    typedef int(__thiscall* GetLocalizedRowFn)(const void* thisPtr, uint32_t spellId, void* outRow);
    GetLocalizedRowFn GetLocalizedRow = reinterpret_cast<GetLocalizedRowFn>(0x004cfd20);


    // Allocate memory for local row storage
    uint8_t localizedRow[680] = { 0 };

    // Fetch localized row using spellId
    if (!GetLocalizedRow(localizedRowThis, spellID, &localizedRow))
    {
		oCastSpell(spellID, itemAddr, targetID, isTrade);
		return;
    }


    if ((isInActionTargetMode && IsHarmfulSpell(localizedRow, spellID) == 1) && Hooks::enableActionTarget)
    {
        if (targetID != Hooks::actionTargetGUID)
        {
            targetID = Hooks::actionTargetGUID;
            oTarget(targetID);
            isInActionTargetMode = true;
        }

        oCastSpell(spellID, itemAddr, targetID, isTrade);
        isInActionTargetMode = true;
        return;
    }

    oCastSpell(spellID, itemAddr, targetID, isTrade);  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef char** (__cdecl* GetKeywordsByGuid_Fn)(uint64_t* guid, size_t* size);
GetKeywordsByGuid_Fn Original_GetKeywordsByGuid = nullptr;
GetKeywordsByGuid_Fn GetKeywordsByGuid_Ptr = reinterpret_cast<GetKeywordsByGuid_Fn>(0x0060BB70);

char** __cdecl Hooked_GetKeywordsByGuid(uint64_t* guid, size_t* size)
{
    char** buf = Original_GetKeywordsByGuid(guid, size);
    if (!buf) return buf;
    for (auto& [token, conv] : s_customTokens) {
        if (*size >= 8) break;
        if (conv.hasN) {
            int id = conv.getIdN(*guid);
            if (id >= 0)
                snprintf(buf[(*size)++], 32, "%s%d", token.c_str(), id + 1);
        }
        else {
            if (conv.getId(*guid))
                snprintf(buf[(*size)++], 32, "%s", token.c_str());
        }
    }
    return buf;
}

static std::vector<Hooks::DummyCallback_t> s_customOnUpdate;
void Hooks::FrameScript::registerOnUpdate(DummyCallback_t func) { s_customOnUpdate.push_back(func); }

typedef int(__cdecl* FrameScript_FireOnUpdate_Fn)(int a1, int a2, int a3, int a4);
FrameScript_FireOnUpdate_Fn Original_FrameScript_FireOnUpdate = nullptr;
FrameScript_FireOnUpdate_Fn FrameScript_FireOnUpdate_Ptr = reinterpret_cast<FrameScript_FireOnUpdate_Fn>(0x00495810);

int __cdecl Hooked_FrameScript_FireOnUpdate(int a1, int a2, int a3, int a4)
{
    for (auto func : s_customOnUpdate)
        func();
    return Original_FrameScript_FireOnUpdate(a1, a2, a3, a4);
}  

/////////////////////////////////////////////////// ConsoleXP Hooks //////////////////////////////////////////////////////////

typedef HRESULT(__stdcall* EndSceneFn)(IDirect3DDevice9* pDevice);
EndSceneFn oEndScene = nullptr;
HWND tempWnd = NULL;

typedef void(__fastcall* FUN_004f6f90_Fn)(int param_1);
FUN_004f6f90_Fn Original_FUN_004f6f90 = nullptr;
FUN_004f6f90_Fn PriorHook_FUN_004f6f90 = nullptr;


static int frameCount = 0;
static int loadedBinds = 0;

/// <summary>
/// The original function is part of the CGWorldFrame_Render, Hooking it so I can pass an additional CGUnit::virt50() which is
/// responsible for rendering target markers. 
/// </summary>
/// <param name="param_1">Unknown parameter. better not to mess with it</param>
/// <returns></returns>
void __fastcall Hooked_FUN_004f6f90(int param_1) { 
	
	if (PriorHook_FUN_004f6f90)
		PriorHook_FUN_004f6f90(param_1); 
    else 
		Original_FUN_004f6f90(param_1); 

    if (Hooks::actionTargetGUID != 0 && isInActionTargetMode)
    {
        Game::RenderTargetMarker(reinterpret_cast<void*>(Game::GetObjectPointer(Hooks::actionTargetGUID)));
    }

}  

/*

The next two functions are responsible for rendering the action target names over their heads, finding this out was a pain in the ass. took me
days because rendering names relies on alot functions and numerous checks, and finding the correct ones was very hard.

*/

typedef void(__cdecl* FUN_007e6390_Fn)(int param_1);
FUN_007e6390_Fn Original_FUN_007e6390 = nullptr;

typedef void(__cdecl* FUN_007e6090_Fn)(void* param1, void* param2, void* param3);
FUN_007e6090_Fn FUN_007e6090 = (FUN_007e6090_Fn)0x007e6090;
 
typedef void(__thiscall* CGUnitVirtB0Fn)(int thisPointer);

void __cdecl Hooked_FUN_007e6390(int param_1) {
    // Call the original function first to ensure proper setup.
    Original_FUN_007e6390(param_1);

    // Retrieve the GUID from the parameters.
    uint32_t guidLow = *(uint32_t*)(param_1 + 0x10);
    uint32_t guidHigh = *(uint32_t*)(param_1 + 0x14);

    // Combine the high and low parts into a 64-bit GUID.
    guid_t fullGuid = (static_cast<uint64_t>(guidHigh) << 32) | guidLow;

    int* unit = (int*)Game::GetObjectPointer(fullGuid);

    // Some of the following code was taken from ghidra, I can't explain what it does but 
    // It will do what normally a PlayerName object would require to be displayed.
    if (unit) {
        if (Hooks::actionTargetGUID == fullGuid && isInActionTargetMode) {
            int iVar1 = unit[0x2d];
            if (iVar1) {

                *(uint32_t*)(param_1 + 0x1c) = 0x00d380a4;

                if ((*(uint32_t*)(param_1 + 0x18) & 8) == 0) {
                    *(uint32_t*)(param_1 + 0x18) |= 8;

                    CGUnitVirtB0Fn virtB0 = (CGUnitVirtB0Fn)0x00715560;
                    virtB0((int)unit);
                }

                *(uint32_t*)(iVar1 + 0x10) &= 0xffffffdf;

                *(void**)(iVar1 + 0x1bc) = (void*)FUN_007e6090;
                *(void**)(iVar1 + 0x1c0) = (void*)param_1;
            }
        }
    }
}

typedef int(__thiscall* CGUnitVirtB8Fn)(int* param_1, uint32_t param_2);
CGUnitVirtB8Fn Original_CGUnitVirtB8 = nullptr;

int __fastcall Hooked_CGUnitVirtB8(int* param_1, void* unused, uint32_t param_2) {
    uint64_t guid = *reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(param_1) + 0x30);

    // Check if the unit is the action target.
    if (guid == Hooks::actionTargetGUID && isInActionTargetMode) {
        return 1; // This indicates that the action target name should be rendered. the original function will do more checks but this is enough.
    }

    // Call the original function for all other units.
    return Original_CGUnitVirtB8(param_1, param_2);
}

/// <summary>
/// We detour endscene so we can call specific functions every frame, to check targets, load binds, etc.
/// </summary>
/// <param name="pDevice"></param>
/// <returns></returns>
/// 
/// 
/// 
/// 
#ifdef _DEBUG
void DebugDrawVisibleObjects(IDirect3DDevice9* device)
{
    if (!device) return;

    // Get screen size
    int screenW, screenH;
    Game::GetScreenSize(&screenW, &screenH);

    float centerX = screenW / 2.0f;
    float centerY = screenH / 2.0f;

    ID3DXLine* line = nullptr;
    if (FAILED(D3DXCreateLine(device, &line))) return;

    // Get camera info (not directly used here but can help debug projection)
    C3Vector camPos = Game::GetCameraPosition();

    uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
    uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
    uint32_t current = *reinterpret_cast<uint32_t*>(objects + 0xAC);

    uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
    uint32_t player = Game::GetObjectPointer(playerGUID);

    C3Vector pPos = Game::GetUnitPosition(player);
    C3Vector oPos;

    while (current && !(current & 1))
    {
        uint32_t type = *reinterpret_cast<uint32_t*>(current + 0x14);
        if ((type == ObjectType::UNIT || type == ObjectType::GAMEOBJECT))
        {
            C3Vector worldPos = Game::GetObjectPosition(current);

            //bounding box radius and height default values
            float radius = 0.5f;
            float height = 2.0f;

            float descriptorScale = *(float*)(current + 0x9C);
            float baseScale = *(float*)(current + 0x98);
            float trueScale = descriptorScale * baseScale;


            oPos = Game::GetObjectPosition(current);

            uint64_t summonedByGUID = *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint32_t*>(current + 0x8) + 0x30);
            uint32_t summonedBy = Game::GetObjectPointer(summonedByGUID);

            if (summonedByGUID != 0 && summonedBy != 0)
            {
                uint32_t owner = *reinterpret_cast<uint32_t*>(summonedBy + 0x14);
                if (owner == ObjectType::PLAYER)
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }
            }



            if (!Game::IsFacingMelee(player, current) || distance3D(oPos, pPos) > 5.0)
            {
                current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                continue;
            }

            if (type == ObjectType::GAMEOBJECT)
            {
                if (!Game::IsUnitInLosTo(player, current)) // standard check
                {
                    if (!Game::IsUnitInLosTo(player, current, 0x00000011))
                    { 
                        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                        continue; 
                    } 
                }

                if (!Game::IsGameObjectInteractable(current))
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }

                uintptr_t model = *(uintptr_t*)(current + 0xB4);               // CM2Model*
                if (model)
                {
                    uintptr_t renderInfo = *(uintptr_t*)(model + 0x2C);         // maybe CM2Scene or something
                    if (renderInfo)
                    {
                        uintptr_t boundsStruct = *(uintptr_t*)(renderInfo + 0x150); // struct with radius/height

                        if (boundsStruct)
                        {
                            radius = *(float*)(boundsStruct + 0xB0) * trueScale;
                            height = *(float*)(boundsStruct + 0xB4) * trueScale;
                        }
                    }
                }
            }
            else
            { 
                if (!Game::IsUnitInLosTo(player, current))
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }

                if (!Game::IsUnitInteractable(current))
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }


                uint32_t movement = *reinterpret_cast<uint32_t*>(current + 0xD8); // `CMovement` offset
                if (movement)
                {
                    radius = *reinterpret_cast<float*>(movement + 0xC8) * trueScale;
                    height = *reinterpret_cast<float*>(movement + 0xCC) * trueScale;
                } 

                uintptr_t model = *(uintptr_t*)(current + 0x970);               // ???
                if (model)
                {
                    radius = ((*(float*)(model + 0x54)) * (*(float*)(model + 0x220))) * trueScale;
                    height = (((*(float*)(model + 0x58)) * (*(float*)(model + 0x220))) * (*(float*)(model + 0xD0))) * trueScale;
                }

                if (Game::IsUnitDead(current))
                { 
                    height /= 4;
                }
            }

            // Define 4 corners (top and bottom of cylinder) in 3D
            C3Vector topCorners[4] = {
                { worldPos.x - radius, worldPos.y - radius, worldPos.z + height },
                { worldPos.x + radius, worldPos.y - radius, worldPos.z + height },
                { worldPos.x + radius, worldPos.y + radius, worldPos.z + height },
                { worldPos.x - radius, worldPos.y + radius, worldPos.z + height }
            };
            C3Vector bottomCorners[4] = {
                { worldPos.x - radius, worldPos.y - radius, worldPos.z },
                { worldPos.x + radius, worldPos.y - radius, worldPos.z },
                { worldPos.x + radius, worldPos.y + radius, worldPos.z },
                { worldPos.x - radius, worldPos.y + radius, worldPos.z }
            };

            // Project to screen
            C2Vector screenTop[4];
            C2Vector screenBottom[4];
            bool visible = true;

            for (int i = 0; i < 4; ++i)
            {
                if (!Game::WorldToScreen(topCorners[i].x, topCorners[i].y, topCorners[i].z, screenTop[i]) ||
                    !Game::WorldToScreen(bottomCorners[i].x, bottomCorners[i].y, bottomCorners[i].z, screenBottom[i]))
                {
                    visible = false;
                    break;
                }
            }

            if (visible)
            {
                // Construct line segments (draw top, bottom, verticals)
                D3DXVECTOR2 lines[16] = {
                    { screenTop[0].x, screenTop[0].y }, { screenTop[1].x, screenTop[1].y },
                    { screenTop[1].x, screenTop[1].y }, { screenTop[2].x, screenTop[2].y },
                    { screenTop[2].x, screenTop[2].y }, { screenTop[3].x, screenTop[3].y },
                    { screenTop[3].x, screenTop[3].y }, { screenTop[0].x, screenTop[0].y },

                    { screenBottom[0].x, screenBottom[0].y }, { screenBottom[1].x, screenBottom[1].y },
                    { screenBottom[1].x, screenBottom[1].y }, { screenBottom[2].x, screenBottom[2].y },
                    { screenBottom[2].x, screenBottom[2].y }, { screenBottom[3].x, screenBottom[3].y },
                    { screenBottom[3].x, screenBottom[3].y }, { screenBottom[0].x, screenBottom[0].y }
                };

                line->Begin();
                line->Draw(lines, 16, D3DCOLOR_ARGB(255, 0, 255, 0)); // Green
                line->End();

                // Draw vertical lines connecting top and bottom
                D3DXVECTOR2 verticals[8] = {
                    { screenTop[0].x, screenTop[0].y }, { screenBottom[0].x, screenBottom[0].y },
                    { screenTop[1].x, screenTop[1].y }, { screenBottom[1].x, screenBottom[1].y },
                    { screenTop[2].x, screenTop[2].y }, { screenBottom[2].x, screenBottom[2].y },
                    { screenTop[3].x, screenTop[3].y }, { screenBottom[3].x, screenBottom[3].y }
                };

                line->Begin();
                line->Draw(verticals, 8, D3DCOLOR_ARGB(255, 0, 255, 0));
                line->End();
            }
        }
        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
    }

    // Draw crosshair
    D3DXVECTOR2 crosshairLines[4] = {
        { centerX - 3, centerY },
        { centerX + 3, centerY },
        { centerX, centerY - 3 },
        { centerX, centerY + 3 }
    };

    line->Begin();
    line->Draw(&crosshairLines[0], 2, D3DCOLOR_ARGB(255, 255, 255, 255));
    line->Draw(&crosshairLines[2], 2, D3DCOLOR_ARGB(255, 255, 255, 255));
    line->End();

    line->Release();
}
#endif

HRESULT __stdcall detoured_EndScene(IDirect3DDevice9* pDevice)
{
    if (Game::pDevice != pDevice)
    {
        Game::pDevice = pDevice;
    } 

    if (!Hooks::Cam->initialized)
    {
        Hooks::Cam->Initialize();
    }

    if (Game::IsInWorld())
    {
#ifdef _DEBUG
        IDirect3DStateBlock9* stateBlock = nullptr;
        if (SUCCEEDED(pDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock)))
        {
            stateBlock->Capture();
        }

        DebugDrawVisibleObjects(pDevice);

        if (stateBlock)
        {
            stateBlock->Apply();
            stateBlock->Release();
        }
#endif
        if (loadedBinds == 0)
        {
            //
            // This code will create keybindings in our client without the need of an addOn or modifying FrameXML files directly, this helps prevent
            // taint issues because some functions like interact or settarget cannot be called from an addOn.
            //

            Game::LoadBinding("CXPINTERACT", "Interact", "CONSOLEXP", "ConsoleXP Enhancements", "C_ConsoleXP.InteractNearest()"); //"UnitXP(\"interact\", 1)");
            Game::LoadBinding("CXPMOUSEOVER", "Interact with MouseOver", "", "ConsoleXP Enhancements", "C_ConsoleXP.InteractMouseOver()"); //"UnitXP(\"interact\", 1)");
            Game::LoadBinding("CXPNEARESTENEMY", "Target Nearest Enemy", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetNearestEnemy()"); //"UnitXP(\"target\", \"nearestEnemy\")");
            Game::LoadBinding("CXPWORLDBOSS", "Target World Boss", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetWorldBoss()"); //"UnitXP(\"target\", \"worldBoss\")");
            Game::LoadBinding("CXPNEXTENEMY", "Target Next Enemy", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetNextEnemyInCycle()"); //"UnitXP(\"target\", \"nextEnemyInCycle\")");
            Game::LoadBinding("CXPPREVENEMY", "Target Previous Enemy", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetPreviousEnemyInCycle()"); //"UnitXP(\"target\", \"previousEnemyInCycle\")");
            Game::LoadBinding("CXPNEXTENEMYPRIMELEE", "Target Next Enemy Prioritizing Melee", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetNextEnemyConsideringDistance()"); //"UnitXP(\"target\", \"nextEnemyConsideringDistance\")");
            Game::LoadBinding("CXPPREVENEMYPRIMELEE", "Target Previous Enemy Prioritizing Melee", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetPreviousEnemyConsideringDistance()"); //"UnitXP(\"target\", \"previousEnemyConsideringDistance\")");
            Game::LoadBinding("CXPNEXTTARGETMARKER", "Next Target Marker", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetNextMarkedEnemyInCycle()"); //"UnitXP(\"target\", \"nextMarkedEnemyInCycle\")");
            Game::LoadBinding("CXPPREVTARGETMARKER", "Previous Target Marker", "", "ConsoleXP Enhancements", "C_ConsoleXP.TargetPreviousMarkedEnemyInCycle()"); //"UnitXP(\"target\", \"previousMarkedEnemyInCycle\")");

            loadedBinds++;
        }

        UpdateActionTargetLogic();

        if (frameCount == 14)
        {
            if (Hooks::enableActionTarget && isInActionTargetMode)
            {
                if (!Targeting::TargetNearestEnemyFacing(41.f, &Hooks::actionTargetGUID, Hooks::actionTargetingCone))
                {
                    Hooks::actionTargetGUID = 0;
                }
            }
            else
            {
                Hooks::actionTargetGUID = 0;
            }

            if (Hooks::enableHighlightInteract)
            {
                Highlight::HighlightInteract(Hooks::enableHighlightAura, Hooks::highlightSpellID);
            }

            if (Hooks::enableHighlightMouseOver)
            {
                Highlight::HighlightMouseOver(Hooks::enableHighlightAura, Hooks::highlightSpellID);
            }

            frameCount = 0;
        }
        else
        {
            frameCount++;
        }
    }
    else
    {
        loadedBinds = 0;
    }


    Hooks::Cam->Direct3D_EndScene();

    // Call original EndScene
    return oEndScene(pDevice);
}


BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId = 0;
    GetWindowThreadProcessId(handle, &wndProcId);

    if (GetCurrentProcessId() != wndProcId)
        return TRUE;

    tempWnd = handle;
    return FALSE;
}

HWND GetProcessWindow()
{
    tempWnd = (HWND)NULL;
    EnumWindows(EnumWindowsCallback, NULL);
    return tempWnd;
}

// Hook initialization
DWORD WINAPI HookEndScene()
{
    // Create a dummy Direct3D9 device
    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) return false;

    IDirect3DDevice9* pDevice = nullptr;
    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = GetProcessWindow();

    // Create a temporary device
    if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice) != D3D_OK)
    {
        d3dpp.Windowed = !d3dpp.Windowed;

        if (pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice) != D3D_OK)
        {
            pD3D->Release();
            return false;
        }
    }

    // Get EndScene function pointer
    void** vtable = *reinterpret_cast<void***>(pDevice);
    void* pEndScene = vtable[42];

    // Clean up temporary device
    pDevice->Release();
    pD3D->Release();

    // Hook EndScene with MinHook
    if (MH_CreateHook(pEndScene, &detoured_EndScene, reinterpret_cast<LPVOID*>(&oEndScene)) != MH_OK)
    {
        return false;
    }

    if (MH_EnableHook(pEndScene) != MH_OK)
    {
        return false;
    }

    return true;
}

void Hooks::WaitForEndSceneHook()
{
    while (true)
    {
        if (HookEndScene())
        {
            Log::Write("Successfully hooked EndScene");
            break;
        }

        Log::Write("EndScene not ready, retrying in 1 second...");
        Sleep(1000);
    }
}

int Hooks::Initialize()
{

    MH_STATUS status;

    // CVars_Initialize_orig
    status = MH_CreateHook(reinterpret_cast<LPVOID>(CVars_Initialize_Ptr),
        reinterpret_cast<LPVOID>(&Hooked_CVars_Initialize),
        reinterpret_cast<LPVOID*>(&Original_CVars_Initialize));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: CVars_Initialize_orig (Error: %s)", MH_StatusToString(status)); 
    }

    // FrameScript_FireOnUpdate
    status = MH_CreateHook(reinterpret_cast<LPVOID>(FrameScript_FireOnUpdate_Ptr),
        reinterpret_cast<LPVOID>(&Hooked_FrameScript_FireOnUpdate),
        reinterpret_cast<LPVOID*>(&Original_FrameScript_FireOnUpdate));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: FrameScript_FireOnUpdate (Error: %s)", MH_StatusToString(status));
    }

    // FrameScript_FillEvents
    status = MH_CreateHook(reinterpret_cast<LPVOID>(FrameScript_FillEvents_Ptr),
        reinterpret_cast<LPVOID>(&Hooked_FrameScript_FillEvents),
        reinterpret_cast<LPVOID*>(&Original_FrameScript_FillEvents));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: FrameScript_FillEvents (Error: %s)", MH_StatusToString(status));
    }

    // GetKeywordsByGuid
    status = MH_CreateHook(reinterpret_cast<LPVOID>(GetKeywordsByGuid_Ptr),
        reinterpret_cast<LPVOID>(&Hooked_GetKeywordsByGuid),
        reinterpret_cast<LPVOID*>(&Original_GetKeywordsByGuid));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: GetKeywordsByGuid (Error: %s)", MH_StatusToString(status));
    }

    // Script_GetGUIDFromToken
    status = MH_CreateHook(reinterpret_cast<void*>(0x0060abf0), &hkScript_GetGUIDFromToken, reinterpret_cast<void**>(&oScript_GetGUIDFromToken));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: Script_GetGUIDFromToken (Error: %s)", MH_StatusToString(status));
    }

    // FUN_004f6f90
    status = MH_CreateHook(reinterpret_cast<void*>(0x004f6f90),
        reinterpret_cast<void*>(&Hooked_FUN_004f6f90),
        reinterpret_cast<void**>(&Original_FUN_004f6f90));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: FUN_004f6f90 (Error: %s)", MH_StatusToString(status));
    } 

    // FUN_007e6390
    status = MH_CreateHook(reinterpret_cast<void*>(0x007e6390),
        reinterpret_cast<void*>(&Hooked_FUN_007e6390),
        reinterpret_cast<void**>(&Original_FUN_007e6390));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: FUN_007e6390 (Error: %s)", MH_StatusToString(status));
    }
     
    // CGUnitVirtB8
    status = MH_CreateHook(reinterpret_cast<void*>(0x00729c70),
        reinterpret_cast<void*>(&Hooked_CGUnitVirtB8),
        reinterpret_cast<void**>(&Original_CGUnitVirtB8));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: CGUnitVirtB8 (Error: %s)", MH_StatusToString(status));
    }

    /*// UnitXP
    status = MH_CreateHook(p_UnitXP, &detoured_UnitXP, reinterpret_cast<LPVOID*>(&p_original_UnitXP));
    if (status != MH_OK) {
        printf("Failed to Create Hook: UnitXP (Error: %s)\n", MH_StatusToString(status));
    }
    */

    // CGGameUI::Target
    status = MH_CreateHook(reinterpret_cast<void*>(0x00524bf0), &hkTarget, reinterpret_cast<void**>(&oTarget));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: CGGameUI::Target (Error: %s)", MH_StatusToString(status));
    }

    // Spell_C::CastSpell
    status = MH_CreateHook(reinterpret_cast<void*>(0x0080da40), &hkCastSpell, reinterpret_cast<void**>(&oCastSpell));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: Spell_C::CastSpell (Error: %s)", MH_StatusToString(status));
    }

    // CGPlayer_C::OnAttackIconPressed 
    status = MH_CreateHook(reinterpret_cast<void*>(0x0072c2b0), reinterpret_cast<void*>(&hkOnAttackIconPressed), reinterpret_cast<void**>(&oOnAttackIconPressed));
    if (status != MH_OK) {
        Log::Write("Failed to Create Hook: CGPlayer_C::OnAttackIconPressed (Error: %s)", MH_StatusToString(status));
    }

    // Enable every hook now.

    status = MH_EnableHook(reinterpret_cast<LPVOID>(CVars_Initialize_Ptr));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: CVars_Initialize_orig (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(FrameScript_FireOnUpdate_Ptr));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: Original_FrameScript_FireOnUpdate (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(FrameScript_FillEvents_Ptr));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: Original_FrameScript_FillEvents (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<LPVOID>(GetKeywordsByGuid_Ptr));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: Original_GetKeywordsByGuid (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<void*>(0x0060abf0));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: Script_GetGUIDFromToken (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<void*>(0x004f6f90));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: 0x004f6f90 (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<void*>(0x007e6390));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: 0x007e6390 (Error: %s)", MH_StatusToString(status));
    }


    status = MH_EnableHook(reinterpret_cast<void*>(0x00729c70));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: CGUnitVirtB8 (Error: %s)", MH_StatusToString(status));
    }

    /*
    status = MH_EnableHook(p_UnitXP);
    if (status != MH_OK) {
        printf("Failed to Enable Hook: p_UnitXP (Error: %s)\n", MH_StatusToString(status));
    }
    */

    status = MH_EnableHook(reinterpret_cast<void*>(0x00524bf0));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: CGGameUI::Target (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<void*>(0x0080da40));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: CGGameUI::Target (Error: %s)", MH_StatusToString(status));
    }

    status = MH_EnableHook(reinterpret_cast<void*>(0x0072c2b0));
    if (status != MH_OK) {
        Log::Write("Failed to Enable Hook: CGPlayer_C::OnAttackIconPressed  (Error: %s)", MH_StatusToString(status));
    }

    return 0;

}