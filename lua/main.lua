local main = {}

local camera
local teapot

function main.start()
    camera = create_entity()
    teapot = create_entity()
    local teapot_parent = create_entity()

    add_component_Transform(teapot_parent)
    local mr = add_component_MeshRenderer(teapot_parent)
    mr.mesh = "models/m64_bob.umesh"
    mr.material = "materials/m64_bob.umat"
    set_component_MeshRenderer(teapot_parent, mr)

    local tt = add_component_Transform(teapot)
    tt.name = "Child"
    tt.position.x = 0.5
    tt.rotation = quat(math.random(), math.random(), math.random(), math.random()):normalize()
    tt.parent = teapot_parent
    set_component_Transform(teapot, tt)

    local ct = add_component_Transform(camera)
    ct.name = "Main Camera"
    --ct.position.z = -10
    set_component_Transform(camera, ct)
    add_component_Camera(camera)
end

function main.update()
--  local t = get_component_Transform(teapot)
--  t.position.x = math.sin(5 *  os.clock())
--  set_component_Transform(teapot, t)

--  t = get_component_Transform(camera)
--  t.position.z = 5 + 2*math.cos(5 *  os.clock())
--  set_component_Transform(camera, t)
end

return main
