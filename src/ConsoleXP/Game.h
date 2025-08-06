#pragma once

#include "Client.h"
#include "C3Vector.h" 
#include <string>

enum ObjectType
{
    NONE,
    ITEM,
    CONTAINER,
    UNIT,
    PLAYER,
    GAMEOBJECT,
    DYNAMICOBJECT,
    CORPSE
};

enum CreatureType
{
    CREATURE_TYPE_BEAST = 1,
    CREATURE_TYPE_DRAGONKIN = 2,
    CREATURE_TYPE_DEMON = 3,
    CREATURE_TYPE_ELEMENTAL = 4,
    CREATURE_TYPE_GIANT = 5,
    CREATURE_TYPE_UNDEAD = 6,
    CREATURE_TYPE_HUMANOID = 7,
    CREATURE_TYPE_CRITTER = 8,
    CREATURE_TYPE_MECHANICAL = 9,
    CREATURE_TYPE_NOT_SPECIFIED = 10,
    CREATURE_TYPE_TOTEM = 11,
    CREATURE_TYPE_NON_COMBAT_PET = 12,
    CREATURE_TYPE_GAS_CLOUD = 13
};

enum CreatureRank
{
    CREATURERANK_NORMAL = 0,
    CREATURERANK_ELITE = 1,
    CREATURERANK_RAREELITE = 2,
    CREATURERANK_WORLDBOSS = 3,
    CREATURERANK_RARE = 4,
    CREATURERANK_UNKNOWN = 5                      // found in 2.2.3 for 2 mobs
};

typedef enum UnitReaction
{
    Hated,
    Hostile,
    Unfriendly,
    Neutral,
    Friendly,
    Honored,
    Revered,
    Exalted
} UnitReaction;


enum MovementFlags
{
    MOVEMENTFLAG_NONE = 0x00000000,
    MOVEMENTFLAG_FORWARD = 0x00000001,
    MOVEMENTFLAG_BACKWARD = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT = 0x00000008,
    MOVEMENTFLAG_LEFT = 0x00000010,
    MOVEMENTFLAG_RIGHT = 0x00000020,
    MOVEMENTFLAG_PITCH_UP = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN = 0x00000080,
    MOVEMENTFLAG_WALKING = 0x00000100,               // Walking
    MOVEMENTFLAG_ONTRANSPORT = 0x00000200,               // Used for flying on some creatures
    MOVEMENTFLAG_DISABLE_GRAVITY = 0x00000400,               // Former MOVEMENTFLAG_LEVITATING. This is used when walking is not possible.
    MOVEMENTFLAG_ROOT = 0x00000800,               // Must not be set along with MOVEMENTFLAG_MASK_MOVING
    MOVEMENTFLAG_FALLING = 0x00001000,               // damage dealt on that type of falling
    MOVEMENTFLAG_FALLING_FAR = 0x00002000,
    MOVEMENTFLAG_PENDING_STOP = 0x00004000,
    MOVEMENTFLAG_PENDING_STRAFE_STOP = 0x00008000,
    MOVEMENTFLAG_PENDING_FORWARD = 0x00010000,
    MOVEMENTFLAG_PENDING_BACKWARD = 0x00020000,
    MOVEMENTFLAG_PENDING_STRAFE_LEFT = 0x00040000,
    MOVEMENTFLAG_PENDING_STRAFE_RIGHT = 0x00080000,
    MOVEMENTFLAG_PENDING_ROOT = 0x00100000,
    MOVEMENTFLAG_SWIMMING = 0x00200000,               // appears with fly flag also
    MOVEMENTFLAG_ASCENDING = 0x00400000,               // press "space" when flying
    MOVEMENTFLAG_DESCENDING = 0x00800000,
    MOVEMENTFLAG_CAN_FLY = 0x01000000,               // Appears when unit can fly AND also walk
    MOVEMENTFLAG_FLYING = 0x02000000,               // unit is actually flying. pretty sure this is only used for players. creatures use disable_gravity
    MOVEMENTFLAG_SPLINE_ELEVATION = 0x04000000,               // used for flight paths
    MOVEMENTFLAG_SPLINE_ENABLED = 0x08000000,               // used for flight paths
    MOVEMENTFLAG_WATERWALKING = 0x10000000,               // prevent unit from falling through water
    MOVEMENTFLAG_FALLING_SLOW = 0x20000000,               // active rogue safe fall spell (passive)
    MOVEMENTFLAG_HOVER = 0x40000000,               // hover, cannot jump

    MOVEMENTFLAG_MASK_MOVING =
    MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT |
    MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING |
    MOVEMENTFLAG_SPLINE_ELEVATION,

    MOVEMENTFLAG_MASK_TURNING =
    MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT | MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN,

    MOVEMENTFLAG_MASK_MOVING_FLY =
    MOVEMENTFLAG_FLYING | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING,

    /// @todo if needed: add more flags to this masks that are exclusive to players
    MOVEMENTFLAG_MASK_PLAYER_ONLY =
    MOVEMENTFLAG_FLYING,

    /// Movement flags that have change status opcodes associated for players
    MOVEMENTFLAG_MASK_HAS_PLAYER_STATUS_OPCODE = MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_ROOT |
    MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_WATERWALKING | MOVEMENTFLAG_FALLING_SLOW | MOVEMENTFLAG_HOVER
};


// updated offsets for 3.3.5a
enum Offsets
{
    FUN_OBJECT_POINTER =        0x004D4DB0,
    FUN_IS_IN_WORLD =           0x00BEBA40,
    FUN_LUA_CFUNCTION =         0x0084DBD0,
    FUN_LUA_TOSTRING =          0x0084E0E0,
    FUN_LUA_TONUMBER =          0x0084E030,
    FUN_LUA_PUSHBOOLEAN =       0x0084E4D0,
    FUN_LUA_PUSHNUMBER =        0x0084E2A0,
    FUN_LUA_PUSHSTRING =        0x0084E350,
    FUN_LUA_PUSHNIL =           0x0084E280,
    FUN_LUA_ISSTRING =          0x00817FD0,
    FUN_LUA_ISNUMBER =          0x0084DF20,
    FUN_LUA_SETFIELD =          0x0084E900,
    FUN_RIGHT_CLICK_UNIT =      0x00731260,
    FUN_RIGHT_CLICK_OBJECT =    0x00711140,
    FUN_SET_TARGET =            0x00524BF0,
    FUN_UNITXP =                0x0060EA60,
    VISIBLE_OBJECTS =           0x00C79CE0, // this one requires an offset.
};

struct UnkData
{
    void* unkPointer1;  // ???
    void* unkPointer2;  // ???
    uint32_t unk0;      // ???
    uint32_t unk1;       // ???
};

struct PlaySpellVisualKitData
{
    void* UnkData;           // First pointer (maybe runtime-specific or effect-related data but i'm not sure)
    void* visualKitPointer;  // Second pointer (SpellVisualKitRec DBC row)
    uint32_t effectType;     // Effect type or flags (goes from 0 to 8. not sure about all of them, 0 seems normal, while 2 and 4 seems to have additional looping logic)
    uint32_t reserved1;      // Reserved/Zero
    uint32_t reserved2;      // Reserved/Zero
    uint32_t reserved3;      // Reserved/Zero
    uint32_t unk0;           // Typically set to 1
    uint32_t reserved4;      // Reserved/Zero
    uint32_t reserved5;      // Reserved/Zero
    uint32_t unk1;           // Unsure what this is, maybe duration or special flag (e.g., 0xFFFFFFFF for indefinite)
};

enum class SpellVisualKitOffset : uint32_t
{
    PrecastKit = 0x04,            // m_precastKit
    CastKit = 0x08,               // m_castKit
    ImpactKit = 0x0C,             // m_impactKit
    StateKit = 0x10,              // m_stateKit
    StateDoneKit = 0x14,          // m_stateDoneKit
    ChannelKit = 0x18,            // m_channelKit
    CasterImpactKit = 0x38,       // m_casterImpactKit
    TargetImpactKit = 0x3C,       // m_targetImpactKit
    MissileTargetingKit = 0x58,   // m_missileTargetingKit
    InstantAreaKit = 0x5C,        // m_instantAreaKit
    ImpactAreaKit = 0x60,         // m_impactAreaKit
    PersistentAreaKit = 0x64,     // m_persistentAreaKit
};

enum class EffectKitSlot : int
{
    CastKit = 0,  // offset 0x08
    ImpactKit = 1,  // offset 0x0C
    StateKit = 2,  // offset 0x10
    PrecastKit = 4,  // offset 0x04
    CasterImpactKit = 5,  // offset 0x38
    TargetImpactKit = 6,  // offset 0x3C
    MissileTargetingKit = 7,  // offset 0x58
    StateDoneKit = 8,  // offset 0x14
};


struct MockCStatus {
    void** vtable; 
};

typedef int(__cdecl* LUA_CFUNCTION)(void* L);
typedef const char* (__cdecl* LUA_TOSTRING)(void* L, int index, int zero);
typedef double(__cdecl* LUA_TONUMBER)(void* L, int index);
typedef const bool (__cdecl* LUA_ISSTRING)(void* L, int index);
typedef bool(__cdecl* LUA_ISNUMBER)(void* L, int index);
typedef const void(__cdecl* LUA_PUSHBOOLEAN)(void* L, int boolean);
typedef void(__cdecl* LUA_PUSHNIL)(void* L);
typedef void(__cdecl* LUA_PUSHSTRING)(void* L, const char* str);
typedef void(__cdecl* LUA_PUSHNUMBER)(void* L, double n);
typedef void(__cdecl* LUA_SETTOP)(void* L, int index);
typedef int(__cdecl* LUA_LDBUFFER)(void* L, const char* buffer, int bufferLen, const char* chunkName);
typedef int(__cdecl* LUA_PCALL)(void* L, int nargs, int nresults, int err);
typedef void(__cdecl* LUA_SETFIELD)(void* L, int idx, const char* str);

namespace Game
{
    extern void* pDevice;

    bool GetScreenSize(int* outW, int* outH);
    C3Vector GetCameraPosition(); 
    C3Matrix GetCameraMatrix();
    float GetCameraFoV();
    float GetCameraNearClip();
    float GetCameraFarClip();

    template<typename DSC>
    DSC GetDescriptor(int pointer, int offset);

    bool inViewingFrustum(uint64_t playerGUID, const C3Vector& posObject, float checkCone);
    bool IsFacing(uint32_t player, uint32_t object, float angle);
    bool IsFacingMelee(uint32_t player, uint32_t object);
    bool IsFacingRanged(uint32_t player, uint32_t object);

    bool WorldToScreen(float worldX, float worldY, float worldZ, C2Vector& outScreen);

    bool Traceline(C3Vector& start, C3Vector& end, C3Vector& result, int flags);
    uint32_t __stdcall GetObjectPointer(uint64_t guid);
    C3Vector GetObjectPosition(uint32_t pointer);
    int GetUnitHealth(uint32_t unit);
    C3Vector GetUnitPosition(uint32_t unit);
    float Rotation(uint32_t pointer);
    int UnitRaidMarker(uint32_t unit);

    void Interact(uint32_t pointer, int autoloot, int fun_ptr);

    bool IsUnitLootable(uint32_t unit);
    bool IsUnitSkinnable(uint32_t unit);
    bool IsUnitInCombat(uint32_t unit);
    bool IsUnitControlledByPlayer(uint32_t unit);
    bool IsUnitDead(uint32_t unit);
    uint64_t UnitTargetGuid(uint32_t unit);
    bool UnitCanBeAttacked(uint32_t player, uint32_t target);
    CreatureType UnitCreatureType(uint32_t unit);
    CreatureRank UnitCreatureRank(uint32_t unit);
    bool IsUnitInLosTo(uint32_t unit, uint32_t target, uint32_t flags = 0x100011);
    float UnitCombatReach(uint32_t unit);
    float IsUnitMounted(uint32_t unit);

    int UnitReaction(uint32_t unit);
    int UnitMovementFlags(uint32_t unit);

    void SetTarget(uint64_t guid);
    void SetTargetInteract(uint32_t candidate, uint64_t guid); 
    void InteractUnit(uint32_t candidate, uint64_t guid);
    void InteractObject(uint32_t candidate, uint64_t guid);

    uint32_t GetVisualKitIdFromSpellId(uint32_t spellId, SpellVisualKitOffset offset);
    void PlayVisual(void* unitPtr, uint32_t visualKitId);
    void ClearEffect(void* unit, uint32_t visualKitId, EffectKitSlot slot);

    void ApplyHighlight(void* unit, uint8_t flag);
    void ClearHighlight(void* unit, uint8_t flag);

    void RenderTargetMarker(void* unit);

    void LoadBinding(const char* bindingName, const char* bindingHeaderName, const char* bindingHeaderText, const char* luaScript);

    void CastReticle();
    void ToggleIgnoreFacing(uint32_t ignore);
    float GetDeltaTime();

    bool IsGameObjectInteractable(uint32_t gameObj);
    bool IsUnitInteractable(uint32_t unit);

    inline bool IsInWorld() { return *(char*)Offsets::FUN_IS_IN_WORLD; }
}

namespace Lua
{
    const char* ToString(void* L, int index);
    const char* ToLString(void* L, int index, int zero = 0);

    double ToNumber(void* L, int index);

    int GetTop(void* L);

    bool IsString(void* L, int n_param);
    bool IsNumber(void* L, int index);

    void PushBoolean(void* L, int boolean);
    void PushNumber(void* L, double number);
    void PushString(void* L, const char* str);
    void SetField(void* L, int idx, const char* str);

    void PushNil(void* L);

    std::string GetLuaReturn(std::string command);

}