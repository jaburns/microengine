local main = {}

local camera
local teapot

function main.start()
    camera = create_entity()
    teapot = create_entity()
    local teapot_parent = create_entity()

    local tpt = add_component_Transform(teapot_parent)
    tpt.name = "Top Dog"
    --set_component_Transform(teapot_parent, tpt)
    add_component_Teapot(teapot_parent)

    local tt = add_component_Transform(teapot)
    tt.name = "Child"
    tt.parent = teapot_parent
    set_component_Transform(teapot, tt)
    add_component_Teapot(teapot)

    local ct = add_component_Transform(camera)
    ct.name = "Main Camera"
    set_component_Transform(camera, ct)
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

return main
