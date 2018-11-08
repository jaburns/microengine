local m_player_entity
local m_clock_entity
local m_start_pos

return {
    start = function()
        m_player_entity = find_entity_with_Player()
        m_clock_entity = find_entity_with_ClockInfo()
        m_start_pos = get_component_Transform(m_player_entity).position
    end,

    update = function()
        local transform = get_component_Transform(m_player_entity)
        local clock = get_component_ClockInfo(m_clock_entity)

        transform.position = m_start_pos + vec3(0, 5 * math.sin(clock.millis_since_start / 1000), 0)

        set_component_Transform(m_player_entity, transform)
    end
}