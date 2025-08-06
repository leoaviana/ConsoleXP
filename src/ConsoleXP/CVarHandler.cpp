#include "pch.h"
#include "CVarHandler.h"
#include "Hooks.h"
#include "Targeting.h" 
#include "Highlight.h"
#include "Log.h"


Console::CVar* CVarHandler::s_cvar_ConsoleXP_Init;

Console::CVar* CVarHandler::s_cvar_cxp_enableActionTarget;
Console::CVar* CVarHandler::s_cvar_cxp_enableHighlightAura;
Console::CVar* CVarHandler::s_cvar_cxp_enableHighlightInteract;
Console::CVar* CVarHandler::s_cvar_cxp_enableHighlightMouseOver;



Console::CVar* CVarHandler::s_cvar_cxp_virtualMouseX;
Console::CVar* CVarHandler::s_cvar_cxp_virtualMouseY;



Console::CVar* CVarHandler::s_cvar_cxp_highlightAuraSpellID;
Console::CVar* CVarHandler::s_cvar_cxp_targetingRangeCone;
Console::CVar* CVarHandler::s_cvar_cxp_actionTargetingCone; 


Console::CVar* CVarHandler::s_cvar_dynacamOffset;
Console::CVar* CVarHandler::s_cvar_dynacamZoom;
Console::CVar* CVarHandler::s_cvar_dynacamPitchOffset;
Console::CVar* CVarHandler::s_cvar_dynacamPitchCamHeightOffset;
Console::CVar* CVarHandler::s_cvar_dynacamPitchFlyingOffset;
Console::CVar* CVarHandler::s_cvar_dynacamPitchCamHeightFlyingOffset;
Console::CVar* CVarHandler::s_cvar_dynacamYawTimeout;
Console::CVar* CVarHandler::s_cvar_dynacamTargetEnemyTrack;
Console::CVar* CVarHandler::s_cvar_dynacamTargetInteractTrack;
Console::CVar* CVarHandler::s_cvar_dynacamTargetTrackMinDistance;
Console::CVar* CVarHandler::s_cvar_dynacamTargetTrackMaxDistance;
Console::CVar* CVarHandler::s_cvar_dynacamTargetEnemyStrengthYaw;
Console::CVar* CVarHandler::s_cvar_dynacamTargetEnemyStrengthPitch;
Console::CVar* CVarHandler::s_cvar_dynacamTargetInteractStrengthYaw;
Console::CVar* CVarHandler::s_cvar_dynacamTargetInteractStrengthPitch;
Console::CVar* CVarHandler::s_cvar_dynacamEnablePitchMod;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementStrength;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementRangeScale;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementMovingStrength;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementStandingStrength;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementMovingDampRate;
Console::CVar* CVarHandler::s_cvar_dynacamHeadMovementStandingDampRate;


static int CVarHandler_EnableActionTarget(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);

    if (tmp != 0 && tmp != 1) {
        return 1;
    }

    Hooks::enableActionTarget = tmp;

    return 1;
}

static int CVarHandler_EnableHighlightAura(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value); 

    if (tmp != 0 && tmp != 1) {
        return 1;
    }

    Hooks::enableHighlightAura = tmp;

    return 1;
}

static int CVarHandler_EnableHighlightInteract(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);

    if (tmp != 0 && tmp != 1) {
        return 1;
    }

    Hooks::enableHighlightInteract = tmp;

    return 1;
}

static int CVarHandler_EnableHighlightMouseOver(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);

    if (tmp != 0 && tmp != 1) {
        return 1;
    }

    Hooks::enableHighlightMouseOver = tmp;

    return 1;
}

static int CVarHandler_VirtualMouseX(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);

    Targeting::virtualMouseX = tmp;

    return 1;
}

static int CVarHandler_VirtualMouseY(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);
    

    Targeting::virtualMouseY = tmp;

    return 1;
}



static int CVarHandler_HighlightAuraSpellID(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);

    if (Hooks::enableHighlightAura && Highlight::lastCandidateInteract != 0 && tmp != Hooks::highlightSpellID)
	{ 
		Game::ClearEffect((void*)Game::GetObjectPointer(Highlight::lastCandidateInteract), Hooks::highlightSpellID, EffectKitSlot::StateKit);
    }

    if (Hooks::enableHighlightAura && Highlight::lastCandidateMouseOver != 0 && tmp != Hooks::highlightSpellID)
    {
        Game::ClearEffect((void*)Game::GetObjectPointer(Highlight::lastCandidateMouseOver), Hooks::highlightSpellID, EffectKitSlot::StateKit);
    }

    Hooks::highlightSpellID = tmp;
    

    return 1;
}

static int CVarHandler_TargetingRangeCone(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Targeting::targetingRangeCone = tmp;

    return 1;
}

static int CVarHandler_ActionTargetingCone(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::actionTargetingCone = tmp;

    return 1;
}

#pragma region DynamicCam CVar Handlers

static int CVarHandler_DynamicCam(Console::CVar*, const char*, const char* value, LPVOID)
{
    return 1;
}

static int CVarHandler_DynamicCamOffset(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value); 
    Hooks::Cam->SetCameraOffset(MathF::Clamp(MathF::LinearRemap(tmp, -15.f, 15.f, -0.5f, 0.5f), -0.5f, 0.5f));

    return 1;
}

static int CVarHandler_DynamicCamPitchOffset(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetPitchOffset(MathF::Clamp(MathF::LinearRemap(tmp, 0.f, 1.f, 0.f, 0.35f), 0.f, 0.35f));

    return 1;
}

static int CVarHandler_DynamicCamPitchCamHeightOffset(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetCameraHeightOffset(MathF::Clamp(MathF::LinearRemap(tmp, 0.f, 1.f, 0.3f, 3.0), 0.3f, 3.0));

    return 1;
}

static int CVarHandler_DynamicCamTargetEnemyStrengthYaw(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetYawStrengthEnemy(MathF::Clamp(tmp, 0.f, 1.f));

    return 1;
}

static int CVarHandler_DynamicCamTargetEnemyStrengthPitch(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetPitchStrengthEnemy(MathF::Clamp(tmp, 0.f, 1.f));

    return 1;
}

static int CVarHandler_DynamicCamTargetInteractStrengthYaw(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetYawStrengthInteract(MathF::Clamp(tmp, 0.f, 1.f));

    return 1;
}

static int CVarHandler_DynamicCamTargetInteractStrengthPitch(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetYawStrengthInteract(MathF::Clamp(tmp, 0.f, 1.f));

    return 1;
}

static int CVarHandler_DynamicCamTargetTrackMinDistance(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetTargetTrackMinDistance(MathF::Clamp(tmp, 0.f, 20.f));

    return 1;
}

static int CVarHandler_DynamicCamTargetTrackMaxDistance(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetTargetTrackMaxDistance(MathF::Clamp(tmp, 21.f, 100.f));

    return 1;
}

static int CVarHandler_DynamicCamPitchFlyingOffset(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetPitchFlyingOffset(MathF::Clamp(MathF::LinearRemap(tmp, 0.f, 1.f, 0.f, 0.35f), 0.f, 0.35f));

    return 1;
}

static int CVarHandler_DynamicCamPitchCamHeightFlyingOffset(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetCameraHeightFlyingOffset(MathF::Clamp(MathF::LinearRemap(tmp, 0.f, 1.f, 0.3f, 3.0f), 0.3f, 3.0f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementStrength(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementStrength(MathF::Clamp(tmp, 0.f, 2.f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementRangeScale(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementRangeScale(MathF::Clamp(tmp, 0.f, 50.f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementMovingStrength(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementMovingStrength(MathF::Clamp(tmp, 0.f, 2.f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementStandingStrength(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementMovingStrength(MathF::Clamp(tmp, 0.f, 2.f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementMovingDampRate(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementMovingDampRate(MathF::Clamp(tmp, 0.f, 20.f));

    return 1;
}

static int CVarHandler_DynamicCamHeadMovementStandingDampRate(Console::CVar*, const char*, const char* value, LPVOID)
{
    double tmp = atof(value);
    Hooks::Cam->SetHeadMovementStandingDampRate(MathF::Clamp(tmp, 0.f, 20.f));

    return 1;
}

static int CVarHandler_DynamicCamYawTimeout(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);
    Hooks::Cam->SetCameraTimeout((uint8_t)tmp);

    return 1;
}

static int CVarHandler_DynamicCamEnablePitchMod(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);
    Hooks::Cam->SetEnablePitchMod(tmp);

    return 1;
}

static int CVarHandler_DynamicCamTargetEnemyTrack(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);
    Hooks::Cam->SetEnableTargetEnemyTrack(tmp);

    return 1;
}

static int CVarHandler_DynamicCamTargetInteractTrack(Console::CVar*, const char*, const char* value, LPVOID)
{
    int tmp = atoi(value);
    Hooks::Cam->SetEnableTargetInteractTrack(tmp);

    return 1;
}

#pragma endregion 

void CVarHandler::Initialize()
{
    Log::Write("Initializing Cvars...");
    
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_enableActionTarget, "cxp_enableActionTarget", NULL, (Console::CVarFlags)1, "0", CVarHandler_EnableActionTarget);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_enableHighlightAura, "cxp_enableHighlightAura", NULL, (Console::CVarFlags)1, "0", CVarHandler_EnableHighlightAura);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_enableHighlightInteract, "cxp_enableHighlightInteract", NULL, (Console::CVarFlags)1, "1", CVarHandler_EnableHighlightInteract);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_enableHighlightMouseOver, "cxp_enableHighlightMouseOver", NULL, (Console::CVarFlags)1, "1", CVarHandler_EnableHighlightMouseOver);


    Hooks::FrameXML::registerCVar(&s_cvar_cxp_virtualMouseX, "cxp_virtualMouseX", NULL, (Console::CVarFlags)1, "0", CVarHandler_VirtualMouseX);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_virtualMouseY, "cxp_virtualMouseY", NULL, (Console::CVarFlags)1, "0", CVarHandler_VirtualMouseY);


    Hooks::FrameXML::registerCVar(&s_cvar_cxp_highlightAuraSpellID, "cxp_highlightAuraSpellID", NULL, (Console::CVarFlags)1, "54273", CVarHandler_HighlightAuraSpellID);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_targetingRangeCone, "cxp_targetingRangeCone", NULL, (Console::CVarFlags)1, "2.2", CVarHandler_TargetingRangeCone);
    Hooks::FrameXML::registerCVar(&s_cvar_cxp_actionTargetingCone, "cxp_actionTargetingCone", NULL, (Console::CVarFlags)1, "0.30", CVarHandler_ActionTargetingCone);

    Hooks::FrameXML::registerCVar(&s_cvar_dynacamOffset, "test_cameraOverShoulder", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamOffset);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamPitchOffset, "test_cameraDynamicPitchBaseFovPad", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamPitchOffset);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamPitchCamHeightOffset, "test_cameraDynamicPitchHeight", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamPitchCamHeightOffset);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamPitchFlyingOffset, "test_cameraDynamicPitchBaseFovPadFlying", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamPitchFlyingOffset);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamPitchCamHeightFlyingOffset, "test_cameraDynamicPitchHeightFlying", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamPitchCamHeightFlyingOffset);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamYawTimeout, "test_dynacamYawTimeout", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamYawTimeout);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetEnemyTrack, "test_cameraTargetFocusEnemyEnable", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamTargetEnemyTrack);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetInteractTrack, "test_cameraTargetFocusInteractEnable", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamTargetInteractTrack);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetTrackMinDistance, "test_cameraTargetFocusMinDistance", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamTargetTrackMinDistance);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetTrackMaxDistance, "test_cameraTargetFocusMaxDistance", NULL, (Console::CVarFlags)1, "100", CVarHandler_DynamicCamTargetTrackMaxDistance);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetEnemyStrengthYaw, "test_cameraTargetFocusEnemyStrengthYaw", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamTargetEnemyStrengthYaw);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetEnemyStrengthPitch, "test_cameraTargetFocusEnemyStrengthPitch", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamTargetEnemyStrengthPitch);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetInteractStrengthYaw, "test_cameraTargetFocusInteractStrengthYaw", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamTargetInteractStrengthYaw);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamTargetInteractStrengthPitch, "test_cameraTargetFocusInteractStrengthPitch", NULL, (Console::CVarFlags)1, "0.5", CVarHandler_DynamicCamTargetInteractStrengthPitch);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamEnablePitchMod, "test_cameraDynamicPitch", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamEnablePitchMod);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementStrength, "test_cameraHeadMovementStrength", NULL, (Console::CVarFlags)1, "0", CVarHandler_DynamicCamHeadMovementStrength);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementRangeScale, "test_cameraHeadMovementRangeScale", NULL, (Console::CVarFlags)1, "50", CVarHandler_DynamicCamHeadMovementRangeScale);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementMovingStrength, "test_cameraHeadMovementMovingStrength", NULL, (Console::CVarFlags)1, "1", CVarHandler_DynamicCamHeadMovementMovingStrength);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementStandingStrength, "test_cameraHeadMovementStandingStrength", NULL, (Console::CVarFlags)1, "1", CVarHandler_DynamicCamHeadMovementStandingStrength);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementMovingDampRate, "test_cameraHeadMovementMovingDampRate", NULL, (Console::CVarFlags)1, "10", CVarHandler_DynamicCamHeadMovementMovingDampRate);
    Hooks::FrameXML::registerCVar(&s_cvar_dynacamHeadMovementStandingDampRate, "test_cameraHeadMovementStandingDampRate", NULL, (Console::CVarFlags)1, "10", CVarHandler_DynamicCamHeadMovementStandingDampRate);

}

void CVarHandler::ReapplyCVars()
{
	if (s_cvar_cxp_enableActionTarget) CVarHandler_EnableActionTarget(s_cvar_cxp_enableActionTarget, nullptr, s_cvar_cxp_enableActionTarget->vStr, nullptr);
	if (s_cvar_cxp_enableHighlightAura) CVarHandler_EnableHighlightAura(s_cvar_cxp_enableHighlightAura, nullptr, s_cvar_cxp_enableHighlightAura->vStr, nullptr);
    if (s_cvar_cxp_enableHighlightInteract) CVarHandler_EnableHighlightInteract(s_cvar_cxp_enableHighlightInteract, nullptr, s_cvar_cxp_enableHighlightInteract->vStr, nullptr);
    if (s_cvar_cxp_enableHighlightMouseOver) CVarHandler_EnableHighlightMouseOver(s_cvar_cxp_enableHighlightMouseOver, nullptr, s_cvar_cxp_enableHighlightMouseOver->vStr, nullptr);
    if (s_cvar_cxp_virtualMouseX) CVarHandler_EnableHighlightMouseOver(s_cvar_cxp_virtualMouseX, nullptr, s_cvar_cxp_virtualMouseX->vStr, nullptr);
    if (s_cvar_cxp_virtualMouseY) CVarHandler_EnableHighlightMouseOver(s_cvar_cxp_virtualMouseY, nullptr, s_cvar_cxp_virtualMouseY->vStr, nullptr);
	if (s_cvar_cxp_highlightAuraSpellID) CVarHandler_HighlightAuraSpellID(s_cvar_cxp_highlightAuraSpellID, nullptr, s_cvar_cxp_highlightAuraSpellID->vStr, nullptr);
	if (s_cvar_cxp_targetingRangeCone) CVarHandler_TargetingRangeCone(s_cvar_cxp_targetingRangeCone, nullptr, s_cvar_cxp_targetingRangeCone->vStr, nullptr);
	if (s_cvar_cxp_actionTargetingCone) CVarHandler_ActionTargetingCone(s_cvar_cxp_actionTargetingCone, nullptr, s_cvar_cxp_actionTargetingCone->vStr, nullptr);
	if (s_cvar_dynacamOffset) CVarHandler_DynamicCamOffset(s_cvar_dynacamOffset, nullptr, s_cvar_dynacamOffset->vStr, nullptr);
	if (s_cvar_dynacamPitchOffset) CVarHandler_DynamicCamPitchOffset(s_cvar_dynacamPitchOffset, nullptr, s_cvar_dynacamPitchOffset->vStr, nullptr);
	if (s_cvar_dynacamPitchCamHeightOffset) CVarHandler_DynamicCamPitchCamHeightOffset(s_cvar_dynacamPitchCamHeightOffset, nullptr, s_cvar_dynacamPitchCamHeightOffset->vStr, nullptr);
	if (s_cvar_dynacamPitchFlyingOffset) CVarHandler_DynamicCamPitchFlyingOffset(s_cvar_dynacamPitchFlyingOffset, nullptr, s_cvar_dynacamPitchFlyingOffset->vStr, nullptr);
	if (s_cvar_dynacamPitchCamHeightFlyingOffset) CVarHandler_DynamicCamPitchCamHeightFlyingOffset(s_cvar_dynacamPitchCamHeightFlyingOffset, nullptr, s_cvar_dynacamPitchCamHeightFlyingOffset->vStr, nullptr);
	if (s_cvar_dynacamYawTimeout) CVarHandler_DynamicCamYawTimeout(s_cvar_dynacamYawTimeout, nullptr, s_cvar_dynacamYawTimeout->vStr, nullptr);
	if (s_cvar_dynacamTargetEnemyTrack) CVarHandler_DynamicCamTargetEnemyTrack(s_cvar_dynacamTargetEnemyTrack, nullptr, s_cvar_dynacamTargetEnemyTrack->vStr, nullptr);
	if (s_cvar_dynacamTargetInteractTrack) CVarHandler_DynamicCamTargetInteractTrack(s_cvar_dynacamTargetInteractTrack, nullptr, s_cvar_dynacamTargetInteractTrack->vStr, nullptr);
	if (s_cvar_dynacamTargetTrackMinDistance) CVarHandler_DynamicCamTargetTrackMinDistance(s_cvar_dynacamTargetTrackMinDistance, nullptr, s_cvar_dynacamTargetTrackMinDistance->vStr, nullptr);
	if (s_cvar_dynacamTargetTrackMaxDistance) CVarHandler_DynamicCamTargetTrackMaxDistance(s_cvar_dynacamTargetTrackMaxDistance, nullptr, s_cvar_dynacamTargetTrackMaxDistance->vStr, nullptr);
	if (s_cvar_dynacamTargetEnemyStrengthYaw) CVarHandler_DynamicCamTargetEnemyStrengthYaw(s_cvar_dynacamTargetEnemyStrengthYaw, nullptr, s_cvar_dynacamTargetEnemyStrengthYaw->vStr, nullptr);
	if (s_cvar_dynacamTargetEnemyStrengthPitch) CVarHandler_DynamicCamTargetEnemyStrengthPitch(s_cvar_dynacamTargetEnemyStrengthPitch, nullptr, s_cvar_dynacamTargetEnemyStrengthPitch->vStr, nullptr);
	if (s_cvar_dynacamTargetInteractStrengthYaw) CVarHandler_DynamicCamTargetInteractStrengthYaw(s_cvar_dynacamTargetInteractStrengthYaw, nullptr, s_cvar_dynacamTargetInteractStrengthYaw->vStr, nullptr);
	if (s_cvar_dynacamTargetInteractStrengthPitch) CVarHandler_DynamicCamTargetInteractStrengthPitch(s_cvar_dynacamTargetInteractStrengthPitch, nullptr, s_cvar_dynacamTargetInteractStrengthPitch->vStr, nullptr);
	if (s_cvar_dynacamEnablePitchMod) CVarHandler_DynamicCamEnablePitchMod(s_cvar_dynacamEnablePitchMod, nullptr, s_cvar_dynacamEnablePitchMod->vStr, nullptr);
	if (s_cvar_dynacamHeadMovementStrength) CVarHandler_DynamicCamHeadMovementStrength(s_cvar_dynacamHeadMovementStrength, nullptr, s_cvar_dynacamHeadMovementStrength->vStr, nullptr);
	if (s_cvar_dynacamHeadMovementRangeScale) CVarHandler_DynamicCamHeadMovementRangeScale(s_cvar_dynacamHeadMovementRangeScale, nullptr, s_cvar_dynacamHeadMovementRangeScale->vStr, nullptr);
	if (s_cvar_dynacamHeadMovementMovingStrength) CVarHandler_DynamicCamHeadMovementMovingStrength(s_cvar_dynacamHeadMovementMovingStrength, nullptr, s_cvar_dynacamHeadMovementMovingStrength->vStr, nullptr);
	if (s_cvar_dynacamHeadMovementStandingStrength) CVarHandler_DynamicCamHeadMovementStandingStrength(s_cvar_dynacamHeadMovementStandingStrength, nullptr, s_cvar_dynacamHeadMovementStandingStrength->vStr, nullptr);
    if (s_cvar_dynacamHeadMovementMovingDampRate) CVarHandler_DynamicCamHeadMovementMovingDampRate(s_cvar_dynacamHeadMovementMovingDampRate, nullptr, s_cvar_dynacamHeadMovementMovingDampRate->vStr, nullptr);
    if (s_cvar_dynacamHeadMovementStandingDampRate) CVarHandler_DynamicCamHeadMovementStandingDampRate(s_cvar_dynacamHeadMovementStandingDampRate, nullptr, s_cvar_dynacamHeadMovementStandingDampRate->vStr, nullptr);
}