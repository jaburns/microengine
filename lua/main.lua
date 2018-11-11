local m_player_entity
local m_clock_entity
local m_camera_entity
local m_start_pos

function printvec(v)
    print(v.x .. " " .. v.y .. " " .. v.y)
end

function update_player(transform, t)
    transform.position = m_start_pos + vec3(0, 5 * math.sin(t), 0)
end

function update_camera(target_transform, camera_transform)
    local delta = target_transform.position - camera_transform.position

    --printvec(delta)

    local look = quat.look_rotation(
        delta:normalize(),
        vec3(0,0,1),
        vec3(0,1,0)
    )

    printvec(look)

    camera_transform.rotation = look
end

return {
    start = function()
        m_player_entity = find_entity_with_Player()
        m_clock_entity = find_entity_with_ClockInfo()
        m_camera_entity = find_entity_with_GameCamera()
        m_start_pos = get_component_Transform(m_player_entity).position
    end,

    update = function()
        local clock = get_component_ClockInfo(m_clock_entity)

        local t = clock.millis_since_start / 1000

        local player_transform = get_component_Transform(m_player_entity)
        update_player(player_transform, t)
        set_component_Transform(m_player_entity, player_transform)

        local camera_transform = get_component_Transform(m_camera_entity)
        local camera_camera = get_component_GameCamera(m_camera_entity)
        local target_transform = get_component_Transform(camera_camera.target)

        update_camera(target_transform, camera_transform)
            
        set_component_Transform(m_camera_entity, camera_transform)
    end
}