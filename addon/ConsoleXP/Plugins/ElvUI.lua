local frame = CreateFrame("Frame")
frame:RegisterEvent("ADDON_LOADED")
frame:RegisterEvent("PLAYER_LOGIN")

local ElvUF

local function SetupActionTargetFrameSettings(E)
    -- It will just inherit all settings from the target unit frame...
    E.db.unitframe.units["actiontarget"] = E.db.unitframe.units["target"] 
end


local function CreateActionTargetFrame(UF, E, L)
    UF.Construct_ActionTargetFrame = function(self, frame)
        frame.Health = self:Construct_HealthBar(frame, true, true, "RIGHT")
        frame.Health.frequentUpdates = true
        frame.Power = self:Construct_PowerBar(frame, true, true, "LEFT")
        frame.Power.frequentUpdates = true
        frame.Name = self:Construct_NameText(frame)
        frame.Portrait3D = self:Construct_Portrait(frame, "model")
        frame.Portrait2D = self:Construct_Portrait(frame, "texture")
        frame.Buffs = self:Construct_Buffs(frame)
        frame.Debuffs = self:Construct_Debuffs(frame)
        frame.ThreatIndicator = self:Construct_Threat(frame)
        frame.RaidTargetIndicator = self:Construct_RaidIcon(frame)            
        frame.HealCommBar = self:Construct_HealComm(frame)
        frame.DebuffHighlight = self:Construct_DebuffHighlight(frame)
        frame.InfoPanel = self:Construct_InfoPanel(frame)
        frame.MouseGlow = self:Construct_MouseGlow(frame)
        frame.TargetGlow = self:Construct_TargetGlow(frame)
        frame.GPS = self:Construct_GPS(frame)
        frame.PvPIndicator = self:Construct_PvPIcon(frame)
        frame.Fader = self:Construct_Fader()
        frame.Cutaway = self:Construct_Cutaway(frame)
        frame.customTexts = {}
        frame:Point("BOTTOMRIGHT", E.UIParent, "BOTTOM", 413, 68)
        E:CreateMover(frame, frame:GetName().."Mover", L["Action Target Frame"], nil, nil, nil, "ALL,SOLO", nil, "unitframe,actiontarget,generalGroup")
    
        frame.unitframeType = "actiontarget"
    end
    
    UF.Update_ActionTargetFrame = function(self, frame, db)
        frame.db = db
    
        do
            frame.ORIENTATION = db.orientation --allow this value to change when unitframes position changes on screen?
            frame.UNIT_WIDTH = db.width
            frame.UNIT_HEIGHT = db.infoPanel.enable and (db.height + db.infoPanel.height) or db.height
    
            frame.USE_POWERBAR = db.power.enable
            frame.POWERBAR_DETACHED = db.power.detachFromFrame
            frame.USE_INSET_POWERBAR = not frame.POWERBAR_DETACHED and db.power.width == "inset" and frame.USE_POWERBAR
            frame.USE_MINI_POWERBAR = (not frame.POWERBAR_DETACHED and db.power.width == "spaced" and frame.USE_POWERBAR)
            frame.USE_POWERBAR_OFFSET = db.power.offset ~= 0 and frame.USE_POWERBAR and not frame.POWERBAR_DETACHED
            frame.POWERBAR_OFFSET = frame.USE_POWERBAR_OFFSET and db.power.offset or 0
    
            frame.POWERBAR_HEIGHT = not frame.USE_POWERBAR and 0 or db.power.height
            frame.POWERBAR_WIDTH = frame.USE_MINI_POWERBAR and (frame.UNIT_WIDTH - (frame.BORDER*2))/2 or (frame.POWERBAR_DETACHED and db.power.detachedWidth or (frame.UNIT_WIDTH - ((frame.BORDER+frame.SPACING)*2)))
    
            frame.USE_PORTRAIT = db.portrait and db.portrait.enable
            frame.USE_PORTRAIT_OVERLAY = frame.USE_PORTRAIT and (db.portrait.overlay or frame.ORIENTATION == "MIDDLE")
            frame.PORTRAIT_WIDTH = (frame.USE_PORTRAIT_OVERLAY or not frame.USE_PORTRAIT) and 0 or db.portrait.width
    
            frame.CAN_HAVE_CLASSBAR = db.combobar.enable
            frame.MAX_CLASS_BAR = MAX_COMBO_POINTS
            frame.USE_CLASSBAR = db.combobar.enable
            frame.CLASSBAR_SHOWN = frame.CAN_HAVE_CLASSBAR
            frame.CLASSBAR_DETACHED = db.combobar.detachFromFrame
            frame.USE_MINI_CLASSBAR = db.combobar.fill == "spaced" and frame.USE_CLASSBAR
            frame.CLASSBAR_HEIGHT = frame.USE_CLASSBAR and db.combobar.height or 0
            frame.CLASSBAR_WIDTH = frame.UNIT_WIDTH - ((frame.BORDER+frame.SPACING)*2) - frame.PORTRAIT_WIDTH - frame.POWERBAR_OFFSET
            frame.CLASSBAR_YOFFSET = (not frame.USE_CLASSBAR or not frame.CLASSBAR_SHOWN or frame.CLASSBAR_DETACHED) and 0 or (frame.USE_MINI_CLASSBAR and (frame.SPACING+(frame.CLASSBAR_HEIGHT/2)) or (frame.CLASSBAR_HEIGHT + frame.SPACING))
    
            frame.USE_INFO_PANEL = not frame.USE_MINI_POWERBAR and not frame.USE_POWERBAR_OFFSET and db.infoPanel.enable
            frame.INFO_PANEL_HEIGHT = frame.USE_INFO_PANEL and db.infoPanel.height or 0
    
            frame.BOTTOM_OFFSET = UF:GetHealthBottomOffset(frame)
    
            frame.VARIABLES_SET = true
        end
    
        frame.colors = ElvUI[1].oUF.colors
        frame.Portrait = frame.Portrait or (db.portrait.style == "2D" and frame.Portrait2D or frame.Portrait3D)
        frame:RegisterForClicks(self.db.targetOnMouseDown and "AnyDown" or "AnyUp")
        frame:Size(frame.UNIT_WIDTH, frame.UNIT_HEIGHT)
        _G[frame:GetName().."Mover"]:Size(frame:GetSize())
    
        if not IsAddOnLoaded("Clique") then
            if db.middleClickFocus then
                frame:SetAttribute("type3", "focus")
            elseif frame:GetAttribute("type3") == "focus" then
                frame:SetAttribute("type3", nil)
            end
        end
    
        UF:Configure_InfoPanel(frame)
    
        --Health
        UF:Configure_HealthBar(frame)
    
        --Name
        UF:UpdateNameSettings(frame)
    
        --Power
        UF:Configure_Power(frame)
    
        --Portrait
        UF:Configure_Portrait(frame)
    
        --Threat
        UF:Configure_Threat(frame)
 
        --Fader
        UF:Configure_Fader(frame)
    
        --Cutaway
        UF:Configure_Cutaway(frame)
    
        --Debuff Highlight
        UF:Configure_DebuffHighlight(frame)
    
        --OverHealing
        UF:Configure_HealComm(frame)
    
        --Raid Icon
        UF:Configure_RaidIcon(frame)
    
        --AuraBars
        UF:Configure_AuraBars(frame)
    
        --GPS
        UF:Configure_GPS(frame)
    
        --PvP
        UF:Configure_PVPIcon(frame)
    
        --CustomTexts
        UF:Configure_CustomTexts(frame)
    
        E:SetMoverSnapOffset(frame:GetName().."Mover", -(12 + db.castbar.height))
        frame:UpdateAllElements("ForceUpdate")
    end

    SetupActionTargetFrameSettings(E)
     
end


local function OnInitialize() 
    local ElvUF = ElvUI[1].UnitFrames
    local L = ElvUI[2]
    local E = ElvUI[1]

    if ElvUI and ElvUF then
        local originalOnInitialize = ElvUF.Initialize 
        local originalUpdateAllFrames = ElvUF.Update_AllFrames

        -- Hook the UF:Initialize function
        ElvUF.Initialize = function(...)
            L["Action Target Frame"] = "Action Target Frame"

            CreateActionTargetFrame(ElvUF, ElvUI[1], L)
            tinsert(ElvUF.unitstoload, "actiontarget")

            originalOnInitialize(...)

            CXP_ActionTargetFrame:UnregisterAllEvents() 
            CXP_ActionTargetFrame:Hide() 
            CXP_ActionTargetFrameHealthBar:UnregisterAllEvents()
            CXP_ActionTargetFrameManaBar:UnregisterAllEvents() 
        end

        ElvUF.Update_AllFrames = function(...)
            SetupActionTargetFrameSettings(E)
            originalUpdateAllFrames(...)
        end

    end
end

frame:SetScript("OnEvent", function(self, event, addonName)
    if event == "ADDON_LOADED" and addonName == "ElvUI" then
        -- ElvUI has been loaded, perform setup
        if ElvUI then
            OnInitialize()
        end
    elseif event == "PLAYER_LOGIN" then
        -- Check if ElvUI is already loaded at login
        if IsAddOnLoaded("ElvUI") and ShadowUF then
            OnInitialize()
        end
    end
end)
