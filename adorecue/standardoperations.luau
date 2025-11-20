--!nonstrict

local operations = require("./operations")
local devices = require("./devices")
local state = require("./state")

local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")

local standard = {}

type WaitType = "frames" | "seconds"

standard.Wait = operations.register(
    "Wait",
    function(type: WaitType, amount: number): (boolean, string?)

        if type == "frames" then
            local frameDuration = 1 / 60
            amount = amount * frameDuration
        elseif type == "seconds" then
            task.wait(amount)
        elseif type ~= "seconds" then
            return false, "Invalid wait type: " .. tostring(type)
        end

        return true
    end :: (WaitType, number) -> (boolean, string?)
)

standard.PlayClip = operations.register(
    "Play Clip",
    function(deviceId: number, clipId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "hyperdeck" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not a HyperDeck."
        end

        local hd = device.object :: hyperdeck.HyperDeck
        hd:play(clipId, {single = true, loop = false})

        return true
    end :: (number) -> (boolean, string?)
)

standard.LoopClip = operations.register(
    "Loop Clip",
    function(deviceId: number, clipId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "hyperdeck" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not a HyperDeck."
        end

        local hd = device.object :: hyperdeck.HyperDeck
        hd:play(clipId, {single = true, loop = true})

        return true
    end :: (number) -> (boolean, string?)
)

standard.StopClip = operations.register(
    "Stop Clip",
    function(deviceId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "hyperdeck" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not a HyperDeck."
        end

        local hd = device.object :: hyperdeck.HyperDeck
        hd:stop()

        return true
    end :: (number) -> (boolean, string?)
)

standard.SetProgram = operations.register(
    "Set Program",
    function(deviceId: number, sourceId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "atem" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not an ATEM."
        end

        local atemDevice = device.object :: atem.Atem
        atemDevice:setprogram(sourceId)

        return true
    end :: (number, number) -> (boolean, string?)
)

standard.SetPreview = operations.register(
    "Set Preview",
    function(deviceId: number, sourceId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "atem" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not an ATEM."
        end

        local atemDevice = device.object :: atem.Atem
        atemDevice:setpreview(sourceId)

        return true
    end :: (number, number) -> (boolean, string?)
)

standard.Cut = operations.register(
    "Cut",
    function(deviceId: number): (boolean, string?)
        local device = devices.getDeviceById(deviceId)
        if not device then
            return false, "Device with ID " .. tostring(deviceId) .. " not found."
        end

        if device.type ~= "atem" then
            return false, "Device with ID " .. tostring(deviceId) .. " is not an ATEM."
        end

        local atemDevice = device.object :: atem.Atem
        atemDevice:cut()

        return true
    end :: (number) -> (boolean, string?)
)

return standard
