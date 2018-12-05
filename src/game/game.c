#include "game.h"

#include <stdlib.h>

#include "../gl.h"
#include "../component_defs.h"
#include "../systems/input_sys.h"
#include "../systems/collision_sys.h"
#include "../utils.h"

struct Game
{
    float start_pos;
};

Game *game_new( ECS *ecs )
{
    Game *game = malloc( sizeof( Game ) );

    Entity player_entity = 0;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( Player, ecs, &player_entity );
    ECS_VIEW_COMPONENT_DECL( Transform, player_transform, ecs, player_entity );

    game->start_pos = player_transform->position[1];

    return game;
}

static void update_player_position( 
    float dt, const InputState *input_state, const Transform *camera_transform, const WorldCollisionInfo *collision,
    Transform *player_transform, Player *player 
) {
// Gravitational acceleration
    glm_vec_add( (vec3){ 0, -100.f*dt, 0 }, player->velocity, player->velocity );

// Input-based acceleration
    vec3 move_vec;
    glm_vec_sub( player_transform->position, UTILS_UNCONST_VEC( camera_transform->position ), move_vec );
    move_vec[1] = 0.f;
    glm_vec_normalize( move_vec );
    glm_vec_scale( move_vec, .01f, move_vec );

    bool moved = false;

    if( input_state_is_key_down( input_state, SDLK_w ) )
    {
        glm_vec_add( player->velocity, move_vec, player->velocity );
        moved = true;
    }
    else if( input_state_is_key_down( input_state, SDLK_s ) )
    {
        glm_vec_sub( player->velocity, move_vec, player->velocity );
        moved = true;
    }

    glm_vec_rotate( move_vec, (float)M_PI_2, (vec3){ 0, 1, 0 } );

    if( input_state_is_key_down( input_state, SDLK_d ) )
    {
        glm_vec_add( player->velocity, move_vec, player->velocity );
        moved = true;
    }
    else if( input_state_is_key_down( input_state, SDLK_a ) )
    {
        glm_vec_sub( player->velocity, move_vec, player->velocity );
        moved = true;
    }

    if( !moved )
    {
        player->velocity[0] *= 0.99f;
        player->velocity[2] *= 0.99f;
    }

// Integrate velocity in to position
    vec3 vdt;
    glm_vec_scale( player->velocity, dt, vdt );
    glm_vec_add( vdt, player_transform->position, player_transform->position );

// Collision resolution
    vec3 intersect_pt;
    if( world_collision_info_raycast( collision, player_transform->position, (vec3){ 0, -1, 0 }, intersect_pt ) )
    {
        glm_vec_copy( intersect_pt, player_transform->position );
        glm_vec_add( player_transform->position, (vec3){ 0, 1, 0 }, player_transform->position );
        player->velocity[1] = 0.f;
    }
}

static void update_game_camera( const vec3 target, Transform *camera_transform, GameCamera *game_camera )
{
    camera_transform->position[1] = target[1] + 5.f;

    vec3 camera_from_target;
    glm_vec_sub( camera_transform->position, UTILS_UNCONST_VEC( target ), camera_from_target );
    glm_vec_normalize( camera_from_target );
    glm_vec_scale( camera_from_target, 10, camera_from_target );
    glm_vec_add( UTILS_UNCONST_VEC( target ), camera_from_target, camera_transform->position );

    mat4 m;
    versor q;
    glm_lookat( UTILS_UNCONST_VEC( target ), camera_transform->position, (vec3){0,1,0}, m );
    glm_mat4_quat( m, q ); 
    glm_quat_inv( q, camera_transform->rotation );
}

void game_update( Game *game, ECS *ecs )
{
    ECS_VIEW_SINGLETON_DECL( ClockInfo, ecs, clock_info );
    ECS_VIEW_SINGLETON_DECL( InputState, ecs, input_state );
    ECS_VIEW_SINGLETON_DECL( WorldCollisionInfo, ecs, collision );

    Entity player_entity, camera_entity;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( Player, ecs, &player_entity );
    ECS_BORROW_COMPONENT_DECL( Transform, player_transform, ecs, player_entity );
    ECS_BORROW_COMPONENT_DECL( Player, player, ecs, player_entity );

    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( GameCamera, ecs, &camera_entity );
    ECS_BORROW_COMPONENT_DECL( GameCamera, game_camera, ecs, camera_entity );
    ECS_BORROW_COMPONENT_DECL( Transform, camera_transform, ecs, camera_entity );
    ECS_VIEW_COMPONENT_DECL( Transform, camera_target_transform, ecs, game_camera->target );

    update_player_position( clock_info->delta_secs, input_state, camera_transform, collision, player_transform, player );
    update_game_camera( camera_target_transform->position, camera_transform, game_camera );

    ECS_RETURN_COMPONENT( ecs, player_transform );
    ECS_RETURN_COMPONENT( ecs, player );
    ECS_RETURN_COMPONENT( ecs, game_camera );
    ECS_RETURN_COMPONENT( ecs, camera_transform );
}

void game_delete( Game *game )
{
    if( !game ) return;

    free( game );
}
