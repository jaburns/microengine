// Generated
#pragma once
#include <linmath.h>
#include <stdint.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "ecs.h"
#include "components.h"

static ECS *s_ecs_lua;

void ecs_lua_bind_ecs(ECS *ecs)
{
    s_ecs_lua = ecs;
}


static void lcb_push_float(lua_State *L, float *v)
{
    lua_pushnumber(L, *v);
}

static void lcb_pop_float(lua_State *L, float *v, int stack_index)
{
    *v = luaL_checknumber(L, stack_index);
}

static void lcb_push_vec3(lua_State *L, vec3 *v)
{
    lua_newtable(L);
    lua_pushstring(L, "x"); lua_pushnumber(L, (*v)[0]); lua_settable(L, -3);
    lua_pushstring(L, "y"); lua_pushnumber(L, (*v)[1]); lua_settable(L, -3);
    lua_pushstring(L, "z"); lua_pushnumber(L, (*v)[2]); lua_settable(L, -3);
}

static void lcb_pop_vec3(lua_State *L, vec3 *v, int stack_index)
{
    lua_getfield(L, stack_index, "x");
    (*v)[0] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);

    lua_getfield(L, stack_index, "y");
    (*v)[1] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);

    lua_getfield(L, stack_index, "z");
    (*v)[2] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);
}


static void lcb_push_quat(lua_State *L, quat *v)
{
    lua_newtable(L);
    lua_pushstring(L, "x"); lua_pushnumber(L, (*v)[0]); lua_settable(L, -3);
    lua_pushstring(L, "y"); lua_pushnumber(L, (*v)[1]); lua_settable(L, -3);
    lua_pushstring(L, "z"); lua_pushnumber(L, (*v)[2]); lua_settable(L, -3);
    lua_pushstring(L, "w"); lua_pushnumber(L, (*v)[3]); lua_settable(L, -3);
}

static void lcb_pop_quat(lua_State *L, quat *v, int stack_index)
{
    lua_getfield(L, stack_index, "x");
    (*v)[0] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);

    lua_getfield(L, stack_index, "y");
    (*v)[1] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);

    lua_getfield(L, stack_index, "z");
    (*v)[2] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);

    lua_getfield(L, stack_index, "w");
    (*v)[3] = luaL_checknumber(L, stack_index);
    lua_pop(L, 1);
}


static void lcb_push_Transform(lua_State *L, Transform *v);
static void lcb_pop_Transform(lua_State *L, Transform *v, int stack_index);
static void lcb_push_Teapot(lua_State *L, Teapot *v);
static void lcb_pop_Teapot(lua_State *L, Teapot *v, int stack_index);
static void lcb_push_Camera(lua_State *L, Camera *v);
static void lcb_pop_Camera(lua_State *L, Camera *v, int stack_index);


static void lcb_push_Transform(lua_State *L, Transform *v)
{
    lua_newtable(L);
    lua_pushstring(L, "position"); lcb_push_vec3(L, &v->position); lua_settable(L, -3);
    lua_pushstring(L, "rotation"); lcb_push_quat(L, &v->rotation); lua_settable(L, -3);
    lua_pushstring(L, "scale"); lcb_push_vec3(L, &v->scale); lua_settable(L, -3);
    return 1;
}

static void lcb_pop_Transform(lua_State *L, Transform *v, int stack_index)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);
    lua_getfield(L, stack_index, "position");
    lcb_pop_vec3(L, &v->position, -1);
    lua_pop(L, 1);
    lua_getfield(L, stack_index, "rotation");
    lcb_pop_quat(L, &v->rotation, -1);
    lua_pop(L, 1);
    lua_getfield(L, stack_index, "scale");
    lcb_pop_vec3(L, &v->scale, -1);
    lua_pop(L, 1);
    return 1;
}

static int lcb_get_component_Transform(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Transform, x, s_ecs_lua, e);
    lcb_push_Transform(L, x);
    return 1;
}

static int lcb_set_component_Transform(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Transform, x, s_ecs_lua, e);
    lcb_pop_Transform(L, x, 2);
    return 1;
}

static void lcb_push_Teapot(lua_State *L, Teapot *v)
{
    lua_newtable(L);
    lua_pushstring(L, "nothing"); lcb_push_float(L, &v->nothing); lua_settable(L, -3);
    return 1;
}

static void lcb_pop_Teapot(lua_State *L, Teapot *v, int stack_index)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);
    lua_getfield(L, stack_index, "nothing");
    lcb_pop_float(L, &v->nothing, -1);
    lua_pop(L, 1);
    return 1;
}

static int lcb_get_component_Teapot(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Teapot, x, s_ecs_lua, e);
    lcb_push_Teapot(L, x);
    return 1;
}

static int lcb_set_component_Teapot(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Teapot, x, s_ecs_lua, e);
    lcb_pop_Teapot(L, x, 2);
    return 1;
}

static void lcb_push_Camera(lua_State *L, Camera *v)
{
    lua_newtable(L);
    lua_pushstring(L, "fov"); lcb_push_float(L, &v->fov); lua_settable(L, -3);
    lua_pushstring(L, "near"); lcb_push_float(L, &v->near); lua_settable(L, -3);
    lua_pushstring(L, "far"); lcb_push_float(L, &v->far); lua_settable(L, -3);
    return 1;
}

static void lcb_pop_Camera(lua_State *L, Camera *v, int stack_index)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);
    lua_getfield(L, stack_index, "fov");
    lcb_pop_float(L, &v->fov, -1);
    lua_pop(L, 1);
    lua_getfield(L, stack_index, "near");
    lcb_pop_float(L, &v->near, -1);
    lua_pop(L, 1);
    lua_getfield(L, stack_index, "far");
    lcb_pop_float(L, &v->far, -1);
    lua_pop(L, 1);
    return 1;
}

static int lcb_get_component_Camera(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Camera, x, s_ecs_lua, e);
    lcb_push_Camera(L, x);
    return 1;
}

static int lcb_set_component_Camera(lua_State *L)
{
    Entity e = (Entity)luaL_checknumber(L, 1);
    ECS_GET_COMPONENT_DECL(Camera, x, s_ecs_lua, e);
    lcb_pop_Camera(L, x, 2);
    return 1;
}

void ecs_lua_bind_functions(lua_State *L)
{
    lua_pushcfunction(L, lcb_get_component_Transform);
    lua_setglobal(L, "get_component_Transform");
    lua_pushcfunction(L, lcb_set_component_Transform);
    lua_setglobal(L, "set_component_Transform");
    lua_pushcfunction(L, lcb_get_component_Teapot);
    lua_setglobal(L, "get_component_Teapot");
    lua_pushcfunction(L, lcb_set_component_Teapot);
    lua_setglobal(L, "set_component_Teapot");
    lua_pushcfunction(L, lcb_get_component_Camera);
    lua_setglobal(L, "get_component_Camera");
    lua_pushcfunction(L, lcb_set_component_Camera);
    lua_setglobal(L, "set_component_Camera");
}

