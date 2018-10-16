const types = [
    {
        name: "Transform",
        fields: {
            position: "vec3",
            rotation: "quat",
            scale:    "vec3",
        },
    },
    {
        name: "Teapot",
        fields: {
            nothing: "float",
        },
    },
    {
        name: "Camera",
        fields: {
            fov:  "float",
            near: "float",
            far:  "float",
        },
    },
];

const baseTypeInterop = `
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
`;


const generateStruct = type => {
    const result = [`typedef struct ${type.name}`];
    result.push('{');

    for (var k in type.fields)
        result.push(`    ${type.fields[k]} ${k};`);

    result.push('}');
    result.push(`${type.name};`);
    return result.join('\n') + '\n';
};


const generateLuaPush = (type, proto) => {
    const result = [`static void lcb_push_${type.name}(lua_State *L, ${type.name} *v)`];
    if (proto) return result[0] + ';';
    result.push('{');
    result.push('    lua_newtable(L);');

    for (var k in type.fields)
        result.push(`    lua_pushstring(L, "${k}"); lcb_push_${type.fields[k]}(L, &v->${k}); lua_settable(L, -3);`);

    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaPop = (type, proto) => {
    const result = [`static void lcb_pop_${type.name}(lua_State *L, ${type.name} *v, int stack_index)`]
    if (proto) return result[0] + ';';
    result.push('{');
    result.push('    luaL_checktype(L, stack_index, LUA_TTABLE);');

    for (var k in type.fields) {
        result.push(`    lua_getfield(L, stack_index, "${k}");`);
        result.push(`    lcb_pop_${type.fields[k]}(L, &v->${k}, -1);`);
        result.push('    lua_pop(L, 1);');
    }

    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaGetComponent = type => {
    const result = [`static int lcb_get_component_${type.name}(lua_State *L)`];
    result.push('{');
    result.push('    Entity e = (Entity)luaL_checknumber(L, 1);');
    result.push(`    ECS_GET_COMPONENT_DECL(${type.name}, x, s_ecs_lua, e);`);
    result.push(`    lcb_push_${type.name}(L, x);`);
    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaSetComponent = type => {
    const result = [`static int lcb_set_component_${type.name}(lua_State *L)`];
    result.push('{');
    result.push('    Entity e = (Entity)luaL_checknumber(L, 1);');
    result.push(`    ECS_GET_COMPONENT_DECL(${type.name}, x, s_ecs_lua, e);`);
    result.push(`    lcb_pop_${type.name}(L, x, 2);`);
    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const bindingProc = types => {
    const result = ['void ecs_lua_bind_functions(lua_State *L)']
    result.push('{');

    for (let i = 0; i < types.length; ++i)
    {
        result.push(`    lua_pushcfunction(L, lcb_get_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "get_component_${types[i].name}");`);
        result.push(`    lua_pushcfunction(L, lcb_set_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "set_component_${types[i].name}");`);
    }

    result.push('}');
    return result.join('\n') + '\n';
};





const header = false;

if (header)
{
    console.log(`// Generated
#pragma once
#include <linmath.h>
#include <stdint.h>
`);
    for (let i = 0; i < types.length; ++i)
        console.log(generateStruct(types[i]));
}
else
{
    console.log(`// Generated
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

${baseTypeInterop}
`);
    for (let i = 0; i < types.length; ++i)
    {
        console.log(generateLuaPush(types[i], true));
        console.log(generateLuaPop(types[i], true));
    }
    console.log('\n');
    for (let i = 0; i < types.length; ++i)
    {
        console.log(generateLuaPush(types[i]));
        console.log(generateLuaPop(types[i]));
        console.log(generateLuaGetComponent(types[i]));
        console.log(generateLuaSetComponent(types[i]));
    }
    console.log(bindingProc(types));
}
