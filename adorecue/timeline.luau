
local state = require("./state")

local timeline = {}

function timeline.create(name: string): state.Timeline
    local newTimeline: state.Timeline = {
        name = name,
        tasks = {},
    }
    table.insert(state.timelines, newTimeline)
    return newTimeline
end

function timeline.addTask(
    timeline: state.Timeline,
    operationId: number,
    params: { any }
): state.Task
    local newTask: state.Task = {
        operationId = operationId,
        params = params,
    }
    table.insert(timeline.tasks, newTask)
    return newTask
end

return timeline
