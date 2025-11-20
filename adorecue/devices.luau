
local state = require("./state")
local atem = require("@adore/atem")
local hyperdeck = require("@adore/hyperdeck")

local devices = {}

local id_counter = 0

function devices.connectAtem(address: string): atem.Atem?
    local at = atem.connect(address)
    if at then
        table.insert(state.devices, {
            name = address,
            id = id_counter,
            type = "atem",
            object = at,
        } :: state.Device)
        id_counter += 1
        return at
    end
    return nil
end

function devices.connectHyperdeck(address: string): hyperdeck.HyperDeck?
    local hd = hyperdeck.connect(address)
    if hd then
        table.insert(state.devices, {
            name = address,
            id = id_counter,
            type = "hyperdeck",
            object = hd,
        } :: state.Device)
        id_counter += 1
        return hd
    end
    return nil
end

function devices.getDeviceById(id: number): state.Device?
    for _, device in state.devices do
        if device.id == id then
            return device
        end
    end
    return nil
end

return devices
