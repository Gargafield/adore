
local atem = require("@adore/atem")
local task = require("@lute/task")

local myatem = atem.connect("192.168.0.90")

print(myatem.name)
myatem:fadetoblack(true)
task.wait(3)
myatem:fadetoblack(false)

myatem:close()

