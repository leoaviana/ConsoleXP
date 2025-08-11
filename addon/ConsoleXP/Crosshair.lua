local addonName, addon = ...

ConsoleXPSettings = ConsoleXPSettings or {}

local Crosshair = CreateFrame("Frame", "ConsoleXP_Crosshair", UIParent)
Crosshair:SetSize(48, 48)

Crosshair.texture = Crosshair:CreateTexture(nil, "OVERLAY")
Crosshair.texture:SetAllPoints()
Crosshair.texture:SetTexture("Interface\\AddOns\\ConsoleXP\\Assets\\Crosshair.blp")

Crosshair:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
Crosshair:Show()

local function UpdateCrosshair()
    if(not ConsoleXPSettings.enableCrosshair) then
        Crosshair.texture:Hide()
        return
    end
    if(not ConsoleXPSettings.alwaysShowCrosshair) then
        if(not Crosshair.isDragging) then 
            -- Check if we're in mouselook mode
            if IsMouselooking() == nil then
                Crosshair.texture:Hide()
                return
            end
        end
    end

    Crosshair.texture:Show()

    -- Read CVars
    local offsetX = tonumber(GetCVar("cxp_virtualMouseX")) or 0
    local offsetY = tonumber(GetCVar("cxp_virtualMouseY")) or 0

    -- Update position
    Crosshair:SetPoint("CENTER", UIParent, "CENTER", offsetX, offsetY)
end

Crosshair:SetScript("OnUpdate", function(self, elapsed)
    if not Crosshair.isDragging then
        UpdateCrosshair()
    end
end)

StaticPopupDialogs["CXP_CROSSHAIR_DRAG"] = {
    text = "Drag the crosshair to the desired position.\n\nClose this window to stop dragging.",
    button1 = CANCEL,
    timeout = 0,
    whileDead = true,
    hideOnEscape = true,
    preferredIndex = 3,
    OnHide = function()
        DisableCrosshairDrag()
    end,
}

function addon:EnableCrosshairDrag() 

    if InterfaceOptionsFrame:IsShown() then
        HideUIPanel(InterfaceOptionsFrame)
    end 
    if GameMenuFrame:IsShown() then
        HideUIPanel(GameMenuFrame)
    end
    if ConsolePortOldConfig and ConsolePortOldConfig:IsShown() then
        ConsolePortOldConfig:Hide()
    end

    Crosshair.isDragging = true
    Crosshair.texture:Show()
    Crosshair:EnableMouse(true)
    Crosshair:RegisterForDrag("LeftButton")
    Crosshair:SetMovable(true)

    Crosshair:SetScript("OnDragStart", function(self)
        self:StartMoving()
    end)

    Crosshair:SetScript("OnDragStop", function(self)
        self:StopMovingOrSizing()

        local centerX, centerY = UIParent:GetCenter()
        local posX, posY = self:GetCenter()

        local offsetX = math.floor(posX - centerX + 0.5)
        local offsetY = math.floor(posY - centerY + 0.5)

        SetCVar("cxp_virtualMouseX", offsetX)
        SetCVar("cxp_virtualMouseY", offsetY)

        UpdateCrosshair() 
    end)

    StaticPopup_Show("CXP_CROSSHAIR_DRAG")
end

-- Disable Drag Mode
function DisableCrosshairDrag()
    Crosshair:EnableMouse(false)
    Crosshair:SetMovable(false)
    Crosshair:SetScript("OnDragStart", nil)
    Crosshair:SetScript("OnDragStop", nil)
    Crosshair.isDragging = false
end