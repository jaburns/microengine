local main = {}

function main.start()
    t = get_component_Transform(0)
    t.position.z = t.position.z - 1
    set_component_Transform(0, t)
end

function main.update()

end

return main
