
local color = require("@adore/color")

local graphics = {}

type TextureFilter = number 

export type Texture = {
    id: number,
    width: number,
    height: number,
}

export type Image = {
    width: number,
    height: number,
    format: ImageFormat,
}

export type ImageFormat = 
    "grayscale" |
    "gray_alpha" |
    "r5g6b5" |
    "r8g8b8" |
    "r5g5b5a1" |
    "r4g4b4a4" |
    "r8g8b8a8" |
    "r32" |
    "r32g32b32" |
    "r32g32b32a32" |
    "r16" |
    "r16g16b16" |
    "r16g16b16a16" |
    "unknown"

graphics.texture = {} :: {
    load: (path: string) -> Texture,
    fromimage: (image: Image) -> Texture,
    draw: ((self: Texture, x: number, y: number, tint: color.Color?) -> ())
        & (self: Texture, x: number, y: number, rotation: number, scale: number, tint: color.Color?) -> (),
    setfilter: (self: Texture, filter: TextureFilter) -> (),
    filter: {
        point: TextureFilter,
        bilinear: TextureFilter,
        trilinear: TextureFilter,
        anisotropic_4x: TextureFilter,
        anisotropic_8x: TextureFilter,
        anisotropic_16x: TextureFilter,
    }
}

graphics.image = {} :: {
    load: (path: string, size: number?) -> Image,
    format: (image: Image, format: ImageFormat) -> Image,
    export: (image: Image, path: string) -> (),
}

type Font = {
    texture: Texture,
}

graphics.font = {} :: {
    load: (path: string) -> Font,
    draw: (font: Font, x: number, y: number, size: number, text: string, color: color.Color, spacing: number?) -> (),
    measure: (font: Font, size: number, text: string, spacing: number?) -> vector,
    getdefault: () -> Font,
}

function graphics.clearscreen(color: color.Color)
    error("Not implemented")
end
function graphics.print(text: string, x: number, y: number, fontsize: number, color: color.Color)
    error("Not implemented")
end
function graphics.circle(mode: string, x: number, y: number, radius: number, color: color.Color)
    error("Not implemented")
end

type RectangleDrawMode = "fill" | "line"

graphics.rectangle = (nil :: any)
    :: ((mode: RectangleDrawMode, x: number, y: number, width: number, height: number, color: color.Color) -> ())
    & ((mode: RectangleDrawMode, box: { number }, color: color.Color) -> ())

return graphics