
local window = require("@adore/window")
local graphics = require("@adore/graphics")
local color = require("@adore/color")

window.setstate(bit32.bor(
    window.states.topmost,
    window.states.transparent,
    window.states.always_run,
    window.states.mouse_passthrough,
    window.states.undecorated
))
local SCREEN_WIDTH = 100
local SCREEN_HEIGHT = 600

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "Christmas Candle Example")
window.setfps(30)
local size = window.getmonitorsize()
window.setposition(0, size.y - SCREEN_HEIGHT)

-- Fire particle system for candle flame
type Particle = {
    position: vector,
    velocity: vector,
    size: number,
    color: color.Color,
    lifetime: number,
}
local particles = {} :: {Particle}
local MAX_PARTICLES = 100

local function time_to_christmas()
    local now = os.time()
    local current_year = os.date("*t", now).year
    local christmas = os.time({year = current_year, month = 12, day = 25, hour = 0, min = 0, sec = 0})
    if now - christmas > (24 * 60 * 60 * 3) then
        christmas = os.time({year = current_year + 1, month = 12, day = 25, hour = 0, min = 0, sec = 0})
    end
    return christmas - now
end

local function get_candle_y(total_seconds_left: number)
    if total_seconds_left < 0 then
        return SCREEN_HEIGHT - 50
    end

    return 70 + 480 - (total_seconds_left / (3600 * 24)) * 20
end

local time_left = time_to_christmas()
local time_since_calibration = 0.0

local SMOKE_DELTA_COLOR = color.rgb(-1, -1, -1, 3)
local FIRE_DELTA_COLOR = color.rgb(0, 6, 0, 3)

function window.update(dt: number)
    if time_since_calibration >= 10.0 then
        time_left = time_to_christmas()
        time_since_calibration = 0.0
    else
        time_since_calibration = time_since_calibration + dt
    end

    local is_past_christmas = time_left < 0
    local delta_color = if is_past_christmas then SMOKE_DELTA_COLOR else FIRE_DELTA_COLOR

    for i = #particles, 1, -1 do
        local p = particles[i]
        p.position += p.velocity * dt * 60
        p.velocity += vector.create(math.cos(p.lifetime * 4) * 0.025, -0.02)
        p.size *= 0.98
        p.color -= delta_color
        p.lifetime = p.lifetime - dt
        if p.lifetime <= 0 then
            table.remove(particles, i)
        end
    end

    if #particles < MAX_PARTICLES then
        local newParticle: Particle = {
            position = vector.create(
                SCREEN_WIDTH / 2 + math.random(-5, 5),
                get_candle_y(time_left) - 15
            ),
            velocity = vector.create(0, 
                if is_past_christmas then -0.1 else -0.20),
            size = math.random(5, 10),
            color = if 
                is_past_christmas then color.rgb(50, 50, 50)
                else color.rgb(255, math.random(180, 200), 0),
            lifetime = math.random(30, 60) / 30,
        }
        table.insert(particles, newParticle)
    end
end

function window.draw()
    graphics.clearscreen(color.blank)

    -- draw candle base
    local candle_y = get_candle_y(time_left)
    graphics.rectangle(
        "fill",
        SCREEN_WIDTH / 2 - 15,
        candle_y,
        30,
        SCREEN_HEIGHT - candle_y,
        color.white
    )

    graphics.rectangle(
        "fill",
        SCREEN_WIDTH / 2 - 2,
        candle_y - 10,
        4,
        13,
        color.black
    )

    graphics.rectangle(
        "fill",
        SCREEN_WIDTH / 2 - 60,
        SCREEN_HEIGHT - 40,
        120,
        40,
        color.maroon
    )

    -- day markings
    local days_left = math.floor(time_left / (24 * 60 * 60))
    for day = days_left, 1, -1 do
        local y = get_candle_y(day * 24 * 60 * 60)
        graphics.rectangle(
            "fill",
            SCREEN_WIDTH / 2 - 15,
            y,
            15,
            3,
            color.black
        )
        graphics.print(
            tostring(day),
            SCREEN_WIDTH / 2 + 5,
            y - 3,
            5,
            color.black
        )
    end

    for i = #particles, 1, -1 do
        local p = particles[i]
        graphics.circle("fill", p.position.x, p.position.y, p.size, p.color)
    end
end

