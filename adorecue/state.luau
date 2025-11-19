
local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")

export type DeviceType = "hyperdeck" | "atem"

export type Device = {
    name: string,
    type: DeviceType,
    id: number,
    object: hyperdeck.HyperDeck | atem.Atem,
}

return {
    devices = {} :: { Device }
}