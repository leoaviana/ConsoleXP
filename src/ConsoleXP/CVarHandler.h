#pragma once
#include "Client.h" 

namespace CVarHandler
{
	extern Console::CVar* s_cvar_ConsoleXP_Init;

	extern Console::CVar* s_cvar_cxp_enableActionTarget; 
	extern Console::CVar* s_cvar_cxp_enableHighlightAura;
	extern Console::CVar* s_cvar_cxp_enableHighlightInteract;
	extern Console::CVar* s_cvar_cxp_enableHighlightMouseOver;


	extern Console::CVar* s_cvar_cxp_virtualMouseX;
	extern Console::CVar* s_cvar_cxp_virtualMouseY;


	extern Console::CVar* s_cvar_cxp_highlightAuraSpellID;
	extern Console::CVar* s_cvar_cxp_targetingRangeCone;
	extern Console::CVar* s_cvar_cxp_actionTargetingCone; 

	extern Console::CVar* s_cvar_dynacamOffset;
	extern Console::CVar* s_cvar_dynacamZoom;
	extern Console::CVar* s_cvar_dynacamPitchOffset;
	extern Console::CVar* s_cvar_dynacamPitchCamHeightOffset;
	extern Console::CVar* s_cvar_dynacamPitchFlyingOffset;
	extern Console::CVar* s_cvar_dynacamPitchCamHeightFlyingOffset;
	extern Console::CVar* s_cvar_dynacamYawTimeout;
	extern Console::CVar* s_cvar_dynacamTargetEnemyTrack;
	extern Console::CVar* s_cvar_dynacamTargetInteractTrack;
	extern Console::CVar* s_cvar_dynacamTargetTrackMinDistance;
	extern Console::CVar* s_cvar_dynacamTargetTrackMaxDistance;
	extern Console::CVar* s_cvar_dynacamTargetEnemyStrengthYaw;
	extern Console::CVar* s_cvar_dynacamTargetEnemyStrengthPitch;
	extern Console::CVar* s_cvar_dynacamTargetInteractStrengthYaw;
	extern Console::CVar* s_cvar_dynacamTargetInteractStrengthPitch;
	extern Console::CVar* s_cvar_dynacamEnablePitchMod;
	extern Console::CVar* s_cvar_dynacamHeadMovementStrength;
	extern Console::CVar* s_cvar_dynacamHeadMovementRangeScale;
	extern Console::CVar* s_cvar_dynacamHeadMovementMovingStrength;
	extern Console::CVar* s_cvar_dynacamHeadMovementStandingStrength;
	extern Console::CVar* s_cvar_dynacamHeadMovementMovingDampRate;
	extern Console::CVar* s_cvar_dynacamHeadMovementStandingDampRate;

	void Initialize();
	void ReapplyCVars();
}