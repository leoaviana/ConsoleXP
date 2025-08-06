local frame = CreateFrame("Frame")
frame:RegisterEvent("ADDON_LOADED")
frame:RegisterEvent("PLAYER_LOGIN") -- To handle cases where ShadowedUnitFrames is already loaded

local function InitializeActionTargetFrameWithShadowUF()
    
    -- Load UnitFrame for actionTarget based on the target frame.
    -- if target frame is enabled, we enable our frame too.. 
	ShadowUF.Units:InitializeFrame("actiontarget") 
end

local function OnInitialize() 
    if ShadowUF and ShadowUF.OnInitialize then
        local originalOnInitialize = ShadowUF.OnInitialize
        local originalLoadDefaultLayout = ShadowUF.LoadDefaultLayout
        local originalCheckUpgrade = ShadowUF.CheckUpgrade 

        -- Hook the OnInitialize function
        ShadowUF.OnInitialize = function(...)

            local L = ShadowUF.L 

            L["Action Target"] = "Action Target"
            -- Run your custom logic before ShadowUF initializes 
            ShadowUF.unitList[#ShadowUF.unitList + 1] = "actiontarget"
            L.units["actiontarget"] = L["Action Target"]
            ShadowUF.fakeUnits["actiontarget"] = true

            -- Call the original function

            originalOnInitialize(...) 
            
            ShadowUF.Units.unitFrames["actiontarget"].isUnitVolatile = true
            ShadowUF.Units.unitFrames["actiontarget"]:RegisterNormalEvent("PLAYER_ACTIONTARGET_CHANGED", ShadowUF.Units, "CheckUnitStatus")
 
            CXP_ActionTargetFrame:UnregisterAllEvents()
            CXP_ActionTargetFrame.Show = ShadowUF.noop
            CXP_ActionTargetFrame:Hide() 
            CXP_ActionTargetFrameHealthBar:UnregisterAllEvents()
            CXP_ActionTargetFrameManaBar:UnregisterAllEvents() 

        end

        -- Hook the LoadDefaultLayout function
        ShadowUF.LoadDefaultLayout = function(...)
            -- call original first
            originalLoadDefaultLayout(...) 

            ShadowUF.db.profile.positions.actiontarget = {anchorPoint = "RC", anchorTo = "#SUFUnitplayer", x = 50, y = -40}
            ShadowUF.db.profile.units.actiontarget = ShadowUF.db.profile.units.target
        end


        --Hook CheckUpgrade function
        ShadowUF.CheckUpgrade = function(...) 
            if(not ShadowUF.db.profile.positions.actiontarget) then
                ShadowUF.db.profile.positions.actiontarget = {anchorPoint = "RC", anchorTo = "#SUFUnitplayer", x = 50, y = -40}
            end
            
            ShadowUF.db.profile.units.actiontarget = ShadowUF.db.profile.units.target

            return originalCheckUpgrade(...)
        end 
    end
end

frame:SetScript("OnEvent", function(self, event, addonName)
    if event == "ADDON_LOADED" and addonName == "ShadowedUnitFrames" then
        -- ShadowedUnitFrames has been loaded, perform setup
        if ShadowUF then
            OnInitialize()
        end
    elseif event == "PLAYER_LOGIN" then
        -- Check if ShadowedUnitFrames is already loaded at login
        if IsAddOnLoaded("ShadowedUnitFrames") and ShadowUF then
            OnInitialize()
        end
    end
end)
