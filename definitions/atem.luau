
local atem = {}

export type AtemInput = {
    id: number,
    shortname: string,
    name: string,
    type: string,
}

type AtemInputId = AtemInput

export type Atem = {
    name: string,
    input: { AtemInput },
    sources: { AtemInput },

    setprogram: (self: Atem, input: AtemInputId) -> (),
    getprogram: (self: Atem) -> AtemInput?,
    setpreview: (self: Atem, input: AtemInputId) -> (),
    getpreview: (self: Atem) -> AtemInput?,
    fadetoblack: (self: Atem, on: boolean) -> (),

    transition: ((self: Atem) -> ())
        & (self: Atem, input: AtemInputId) -> (),
    cut: ((self: Atem) -> ())
        & (self: Atem, input: AtemInputId) -> (),
    
    close: (self: Atem) -> (),
}

function atem.connect(address: string): Atem
    error("Not implemented")
end

return atem