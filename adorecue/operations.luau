
local state = require("./state")

local operations = {}

function operations.getById<T...>(id: number): state.Operation<T...>?
    for _, operation in state.operations do
        if operation.id == id then
            return operation :: state.Operation<T...>
        end
    end
    return nil
end

local idCounter = 0

function operations.register<T...>(
    name: string,
    action: (T...) -> (boolean, string?)
)
    idCounter += 1
    local newOperation: state.Operation<T...> = {
        name = name,
        id = idCounter,
        action = action,
    }
    table.insert(state.operations, newOperation :: state.Operation<...any>)
    return newOperation
end

return operations
