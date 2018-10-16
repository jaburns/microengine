#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

#include "ecs.h"
#include "shell.h"
#include "components.h"
#include "components_ext.h"
#include "ecs_lua_interop.h"
#include "systems/render_sys.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>

#include <imgui_impl.h>

/* ----------------------------------- */

static const char *lua_script = ""
    "t = get_component_Transform(0)   \n"
    "t.position.z = t.position.z - 1  \n"
    "set_component_Transform(0, t)    \n"
    "";

static void run_lua(void)
{
    int error;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    ecs_lua_bind_functions(L);

    error = luaL_loadbuffer(L, lua_script, strlen(lua_script), "line")
        || lua_pcall(L, 0, 0, 0);

    if (error) 
    {
        printf("%s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    else 
        printf("Lua success!");

    lua_close(L);
}


static void run_game()
{
    ShellContext *ctx = shell_new("Hello world", 1024, 768);
    RenderSystem *rendersystem = rendersystem_new();
    ECS *ecs = ecs_new();

    Entity teapot = ecs_create_entity(ecs);
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, teapot);
        *tr = Transform_default;
        ECS_ADD_COMPONENT_DECL(Teapot, tp, ecs, teapot);
        *tp = Teapot_default;
    }

    Entity camera = ecs_create_entity(ecs);
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, camera);
        *tr = Transform_default;
        tr->position[1] = 1.f;
        tr->position[2] = 5.f;
        ECS_ADD_COMPONENT_DECL(Camera, ca, ecs, camera);
        *ca = Camera_default;
    }

    Entity teapot2 = ecs_create_entity(ecs);
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, teapot2);
        *tr = Transform_default;
        tr->position[1] = 1.f;
        ECS_ADD_COMPONENT_DECL(Teapot, tp, ecs, teapot2);
        *tp = Teapot_default;
    }

    ecs_lua_bind_ecs(ecs);
    run_lua();

    do 
    {
        rendersystem_run(rendersystem, ecs);

        bool x;
        igShowDemoWindow(&x);
    } 
    while (shell_flip_frame_poll_events(ctx));

    ecs_delete(ecs);
    rendersystem_delete(rendersystem);
    shell_delete(ctx);
}




int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_all_tests();
    #endif

    run_game();

    return 0;
}