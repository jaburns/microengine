local main = {}

local camera
local teapot

function main.start()
    teapot = create_entity()
    set_component_Transform(teapot, {
        position={x=0, y=0, z=0},
        rotation={x=0, y=0, z=0, w=1},
        scale={x=1, y=1, z=1}
    })
    set_component_Teapot(teapot, {
        placeholder = 0
    })

    camera = create_entity()
    set_component_Transform(camera, {
        position={x=0, y=0, z=5},
        rotation={x=0, y=0, z=0, w=1},
        scale={x=1, y=1, z=1}
    })
    set_component_Camera(camera, {
        fov = 3.14159 / 2, 
        near = 0.01, 
        far = 1024
    })
end

function main.update()
    t = get_component_Transform(teapot)
    t.position.x = math.sin(5 *  os.clock())
    set_component_Transform(teapot, t)

    t = get_component_Transform(camera)
    t.position.z = 5 + 2*math.cos(5 *  os.clock())
    set_component_Transform(camera, t)
end

print("Hello from Lua")

return main
