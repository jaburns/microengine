local main = {}

local camera
local teapot

function main.start()
    teapot = create_entity()
    add_component_Transform(teapot)
    add_component_Teapot(teapot)

    camera = create_entity()
    add_component_Transform(camera)
    add_component_Camera(camera)
end

function main.update()
    local t = get_component_Transform(teapot)
    t.position.x = math.sin(5 *  os.clock())
    set_component_Transform(teapot, t)

    t = get_component_Transform(camera)
    t.position.z = 5 + 2*math.cos(5 *  os.clock())
    set_component_Transform(camera, t)
end

print("Hello from Lua")

return main
