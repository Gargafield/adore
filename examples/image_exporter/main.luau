--!strict
--[[
    Example file for Adore GUI module, based on raygui's image exporter example.

    https://github.com/raysan5/raygui/blob/master/examples/image_exporter/image_exporter.c

    Original copyright: Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
    Original license: zlib/libpng
--]]


local window = require("@adore/window")
local graphics = require("@adore/graphics")
local color = require("@adore/color")
local gui = require("@adore/gui")
local task = require("@lute/task")

local SCREEN_WIDTH = 800
local SCREEN_HEIGHT = 450

window.init(SCREEN_WIDTH, SCREEN_HEIGHT, "ray gui - image exporter")
window.setfps(60)

local windowBox = { SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 - 100, 220, 190 }
local windowBoxActive = false;

local fileFormats = { "IMAGE (.png)", "RAW (.raw)", "CODE (.h)" }
local fileFormat = fileFormats[1]

local pixelFormat: graphics.ImageFormat = "r8g8b8a8"
local pixelFormats = { "grayscale", "r8g8b8a8", "r8g8b8", "r5g6b5", "r32g32b32", "r32g32b32a32" }

local textBoxEditMode = false
local fileName = buffer.create(64 + 1)

local image: graphics.Image? = nil
local texture: graphics.Texture? = nil

local imageScale = 1
local imageBox = { 0, 0, 0, 0 }

function window.update(dt)
    if window.isfiledropped() then
        local files = window.getdroppedfiles()

        if #files > 0 then
            image = graphics.image.load(files[1])
            if image then
                texture = graphics.texture.fromimage(image) :: graphics.Texture

                imageScale =
                    if texture.height > SCREEN_HEIGHT then
                        (SCREEN_HEIGHT - 100) / texture.height
                    else
                        (SCREEN_WIDTH - 100) / texture.width;
            else
                print("Failed to load image!")
            end
        end
    end

    if image then
        imageScale += (window.getmousescroll().y * 0.05)

        imageScale = math.clamp(imageScale, 0.1, 5)
        imageBox = {
            SCREEN_WIDTH / 2 - (image.width * imageScale) / 2,
            SCREEN_HEIGHT / 2 - (image.height * imageScale) / 2,
            image.width * imageScale,
            image.height * imageScale,
        }
    end
end

local function clean_to_string(buf: buffer)
    local endpos = 1
    while endpos < buffer.len(buf) and buffer.readu8(buf, endpos) ~= 0 do
        endpos += 1
    end
    return buffer.readstring(buf, 0, endpos)
end

function button_export_pressed()
    if image then
        local strfile = clean_to_string(fileName)
        local newimage = graphics.image.format(image, pixelFormat)
        print(strfile, fileFormat)
        if not strfile:find("%.%a+$") then
            if fileFormat == "IMAGE (.png)" then
                strfile ..= ".png"
            elseif fileFormat == "RAW (.raw)" then
                strfile ..= ".raw"
            elseif fileFormat == "CODE (.h)" then
                strfile ..= ".h"
            end
        end
        graphics.image.export(newimage, strfile)
    end

    windowBoxActive = false
end

function aabb(box: { number }, point: vector)
    return
        point.x >= box[1] and
        point.x <= box[1] + box[3] and
        point.y >= box[2] and
        point.y <= box[2] + box[4]
end

function window.draw()
    graphics.clearscreen(color.raywhite)
    if texture then
        graphics.texture.draw(
            texture,
            SCREEN_WIDTH / 2 - (texture.width * imageScale) / 2,
            SCREEN_HEIGHT / 2 - (texture.height * imageScale) / 2,
            0,
            imageScale,
            color.white
        )

        graphics.rectangle("line", imageBox, if aabb(imageBox, window.getmousepos()) then color.red else color.darkgray)
        graphics.print(`SCALE: {math.round(imageScale * 100)}%`, 20, SCREEN_HEIGHT - 40, 20, color.darkgray)
    else
        graphics.print("DRAG & DROP YOUR IMAGE!", 350, 200, 10, color.darkgray)
        gui.disable()
    end

    if gui.button(SCREEN_WIDTH - 170, SCREEN_HEIGHT - 50, 150, 30, "EXPORT IMAGE") then
        windowBoxActive = true
    end
    gui.enable()

    if windowBoxActive then
        graphics.rectangle("fill", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, color.fade(color.raywhite, 0.7))
        windowBoxActive = gui.windowbox(windowBox, "Image Export Options") == 0
        gui.label(windowBox[1] + 10, windowBox[2] + 35, 60, 25, "File Format:")
        fileFormat = gui.combobox(windowBox[1] + 80, windowBox[2] + 35, 130, 25, fileFormats, fileFormat)
        gui.label(windowBox[1] + 10, windowBox[2] + 70, 60, 25, "Pixel Format:")
        pixelFormat = gui.combobox(windowBox[1] + 80, windowBox[2] + 70, 130, 25, pixelFormats, pixelFormat) :: graphics.ImageFormat
        gui.label(windowBox[1] + 10, windowBox[2] + 105, 60, 25, "File Name:")
        if (gui.textbox(windowBox[1] + 80, windowBox[2] + 105, 130, 25, fileName, textBoxEditMode)) then
            textBoxEditMode = not textBoxEditMode
        end
        if gui.button(windowBox[1] + 10, windowBox[2] + 145, 200, 30, "Export Image") then
            task.spawn(button_export_pressed)
        end
    end
end
