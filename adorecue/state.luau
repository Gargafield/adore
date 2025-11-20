
local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")

export type DeviceType = "hyperdeck" | "atem"

export type Device = {
    name: string,
    type: DeviceType,
    id: number,
    object: hyperdeck.HyperDeck | atem.Atem,
}

export type SourceType = "clip"

export type Source = {
    name: string,
    id: number,
    deviceId: number,
    type: SourceType
}

export type Operation<T...> = {
    name: string,
    id: number,
    action: (T...) -> (boolean, string?),
}

export type Task = {
    operationId: number,
    params: { any },
}

export type Timeline = {
    name: string,
    tasks: { Task },
}

return {
    devices = {} :: { Device },
    operations = {} :: { Operation<...any> },
    timelines = {} :: { Timeline },
}