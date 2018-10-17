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

static int l_print (lua_State *L) 
{	
    const char *d = luaL_checkstring(L, 1);
    printf("L: %s\n", d);
    return 1;
}

static ECS *s_ecs_lua;

static int lcb_create_entity(lua_State *L)
{
    Entity ent = ecs_create_entity(s_ecs_lua);
    lua_pushnumber(L, (double)ent);
    return 1;
}

static int lcb_destroy_entity(lua_State *L)
{
    double entity = luaL_checknumber(L, 1);
    ecs_destroy_entity(s_ecs_lua, (Entity)entity);
    return 0;
}

void ecs_lua_bind_entity_functions(lua_State *L)
{
    lua_pushcfunction(L, lcb_create_entity);
    lua_setglobal(L, "create_entity");
    lua_pushcfunction(L, lcb_destroy_entity);
    lua_setglobal(L, "destroy_entity");
}

static void run_lua_main_func(lua_State *L, const char *func)
{
    int error;

    lua_getglobal(L, "require");
    lua_pushstring(L, "lua/main");
    error = lua_pcall(L, 1, 1, 0);
    lua_getfield(L, -1, func);
    error = lua_pcall(L, 0, 0, 0);

    if (error) 
    {
        printf("%s", lua_tostring(L, -1));
        exit(1);
    }
}

static void run_game()
{
    ShellContext *ctx = shell_new("Hello world", 1024, 768);
    RenderSystem *rendersystem = render_sys_new();
    ECS *ecs = ecs_new();

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, l_print);
    lua_setglobal(L, "print");

    ecs_lua_bind_ecs(ecs);
    ecs_lua_bind_functions(L);
    s_ecs_lua = ecs;
    ecs_lua_bind_entity_functions(L);

    run_lua_main_func(L, "start");

    do 
    {
        run_lua_main_func(L, "update");

        render_sys_run(rendersystem, ecs);

        //igShowDemoWindow(NULL);
    } 
    while (shell_flip_frame_poll_events(ctx));

    lua_close(L);
    ecs_delete(ecs);
    render_sys_delete(rendersystem);
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