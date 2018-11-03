#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <cglm.lua.h>
#include <ns_clock.h>

#ifdef RUN_TESTS
    #include "testing.h"
#endif

#include "shell.h"
#include "components.h"
#include "containers/ecs.h"
#include "containers/hashcache.h"
#include "systems/input_sys.h"
#include "systems/transform_sys.h"
#include "systems/editor_sys.h"
#include "systems/render_sys.h"
#include "systems/clock_sys.h"
#include "resources/material.h"
#include "resources/texture.h"
#include "resources/shader.h"
#include "resources/mesh.h"

static int l_print( lua_State *L )
{
    const char *d = luaL_checkstring( L, 1 );
    printf( ":: %s\n", d );
    return 1;
}

static void run_lua_main_func( lua_State *L, const char *func )
{
    int error;

    lua_getglobal( L, "require" );
    lua_pushstring( L, "lua/main" );
    error = lua_pcall( L, 1, 1, 0 );
    lua_getfield( L, -1, func );
    error = lua_pcall( L, 0, 0, 0 );

    if( error )
    {
        printf( "%s", lua_tostring( L, -1 ) );
        exit( 1 );
    }
}

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

    ShellContext *ctx = shell_new( "Microengine", 1024, 768 );
    ECS *ecs = ecs_new();
    HashCache *resources = hashcache_new();
    resources_init( resources );

    ClockSystem *clocksystem = clock_sys_new();
    InputSystem *inputsystem = input_sys_new( ctx );
    TransformSystem *transformsystem = transform_sys_new();
    RenderSystem *rendersystem = render_sys_new( resources );
    EditorSystem *editorsystem = editor_sys_new();

    lua_State *L = luaL_newstate();
    luaL_openlibs( L );
    glmlua_load_types( L );

    components_init( L, ecs );

    lua_pushcfunction( L, l_print );
    lua_setglobal( L, "print" );
    run_lua_main_func( L, "start" );

    do
    {
        clock_sys_run( clocksystem, ecs );
        input_sys_run( inputsystem, ecs );

        run_lua_main_func( L, "update" );

        transform_sys_run( transformsystem, ecs );
        editor_sys_run( editorsystem, ecs );
        render_sys_run( rendersystem, ecs, resources, shell_get_aspect( ctx ) );
    }
    while( shell_flip_frame_poll_events( ctx ) );

    lua_close( L );

    editor_sys_delete( editorsystem );
    render_sys_delete( rendersystem );
    transform_sys_delete( transformsystem );
    input_sys_delete( inputsystem );
    clock_sys_delete( clocksystem );

    hashcache_delete( resources );
    ecs_delete( ecs );
    shell_delete( ctx );

    //getchar();

    return 0;
}
