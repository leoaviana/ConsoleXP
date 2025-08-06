#pragma once

#include <cstdint>
#include <unordered_map> 
#include <iostream> 
#include <sstream> 
#pragma once
#include <vector>
#include "Game.h" 
#include "MemoryUtils.h"
#include "MathF.h"




enum AnimationID {
	ANIM_STAND = 0,
	ANIM_WOUND_STAND = 8,
	ANIM_STUN = 14,
	ANIM_IDLE_HANDS_CLOSED = 15,
	ANIM_SIT_GROUND_DOWN = 96,
	ANIM_SIT_GROUND = 97,
	ANIM_SIT_GROUND_UP = 98,
	ANIM_SLEEP_DOWN = 99,
	ANIM_SLEEP = 100,
	ANIM_SLEEP_UP = 101,
	ANIM_JUMP_START = 37,
	ANIM_JUMP_END = 39,
	ANIM_LOOT = 50,
	ANIM_EMOTE_BEG = 79,
	ANIM_KNEEL_START = 114,
	ANIM_KNEEL_LOOP = 115,
	ANIM_KNEEL_END = 116,
	ANIM_LOOT_HOLD = 188,
	ANIM_LOOT_UP = 189,
	ANIM_FALL_LOOP = 40,
	ANIM_JUMP_LAND_RUN = 187,
	ANIM_JUMP_AIR = 38  // inferred
};

class Camera
{
private:
	bool _enPitchMod;
	bool _istoggled = false;
	bool _waitFocus = false;
	bool _enableTargetETrack = false;
	bool _enableTargetITrack = false;
	bool _reapplySettings = false; 
	bool _isTrackingTarget = false;

	int _transitionToMoving = 0;
	int _transitionToIdle = 0;
	int _transitionSitSleepToMoving = 0;
	uint64_t _prevTarget = 0;
	uint32_t _currObj = NULL;

	const uint8_t _camTimeout = 30;
	std::unordered_map<std::string, MemoryUtils::DetourAddress> _Detours;
	uintptr_t _baseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

	uint8_t* _playerMoving = nullptr;
	float* _cameraOffsetAddr = nullptr;
	float* _pitchOffsetAddr = nullptr;
	float* _cameraHeightAddr = nullptr;
	float* _cameraZoomAbsAddr = nullptr;


	float _currentBobOffset = 0.0f;   // Smoothed Z bob offset
	float _currentBobStrength = 0.0f; // Smoothed Z bob strength
	float _currentBobXOffset = 0.0f;  // Smoothed X bob offset (new)
	float _currentBobYOffset = 0.0f;  // Smoothed Y bob offset (new)

	float _bobOffset = 0.f;
	float _bobXOffset = 0.f;
	float _bobYOffset = 0.f;
	float _bobStrength = 0.f;


	uint8_t _cameraMouseTimeout = 0;

	float _yawStrengthEnemy = 0.5f;
	float _pitchStrengthEnemy = 0.5f;

	float _yawStrengthInteract = 0.5f;
	float _pitchStrengthInteract = 0.5f;

	float _targetTrackMinDistance = 0.f;
	float _targetTrackMaxDistance = 100.f;


	float _pitchOffset = 0.f;
	float _cameraHeightOffset = 1.6943f;
	float _pitchFlyingOffset = 0.f;
	float _cameraHeightFlyingOffset = 1.6943f;
	float _headMovementStrength = 0.f;
	float _headMovementRangeScale = 50.f;
	float _headMovementMovingStrength = 1.f;
	float _headMovementStandingStrength = 1.f;
	float _headMovementMovingDampRate = 10.f;
	float _headMovementStandingDampRate = 10.f;

	std::vector<int> camYawOffsets = { 0x7e20, 0x11c };
	std::vector<int> camPitchOffsets = { 0x7e20, 0x120 };


	float GetCameraHeightOffsetInner() {
		__try {
			return IsInGame() ? (_cameraHeightAddr == nullptr ? 0.f : *_cameraHeightAddr) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}
	float GetPitchOffsetInner() {
		__try {
			return IsInGame() ? (_pitchOffsetAddr == nullptr ? 0.f : *_pitchOffsetAddr) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}


	void SetCameraHeightOffsetInner(float value) {
		__try {
			if (IsInGame()) if (_cameraHeightAddr != nullptr) *_cameraHeightAddr = value;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}
	void SetPitchOffsetInner(float value) {
		__try {
			if (IsInGame()) if (_pitchOffsetAddr != nullptr) *_pitchOffsetAddr = value;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}


public: 
	bool initialized = false;

	float _headMovementSwayFactor = 0.02f;

	inline bool IsInGame()
	{
		return Game::IsInWorld();
	}

	bool IsTrackingTarget() const {
		return _isTrackingTarget;
	}

	float GetCameraOffset() {
		__try {
			return IsInGame() ? (_cameraOffsetAddr == nullptr ? 0.f : *_cameraOffsetAddr) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}

	float GetPitchOffset() { return _pitchOffset; }
	float GetCameraHeightOffset() { return _cameraHeightOffset; }

	float GetCameraZoomAbs() {
		__try {
			return IsInGame() ? (_cameraZoomAbsAddr == nullptr ? 0.f : *_cameraZoomAbsAddr) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}
	float GetCameraYaw() {
		__try {
			return IsInGame() ? *(float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x0077436C), camYawOffsets) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}


	float GetCameraPitch() {
		__try {
			return IsInGame() ? *(float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x0077436C), camPitchOffsets) : 0.f;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0.f;
		}
	}

	float GetBobOffset() { return _bobOffset; }
	float GetBobStrength() { return _bobStrength; }


	float GetYawStrengthEnemy() { return _yawStrengthEnemy; }
	float GetPitchStrengthEnemy() { return _pitchStrengthEnemy; }

	float GetYawStrengthInteract() { return _yawStrengthInteract; }
	float GetPitchStrengthInteract() { return _pitchStrengthInteract; }


	float GetTargetTrackMinDistance() { return _targetTrackMinDistance; }
	float GetTargetTrackMaxDistance() { return _targetTrackMaxDistance; }

	float GetPitchFlyingOffset() { return _pitchFlyingOffset; }
	float GetCameraHeightFlyingOffset() { return _cameraHeightFlyingOffset; }
	float GetHeadMovementStrength() { return _headMovementStrength; }
	float GetHeadMovementRangeScale() { return _headMovementRangeScale; }
	float GetHeadMovementMovingStrength() { return _headMovementMovingStrength; }
	float GetHeadMovementStandingStrength() { return _headMovementStandingStrength; }

	uint8_t GetCameraTimeout() { return _cameraMouseTimeout; }
	bool GetEnablePitchMod() const { return _enPitchMod; }
	bool GetEnableTargetInteractTrack() { return _enableTargetITrack; }
	bool GetEnableTargetEnemyTrack() { return _enableTargetETrack; }

	//Setters

	void SetCameraOffset(float value) {
		__try {
			if (IsInGame()) if (_cameraOffsetAddr != nullptr) *_cameraOffsetAddr = value;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}
	void SetPitchOffset(float value) { _pitchOffset = value; }
	void SetCameraHeightOffset(float value) { _cameraHeightOffset = value; }

	void SetCameraZoomAbs(float value) {
		__try {
			if (IsInGame()) if (_cameraZoomAbsAddr != nullptr) *_cameraZoomAbsAddr = value;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}

	void SetCameraYaw(float value) {
		__try {
			if (IsInGame())
			{
				auto camYawAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x0077436C), camYawOffsets);

				if (camYawAddr != nullptr)
					*camYawAddr = value;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}

	void SetCameraPitch(float value) {
		__try {
			if (IsInGame())
			{
				auto camPitchAddr = (float*)MemoryUtils::ReadPointer(reinterpret_cast<void*>(_baseAddress + 0x0077436C), camPitchOffsets);

				if (camPitchAddr != nullptr)
					*camPitchAddr = value;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return;
		}
	}

	void SetBobOffset(float value) { _bobOffset = value; }
	void SetBobXOffset(float value) { _bobXOffset = value; }
	void SetBobYOffset(float value) { _bobYOffset = value; }
	void SetBobStrength(float value) { _bobStrength = value; }


	void SetYawStrengthEnemy(float value) { _yawStrengthEnemy = value; }
	void SetPitchStrengthEnemy(float value) { _pitchStrengthEnemy = value; }
	void SetYawStrengthInteract(float value) { _yawStrengthInteract = value; }
	void SetPitchStrengthInteract(float value) { _pitchStrengthInteract = value; }


	void SetTargetTrackMinDistance(float value) { _targetTrackMinDistance = value; }
	void SetTargetTrackMaxDistance(float value) { _targetTrackMaxDistance = value; }


	void SetPitchFlyingOffset(float value) { _pitchFlyingOffset = value; }
	void SetCameraHeightFlyingOffset(float value) { _cameraHeightFlyingOffset = value; }
	void SetHeadMovementStrength(float value) { _headMovementStrength = value; }
	void SetHeadMovementRangeScale(float value) { _headMovementRangeScale = value; }
	void SetHeadMovementMovingStrength(float value) { _headMovementMovingStrength = value; }
	void SetHeadMovementStandingStrength(float value) { _headMovementStandingStrength = value; }
	void SetHeadMovementMovingDampRate(float value) { _headMovementMovingDampRate = value; }
	void SetHeadMovementStandingDampRate(float value) { _headMovementStandingDampRate = value; }

	void SetCameraTimeout(uint8_t value) { _cameraMouseTimeout = value; }
	void SetEnablePitchMod(bool value)
	{
		if (value == false)
		{
			// set pitch related stuff to default values.
			SetPitchOffset(0.f);
			SetCameraHeightOffset(1.6943f);
		}
		_enPitchMod = value;
	}


	void SetEnableTargetInteractTrack(bool value) { _enableTargetITrack = value; }

	void SetEnableTargetEnemyTrack(bool value) { _enableTargetETrack = value; }

	void Initialize();
	void Release();
	void InitializeCamDetours();
	void ToggleCameraReturnDisable(bool enable);
	void ActionCam_ApplyTargetTracking(int type);
	void ActionCam_ApplyHeadBob();
	void Direct3D_EndScene();
};