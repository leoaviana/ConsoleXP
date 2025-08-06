#include "pch.h"
#include "API.h"
#include "Interact.h"
#include "Targeting.h"
#include "Log.h"


typedef uint32_t(__cdecl* CGGameUI_CheckPermissions_t)(int permissionType);
CGGameUI_CheckPermissions_t CheckPermissions = (CGGameUI_CheckPermissions_t)0x005191C0;

int C_ConsoleXP_InteractNearest(lua_State* L)
{ 
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Interact::InteractKey();
	}
    return 1;
}

int C_ConsoleXP_InteractMouseOver(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Interact::InteractMouseOver();
	}
	return 1;
}

int C_ConsoleXP_GetCameraZoom(lua_State* L)
{
    if (Hooks::Cam->initialized)
        Lua::PushNumber(L, Hooks::Cam->GetCameraZoomAbs());
    else
        Lua::PushNumber(L, 0);

    return 1;
}

int C_ConsoleXP_TargetNearestEnemy(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetNearestEnemy(FLT_MAX));
	}
    return 1;
}

int C_ConsoleXP_TargetNextEnemyConsideringDistance(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetEnemyConsideringDistance(&Targeting::SelectNext));
	}
	return 1;
}

int C_ConsoleXP_TargetPreviousEnemyConsideringDistance(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetEnemyConsideringDistance(&Targeting::SelectPrevious));
	}
	return 1;
}

int C_ConsoleXP_TargetNextEnemyInCycle(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetEnemyInCycle(&Targeting::SelectNext));
	}
	return 1;
}

int C_ConsoleXP_TargetPreviousEnemyInCycle(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetEnemyInCycle(&Targeting::SelectPrevious));
	}
	return 1;
}

int C_ConsoleXP_TargetNextMarkedEnemyInCycle(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		if (Lua::GetTop(L) >= 1 && Lua::IsString(L, 1)) {
			Lua::PushBoolean(L, Targeting::TargetMarkedEnemyInCycle(&Targeting::SelectNextMark, Lua::ToString(L, 1)));
		}
		else {
			Lua::PushBoolean(L, Targeting::TargetMarkedEnemyInCycle(&Targeting::SelectNextMark, ""));
		}
	}
	return 1;
}

int C_ConsoleXP_TargetPreviousMarkedEnemyInCycle(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		if (Lua::GetTop(L) >= 1 && Lua::IsString(L, 1)) {
			Lua::PushBoolean(L, Targeting::TargetMarkedEnemyInCycle(&Targeting::SelectPreviousMark, Lua::ToString(L, 1)));
		}
		else {
			Lua::PushBoolean(L, Targeting::TargetMarkedEnemyInCycle(&Targeting::SelectPreviousMark, ""));
		}
	}
	return 1;
}

int C_ConsoleXP_TargetWorldBoss(lua_State* L)
{
	if (CheckPermissions(0) != 0) // PROTECTED
	{
		Lua::PushBoolean(L, Targeting::TargetWorldBoss(FLT_MAX));
	}
	return 1;
}

int C_ConsoleXP_CastReticle(lua_State* L)
{
	Game::CastReticle();
	return 1;
}

static int lua_openlibconsolexp(lua_State* L)
{
	luaL_Reg methods[] = {
		{"InteractNearest", C_ConsoleXP_InteractNearest},
		{"InteractMouseOver", C_ConsoleXP_InteractMouseOver},
		{"GetCameraZoom", C_ConsoleXP_GetCameraZoom},
		{"TargetNearestEnemy", C_ConsoleXP_TargetNearestEnemy},
		{"TargetNextEnemyConsideringDistance", C_ConsoleXP_TargetNextEnemyConsideringDistance},
		{"TargetPreviousEnemyConsideringDistance", C_ConsoleXP_TargetPreviousEnemyConsideringDistance},
		{"TargetNextEnemyInCycle", C_ConsoleXP_TargetNextEnemyInCycle},
		{"TargetPreviousEnemyInCycle", C_ConsoleXP_TargetPreviousEnemyInCycle},
		{"TargetNextMarkedEnemyInCycle", C_ConsoleXP_TargetNextMarkedEnemyInCycle},
		{"TargetPreviousMarkedEnemyInCycle", C_ConsoleXP_TargetPreviousMarkedEnemyInCycle},
		{"TargetWorldBoss", C_ConsoleXP_TargetWorldBoss},
		{"CastReticle", C_ConsoleXP_CastReticle}, 
	};

	lua_createtable(L, 0, std::size(methods));
	for (size_t i = 0; i < std::size(methods); i++) {
		lua_pushcfunction(L, methods[i].func);
		lua_setfield(L, -2, methods[i].name);
	}
	lua_setglobal(L, "C_ConsoleXP");
	return 0;
}



void API::Initialize()
{
	Log::Write("Registering C_ConsoleXP table and functions.");
	Hooks::FrameXML::registerLuaLib(lua_openlibconsolexp);
}