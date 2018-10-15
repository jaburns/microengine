#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

#include "ecs.h"
#include "shell.h"
#include "render_system.h"
#include "components.h"

#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <imgui_impl.h>


static ECS *s_ecs_lua;

static int l_print (lua_State *L) 
{
    const char *d = luaL_checkstring(L, 1);
    printf("L: %s\n", d);
    return 1;  /* number of results */
}

static int jlua_push_vec3(lua_State *L, vec3 v)
{
    lua_newtable(L);
    lua_pushstring(L, "x"); lua_pushnumber(L, v[0]); lua_settable(L, -3);
    lua_pushstring(L, "y"); lua_pushnumber(L, v[1]); lua_settable(L, -3);
    lua_pushstring(L, "z"); lua_pushnumber(L, v[2]); lua_settable(L, -3);
    return 1;
}

/* ----------------------------------- */
// TODO generate this shit

static int jlua_get_component_Transform(lua_State *L)
{
    double e = luaL_checknumber(L, 1);
    Entity ent = (Entity)e;

    ECS_GET_COMPONENT_DECL(Transform, x, s_ecs_lua, ent);

    lua_newtable(L);
    lua_pushstring(L, "position"); jlua_push_vec3(L, x->position); lua_settable(L, -3);

    printf(":: %f\n", x->position[0]);

    return 1;
}

static int jlua_set_component_Transform(lua_State *L)
{
    double e = luaL_checknumber(L, 1);
    Entity ent = (Entity)e;

    luaL_checktype(L, 2, LUA_TTABLE);
    lua_getfield(L, 2, "position");

    lua_getfield(L, -1, "x");
    double x = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "y");
    double y = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, -1, "z");
    double z = luaL_checknumber(L, -1);
    lua_pop(L, 1);

    ECS_GET_COMPONENT_DECL(Transform, t, s_ecs_lua, ent);

    t->position[0] = (float)x;
    t->position[1] = (float)y;
    t->position[2] = (float)z;

    printf("::\t%f\t%f\t%f", t->position[0], t->position[1], t->position[2]);

    return 1;
}

static void bind_component_funcs(lua_State *L)
{
    lua_pushcfunction(L, jlua_get_component_Transform);
    lua_setglobal(L, "get_component_Transform");
    lua_pushcfunction(L, jlua_set_component_Transform);
    lua_setglobal(L, "set_component_Transform");
}

/* ----------------------------------- */

static const char *lua_script = ""
    "t = get_component_Transform(0)   \n"
    "t.position.x = t.position.x + 1  \n"
    "print(tostring(t.position.x))    \n"
    "set_component_Transform(0, t)    \n"
    "";

static void run_lua(void)
{
    int error;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushcfunction(L, l_print);
    lua_setglobal(L, "print");

    bind_component_funcs(L);

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

    s_ecs_lua = ecs;
    run_lua();

 // do 
 // {
 //     rendersystem_run(rendersystem, ecs);

 //     bool x;
 //     igShowDemoWindow(&x);
 // } 
 // while (shell_flip_frame_poll_events(ctx));

    ecs_delete(ecs);
    rendersystem_delete(rendersystem);
    shell_delete(ctx);
}




int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        //run_all_tests();
    #endif

    run_game();

    return 0;
}