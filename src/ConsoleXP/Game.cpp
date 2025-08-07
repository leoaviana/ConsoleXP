#include "pch.h"
#include "Game.h"
#include <map>
#include <algorithm>
#include <chrono>
#include <d3d9.h>
//#include <d3dx9.h>
#include "MathF.h"
#include "Log.h"

#define LUA_GLOBALSINDEX	(-10002)

LUA_CFUNCTION p_lua_gettop = reinterpret_cast<LUA_CFUNCTION>(Offsets::FUN_LUA_CFUNCTION);
LUA_TOSTRING p_lua_tostring = reinterpret_cast<LUA_TOSTRING>(Offsets::FUN_LUA_TOSTRING);
LUA_TONUMBER p_lua_tonumber = reinterpret_cast<LUA_TONUMBER>(Offsets::FUN_LUA_TONUMBER);
LUA_ISSTRING p_lua_isstring = reinterpret_cast<LUA_ISSTRING>(Offsets::FUN_LUA_ISSTRING);
LUA_ISNUMBER p_lua_isnumber = reinterpret_cast<LUA_ISNUMBER>(Offsets::FUN_LUA_ISNUMBER);
LUA_PUSHBOOLEAN p_lua_pushboolean = reinterpret_cast<LUA_PUSHBOOLEAN>(Offsets::FUN_LUA_PUSHBOOLEAN);
LUA_PUSHNUMBER p_lua_pushnumber = reinterpret_cast<LUA_PUSHNUMBER>(Offsets::FUN_LUA_PUSHNUMBER);
LUA_PUSHSTRING p_lua_pushstring = reinterpret_cast<LUA_PUSHSTRING>(Offsets::FUN_LUA_PUSHSTRING);
LUA_PUSHNIL p_lua_pushnil = reinterpret_cast<LUA_PUSHNIL>(Offsets::FUN_LUA_PUSHNIL);
LUA_SETFIELD p_lua_setfield = reinterpret_cast<LUA_SETFIELD>(Offsets::FUN_LUA_SETFIELD);

typedef uint32_t(__cdecl* GETACTIVECAMERA)(void);
GETACTIVECAMERA p_getCamera = reinterpret_cast<GETACTIVECAMERA>(0x4F5960);

struct TargetData {
    uint64_t targetGUID;
    uint32_t candidate;
};

void* Game::pDevice = NULL;

// Map to store the target data
std::map<UINT_PTR, TargetData> timerData;


namespace Game
{ 
    bool GetScreenSize(int* outW, int* outH) {
        if (!outW || !outH) return false;

        if (Game::pDevice == NULL) return false;

        IDirect3DDevice9* device = (IDirect3DDevice9*)Game::pDevice;
        if (!device) return false;

        D3DVIEWPORT9 viewport = {};
        if (FAILED(device->GetViewport(&viewport))) return false;

        *outW = static_cast<int>(viewport.Width);
        *outH = static_cast<int>(viewport.Height);
        return true;
    }

    ///////////////////////////////// Camera and Sight related stuff ////////////////////////////////
    C3Vector GetCameraPosition() {
        uint32_t cptr = p_getCamera();
        if (cptr == 0) {
            C3Vector nullResult = {};
            return nullResult;
        }

        float* positionPtr = reinterpret_cast<float*>(cptr + 0x8);
        C3Vector result = {};
        result.x = positionPtr[0];
        result.y = positionPtr[1];
        result.z = positionPtr[2];

        return result;
    }

    C3Matrix Game::GetCameraMatrix() {
        C3Matrix mat = {};
        uint32_t cam = p_getCamera();
        if (!cam) return mat;

        float* matrixPtr = reinterpret_cast<float*>(cam + 0x14);
        if (!matrixPtr) return mat;

        mat.m11 = matrixPtr[0];
        mat.m12 = matrixPtr[1];
        mat.m13 = matrixPtr[2];

        mat.m21 = matrixPtr[3];
        mat.m22 = matrixPtr[4];
        mat.m23 = matrixPtr[5];

        mat.m31 = matrixPtr[6];
        mat.m32 = matrixPtr[7];
        mat.m33 = matrixPtr[8];

        return mat;
    }

    float Game::GetCameraFoV() {
        uint32_t cam = p_getCamera();
        if (!cam) return 0.0f;

        // Field of view is at offset 0x40
        return *reinterpret_cast<float*>(cam + 0x40);
    }

    float Game::GetCameraNearClip() {
        uint32_t cam = p_getCamera();
        if (!cam) return 0.0f;

        // Near clip plane at offset 0x38
        return *reinterpret_cast<float*>(cam + 0x38);
    }

    float Game::GetCameraFarClip() {
        uint32_t cam = p_getCamera();
        if (!cam) return 0.0f;

        // Far clip plane at offset 0x3C
        return *reinterpret_cast<float*>(cam + 0x3C);
    }


    bool inViewingFrustum(uint64_t playerGUID, const C3Vector& posObject, float checkCone) {
        if (playerGUID == 0) {
            return false;
        }

        uint32_t player = Game::GetObjectPointer(playerGUID);
        uint32_t type = *reinterpret_cast<uint32_t*>(player + 0x14);

        if (player == 0) {
            return false;
        }

        if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
            return false;
        }

        C3Vector posPlayer = Game::GetUnitPosition(player);
        C3Vector posCamera = Game::GetCameraPosition();

        C3Vector vecObject = {};
        vecObject.x = posObject.x - posCamera.x;
        vecObject.y = posObject.y - posCamera.y;
        vecObject.z = posObject.z - posCamera.z;

        C3Vector vecPlayer = {};
        vecPlayer.x = posPlayer.x - posCamera.x;
        vecPlayer.y = posPlayer.y - posCamera.y;
        vecPlayer.z = posPlayer.z - posCamera.z;

        // I tested in game and find out that even Vanilla Tweaks change this value, the screen border of objects still follow original FoV somehow
        // I suspect game has additional transformation before Direct X FoV
        //float fov = vanilla1121_getCameraFoV();
        const float fov = 1.5708f;

        float angle = angleBetweenVectors(vecObject, vecPlayer);

        if (angle > fov / checkCone) {
            return false;
        }
        else {
            return true;
        }
    }

#pragma region facing

    bool WorldToScreen(float worldX, float worldY, float worldZ, C2Vector& outScreen) {
        const uintptr_t WORLD_FRAME_PTR = 0x00B7436C;
        const uint32_t ACTIVE_CAMERA_OFFSET = 0x7E20;

        uintptr_t worldFrame = *reinterpret_cast<uintptr_t*>(WORLD_FRAME_PTR);
        if (!worldFrame) return false;

        uintptr_t cam = *reinterpret_cast<uintptr_t*>(worldFrame + ACTIVE_CAMERA_OFFSET);
        if (!cam) return false;

        // Read camera position
        C3Vector camPos = *reinterpret_cast<C3Vector*>(cam + 0x08);

        // Read Facing[9] matrix (3x3)
        float* facing = reinterpret_cast<float*>(cam + 0x14);

        C3Vector camForward = C3Vector(facing[0], facing[1], facing[2]);
        C3Vector camUp = C3Vector(facing[3], facing[4], facing[5]);
        C3Vector camRight = C3Vector(facing[6], facing[7], facing[8]);

        // Read FoV, near/far clip, aspect
        float nearClip = *reinterpret_cast<float*>(cam + 0x38);
        float farClip = *reinterpret_cast<float*>(cam + 0x3C);
        float fov = *reinterpret_cast<float*>(cam + 0x40);
        float aspect = *reinterpret_cast<float*>(cam + 0x44);

        if (nearClip <= 0 || farClip <= nearClip || isnan(fov)) return false;

        // Calculate screen size (use your own function to get window size if needed)
        int screenWidth;
        int screenHeight;
        GetScreenSize(&screenWidth, &screenHeight);

        // Normalize forward vector
        camForward = camForward.Normalized();

        // Recalculate look-at target
        C3Vector eye = camPos;
        C3Vector center = eye + camForward;

        // Build view matrix (LookAtRH)
        CMatrix4x4 view = CMatrix4x4::LookAtRH(eye, center, C3Vector(0, 0, 1));

        // Build projection matrix (PerspectiveFovRH)
        CMatrix4x4 projection = CMatrix4x4::PerspectiveFovRH(fov * 0.6f, float(screenWidth) / screenHeight, nearClip, farClip);

        // Final transform
        CMatrix4x4 viewProj = view * projection;

        C4Vector worldVec(worldX, worldY, worldZ, 1.0f);
        C4Vector clip = viewProj * worldVec;

        if (clip.w <= 0.1f) return false;

        // Perspective divide
        float ndcX = clip.x / clip.w;
        float ndcY = clip.y / clip.w;

        // Convert to screen space
        outScreen.x = (ndcX * 0.5f + 0.5f) * screenWidth;
        outScreen.y = (1.0f - (ndcY * 0.5f + 0.5f)) * screenHeight;

        // Optional: FOV angle check (like C#)
        C3Vector dirToTarget = C3Vector(worldX, worldY, worldZ) - camPos;
        dirToTarget = dirToTarget.Normalized();

        float dot = camForward.Dot(dirToTarget);
        float angle = acosf(std::clamp(dot, -1.0f, 1.0f));
        if (angle > (fov * 0.6f)) return false;

        return true;
    }
    
    float NormalizeRadian(float radian)
    {
        while (radian < 0)
        {
            radian = radian + 2 * M_PI;
        }
        while (radian >= 2 * M_PI)
        {
            radian = radian - 2 * M_PI;
        }
        return radian;
    }

    float CalculateNeededFacing(C3Vector start, C3Vector end)
    {
        return atan2((end.y - start.y), (end.x - start.x));
    }

    float GetSideFaceRotation(uint32_t player, uint32_t unit, float halfShootCone)
    {
        float angle = NormalizeRadian(CalculateNeededFacing(Game::GetUnitPosition(player), Game::GetUnitPosition(unit)));
        float faceTo0 = NormalizeRadian(angle - Game::Rotation(player));
        float faceDiff = faceTo0;
        bool objectOnRightSide = false;
        float sideFaceDiff = 0;

        if (faceTo0 >= M_PI)
        {
            faceDiff = 2 * M_PI - faceTo0;
            objectOnRightSide = true;
        }

        if (faceDiff > halfShootCone)
        {
            sideFaceDiff = faceDiff - halfShootCone;

            if (!objectOnRightSide)
            {
                return sideFaceDiff + Game::Rotation(player);
            }
            else
            {
                return Game::Rotation(player) - sideFaceDiff;
            }
        }
        else
        {
            return Game::Rotation(player);
        }
    }

    float GetSideFaceAngle(uint32_t player, uint32_t object)
    {
        float angle = CalculateNeededFacing(Game::GetUnitPosition(player), Game::GetUnitPosition(object));
        float faceTo0 = NormalizeRadian(angle - Game::Rotation(player));

        if (faceTo0 > M_PI)
        {
            faceTo0 = -(2 * M_PI - faceTo0);
        }
        return faceTo0;
    }

    bool IsFacing(uint32_t player, uint32_t object, float angle)
    {
        return Game::Rotation(player) == GetSideFaceRotation(player, object, angle);
    }

    bool IsFacingMelee(uint32_t player, uint32_t object) { return IsFacing(player, object, 0.95f /* halfMeleeShootCone */); }

    bool IsFacingRanged(uint32_t player, uint32_t object) { return IsFacing(player, object, 1.48f /* halfRangedShootCone */ ); }

#pragma endregion

    ///////////////////////////////// Object/Unit related stuff ////////////////////////////////

    uint32_t __stdcall GetObjectPointer(uint64_t guid)
    {
        typedef uint32_t __cdecl func(uint64_t guid, int typemask);
        func* function = (func*)Offsets::FUN_OBJECT_POINTER;

        return function(guid, -1);
    }

    int Descriptors(uint32_t pointer) { return pointer ? *(int*)(pointer + 8) : 0; }

    template<typename DSC>
    DSC GetDescriptor(int pointer, int offset)
    {
        int desc = Descriptors(pointer);
        return desc ? *(DSC*)(desc + offset * 4) : 0;
    }

    void* GetPointerDescriptor(int pointer, int offset)
    {
        return (void*)(Descriptors(pointer) + offset * 4);
    }

    static int GetVirtualFuncAddr(int addr, int offset) { return addr ? *(int*)(*(int*)addr + 4 * offset) : 0; }

    bool HasFlag(int pointer, int idx, int flag)
    {
        int flags = GetDescriptor<int>(pointer, idx);
        return (flags & flag) != 0;
    }

    C3Vector GetObjectPosition(uint32_t pointer)
    {
        C3Vector res{};

        if (pointer)
            ((void(__thiscall*)(int, C3Vector*)) GetVirtualFuncAddr(pointer, 12))(pointer, &res);

        return res;
    }

    float Rotation(uint32_t pointer)
    {
        return pointer ? ((float(__thiscall*)(int))GetVirtualFuncAddr(pointer, 14))(pointer) : 0;
    }

    int GetUnitHealth(uint32_t unit)
    {
        return GetDescriptor<int>(unit, 24);
    }

    C3Vector GetUnitPosition(uint32_t unit)
    {
        return GetObjectPosition(unit);
    }


    uint8_t* UnitBytes1(uint32_t unit) { return (uint8_t*)GetPointerDescriptor(unit, 0x0006 + 0x0044 /* UNIT_FIELD_BYTES_1 */); }

    int UnitRaidMarker(uint32_t unit) { return UnitBytes1(unit)[2]; }

    void Interact(uint32_t pointer, int autoloot, int fun_ptr)
    {
        uint32_t ptr = GetVirtualFuncAddr(pointer, 44);
        ((void(__fastcall*)(int, int))GetVirtualFuncAddr(pointer, 44))(pointer, autoloot);
    }

    bool IsUnitLootable(uint32_t unit)
    {
        return HasFlag(unit, 0x0006 + 0x0049 /*UNIT_DYNAMIC_FLAGS */, 0x0001 /* UNIT_DYNFLAG_LOOTABLE */);
    }

    bool IsUnitSkinnable(uint32_t unit)
    {
        return HasFlag(unit, 0x0006 + 0x0035 /* UNIT_FIELD_FLAGS */, 0x04000000 /* UNIT_FLAG_SKINNABLE */);
    }

    bool IsUnitInCombat(uint32_t unit)
    {
        return HasFlag(unit, 0x0006 + 0x0035/*UNIT_FIELD_FLAGS*/, 0x00080000 /*UNIT_FLAG_IN_COMBAT*/);
    }

    bool IsUnitControlledByPlayer(uint32_t unit)
    {
        return HasFlag(unit, 0x0006 + 0x0035/*UNIT_FIELD_FLAGS*/, 0x01000000 /*UNIT_FLAG_PLAYER_CONTROLLED */);
    }

    bool IsUnitDead(uint32_t unit)
    {
        return GetUnitHealth(unit) <= 0 || HasFlag(unit, 0x0006 + 0x0049 /*UNIT_DYNAMIC_FLAGS */, 0x0020 /* UNIT_DYNFLAG_DEAD */);
    }

    uint64_t UnitTargetGuid(uint32_t unit)
    {
        return GetDescriptor<uint64_t>(unit, 0x006 + 0x000C /* UNIT_FIELD_TARGET */);
    }

    float UnitCombatReach(uint32_t unit)
    {
        return GetDescriptor<float>(unit, 0x006 + 0x003C /* UNIT_FIELD_COMBATREACH */);
    }  

    float IsUnitMounted(uint32_t unit)
    {
        return GetDescriptor<int>(unit, 0x006 + 0x003F /* UNIT_FIELD_MOUNTDISPLAYID */) != 0;

    }

    bool CanAttack(uint32_t unit, uint32_t other)
    {
        if (other == 0)
            return false;

        if (!((GetDescriptor<int>(other, 0x0002 /*OBJECT_FIELD_TYPE */) & ObjectType::PLAYER) != 0) && IsUnitDead(other))
            return false;

        return unit ? ((bool(__thiscall*)(int, int))0x00729740)(unit, other) : false;
    }

    bool UnitCanBeAttacked(uint32_t player, uint32_t target) {
        if (target == 0 || player == 0) {
            false;
        }

        return CanAttack(player, target);
    }

    CreatureType UnitCreatureType(uint32_t unit)
    {
        return  unit ? ((CreatureType(__thiscall*)(int))0x0071F300)(unit) : CREATURE_TYPE_NOT_SPECIFIED;
    }
    CreatureRank UnitCreatureRank(uint32_t unit)
    {
        return  unit ? ((CreatureRank(__thiscall*)(int))0x00718A00)(unit) : CREATURERANK_UNKNOWN;
    }

    int UnitReaction(uint32_t unit)
    {
        if (!unit)
            return 0;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0); 

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        return ((int(__thiscall*)(int, int))0x007251C0)(unit, player);
    }

    int UnitMovementFlags(uint32_t unit)
    {
        return *(int*)(*(int*)(unit + 0xD8) + 0x44);
    }

    bool Traceline(C3Vector& start, C3Vector& end, C3Vector& result, int flags)
    {
        float dist = 1.0f;
        return ((bool(__cdecl*)(C3Vector&, C3Vector&, C3Vector&, float&, int, int))0x007A3B70)(start, end, result, dist, flags, 0);
    }

    bool IsUnitInLosTo(uint32_t unit, uint32_t target, uint32_t flags) //0x120171  0x100121 
    {
        if (!unit || !target)
            return false;

        C3Vector other = GetObjectPosition(target); 


        if (!other.x && !other.y && !other.z)
            return false;
        C3Vector start = GetObjectPosition(unit);
        if (!start.x && !start.y && !start.z)
            return false; 

        C3Vector result;
        start.z += 1.3f;
        C3Vector end = { other.x, other.y, other.z + 1.3f };
        return Traceline(start, end, result, flags) == 0;		 
    }

    static void _SetTarget(uint64_t guid)
    {
        typedef void __cdecl func(uint64_t guid);
        func* function = (func*)Offsets::FUN_SET_TARGET;

        function(guid);
    }

    void SetTarget(uint64_t guid)
    {
        _SetTarget(guid);
    }

    void SetTargetInteract(uint32_t candidate, uint64_t guid)
    {
        SetTarget(guid);
        Interact(candidate, 1, Offsets::FUN_RIGHT_CLICK_UNIT);
    }


    void InteractUnit(uint32_t candidate, uint64_t guid)
    {
        Interact(candidate, 1, Offsets::FUN_RIGHT_CLICK_UNIT);
    }


    void InteractObject(uint32_t candidate, uint64_t guid)
    {
        Interact(candidate, 1, Offsets::FUN_RIGHT_CLICK_OBJECT);
    }

    uint32_t GetVisualKitIdFromSpellId(uint32_t spellId, SpellVisualKitOffset offset)
    {
        // This pointer for GetLocalizedRow's Spell DB
        const void* localizedRowThis = reinterpret_cast<void*>(0x00AD49D0);

        typedef int(__thiscall* GetLocalizedRowFn)(const void* thisPtr, uint32_t spellId, void* outRow);
        GetLocalizedRowFn GetLocalizedRow = reinterpret_cast<GetLocalizedRowFn>(0x004cfd20);


        // Allocate memory for local row storage
        uint8_t localizedRow[680] = { 0 };

        // Fetch localized row using spellId
        if (!GetLocalizedRow(localizedRowThis, spellId, &localizedRow))
        {
            Log::Write("Failed to fetch localized row for spellId: %u", spellId);
            return 0;
        }

        typedef void* (__cdecl* GetSpellVisualFn)(void* spellPointer);
        GetSpellVisualFn GetSpellVisualRow = reinterpret_cast<GetSpellVisualFn>(0x007fa290);

        void* spellVisualRow = GetSpellVisualRow(reinterpret_cast<void*>(&localizedRow));

        if (!spellVisualRow)
        {
            Log::Write("Failed to retrieve SpellVisual row for spell pointer: 0x%X", *localizedRow);
            return 0;
        }

        // Resolve the visual kit ID using the offset parameter
        uint32_t visualKitId = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(spellVisualRow) + static_cast<uint32_t>(offset));

        if (visualKitId == 0)
        {
            Log::Write("No visualKitId found for spellId: %u", spellId);
            return 0;
        }

        Log::Write("Retrieved visualKit ID: %u", visualKitId);
        return visualKitId;
    }

    //CGObject_C::PlaySpellVisualKit
    void PlayVisual(void* unit, uint32_t visualKitId)
    {
        if (!unit)
        {
            Log::Write("Unit is null.");
            return;
        }

        const uint32_t* minVisualIdPtr = reinterpret_cast<const uint32_t*>(0x00ad4a4c);
        const uint32_t* maxVisualIdPtr = reinterpret_cast<const uint32_t*>(0x00ad4a48);

        uint32_t minVisualId = *minVisualIdPtr;
        uint32_t maxVisualId = *maxVisualIdPtr;

        if (visualKitId < minVisualId || visualKitId > maxVisualId)
        {
            Log::Write("Invalid visualKitId: %u", visualKitId);
            return;
        }

        // Visual kit table pointer
        const uint32_t* visualKitTable = *reinterpret_cast<const uint32_t**>(0x00ad4a5c);

        uint32_t visualKitIndex = visualKitId - minVisualId;
        void* visualKitRow = reinterpret_cast<void*>(visualKitTable[visualKitIndex]);
        if (!visualKitRow)
        {
            Log::Write("Failed to resolve VisualKitRow for visualKitId: %u", visualKitId);
            return;
        }

        UnkData unk0 = {};
        memset(&unk0, 0, sizeof(UnkData)); // initialize

        PlaySpellVisualKitData visual = {};
        memset(&visual, 0, sizeof(PlaySpellVisualKitData)); // initialize


        visual.UnkData = &unk0; // pointer to something unknown
        visual.visualKitPointer = visualKitRow; // Pointer to a SpellVisualKit DBC row
        visual.effectType = 4; // I am not exactly sure what this do, but setting this to 2 or 4 loops the effect being played, since 2 seems to have more instructions (meaning more stuff to go wrong), i'll choose 4.
        visual.unk0 = 1; // unsure
        visual.unk1 = 0xFFFFFFFF; // unsure
         

        // CGObject_C::PlaySpellVisualKit
        typedef void(__thiscall* PlayVisualFunc)(void* unit, PlaySpellVisualKitData* visual);
        auto Play = reinterpret_cast<PlayVisualFunc>(0x00745230);

        Play(unit, &visual);
    }

    void ClearEffect(void* unit, uint32_t visualKitId, EffectKitSlot slot)
    {
        if (!unit)
        {
            Log::Write("ClearEffect: Unit is null.");
            return;
        }

        // CGObject_C::ClearEffectKit
        typedef void(__thiscall* ClearEffectFunc)(void* unit, uint32_t visualKitId, uint32_t param3, uint32_t param4, uint32_t param5, uint32_t param6);
        auto ClearEffectKit = reinterpret_cast<ClearEffectFunc>(0x00744bd0); 


        uint32_t param4 = 0; // unsure
        uint32_t param5 = 0; // unsure
        uint32_t param6 = 0; // unsure

        ClearEffectKit(unit, visualKitId, (uint32_t)slot, param4, param5, param6);

        Log::Write("Cleared effect kit with ID: %u", visualKitId);
    }

    void ApplyHighlight(void* unit, uint8_t flag)
    {
        if (!unit)
        {
            Log::Write("ApplyHighlight: Unit is null.");
            return;
        }

        
        typedef void(__thiscall* HighlightObj)(void* unit, uint8_t flag);
        auto Highlight = reinterpret_cast<HighlightObj>(0x00743c70);


        Highlight(unit, flag);
    }

    void ClearHighlight(void* unit, uint8_t flag)
    {
        if (!unit)
        {
            Log::Write("ClearHighlight: Unit is null.");
            return;
        }


        typedef void(__thiscall* ClearHighlightObj)(void* unit, uint8_t flag);
        auto ClearHl = reinterpret_cast<ClearHighlightObj>(0x00743bc0);


        ClearHl(unit, flag);
    } 


    void RenderTargetMarker(void* unit)
    { 
        if (!unit)
        {
            Log::Write("RenderTargetMarker: Unit is null.");
            return;
        }


        typedef void(__thiscall* CGUnit_C_virt50_RenderTargetSelection)(void* unit);
        auto Render = reinterpret_cast<CGUnit_C_virt50_RenderTargetSelection>(0x00725980); 
         
        Render(unit); 
    }

    // We must create a keybinding for our library without creating from an addon (because of taint) and without messing with FrameXML
    // First we will create a Mock for the CStatus class logging function. 
    void MockLogFunction(MockCStatus* self, int severity, const char* format, ...) {
        Log::Write("[MockCStatus] Severity %d: ", severity);
        /*
        va_list args;
        va_start(args, format);
        vprintf(format, args);

        va_end(args);

        printf("\n");
        */
    }

    void SetupMockCStatus(MockCStatus* status) {
        static void* mockVtable[4] = { nullptr, nullptr, nullptr, nullptr }; // maybe more ???
        mockVtable[3] = (void*)&MockLogFunction; // Assign log function to offset 0xc (index 3)
        status->vtable = mockVtable;
    }

    // We ensure that the binding text is assigned to a global variable in Lua

    int EnsureLuaBindingHeader(const std::string& headerName, const std::string& headerValue)
    {
        auto state = *(int*)(0x00D3F78C);

        Lua::PushString(reinterpret_cast<void*>(state), headerValue.c_str());
        Lua::SetField(reinterpret_cast<void*>(state), LUA_GLOBALSINDEX, headerName.c_str());

        return 0;
    }

    // finally we call LoadBinding with all our parameters already set.

    void LoadBinding(const char* bindingName, const char* bindingText, const char* bindingHeaderName, const char* bindingHeaderText, const char* luaScript)
    { 
        void* thispointer = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(0x00BEADD8) + 0x0);
         
        typedef void(__thiscall* LoadBindFn)(void* thisptr, void* unk0, XMLObject* node, MockCStatus* status);
         
        auto LdBinding = reinterpret_cast<LoadBindFn>(0x00564470);

        std::string upperHeaderName(bindingHeaderName);
        std::transform(upperHeaderName.begin(), upperHeaderName.end(), upperHeaderName.begin(), ::toupper);
        std::string fullHeaderName = "BINDING_HEADER_" + upperHeaderName;

        if(upperHeaderName.length() > 0)
            EnsureLuaBindingHeader(fullHeaderName.c_str(), bindingHeaderText);


        std::string upperBindingName(bindingName);
        std::transform(upperBindingName.begin(), upperBindingName.end(), upperBindingName.begin(), ::toupper);
        std::string fullBindingName = "BINDING_NAME_" + upperBindingName;

        if (upperBindingName.length() > 0)
            EnsureLuaBindingHeader(fullBindingName.c_str(), bindingText);
         
        XMLObject node(0, "Bindings");
         
        node.setValue("name", upperBindingName.c_str());
        node.setValue("header", upperHeaderName.c_str());
         
        char** scriptLocation = reinterpret_cast<char**>(reinterpret_cast<uint8_t*>(&node) + 0x18);
        *scriptLocation = const_cast<char*>(luaScript);

        char bindsName[19] = "ConsoleXP_Bindings";
         
        MockCStatus status;
        SetupMockCStatus(&status);

         
        LdBinding(thispointer, bindsName, &node, &status);
    }

    // Casts the spell based on the position of the reticle
    void Game::CastReticle()
    {
        struct TerrainClickParams {
            char padding1[0x8];  // Unknown/unused
            float x;             // param1 + 0x8: X coordinate
            float y;             // param1 + 0xC: Y coordinate
            float z;             // param1 + 0x10: Z coordinate
            char padding2[0x14]; // Additional unknown data
            uint32_t unk0;       // ???
            char padding3[0x4];  // moar unknown data
        };

        uintptr_t _baseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr)); 


        TerrainClickParams params = {};
        params.x = *reinterpret_cast<float*>(_baseAddress + 0x774380); // reticle x coordinate.
        params.y = *reinterpret_cast<float*>(_baseAddress + 0x774384); // reticle y coordinate.
        params.z = *reinterpret_cast<float*>(_baseAddress + 0x774388); // reticle z coordinate 


        typedef void(__cdecl* CastReticle)(void* param1); 
        auto castReticle = reinterpret_cast<CastReticle>(0x0080c340);

        castReticle(&params);
    }

    void ToggleIgnoreFacing(uint32_t ignore)
    {
		GETACTIVECAMERA p_getCamera = reinterpret_cast<GETACTIVECAMERA>(0x4F5960);
		uint32_t camera = p_getCamera();

        typedef void(__thiscall* setFreeLookMode)(uint32_t thisCamera);
        auto fnSetFreeLookMode = (setFreeLookMode)0x00601ff0; // Replace with real address


        typedef void(__thiscall* setNormalMode)(uint32_t thisCamera);
        auto fnSetNormalMode = (setNormalMode)0x00601f70; // Replace with real address

        uint32_t cameraFlags = *(uint32_t*)(camera + 0x98);

        if (ignore)
        {
            // Only switch to FreeLook if not already in that mode (bit 0 not set)
            if ((cameraFlags & 1) == 0)
                fnSetFreeLookMode(camera);
        }
        else
        {
            // Only switch to Normal mode if currently in FreeLook mode
            if ((cameraFlags & 1) != 0)
                fnSetNormalMode(camera);
        }
    }

    float GetDeltaTime() {
        using clock = std::chrono::high_resolution_clock;
        static auto lastTime = clock::now();

        auto now = clock::now();
        std::chrono::duration<float> delta = now - lastTime;
        lastTime = now;

        return delta.count(); // seconds elapsed since last call
    }

    bool IsGameObjectInteractable(uint32_t gameObj)
    {
        uint32_t behavior = **reinterpret_cast<uint32_t**>(gameObj + 0x1A0);
        if (!behavior) return false;

        uint32_t func = *reinterpret_cast<uint32_t*>(behavior + 0x18);
        return func != 0x00427A90;
    }

    bool IsUnitInteractable(uint32_t unit)
    {
        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        typedef bool(__thiscall* CanInteract)(uint32_t player, uint32_t target);
        CanInteract PlayerCanInteract = reinterpret_cast<CanInteract>(0x00729530);

        return PlayerCanInteract(player, unit);
    }

}

namespace Lua
{
    const char* ToString(void* L, int index)
    {
        return p_lua_tostring(L, index, 0);
    }

    const char* ToLString(void* L, int index, int zero)
    {
        return p_lua_tostring(L, index, zero);
    }


    double ToNumber(void* L, int index)
    {
        return p_lua_tonumber(L, index);
    }

    int GetTop(void* L)
    {
        return p_lua_gettop(L);
    }

    bool IsString(void* L, int n_param)
    {
        return p_lua_isstring(L, n_param);
    }

    bool IsNumber(void* L, int index)
    {
        return p_lua_isnumber(L, index);
    }

    void PushBoolean(void* L, int boolean) {
        p_lua_pushboolean(L, boolean);
    }

    void PushNumber(void* L, double number)
    {
        p_lua_pushnumber(L, number);
    }
    void PushNil(void* L)
    {
        p_lua_pushnil(L);
    }

    void PushString(void* L, const char* str)
    {
        p_lua_pushstring(L, str);
    }
    void SetField(void* L, int idx, const char* str)
    {
        p_lua_setfield(L, idx, str);
    } 

    auto _LuaSetTop(int state, int index)
    {
        return ((void(__cdecl*)(int, int))0x0084DBF0)(state, index);
    }

    auto _LuaType(int state, int index)
    {
        return ((int(__cdecl*)(int, int))0x0084DEB0)(state, index);
    }

    auto _LuaLoadBuffer(int state, const char* buffer, int bufferLen, const char* chunkName)
    {
        return ((int(__cdecl*)(int, const char*, int, const char*))0x0084F860)(state, buffer, bufferLen, chunkName);
    }

    auto _LuaToBoolean(int state, int index)
    {
        return ((int(__cdecl*)(int, int))0x0084E0B0)(state, index);
    } 

    auto _LuaPCall(int state, int nargs, int nresults, int err)
    {
        return ((int(__cdecl*)(int, int, int, int))0x0084EC50)(state, nargs, nresults, err);
    }

    std::string PopError(int state)
    {
        const char* p = ToLString((void*)state, 1, 0);
        if (p == NULL)
            return std::string("Unknown Error");
        _LuaSetTop(state, -2);
        return std::string(p);
    }

    std::string StackObjectToString(int state, int index)
    {
        auto ltype = (int)_LuaType(state, index);

        switch (ltype)
        {
        case 0: //nil:
            return "nil";
        case 1: //boolean:
            return _LuaToBoolean(state, index) > 0 ? "true" : "false";
        case 3: //number:
            return std::to_string(ToNumber((void*)state, index));
        case 4: //string:
            return std::string(ToLString((void*)state, index, 0));

        default:
            return "<unknown lua type>";
        }
    }

    std::string GetLuaReturn(std::string command)
    {
        auto state = *(int*)(0x00D3F78C);

        std::string query = "return " + command;

        int top = GetTop((void*)state);
        const char* buffer = query.c_str();

        if (_LuaLoadBuffer(state, buffer, query.length(), "consoleXP") > 0)
            return PopError(state);

        if (_LuaPCall(state, 0, -1, 0) > 0)
            return PopError(state);

        std::vector<std::string> ret;

        int returnValueCount = GetTop((void*)state) - top;
        for (int i = 1; i <= returnValueCount; i++)
            ret.push_back(StackObjectToString(state, i));
        _LuaSetTop(state, -(returnValueCount)-1);


        return ret.size() > 0 ? ret.front() : "";
    }

}