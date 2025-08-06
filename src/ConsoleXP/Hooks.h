#pragma once 
#include <cstdint>
#include <functional>
#include <cstdarg>
#include "Client.h"
#include <d3d9.h>
#include "Game.h"
#include <MinHook.h>
#include "Camera.h"

namespace Hooks {

	using DummyCallback_t = void(*)();

	extern HMODULE hModule;
	extern bool Detached;
	extern bool enableActionTarget;
	extern bool enableHighlightAura;
	extern bool enableHighlightInteract;
	extern bool enableHighlightMouseOver;

	extern bool ignoreCameraYaw;

	extern uint32_t highlightSpellID;
	extern float actionTargetingCone;
	extern uint64_t actionTargetGUID;
	extern Camera* Cam;


	namespace FrameScript {
		using TokenGuidGetter = uint64_t();
		using TokenNGuidGetter = uint64_t(int);
		using TokenIdGetter = bool(uint64_t);
		using TokenIdNGetter = int(uint64_t);

		// Alone tokens like player, target, focus
		void registerToken(const char* token, TokenGuidGetter* getGuid, TokenIdGetter* getId);
		// One more tokens like party1, raid1, arena1
		void registerToken(const char* token, TokenNGuidGetter* getGuid, TokenIdNGetter* getId);
		void registerOnUpdate(DummyCallback_t func);
	}

	namespace FrameXML {
		void registerEvent(const char* str);
		void registerCVar(Console::CVar** dst, const char* str, const char* desc, Console::CVarFlags flags, const char* initialValue, Console::CVar::Handler_t func);
		void registerLuaLib(lua_CFunction func);
	}

	void WaitForEndSceneHook();
	int Initialize();

}