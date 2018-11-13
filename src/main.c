#include <stdio.h>
#include <string.h>
#include <ns_clock.h>

#ifdef RUN_TESTS
    #include "testing.h"
#endif

#include "shell.h"
#include "component_defs.h"
#include "containers/ecs.h"
#include "containers/hashcache.h"
#include "systems/input_sys.h"
#include "systems/transform_sys.h"
#include "systems/editor_sys.h"
#include "systems/render_sys.h"
#include "systems/clock_sys.h"
#include "systems/collision_sys.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "resources/shader.h"
#include "resources/mesh.h"
#include "game/game.h"

static void resources_init( HashCache *resources )
{
    hashcache_register( resources, "glsl", shader_load, shader_delete );
    hashcache_register( resources, "jmesh", mesh_load, mesh_delete );
    hashcache_register( resources, "jmat", material_load, material_delete );
    hashcache_register( resources, "png", texture_load, texture_delete );
}

int main( int argc, char **argv )
{
    #ifdef RUN_TESTS
        if( run_all_tests() ) return 1;
    #endif

    ShellContext *ctx = shell_new( "Microengine", 1280, 800 );
    HashCache *resources = hashcache_new();
    resources_init( resources );

    ECS *ecs = components_ecs_new();

    bool switching_mode = false;
    bool play_mode = false;

    ClockSystem *clock_system = clock_sys_new();
    InputSystem *input_system = input_sys_new( ctx );
    TransformSystem *transform_system = transform_sys_new();
    RenderSystem *render_system = render_sys_new( resources );
    EditorSystem *editor_system = editor_sys_new();
    CollisionSystem *collision_system = collision_sys_new();

    Game *game = NULL;

    do
    {
        clock_sys_run( clock_system, ecs, switching_mode );
        input_sys_run( input_system, ecs );
        collision_sys_run( collision_system, ecs, resources );

        if( play_mode )
        {
            if( switching_mode ) game = game_new( ecs );
            game_update( game, ecs );
        }
        else if( switching_mode )
        {
            game_delete( game );
            game = NULL;
        }

        transform_sys_run( transform_system, ecs );
        EditorSystemUpdateResult editor_update = editor_sys_run( editor_system, ecs );
        render_sys_run( render_system, ecs, resources, shell_get_aspect( ctx ), editor_update.in_game_view );

        if( editor_update.new_ecs )
        {
            ecs_delete( ecs );
            ecs = editor_update.new_ecs;
        }

        switching_mode = editor_update.in_play_mode != play_mode;
        play_mode = editor_update.in_play_mode;
    }
    while( shell_flip_frame_poll_events( ctx ) );

    if( game ) game_delete( game );

    collision_sys_delete( collision_system );
    editor_sys_delete( editor_system );
    render_sys_delete( render_system );
    transform_sys_delete( transform_system );
    input_sys_delete( input_system );
    clock_sys_delete( clock_system );

    hashcache_delete( resources );
    ecs_delete( ecs );
    shell_delete( ctx );

    getchar();

    return 0;
}
