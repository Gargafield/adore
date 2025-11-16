--!strict

local hyperdeck = require("@adore/hyperdeck")
local task = require("@lute/task")

print("Starting...")
task.wait(6)
print("Connecting...")
local deck = hyperdeck.connect("192.168.0.95");

while not deck.ready and deck.state ~= "disconnected" do
    print("Waiting for deck to be ready...")
    task.wait(0.1)
end
task.wait(0.5)

if deck.state == "connected" then
    print("Deck is ready!")
    print("Model: ", deck.model)
    for _, clip in deck.clips do
        print("Clip: ", clip.name, clip.duration)
    end
else
    print("Failed to connect to deck.")
end

while deck.state == "connected" do
    task.wait(1)
end

print("Closing connection...")
deck:close()