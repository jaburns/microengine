local main = {}

local camera
local teapot

function main.start()
    camera = create_entity()
    teapot = create_entity()
    local teapot_parent = create_entity()

    add_component_Transform(teapot_parent)
    add_component_Teapot(teapot_parent)

    local tt = add_component_Transform(teapot)
    tt.parent = teapot_parent
    set_component_Transform(teapot, tt)
    add_component_Teapot(teapot)

    add_component_Transform(camera)
    add_component_Camera(camera)
end

function main.update()
 --   local t = get_component_Transform(teapot)
 --   t.position.x = math.sin(5 *  os.clock())
 --   set_component_Transform(teapot, t)

--  t = get_component_Transform(camera)
--  t.position.z = 5 + 2*math.cos(5 *  os.clock())
--  set_component_Transform(camera, t)
end

print("Hello from Lua")

return main
