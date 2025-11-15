--!strict

local hyperdeck = require("@adore/hyperdeck")
local task = require("@lute/task")

print("Starting...")
-- task.wait(6)
print("Connecting...")
local deck = hyperdeck.connect("192.168.0.95");

deck:send("play")
deck:send("disk list: slot id: 1")
while true do
    task.wait(1)
end

deck:close()