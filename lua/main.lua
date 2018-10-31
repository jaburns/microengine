local m_camera
local m_world

function create_world()
    local world = create_entity()

    local world_trans = add_component_Transform(world)
    world_trans.name = "World"
    set_component_Transform(world, world_trans)

    local world_rend = add_component_MeshRenderer(world)
    world_rend.mesh = "models/m64_bob.umesh"
    world_rend.material = "materials/m64_bob.umat"
    set_component_MeshRenderer(world, world_rend)

    return world
end

function create_camera()
    local camera = create_entity()

    local ct = add_component_Transform(camera)
    ct.name = "Main Camera"
    ct.position.z = -10
    set_component_Transform(camera, ct)

    add_component_Camera(camera)

    return camera
end

return {
    start = function()
        m_world = create_world()
        m_camera = create_camera()
    end,

    update = function()

    end
}