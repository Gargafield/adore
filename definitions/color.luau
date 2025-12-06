
export type Color = vector;

local color = {
    white = vector.create(0, 0, 0),
    black = vector.create(0, 0, 0),
    red = vector.create(0, 0, 0),
    green = vector.create(0, 0, 0),
    blue = vector.create(0, 0, 0),
    yellow = vector.create(0, 0, 0),
    orange = vector.create(0, 0, 0),
    pink = vector.create(0, 0, 0),
    purple = vector.create(0, 0, 0),
    maroon = vector.create(0, 0, 0),
    lime = vector.create(0, 0, 0),
    skyblue = vector.create(0, 0, 0),
    violet = vector.create(0, 0, 0),
    darkgray = vector.create(0, 0, 0),
    gray = vector.create(0, 0, 0),
    lightgray = vector.create(0, 0, 0),
    darkblue = vector.create(0, 0, 0),
    darkgreen = vector.create(0, 0, 0),
    brown = vector.create(0, 0, 0),
    darkbrown = vector.create(0, 0, 0),
    raywhite = vector.create(0, 0, 0),
    blank = vector.create(0, 0, 0),
}

function color.from(arg: number | string): Color
    error("Not implemented")
end

function color.fade(col: Color, alpha: number): Color
    error("Not implemented")
end

return color