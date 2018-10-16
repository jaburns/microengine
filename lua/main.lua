local main = {}

print("Hello from Lua")

function main.start()
    print("Hello from Lua main.start")

    print("Hello from Lua main.end")
    teapot = create_entity()

    print("Hello from Lua main.end")

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

    print("Hello from Lua main.end")

--  t = get_component_Transform(0)
--  t.position.z = t.position.z - 1
--  set_component_Transform(0, t)
end

function main.update()
--  t = get_component_Transform(0)
--  t.position.x = math.sin(5 *  os.clock())
--  set_component_Transform(0, t)
end

return main
