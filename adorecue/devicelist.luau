--!strict

local graphics = require("@adore/graphics")
local gui = require("@adore/gui")
local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")
local colors = require("@adore/colors")
local task = require("@lute/task")
local timecode = require("@adore/timecode")
local input = require("@adore/input")

local state = require("./state")
local devices = require("./devices")
local bufutil = require("./bufutil")

local function drawHyperdeck(device: state.Device, x: number, y: number, width: number, height: number)
    local hd = device.object :: hyperdeck.HyperDeck

    graphics.rectangle("fill", x, y, width, 100, colors.lightgray)

    local newValue = gui.spinner(x + 5, y + 5, 80, 20, device.id or 0, 0, 99, nil, false)
    if newValue ~= device.id then
        device.id = newValue
    end

    graphics.print(`{hd.model}`, x + 90, y + 5, 20, colors.black)

    if hd.state ~= "connected" then
        graphics.print("Status: " .. hd.state, x + 5, y + 35, 16, colors.red)
        return 100
    end

    local playingclipid = hd.clip
    local timelineClip = hd.timeline[hd.clip or 1]
    local diskClip = hd.clips[timelineClip.id]

    local thisplaying = (playingclipid ~= nil and timelineClip.id == playingclipid)
    local text = string.format("%03d - %s (%s)", timelineClip.id, diskClip.name, diskClip.duration)
    graphics.print("Clip: " .. text, x + 5, y + 30, 10, colors.black)

    -- Slider
    local format = hyperdeck.formats[diskClip.format]
    local startRange = timecode.parse(timelineClip.start, format.framerate).total + 1
    local endRange = timecode.parse(timelineClip.duration, format.framerate).total + startRange - 1
    local position = math.clamp(hd.position or 0, startRange, endRange)

    local scrub = gui.sliderbar(x + 60, y + 45, width - 120, 10, timelineClip.start, timelineClip.duration, position, startRange, endRange)
    if scrub then
        scrub = math.max(math.round(scrub), 1)
        task.spawn(function()
            hd:goto("position", scrub)
        end)
    end

    -- Skip Backward button
    if gui.button(x + 5, y + 65, 30, 30, "#129#") then
        task.spawn(function()
            hd:goto("clip", math.max((hd.clip or 1) - 1, 1))
        end)
    elseif gui.button(x + 75, y + 65, 30, 30, "#134#") then
        task.spawn(function()
            hd:goto("clip", math.min((hd.clip or 1) + 1, #hd.timeline))
        end)
    end

    -- Play/Stop button
    local isplaying = hd.status ~= "stopped"
    if gui.button(x + 40, y + 65, 30, 30, isplaying and "#133#" or "#131#") then
        task.spawn(function()
            if isplaying then
                hd:stop()
            else
                hd:play()
            end
        end)
    end

    return 100
end

local function get_styles(control: gui.GuiControls, property: gui.GuiProperties)
    local styles = {}
    for _, state: gui.GuiStates in { 'none', 'normal', 'focused', 'pressed', 'disabled' } :: { gui.GuiStates } do
        styles[state] = gui.getstyle(control, property, state)
    end
    return styles
end

local function apply_styles(control: gui.GuiControls, property: gui.GuiProperties, styles: { [gui.GuiStates]: number | vector })
    for state: gui.GuiStates, value in styles do
        gui.setstyle(control, property, state, value)
    end
end

local function drawAtem(device: state.Device, x: number, y: number, width: number, height: number)
    local at = device.object :: atem.Atem

    graphics.rectangle("fill", x, y, width, 100, colors.lightgray)

    local newValue = gui.spinner(x + 5, y + 5, 80, 20, device.id or 0, 0, 99, nil, false)
    if newValue ~= device.id then
        device.id = newValue
    end

    graphics.print(`{at.name}`, x + 90, y + 5, 20, colors.black)

    local sources = at.sources
    local inputs = at.input
    local preview = at:getpreview()
    local program = at:getprogram()

    -- Input controls
    local inputWidth = (width - 15) / #inputs
    local buttonStyles = get_styles("button", "base") :: any
    for i, inputSource in inputs do
        local changedBase = true
        local ispreview = preview ~= nil and inputSource.id == preview.id
        local isprogram = program ~= nil and inputSource.id == program.id
        if ispreview and isprogram then
            gui.setstyle("button", "base", "normal", colors.yellow)
        elseif isprogram then
            gui.setstyle("button", "base", "normal", colors.red)
        elseif ispreview then
            gui.setstyle("button", "base", "normal", colors.green)
        else
            changedBase = false
        end

        if gui.button(x + 5 + ((i - 1) * inputWidth), y + 35, inputWidth - 5, 30, `{inputSource.id}\n{inputSource.shortname}`) then
            task.spawn(function()
                if input.isdown(input.keys["left_shift"]) then
                    at:setprogram(inputSource)
                else
                    at:setpreview(inputSource)
                end
            end)
        end

        if changedBase then
            apply_styles("button", "base", buttonStyles)
        end
    end

    return 90
end

return function ()
    local scroll = -1

    local addressBuffer = buffer.create(16)
    local function resetAddressBuffer()
        buffer.writestring(addressBuffer, 0, "")
    end
    resetAddressBuffer()
    local selectedDeviceType = "deck"
    local deviceTypes = { "deck", "atem" }
    local errorMessage = ""

    local addressEditMode = false

    return function(x: number, y: number, width: number, height: number) 
        local devices = state.devices

        gui.panel(x, y, width, height, "Devices")
        if gui.textbox(x + 10, y + 40, width - 190, 30, addressBuffer, addressEditMode) then
            addressEditMode = not addressEditMode
        end

        selectedDeviceType = gui.combobox(x + width - 170, y + 40, 100, 30, deviceTypes, selectedDeviceType)
        if errorMessage ~= "" then
            graphics.print(errorMessage, x + 10, y + 80, 16, colors.red)
        end
        
        if (gui.button(x + width - 60, y + 40, 50, 30, "Add")) then
            local address = bufutil.readcstring(addressBuffer, 0)
            if address == "Adress..." then
                errorMessage = "Please enter a valid address."
                return
            elseif address == "" then
                errorMessage = "Address cannot be empty."
                return
            elseif not address:match("^%d+%.%d+%.%d+%.%d+$") then
                errorMessage = "Invalid IP address format."
                return
            end
            errorMessage = ""

            -- if selectedDeviceType == "deck" then
            --     task.spawn(devices, address)
            -- elseif selectedDeviceType == "atem" then
            --     task.spawn(devices.connectAtem, address)
            -- end
        end

        scroll = gui.list(x, y + 100, width, height - 95, #devices, scroll, function(index: number, x: number, y: number, width: number, height: number)
            local usedHeight = 0
            local device = devices[index]
            if device.type == "hyperdeck" then
                usedHeight += drawHyperdeck(device, x, y + usedHeight, width, height - usedHeight)
            elseif device.type == "atem" then
                usedHeight += drawAtem(device, x, y + usedHeight, width, height - usedHeight)
            end

            return usedHeight
        end)
    end
end
