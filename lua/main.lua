local main = {}

print("Hello from Lua")

function main.start()
    print("Hello from Lua main.start")

    t = get_component_Transform(0)
    t.position.z = t.position.z - 1
    set_component_Transform(0, t)
end

function main.update()
    print("Hello from Lua main.update")
end

return main
