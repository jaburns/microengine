#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <linmath.lua.h>

#ifdef RUN_TESTS
    #include "testing.h"
#endif

#include "shell.h"
#include "components.h"
#include "containers/ecs.h"
#include "systems/transform_sys.h"
#include "systems/editor_sys.h"
#include "systems/render_sys.h"

static int l_print (lua_State *L)
{
    const char *d = luaL_checkstring(L, 1);
    printf(":: %s\n", d);
    return 1;
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

int main(int argc, char **argv)
{
    #ifdef RUN_TESTS
        if (run_all_tests()) return 1;
    #endif

    ShellContext *ctx = shell_new("Hello world", 1024, 768);
    ECS *ecs = ecs_new();
    TransformSystem *transformsystem = transform_sys_new();
    RenderSystem *rendersystem = render_sys_new();
    EditorSystem *editorsystem = editor_sys_new();

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lml_load_types(L);

    lua_pushcfunction(L, l_print);
    lua_setglobal(L, "print");

    components_init(L, ecs);
    run_lua_main_func(L, "start");

    do
    {
        run_lua_main_func(L, "update");

        transform_sys_run(transformsystem, ecs);
        editor_sys_run(editorsystem, ecs);
        render_sys_run(rendersystem, ecs, shell_get_aspect(ctx));
    }
    while (shell_flip_frame_poll_events(ctx));

    lua_close(L);
    transform_sys_delete(transformsystem);
    render_sys_delete(rendersystem);
    editor_sys_delete(editorsystem);
    ecs_delete(ecs);
    shell_delete(ctx);

    return 0;
}
