#include "game_sys.h"

#include "../components.h"

struct GameSystem
{
    int empty;
};

GameSystem *game_sys_new( void )
{
    return NULL;
}

void game_sys_run( GameSystem *sys, ECS *ecs )
{
    Entity clock_entity = 0;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( ClockInfo, ecs, &clock_entity );
    ECS_GET_COMPONENT_DECL( ClockInfo, clock_info, ecs, clock_entity );

    Entity player_entity = 0;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( Player, ecs, &player_entity );
    ECS_GET_COMPONENT_DECL( Transform, player_transform, ecs, player_entity );

    player_transform->position[1] += 0.001f * sinf(clock_info->millis_since_start / 1000.f);

    Entity game_camera_entity = 0;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( GameCamera, ecs, &game_camera_entity );
    ECS_GET_COMPONENT_DECL( GameCamera, game_camera, ecs, game_camera_entity );
    ECS_GET_COMPONENT_DECL( Transform, camera_target_transform, ecs, game_camera->target );
    ECS_GET_COMPONENT_DECL( Transform, camera_transform, ecs, game_camera_entity );

    mat4 m;
    versor q;
    glm_lookat( camera_target_transform->position, camera_transform->position, (vec3){0,1,0}, m );
    glm_mat4_quat( m, q ); 
    glm_quat_inv( q, camera_transform->rotation );
}


void game_sys_delete( GameSystem *sys )
{
}
