const fs = require('fs');
const componentsJson = require('./src/components.json');

const types = [
    {
        _internal: true,
        name: "vec3",
        fields: {
            x: ["float", '&(*v)[0]'],
            y: ["float", '&(*v)[1]'],
            z: ["float", '&(*v)[2]']
        }
    },
    {
        _internal: true,
        name: "quat",
        fields: {
            x: ["float", '&(*v)[0]'],
            y: ["float", '&(*v)[1]'],
            z: ["float", '&(*v)[2]'],
            w: ["float", '&(*v)[3]'],
        }
    }
].concat(componentsJson);

const baseTypeInterop = `
static void lcb_push_float(lua_State *L, float *v)
{
    double vd = (double)(*v);
    lua_pushnumber(L, vd);
}
static void lcb_pop_float(lua_State *L, float *v, int stack_index)
{
    *v = (float)luaL_checknumber(L, stack_index);
}
`;

const generateStruct = type => {
    if (type._internal) return '';

    const result = [`typedef struct ${type.name}`];
    result.push('{');

    for (let k in type.fields)
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

    for (let k in type.fields) {
        const fieldRef = Array.isArray(type.fields[k])
            ? type.fields[k][1]
            : `&v->${k}`;

        const fieldType = Array.isArray(type.fields[k])
            ? type.fields[k][0]
            : type.fields[k];

        result.push(`    lua_pushstring(L, "${k}"); lcb_push_${fieldType}(L, ${fieldRef}); lua_settable(L, -3);`);
    }

    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaPop = (type, proto) => {
    const result = [`static void lcb_pop_${type.name}(lua_State *L, ${type.name} *v, int stack_index)`]
    if (proto) return result[0] + ';';
    result.push('{');
    result.push('    luaL_checktype(L, stack_index, LUA_TTABLE);');

    for (let k in type.fields) {
        const fieldRef = Array.isArray(type.fields[k])
            ? type.fields[k][1]
            : `&v->${k}`;

        const fieldType = Array.isArray(type.fields[k])
            ? type.fields[k][0]
            : type.fields[k];

        result.push(`    lua_getfield(L, stack_index, "${k}");`);
        result.push(`    lcb_pop_${fieldType}(L, ${fieldRef}, -1);`);
        result.push('    lua_pop(L, 1);');
    }

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
    result.push(`    if (!x) x = ECS_ADD_COMPONENT(${type.name}, s_ecs_lua, e);`);
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
        if (types[i]._internal) continue;

        result.push(`    lua_pushcfunction(L, lcb_get_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "get_component_${types[i].name}");`);
        result.push(`    lua_pushcfunction(L, lcb_set_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "set_component_${types[i].name}");`);
    }

    result.push('}');
    return result.join('\n') + '\n';
};

const generateFile_components_h = () => {
    const result = [`// Generated
#pragma once
#include <linmath.h>
#include <stdint.h>
`];
    for (let i = 0; i < types.length; ++i)
        result.push(generateStruct(types[i]));

    return result.join('\n');
};

const generateFile_ecs_lua_interop_c = () => {
    const result = [`// Generated
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
`];
    for (let i = 0; i < types.length; ++i)
    {
        result.push(generateLuaPush(types[i], true));
        result.push(generateLuaPop(types[i], true));
    }
    result.push('\n');
    for (let i = 0; i < types.length; ++i)
    {
        result.push(generateLuaPush(types[i]));
        result.push(generateLuaPop(types[i]));

        if (!types[i]._internal) {
            result.push(generateLuaGetComponent(types[i]));
            result.push(generateLuaSetComponent(types[i]));
        }
    }
    result.push(bindingProc(types));

    return result.join('\n');
}

fs.writeFileSync("src/components.h", generateFile_components_h());
fs.writeFileSync("src/ecs_lua_interop.c", generateFile_ecs_lua_interop_c());
