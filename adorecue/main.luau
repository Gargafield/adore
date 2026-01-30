
local graphics = require("@adore/graphics")
local input = require("@adore/input")
local window = require("@adore/window")
local hyperdeck = require("@adore/hyperdeck")
local atem = require("@adore/atem")
local gui = require("@adore/gui")
local task = require("@lute/task")

local operations = require("./standardoperations")

local devices = require("./devices")

devices.connectHyperdeck("192.168.0.95")
devices.connectAtem("192.168.0.90")

local transitionHyperdeck = connector.connectHyperdeck("192.168.1.17") :: hyperdeck.HyperDeck
local programHyperdeck = connector.connectHyperdeck("192.168.1.19") :: hyperdeck.HyperDeck
local programAtem = connector.connectAtem("192.168.0.90") :: atem.Atem
local streamAtem = connector.connectAtem("192.168.1.18") :: atem.Atem

local _1GD_INTRO_CLIP = 1
local _1GD_WINNER_CLIP = 2
local _2Z_INTRO_CLIP = 3
local _2Z_WINNER_CLIP = 4
local CINEMATIC_CLIP = 5
local COUNTDOWN_CLIP = 6
local IDLE_CLIP = 7
local MAP_VETO_CLIP = 8

local SCREEN_WIDTH = 1440
local SCREEN_HEIGHT = 810

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "AdoreCue")
window.setfps(30)

local devicelist = require("./devicelist")()

function window.update(dt)

end

local function goCountdown()
    programHyperdeck:play(COUNTDOWN_CLIP, { loop = true, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
end

local function goCinematic()
    programAtem:fadetoblack(true);
    task.wait(6)
    programHyperdeck:play(CINEMATIC_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
    task.wait(1)
    programAtem:fadetoblack(false);
end

local function goIdleHost()
    programAtem:fadetoblack(true);
    task.wait(3)
    programHyperdeck:play(IDLE_CLIP, { loop = true, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
    task.wait(1)
    programAtem:fadetoblack(false);
end

local function go1GDIntro()
    programAtem:fadetoblack(true);
    task.wait(1)
    programHyperdeck:play(_1GD_INTRO_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
    task.wait(1)
    programAtem:fadetoblack(false);
end

local function go2ZIntro()
    programAtem:fadetoblack(true);
    task.wait(1)
    programHyperdeck:play(_2Z_INTRO_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
    task.wait(1)
    programAtem:fadetoblack(false);
end

local function goMapPool()
    programAtem:fadetoblack(true);
    task.wait(1)
    programHyperdeck:play(MAP_VETO_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[3])
    streamAtem:setprogram(streamAtem.input[1])
    task.wait(1)
    programAtem:fadetoblack(false);
end

local function go1GDWinner()
    programHyperdeck:play(_1GD_WINNER_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[2])
end

local function go2ZWinner()
    programHyperdeck:play(_2Z_WINNER_CLIP, { loop = false, single = true })
    programAtem:setprogram(programAtem.input[2])
end

local function goBombTransition()
    transitionHyperdeck:goto("position", 1)
    transitionHyperdeck:play(2, { loop = false, single = true })
    task.wait(0.5)
    programAtem:setprogram(programAtem.input[2])
end

local function goAWPTransition()
    transitionHyperdeck:goto("position", 1)
    transitionHyperdeck:play(1, { loop = false, single = true })
    task.wait(0.5)
    programAtem:transition(programAtem.input[1])
end

function window.draw()
    devicelist(SCREEN_WIDTH - (SCREEN_WIDTH / 4), 0, (SCREEN_WIDTH / 4), SCREEN_HEIGHT)

    if gui.button(10, 10, 100, 30, "Countdown") then
        task.spawn(goCountdown)
    end
    if gui.button(120, 10, 100, 30, "Cinematic") then
        task.spawn(goCinematic)
    end
    if gui.button(230, 10, 100, 30, "Idle Host") then
        task.spawn(goIdleHost)
    end
    if gui.button(340, 10, 100, 30, "1GD Intro") then
        task.spawn(go1GDIntro)
    end
    if gui.button(340, 50, 100, 30, "1GD Winner") then
        task.spawn(go1GDWinner)
    end
    if gui.button(450, 10, 100, 30, "2Z Intro") then
        task.spawn(go2ZIntro)
    end
    if gui.button(450, 50, 100, 30, "2Z Winner") then
        task.spawn(go2ZWinner)
    end
    if gui.button(560, 10, 100, 30, "Map Pool") then
        task.spawn(goMapPool)
    end
    if gui.button(10, 140, 100, 30, "Bomb Transition") then
        task.spawn(goBombTransition)
    end
    if gui.button(120, 140, 100, 30, "AWP Transition") then
        task.spawn(goAWPTransition)
    end
end


