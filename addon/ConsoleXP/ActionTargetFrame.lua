ConsoleXPSettings = ConsoleXPSettings or {}

local ActionTargetFrame = {}
ActionTargetFrame.__index = ActionTargetFrame

-- Mixin function to copy methods from one table to another
local function Mixin(target, source)
    for k, v in pairs(source) do
        target[k] = v
    end
end

function ActionTargetFrame:New()
    local frame = CreateFrame("Button", "CXP_ActionTargetFrame", UIParent, "TargetFrameTemplate")

    Mixin(frame, ActionTargetFrame)
 
    frame:SetMovable(true)
    frame:EnableMouse(true)
    frame:EnableMouseWheel(true)
    frame:RegisterForDrag("LeftButton")
    frame:SetClampedToScreen(true)

    frame:SetScript("OnDragStart", function(self)
        if not InCombatLockdown() then
            self:StartMoving()
        end
    end)

    frame:SetScript("OnDragStop", function(self)
        self:StopMovingOrSizing() 
        local point, relativeTo, relativePoint, xOfs, yOfs = self:GetPoint()
        ConsoleXPSettings.ActionTargetFramePos = { point = point, x = math.floor(xOfs), y = math.floor(yOfs) }
    end)

    local currentScale = 1
 
    frame:SetScript("OnMouseWheel", function(self, delta)
        if not InCombatLockdown() then 
            currentScale = math.max(0.5, math.min(1.0, currentScale + delta * 0.1))
            self:SetScale(currentScale)
 
            ConsoleXPSettings.ActionTargetFrameScale = currentScale
        end
    end)
 
    local savedScale = ConsoleXPSettings.ActionTargetFrameScale
    if savedScale then
        frame:SetScale(savedScale)
        currentScale = savedScale
    end
 
    local pos = ConsoleXPSettings.ActionTargetFramePos
    if pos and pos.point and pos.x and pos.y then
        frame:ClearAllPoints()
        frame:SetPoint(pos.point, UIParent, pos.point, pos.x, pos.y)
    else
        frame:SetPoint(TargetFrame:GetPoint())
    end

    frame.noTextPrefix = true
    frame.showLevel = true
    frame.showPVP = true
    frame.showLeader = true
    frame.showThreat = true
    frame.showPortrait = true
    frame.showClassification = true
    frame.showAuraCount = true
    frame:SetHitRectInsets(96, 40, 10, 9) -- Mouseover over health and mana bars
    frame:SetFrameLevel(5) 

    RegisterStateDriver(frame, "visibility", "[target=actiontarget, exists] show; hide")
    self.registered = true

    TargetFrame_OnLoad(frame, "actiontarget")

    frame.borderTexture:SetTexture("Interface\\AddOns\\ConsoleXP\\Assets\\UI-ActionTargetingFrame"); 

    frame:RegisterEvent("PLAYER_ACTIONTARGET_CHANGED")
    frame:SetScript("OnEvent", frame.OnEvent)

    return frame
end

function ActionTargetFrame:Update()
    if(InCombatLockdown()) then 

    else 
        if ( not UnitExists(self.unit) ) then
	        self:Hide();
	    else
		    self:Show();
            self:SetFrameLevel(5) 
        end
    end 

    if(UnitExists(self.unit)) then
		UnitFrame_Update(self);
		if ( self.showLevel ) then
			TargetFrame_CheckLevel(self);
		end
		TargetFrame_CheckFaction(self);
		if ( self.showClassification ) then
			TargetFrame_CheckClassification(self);
		end
		TargetFrame_CheckDead(self);
		if ( self.showLeader ) then
			if ( UnitIsPartyLeader(self.unit) and (UnitInParty(self.unit) or UnitInRaid(self.unit)) ) then
				if ( HasLFGRestrictions() ) then
					self.leaderIcon:SetTexture("Interface\\LFGFrame\\UI-LFG-ICON-PORTRAITROLES");
					self.leaderIcon:SetTexCoord(0, 0.296875, 0.015625, 0.3125);
				else
					self.leaderIcon:SetTexture("Interface\\GroupFrame\\UI-Group-LeaderIcon");
					self.leaderIcon:SetTexCoord(0, 1, 0, 1);
				end
				self.leaderIcon:Show();
			else
				self.leaderIcon:Hide();
			end
		end

		if ( self.portrait ) then
			self.portrait:SetAlpha(1.0);
		end
	end
end

function ActionTargetFrame:OnEvent(event, ...)
    if event == "PLAYER_ACTIONTARGET_CHANGED" then
        local actionTargetGUID = UnitGUID("actiontarget")
        local targetGUID = UnitGUID("target")

        
    if(ConsoleXPSettings.hideActionTargetFrame) then  
        self:SetAlpha(0)
    else
        self:SetAlpha(1)
    end

        self:Update()
        TargetFrame_UpdateRaidTargetIcon(self)  
    
        self.borderTexture:SetTexture("Interface\\AddOns\\ConsoleXP\\Assets\\UI-ActionTargetingFrame"); 

        if UnitExists(self.unit) then
            if UnitIsEnemy(self.unit, "player") then
                PlaySound("igCreatureAggroSelect")
            elseif UnitIsFriend("player", self.unit) then
                PlaySound("igCharacterNPCSelect")
            else
                PlaySound("igCreatureNeutralSelect")
            end
        end
    end
end

-- Initialize the ActionTargetFrame
local myActionTargetFrame = ActionTargetFrame:New()