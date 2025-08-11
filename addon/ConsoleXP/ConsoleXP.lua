local addonName, addon = ...

local AceConfig = LibStub("AceConfig-3.0")
local AceConfigDialog = LibStub("AceConfigDialog-3.0")

local function GetCVarNumber(cvar, defaultVal)
    local val = tonumber(GetCVar(cvar))
    if val == nil then return defaultVal end
    return val
end

local function GetCVarBoolean(cvar)
    return GetCVar(cvar) == "1"
end

local function CreateSliderResetButton(order, index1, tooltipDefaultValue)

  return {
    type = "execute",

    -- -- You could also take the icon in the name, but this is not clickable.
    -- name = CreateAtlasMarkup("transmog-icon-revert-small", 20, 20),

    name = "Reset",
    image = "Interface\\Transmogrify\\Transmogrify",
    imageCoords = resetButtonImageCoords,
    imageWidth = 25/1.5,
    imageHeight = 24/1.5,
    desc = "Reset to global default:" .. " " .. tooltipDefaultValue,
    order = order,
    width = 0.25,
    func =
      function()
        SetCVar(index1, tooltipDefaultValue)
      end, 
  }
end

local sliderWidth = 1.9

-- ConsoleXP Config Options Table
local options = {
    name = "ConsoleXP Config",
    type = "group",
    args = {
        general = {
            type = "group",
            name = "General",
            order = 1,
            args = {
                cxp_enableActionTarget = {
                    type = "toggle",
                    name = "Enable Action Target",
                    get = function() return GetCVarBoolean("cxp_enableActionTarget") end,
                    set = function(_, val) SetCVar("cxp_enableActionTarget", val and "1" or "0") end,
                    order = 1,
                },
                cxp_enableActionTargetFrame = {
                    type = "toggle",
                    name = "Hide Action Target Frame",
                    get = function() return ConsoleXPSettings.hideActionTargetFrame end,
                    set = function(_, val) ConsoleXPSettings.hideActionTargetFrame = val end,
                    order = 1.1,
                },
                cxp_enableHighlightAura = {
                    type = "toggle",
                    name = "Enable Highlight Aura",
                    get = function() return GetCVarBoolean("cxp_enableHighlightAura") end,
                    set = function(_, val) SetCVar("cxp_enableHighlightAura", val and "1" or "0") end,
                    order = 2,
                },                
                cxp_highlightAuraSpellID = {
                    type = "input",
                    name = "Highlight Aura Spell ID",
                    desc = "Spell ID to use for highlight aura",
                    get = function() return GetCVar("cxp_highlightAuraSpellID") or "54273" end,
                    set = function(_, val)
                        if tonumber(val) then
                            SetCVar("cxp_highlightAuraSpellID", val)
                        end
                    end,
                    order = 3,
                    width = sliderWidth,
                    disabled = function() 
                        return not GetCVarBoolean("cxp_enableHighlightAura") 
                    end,
                },
                cxp_enableHighlightInteract = {
                    type = "toggle",
                    name = "Enable Highlighting Interact Key Target",
                    get = function() return GetCVarBoolean("cxp_enableHighlightInteract") end,
                    set = function(_, val) SetCVar("cxp_enableHighlightInteract", val and "1" or "0") end,
                    order = 4,
                },
                cxp_enableHighlightMouseOver = {
                    type = "toggle",
                    name = "Enable Highlighting Interact On MouseOver Target",
                    get = function() return GetCVarBoolean("cxp_enableHighlightMouseOver") end,
                    set = function(_, val) SetCVar("cxp_enableHighlightMouseOver", val and "1" or "0") end,
                    order = 5,
                },                
                cxp_enableMouselookCrosshair = {
                    type = "toggle",
                    name = "Enable Mouselook Crosshair",
                    get = function() return ConsoleXPSettings.enableCrosshair end,
                    set = function(_, val) ConsoleXPSettings.enableCrosshair = val end,
                    order = 6,
                },
                
                cxp_alwaysShowMouselookCrosshair = {
                    type = "toggle",
                    disabled = function() 
                        if not ConsoleXPSettings.enableCrosshair then
                            return true
                        end
                    end,
                    name = "Always Show Crosshair",
                    desc = "Always show crosshair, default behavior is to show only in Mouselook state.",
                    get = function() return ConsoleXPSettings.alwaysShowCrosshair end,
                    set = function(_, val) ConsoleXPSettings.alwaysShowCrosshair = val end,
                    order = 7,
                },
                cxp_setCrosshairPos = {
                    type = "execute",
                    disabled = function() 
                        if not ConsoleXPSettings.enableCrosshair then
                            return true
                        end
                    end,
                    name = "Set Crosshair Position",
                    desc = "Changes the crosshair position on screen",
                    order = 7.1,
                    func = function()
                        addon:EnableCrosshairDrag() 
                        end,
                },
                
                cxp_resetCrosshairPos = {
                    type = "execute",
                    disabled = function() 
                        if not ConsoleXPSettings.enableCrosshair then
                            return true
                        end
                    end,
                    name = "Reset Crosshair Position",
                    order = 7.2,
                    func = function()
                            SetCVar("cxp_virtualMouseX", 0)
                            SetCVar("cxp_virtualMouseY", 0)
                        end,
                },
            },
        },

        targeting = {
            type = "group",
            name = "Targeting",
            order = 2,
            args = {
                cxp_targetingRangeCone = {
                    type = "range",
                    name = "Targeting Range Cone",
                    desc = "Targeting cone range. Default is 2.2",
                    width = sliderWidth,
                    min = 0,
                    max = 100,
                    step = 0.01,
                    get = function() return GetCVarNumber("cxp_targetingRangeCone", 2.2) end,
                    set = function(_, val) SetCVar("cxp_targetingRangeCone", tostring(val)) end,
                    order = 1,
                },
                cxp_targetingRangeConeReset = CreateSliderResetButton(1.1, "cxp_targetingRangeCone", "2.2"),
                blank1 = { type = "description", name = "\n\n", order = 1.2 },

                cxp_actionTargetingCone = {
                    type = "range",
                    name = "Action Targeting Cone",
                    desc = "Action targeting cone. Default is 0.30",
                    width = sliderWidth,
                    min = 0,
                    max = 10,
                    step = 0.01,
                    get = function() return GetCVarNumber("cxp_actionTargetingCone", 0.30) end,
                    set = function(_, val) SetCVar("cxp_actionTargetingCone", tostring(val)) end,
                    order = 2,
                },
                cxp_actionTargetingConeReset = CreateSliderResetButton(2.1, "cxp_actionTargetingCone", "0.30"),
                blank2 = { type = "description", name = "\n\n", order = 2.2 },
            },
        },
        
        dynamicCam = {
            type = "group",
            name = "Dynamic Camera",
            order = 3,
            childGroups = "tab",
            disabled = function() 
                if DynamicCam then
                    return true
                end
            end,
            args = {
                general = {
                    type = "group",
                    name = "Camera Offset",
                    order = 1,
                    args = {
                        dynacamOffset = {
                            type = "range",
                            name = "Camera Offset",
                            desc = "Camera offset between -15 and 15. Default is 0",
                            width = sliderWidth,
                            min = -15,
                            max = 15,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraOverShoulder", 0) end,
                            set = function(_, val) SetCVar("test_cameraOverShoulder", tostring(val)) end,
                            order = 1,
                        },
                        dynacamOffsetReset = CreateSliderResetButton(1.1, "test_cameraOverShoulder", "0"),
                        blank1 = { type = "description", name = "\n\n", order = 1.2 },

                        dynacamPitchOffset = {
                            type = "range",
                            name = "Pitch Offset",
                            desc = "Adjusts pitch offset. Default is 0",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.001,
                            get = function() return GetCVarNumber("test_cameraDynamicPitchBaseFovPad", 0) end,
                            set = function(_, val) SetCVar("test_cameraDynamicPitchBaseFovPad", tostring(val)) end,
                            order = 2,
                        },
                        dynacamPitchOffsetReset = CreateSliderResetButton(2.1, "test_cameraDynamicPitchBaseFovPad", "0"),
                        blank2 = { type = "description", name = "\n\n", order = 2.2 },

                        dynacamPitchCamHeightOffset = {
                            type = "range",
                            name = "Pitch Cam Height Offset",
                            desc = "Adjusts pitch camera height. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.001,
                            get = function() return GetCVarNumber("test_cameraDynamicPitchHeight", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraDynamicPitchHeight", tostring(val)) end,
                            order = 3,
                        },
                        dynacamPitchCamHeightOffsetReset = CreateSliderResetButton(3.1, "test_cameraDynamicPitchHeight", "0.5"),
                        blank3 = { type = "description", name = "\n\n", order = 3.2 },

                        dynacamPitchFlyingOffset = {
                            type = "range",
                            name = "Pitch Flying Offset",
                            desc = "Adjusts pitch offset while flying. Default is 0",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.001,
                            get = function() return GetCVarNumber("test_cameraDynamicPitchBaseFovPadFlying", 0) end,
                            set = function(_, val) SetCVar("test_cameraDynamicPitchBaseFovPadFlying", tostring(val)) end,
                            order = 4,
                        },
                        dynacamPitchFlyingOffsetReset = CreateSliderResetButton(4.1, "test_cameraDynamicPitchBaseFovPadFlying", "0"),
                        blank4 = { type = "description", name = "\n\n", order = 4.2 },

                        dynacamPitchCamHeightFlyingOffset = {
                            type = "range",
                            name = "Pitch Cam Height Flying Offset",
                            desc = "Height offset while flying. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.001,
                            get = function() return GetCVarNumber("test_cameraDynamicPitchHeightFlying", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraDynamicPitchHeightFlying", tostring(val)) end,
                            order = 5,
                        },
                        dynacamPitchCamHeightFlyingOffsetReset = CreateSliderResetButton(5.1, "test_cameraDynamicPitchHeightFlying", "0.5"),
                        blank5 = { type = "description", name = "\n\n", order = 5.2 },
                    },
                },
                targeting = {
                    type = "group",
                    name = "Targeting",
                    order = 2,
                    args = {
                        dynacamTargetEnemyTrack = {
                            type = "toggle",
                            name = "Target Enemy Focus",
                            get = function() return GetCVarBoolean("test_cameraTargetFocusEnemyEnable") end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusEnemyEnable", val and "1" or "0") end,
                            order = 1,
                        },
                        dynacamTargetInteractTrack = {
                            type = "toggle",
                            name = "Target Interact Focus",
                            get = function() return GetCVarBoolean("test_cameraTargetFocusInteractEnable") end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusInteractEnable", val and "1" or "0") end,
                            order = 2,
                        },

                        dynacamTargetTrackMinDistance = {
                            type = "range",
                            name = "Target Focus Min Distance",
                            desc = "Minimum distance to focus target. Default is 0",
                            width = sliderWidth,
                            min = 0,
                            max = 20,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusMinDistance", 0) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusMinDistance", tostring(val)) end,
                            order = 3,
                        },
                        dynacamTargetTrackMinDistanceReset = CreateSliderResetButton(3.1, "test_cameraTargetFocusMinDistance", "0"),
                        blank3 = { type = "description", name = "\n\n", order = 3.2 },

                        dynacamTargetTrackMaxDistance = {
                            type = "range",
                            name = "Target Focus Max Distance",
                            desc = "Maximum distance before losing focus. Default is 100",
                            width = sliderWidth,
                            min = 21,
                            max = 100,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusMaxDistance", 100) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusMaxDistance", tostring(val)) end,
                            order = 4,
                        },
                        dynacamTargetTrackMaxDistanceReset = CreateSliderResetButton(4.1, "test_cameraTargetFocusMaxDistance", "100"),
                        blank4 = { type = "description", name = "\n\n", order = 4.2 },

                        dynacamTargetEnemyStrengthYaw = {
                            type = "range",
                            name = "Target Enemy Strength Yaw",
                            desc = "Yaw strength toward enemy. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusEnemyStrengthYaw", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusEnemyStrengthYaw", tostring(val)) end,
                            order = 5,
                        },
                        dynacamTargetEnemyStrengthYawReset = CreateSliderResetButton(5.1, "test_cameraTargetFocusEnemyStrengthYaw", "0.5"),
                        blank5 = { type = "description", name = "\n\n", order = 5.2 },

                        dynacamTargetEnemyStrengthPitch = {
                            type = "range",
                            name = "Target Enemy Strength Pitch",
                            desc = "Pitch strength toward enemy. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusEnemyStrengthPitch", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusEnemyStrengthPitch", tostring(val)) end,
                            order = 6,
                        },
                        dynacamTargetEnemyStrengthPitchReset = CreateSliderResetButton(6.1, "test_cameraTargetFocusEnemyStrengthPitch", "0.5"),
                        blank6 = { type = "description", name = "\n\n", order = 6.2 },

                        dynacamTargetInteractStrengthYaw = {
                            type = "range",
                            name = "Target Interact Strength Yaw",
                            desc = "Yaw strength for interactables. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusInteractStrengthYaw", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusInteractStrengthYaw", tostring(val)) end,
                            order = 7,
                        },
                        dynacamTargetInteractStrengthYawReset = CreateSliderResetButton(7.1, "test_cameraTargetFocusInteractStrengthYaw", "0.5"),
                        blank7 = { type = "description", name = "\n\n", order = 7.2 },

                        dynacamTargetInteractStrengthPitch = {
                            type = "range",
                            name = "Target Interact Strength Pitch",
                            desc = "Pitch strength for interactables. Default is 0.5",
                            width = sliderWidth,
                            min = 0,
                            max = 1,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraTargetFocusInteractStrengthPitch", 0.5) end,
                            set = function(_, val) SetCVar("test_cameraTargetFocusInteractStrengthPitch", tostring(val)) end,
                            order = 8,
                        },
                        dynacamTargetInteractStrengthPitchReset = CreateSliderResetButton(8.1, "test_cameraTargetFocusInteractStrengthPitch", "0.5"),
                        blank8 = { type = "description", name = "\n\n", order = 8.2 },
                    },
                },
                headMovement = {
                    type = "group",
                    name = "Head Movement",
                    order = 3,
                    args = {
                        dynacamHeadMovementStrength = {
                            type = "range",
                            name = "Head Movement Strength",
                            desc = "Strength of head movement. Default is 0",
                            width = sliderWidth,
                            min = 0,
                            max = 2,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraHeadMovementStrength", 0) end,
                            set = function(_, val) SetCVar("test_cameraHeadMovementStrength", tostring(val)) end,
                            order = 1,
                        },
                        dynacamHeadMovementStrengthReset = CreateSliderResetButton(1.1, "test_cameraHeadMovementStrength", "0"),
                        blank1 = { type = "description", name = "\n\n", order = 1.2 },

                        dynacamHeadMovementRangeScale = {
                            type = "range",
                            name = "Head Movement Range Scale",
                            desc = "Range scale for head movement. Default is 50",
                            width = sliderWidth,
                            min = 0,
                            max = 50,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraHeadMovementRangeScale", 50) end,
                            set = function(_, val) SetCVar("test_cameraHeadMovementRangeScale", tostring(val)) end,
                            order = 2,
                        },
                        dynacamHeadMovementRangeScaleReset = CreateSliderResetButton(2.1, "test_cameraHeadMovementRangeScale", "50"),
                        blank2 = { type = "description", name = "\n\n", order = 2.2 },

                        dynacamHeadMovementMovingStrength = {
                            type = "range",
                            name = "Head Movement Moving Strength",
                            desc = "Strength when moving. Default is 1",
                            width = sliderWidth,
                            min = 0,
                            max = 2,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraHeadMovementMovingStrength", 1) end,
                            set = function(_, val) SetCVar("test_cameraHeadMovementMovingStrength", tostring(val)) end,
                            order = 3,
                        },
                        dynacamHeadMovementMovingStrengthReset = CreateSliderResetButton(3.1, "test_cameraHeadMovementMovingStrength", "1"),
                        blank3 = { type = "description", name = "\n\n", order = 3.2 },

                        dynacamHeadMovementStandingStrength = {
                            type = "range",
                            name = "Head Movement Standing Strength",
                            desc = "Strength when standing still. Default is 1",
                            width = sliderWidth,
                            min = 0,
                            max = 2,
                            step = 0.01,
                            get = function() return GetCVarNumber("test_cameraHeadMovementStandingStrength", 1) end,
                            set = function(_, val) SetCVar("test_cameraHeadMovementStandingStrength", tostring(val)) end,
                            order = 4,
                        },
                        dynacamHeadMovementStandingStrengthReset = CreateSliderResetButton(4.1, "test_cameraHeadMovementStandingStrength", "1"),
                        blank4 = { type = "description", name = "\n\n", order = 4.2 },
                    },
                },
            }
        },
    },
}


-- Register settings
AceConfig:RegisterOptionsTable("ConsoleXP", options)
AceConfigDialog:AddToBlizOptions("ConsoleXP", "ConsoleXP") 

local AceGUI = LibStub("AceGUI-3.0")  
local WindowMixin = {}

function WindowMixin:CreateLayoutModule() 
    local AceGUI = LibStub("AceGUI-3.0")
    local layout = AceGUI:Create("InlineGroup") 
    layout:SetLayout("Fill")
    layout:SetWidth(600)
    layout:SetHeight(600)
    

    layout.frame:SetParent(self)
    layout.frame:ClearAllPoints()
    layout.frame:SetPoint("TOPLEFT", self, "TOPLEFT", 32, -32)
    layout.frame:SetPoint("BOTTOMRIGHT", self, "BOTTOMRIGHT", -32, 32)

    layout.frame:SetWidth(600)
    layout.frame:SetHeight(600)   
 
    AceConfigDialog:Open("ConsoleXP", layout) 
 
    self.Layout = layout 
    self.CreateLayoutModule = nil
    return layout
end

local function CreateConsolePortConfig() 
    if ConsolePortOldConfig and ConsolePortOldConfig.AddPanel then
        ConsolePortOldConfig:AddPanel({
            name = 'ConsoleXP',
            header = "ConsoleXP",
            mixin = WindowMixin,
            onLoad = function(self, core)
                if not self.Layout then
                    self:CreateLayoutModule()
                end
            end,
        }) 
    end
end
 
local function OnEvent(self, event, addon)
    if event == "ADDON_LOADED" and addon == "ConsolePort" then
        CreateConsolePortConfig()
    end
end
 
local frame = CreateFrame("Frame")
frame:RegisterEvent("ADDON_LOADED")
frame:SetScript("OnEvent", OnEvent)
 
if IsAddOnLoaded("ConsolePort") then
    CreateConsolePortConfig()
end