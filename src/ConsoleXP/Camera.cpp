#include "pch.h"
#include "Camera.h"
#include "CVarHandler.h"
#include "algorithm"

void Camera::Initialize()
{
    std::cout << "Initializing DynamicCam Module..." << std::endl;
    std::locale::global(std::locale("en_US.UTF-8")); // Set locale for string parsing

    initialized = true;

    SetBobOffset(0.f);
    SetCameraTimeout(0);
    InitializeCamDetours();

    std::cout << "Camera Detours Created\nDynamicCam initialized successfuly." << std::endl;
}
void Camera::Release()
{
    // TODO
}

std::vector<uint8_t> GetHexOffset(void* ptr) {
    std::vector<uint8_t> bytes(sizeof(void*));
    std::memcpy(bytes.data(), &ptr, sizeof(void*));

    return bytes;
}

void Camera::InitializeCamDetours()
{
    std::vector<uint8_t> wO = GetHexOffset(&_bobOffset);
    std::vector<uint8_t> wX = GetHexOffset(&_bobXOffset);
    std::vector<uint8_t> wY = GetHexOffset(&_bobYOffset);
    std::vector<uint8_t> bO = GetHexOffset(&_bobStrength);
    std::vector<uint8_t> cM = GetHexOffset(&_cameraMouseTimeout);

    std::vector<uint8_t> newBytesX = {
        0x89, 0x0B,                               // mov [ebx], ecx       ; original camera X
        0xD9, 0x03,                               // fld dword ptr [ebx]  ; load original X
        0xD9, 0x05, wX[0], wX[1], wX[2], wX[3],   // fld dword ptr [BobXOffset]
        0xDE, 0xC1,                               // faddp                ; add offset
        0xD9, 0x1B,                               // fstp dword ptr [ebx] ; write new X
        0x8B, 0x50, 0x04                          // mov edx, [eax+04]    ; resume normal
    };

    std::vector<uint8_t> newBytesY = {
        0x89, 0x53, 0x04,                         // mov [ebx+04], edx    ; original camera y
        0xD9, 0x43, 0x04,                         // fld dword ptr [ebx+04]  ; load original y
        0xD9, 0x05, wY[0], wY[1], wY[2], wY[3],   // fld dword ptr [BobYOffset]
        0xDE, 0xC1,                               // faddp                ; add offset
        0xD9, 0x5B, 0x04,                         // fstp dword ptr [ebx+04] ; write new y
        0xD9, 0x1C, 0x24                          // fstp dword ptr [esp]   ; resume normal
    };

    std::vector<uint8_t> newBytesZ = {           // camera z axis
        0x8B, 0xCE,                              // mov ecx, esi
        0x89, 0x43, 0x08,                        // mov [ebx+08],eax 
        0xD9, 0x43, 0x08,                        // fld dword ptr [ebx+08]
        0xD9, 0x05, bO[0], bO[1], bO[2], bO[3],  // fld dword ptr [BobStrength] 
        0xDE, 0xE9,                              // fsubp
        0xD9, 0x05, wO[0], wO[1], wO[2], wO[3],  // fld dword ptr [BobOffset]                
        0xDE, 0xC1,                              // faddp
        0xD9, 0x5B, 0x08,                        // fstp dword ptr [ebx+08] 
    };

    std::vector<uint8_t> newBytesCamMouseTimeout = {
        0xC6, 0x05, cM[0], cM[1], cM[2], cM[3], _camTimeout,  // mov byte ptr [address], camYawTimeout
        0xD9, 0x9E, 0x1C, 0x01, 0x00, 0x00,                   // fstp dword ptr [esi+0000011C]
    };

    _Detours["CameraOffsetX"] = MemoryUtils::CreateDetour(reinterpret_cast<void*>(_baseAddress + 0x2075AB), 5, newBytesX);
    _Detours["CameraOffsetY"] = MemoryUtils::CreateDetour(reinterpret_cast<void*>(_baseAddress + 0X2075B2), 6, newBytesY);
    _Detours["CameraOffsetZ"] = MemoryUtils::CreateDetour(reinterpret_cast<void*>(_baseAddress + 0x2075C3), 5, newBytesZ);
    _Detours["CameraMouseTimeout"] = MemoryUtils::CreateDetour(reinterpret_cast<void*>(_baseAddress + 0x1FE68B), 6, newBytesCamMouseTimeout);

}

void Camera::ToggleCameraReturnDisable(bool enable) {
    if (enable) {
        if (_istoggled)
            return;

        // Yaw region /////////////////////////////////////////////////////////////////////////////
        unsigned char fstp[] = { 0xDD, 0xD8, 0x90, 0x90, 0x90, 0x90 }; // fstp st(0) + nops
        unsigned char fst[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };  // nops

        // Protect and write to memory addresses
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x204372), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x20434B), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x201442), fst, sizeof(fst));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x1FEB8F), fst, sizeof(fst));


        // Pitch region //////////////////////////////////////////////////////////////////////////// 

        // Protect and write to memory addresses
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x20428A), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x204261), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x2011C2), fst, sizeof(fst));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x1FEA6F), fst, sizeof(fst));

        _istoggled = true;
    }
    else {
        if (!_istoggled)
            return;

        // Yaw region /////////////////////////////////////////////////////////////////////////////

        unsigned char yfstp[] = { 0xD9, 0x9E, 0x1C, 0x01, 0x00, 0x00 }; // fstp dword ptr [esi+0000011C]
        unsigned char yfst[] = { 0xD9, 0x91, 0x1C, 0x01, 0x00, 0x00 };  // fst dword ptr [esi+0000011C] 

        // Protect and write to memory addresses
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x204372), yfstp, sizeof(yfstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x20434B), yfstp, sizeof(yfstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x201442), yfst, sizeof(yfst));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x1FEB8F), yfst, sizeof(yfst));

        ////////////////////////////////////////////////////////////////////////////////////////////

        // Pitch region /////////////////////////////////////////////////////////////////////////////

        unsigned char fstp[] = { 0xD9, 0x9E, 0x20, 0x01, 0x00, 0x00 }; // fstp dword ptr [esi+00000120]
        unsigned char fst[] = { 0xD9, 0x91, 0x20, 0x01, 0x00, 0x00 };  // fst dword ptr [esi+00000120] 

        // Protect and write to memory addresses
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x20428A), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x204261), fstp, sizeof(fstp));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x2011C2), fst, sizeof(fst));
        MemoryUtils::WriteProt(reinterpret_cast<void*>(_baseAddress + 0x1FEA6F), fst, sizeof(fst));

        ////////////////////////////////////////////////////////////////////////////////////////////

        _istoggled = false;
    }
}

void Camera::ActionCam_ApplyTargetTracking(int type)
{
    uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
    uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

    uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
    uint32_t lp = Game::GetObjectPointer(playerGUID);

    if (lp == NULL)
        return;

    if (!_currObj)
    {
        return;
    }

    uint32_t objtype = *reinterpret_cast<uint32_t*>(_currObj + 0x14);
    uint64_t guid = *reinterpret_cast<uint64_t*>(_currObj + 0x30);

    if (objtype != ObjectType::UNIT || guid != Game::UnitTargetGuid(lp) || guid == 0)
    {
        Game::ToggleIgnoreFacing(false);
        SetCameraTimeout(0);
        ToggleCameraReturnDisable(false);
        return;
    }

    int reaction = (int)Game::UnitReaction(_currObj);
    bool canBeAttacked = Game::UnitCanBeAttacked(lp, _currObj);

    bool shouldExit =
        Game::IsUnitDead(_currObj) ||
        (reaction > (int)UnitReaction::Neutral && type == 0) ||
        (reaction < (int)UnitReaction::Neutral && type == 1) ||
        (reaction == (int)UnitReaction::Neutral &&
            ((!canBeAttacked && type == 0) || (canBeAttacked && type == 1)));

    if (shouldExit)
    {
        Game::ToggleIgnoreFacing(false);
        SetCameraTimeout(0);
        ToggleCameraReturnDisable(false);
        return;
    }

    C3Vector playerPos = Game::GetObjectPosition(lp);
    C3Vector currObjPos = Game::GetObjectPosition(_currObj);
    auto diffVector = new C3Vector(currObjPos.x - playerPos.x, currObjPos.y - playerPos.y, currObjPos.z - playerPos.z);

    if (diffVector->Length() < GetTargetTrackMinDistance() || diffVector->Length() > GetTargetTrackMaxDistance())
    {
        Game::ToggleIgnoreFacing(false);
        SetCameraTimeout(0);
        ToggleCameraReturnDisable(false);
        return;
    }

    Game::ToggleIgnoreFacing(true);

    if (Game::UnitTargetGuid(lp) == 0)
        return;

    float facing = *(float*)(_baseAddress + 0x7EBA70);
    int isMouseClickHold = *(int*)(_baseAddress + 0x741828);

    const float pitchOffset = 0.20f;
    const float minPitch = -0.20f;
    const float maxPitch = 0.80f;

    float yawLerpFactor = (reaction < (int)UnitReaction::Neutral) || canBeAttacked ? GetYawStrengthEnemy() : GetYawStrengthInteract();
    float pitchLerpFactor = (reaction < (int)UnitReaction::Neutral) || canBeAttacked ? GetPitchStrengthEnemy() : GetPitchStrengthInteract();

    float desiredPitch = -atan2(currObjPos.z - playerPos.z, diffVector->Length()) + pitchOffset;
    desiredPitch = MathF::Clamp(desiredPitch, minPitch, maxPitch);

    if (GetCameraTimeout() != 0)
    {
        SetCameraTimeout(GetCameraTimeout() - 1);
        return;
    }

    bool isMovingAD = ((Game::UnitMovementFlags(lp) & MOVEMENTFLAG_LEFT) != 0 || (Game::UnitMovementFlags(lp) & MOVEMENTFLAG_RIGHT) != 0);
    float desiredYaw = MathF::NormalizeRadian(diffVector->Angle());
    auto yawLerp = MathF::LerpRadians(GetCameraYaw(), desiredYaw, yawLerpFactor * 0.1f);
    float pitchLerp = MathF::LerpRadians(GetCameraPitch(), desiredPitch, pitchLerpFactor * 0.1f);

    if (Game::IsFacing(lp, _currObj, 1.48f))
    {
        _waitFocus = false;
        if (yawLerpFactor > 0)
        {
            ToggleCameraReturnDisable(true);
            SetCameraYaw(yawLerp);
        }
        if (pitchLerpFactor > 0)
            SetCameraPitch(pitchLerp);
    }
    else
    { 
        if (isMouseClickHold == 1)
        {
            ToggleCameraReturnDisable(false);
        }
        else
        {
            if (yawLerp == 0.f)
            {
                _waitFocus = true;
                ToggleCameraReturnDisable(false);
            }
            else if (!_waitFocus)
            {
                yawLerp = MathF::LerpRadians(GetCameraYaw(), facing, yawLerpFactor * 0.1f);
                SetCameraYaw(yawLerp);
            }
        }
    }
}


bool SetBobAndAnim(float* bobUnk0, int* playerAnimID, float& bob, int& playerAnim)
{
    float _bob = 0.0f;
    int _pa = 0;

    __try {
        _bob = *bobUnk0;
        _pa = *playerAnimID;

        bob = _bob;
        playerAnim = _pa;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }

    return true;
}

void Camera::ActionCam_ApplyHeadBob() {
    float* bobUnk0Addr = nullptr;
    int* playerAnimID = nullptr;

    float _rawBobValue = 0.f;
	int _currentAnimID = 0;

    std::vector<int> bobUnk0Offsets = { 0x20, 0x4, 0x34, 0x94, 0xBC };
    std::vector<int> playerAnimOffsets = { 0x34, 0x94, 0x90 };

    bobUnk0Addr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x008D87A8), bobUnk0Offsets);
    playerAnimID = (int*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x008D87A8), playerAnimOffsets);

    if (!SetBobAndAnim(bobUnk0Addr, playerAnimID, _rawBobValue, _currentAnimID))
        return;

    // Get animation strength scaling
    float hms = (_headMovementStrength * 5.0f);
    float hmss = (_headMovementStandingStrength * 5.0f);
    float hmms = (_headMovementMovingStrength * 5.0f);
    

    // Get movement state
    uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
    uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
    uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
    uint32_t lp = Game::GetObjectPointer(playerGUID);
    if (!lp) return;

    bool isMoving = (Game::UnitMovementFlags(lp) & (int)MOVEMENTFLAG_MASK_MOVING) != 0;

    // Get damp rate CVars
	float dampRateStanding = _headMovementStandingDampRate;
    float dampRateMoving = _headMovementMovingDampRate;
    float dampRate = isMoving ? dampRateMoving : dampRateStanding;

    float deltaTime = Game::GetDeltaTime();

    float bobCos = std::cos(_rawBobValue);  // Z-axis bob
    float bobSin = std::sin(_rawBobValue);  // X-axis sway

    float targetZOffset = 0.0f;
    float targetZStrength = 0.0f;
    float targetXOffset = 0.0f;

    auto setInstantZero = [&]() {
        _currentBobOffset = 0.0f;
        _currentBobStrength = 0.0f;
        _currentBobXOffset = 0.0f;
    };

    // Handle animation transitions
    if (!isMoving) {
        switch (_currentAnimID) {
        case ANIM_STAND:
        case ANIM_WOUND_STAND:
        case ANIM_STUN:
        case ANIM_IDLE_HANDS_CLOSED:
            if (_transitionToIdle > 0) {
                _transitionToIdle--;
                break;
            }
            targetZOffset = hms * std::max(1.0f, hmss) * bobCos;
            targetZStrength = bobCos > 0.0f ? hms * std::max(1.0f, hmss) : 0.0f;
            targetXOffset = hms * std::max(1.0f, hmss) * bobSin * _headMovementSwayFactor;
            break;

        case ANIM_SIT_GROUND_DOWN:
        case ANIM_SIT_GROUND:
        case ANIM_SIT_GROUND_UP:
        case ANIM_SLEEP_DOWN:
        case ANIM_SLEEP:
        case ANIM_SLEEP_UP:
            _transitionSitSleepToMoving = 10;
            setInstantZero();
            break;

        case ANIM_JUMP_START:
        case ANIM_JUMP_END:
        case ANIM_LOOT:
        case ANIM_EMOTE_BEG:
        case ANIM_KNEEL_START:
        case ANIM_KNEEL_LOOP:
        case ANIM_KNEEL_END:
        case ANIM_LOOT_HOLD:
        case ANIM_LOOT_UP:
            _transitionToMoving = (_currentAnimID != ANIM_JUMP_START) ? 5 : 0;
            _transitionToIdle = 8;
            targetZOffset = hms * std::min(_headMovementStrength, _headMovementMovingStrength) * bobCos;
            targetZStrength = bobCos > 0.0f ? hms * std::min(_headMovementStrength, _headMovementMovingStrength) : 0.0f;
            targetXOffset = hms * std::min(_headMovementStrength, _headMovementMovingStrength) * bobSin * _headMovementSwayFactor;
            break;

        default:
            _transitionToMoving = 0;
            _transitionToIdle = 8;
            targetZOffset = hms * bobCos;
            targetZStrength = bobCos > 0.0f ? hms : 0.0f;
            targetXOffset = hms * bobSin * _headMovementSwayFactor;
            break;
        }
    }
    else {
        switch (_currentAnimID) {
        case ANIM_JUMP_START:
        case ANIM_JUMP_AIR:
        case ANIM_FALL_LOOP:
            targetZOffset = hms * std::min(_headMovementStrength, _headMovementMovingStrength) * bobCos;
            targetZStrength = bobCos > 0.0f ? hms * std::min(_headMovementStrength, _headMovementMovingStrength) : 0.0f;
            targetXOffset = hms * std::min(_headMovementStrength, _headMovementMovingStrength) * bobSin * _headMovementSwayFactor;
            _transitionToIdle = 8;
            break;

        case ANIM_JUMP_LAND_RUN:
            targetZOffset = hms * std::max(_headMovementStrength, _headMovementMovingStrength) * bobCos;
            targetZStrength = bobCos > 0.0f ? hms * std::max(_headMovementStrength, _headMovementMovingStrength) : 0.0f;
            targetXOffset = hms * std::max(_headMovementStrength, _headMovementMovingStrength) * bobSin * _headMovementSwayFactor;
            _transitionToIdle = 8;
            break;

        default:
            if (_transitionToMoving > 0) {
                targetZOffset = hms * std::max(_headMovementStrength, _headMovementMovingStrength) * bobCos;
                targetZStrength = bobCos > 0.0f ? hms * std::max(_headMovementStrength, _headMovementMovingStrength) : 0.0f;
                targetXOffset = hms * std::max(_headMovementStrength, _headMovementMovingStrength) * bobSin * _headMovementSwayFactor;
                _transitionToMoving--;
            }
            else if (_transitionSitSleepToMoving > 0) {
                setInstantZero();
                _transitionSitSleepToMoving--;
            }
            else {
                targetZOffset = hms * std::max(1.0f, hmms) * bobCos;
                targetZStrength = bobCos > 0.0f ? hms * std::max(1.0f, hmms) : 0.0f;
                targetXOffset = hms * std::max(1.0f, hmms) * bobSin * _headMovementSwayFactor;
                _transitionToIdle = 8;
            }
            break;
        }
    }


    // Smooth transition using damp rate
    _currentBobOffset = MathF::SmoothDamp(_currentBobOffset, targetZOffset, deltaTime, dampRate);
    _currentBobStrength = MathF::SmoothDamp(_currentBobStrength, targetZStrength, deltaTime, dampRate);
    _currentBobXOffset = MathF::SmoothDamp(_currentBobXOffset, targetXOffset, deltaTime, dampRate); 
    _currentBobYOffset = MathF::SmoothDamp(_currentBobYOffset, targetXOffset, deltaTime, dampRate);

    float yaw = *(float*)(_baseAddress + 0x7EBA70);


    float worldBobX = _currentBobXOffset * std::cos(yaw) - _currentBobYOffset * std::sin(yaw);
    float worldBobY = _currentBobXOffset * std::sin(yaw) + _currentBobYOffset * std::cos(yaw);

    SetBobOffset(_currentBobOffset);
    SetBobStrength(_currentBobStrength);
    SetBobXOffset(worldBobX);                 // rotated side bob in world coords
    SetBobYOffset(worldBobY);                 // rotated forward/back bob in world coords
}


void Camera::Direct3D_EndScene()
{
    if (!GetEnableTargetEnemyTrack() || !GetEnableTargetInteractTrack())
        ToggleCameraReturnDisable(false);

    if (IsInGame()) {
        // These are pointers to get camera values. these are only valid once the world is loaded.

        std::vector<int> cA = { 0x7E20, 0x2E4 };
        _cameraOffsetAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(0x00B7436C), cA);

        std::vector<int> pA = { 0x7E20, 0x2F8 };
        _pitchOffsetAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(0x00B7436C), pA);

        std::vector<int> hA = { 0x7E20, 0x150 };
        _cameraHeightAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(0x00B7436C), hA);

        std::vector<int> zA = { 0x7E20, 0x1E8 };
        _cameraZoomAbsAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(0x00B7436C), zA);


        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t lp = Game::GetObjectPointer(playerGUID);

        _currObj = Game::GetObjectPointer(Game::UnitTargetGuid(lp)); 


        if (_reapplySettings)
        { 
            CVarHandler::ReapplyCVars();
			_reapplySettings = false;
        }

        if ((Game::UnitMovementFlags(lp) & MovementFlags::MOVEMENTFLAG_MASK_MOVING_FLY) != 0) {
            float currentHeightOffset = GetCameraHeightOffsetInner();
            float flyingHeightOffset = GetCameraHeightFlyingOffset();

            if (currentHeightOffset != flyingHeightOffset) {
                SetCameraHeightOffsetInner(MathF::Lerp(currentHeightOffset, flyingHeightOffset, 0.1f));
            }

            float currentPitchOffset = GetPitchOffsetInner();
            float flyingPitchOffset = GetPitchFlyingOffset();
            if (currentPitchOffset != flyingPitchOffset) {
                SetPitchOffsetInner(MathF::Lerp(currentPitchOffset, flyingPitchOffset, 0.1f));
            }
        }
        else {
            float currentHeightOffset = GetCameraHeightOffsetInner();
            float normalHeightOffset = GetCameraHeightOffset();

            if (currentHeightOffset != normalHeightOffset) {
                SetCameraHeightOffsetInner(MathF::Lerp(currentHeightOffset, normalHeightOffset, 0.1f));
            }

            float currentPitchOffset = GetPitchOffsetInner();
            float normalPitchOffset = GetPitchOffset();
            if (currentPitchOffset != normalPitchOffset) {
                SetPitchOffsetInner(MathF::Lerp(currentPitchOffset, normalPitchOffset, 0.1f));
            }
        }

        if (_prevTarget != Game::UnitTargetGuid(lp) || (_prevTarget == Game::UnitTargetGuid(lp) && _currObj == NULL))
        {
            if (_currObj != NULL)
            {
                SetCameraTimeout(0);
                _prevTarget = *reinterpret_cast<uint64_t*>(_currObj + 0x30);
            }
            else
            {
                if (GetEnableTargetInteractTrack() || GetEnableTargetEnemyTrack())
                {
                    Game::ToggleIgnoreFacing(false);
                    ToggleCameraReturnDisable(false);
                }
            }
        }


        // Headbobbing, very hacky, but works very similarly how retail works.
		// But it is not as smooth as retail, it is a bit more janky and camera might get below ground or over walls sometimes.
		// Because it messes with the camera position directly without checking with CWorld_Intersect.
        ActionCam_ApplyHeadBob();

        // Target camera tracking/focusing, might feel janky but it works. 
        if (!GetEnableTargetInteractTrack() && GetEnableTargetEnemyTrack())
            ActionCam_ApplyTargetTracking(0);
        else if (GetEnableTargetInteractTrack() && !GetEnableTargetEnemyTrack())
            ActionCam_ApplyTargetTracking(1);
        else if (GetEnableTargetInteractTrack() && GetEnableTargetEnemyTrack())
            ActionCam_ApplyTargetTracking(2);
    }
    else
    {
        _reapplySettings = true;
    }
}



