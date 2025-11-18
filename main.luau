--!strict

local atem = require("@adore/atem")
local task = require("@lute/task")

local myatem = atem.connect("192.168.0.90")

print(myatem.name)
for _, input in myatem.input do
    print(`{input.id}: {input.name} ({input.shortname}) - {input.type}`)
end

myatem:setprogram(myatem.input[1])
myatem:setpreview(myatem.input[2])

task.wait(2.0)

myatem:transition()

task.wait(2.0)

myatem:close()

