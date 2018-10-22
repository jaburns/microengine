#include <cJSON.h>

#include "../src/utils.h"


void generate_components(void)
{
    const char *json_file = read_file_alloc("components.json");
    cJSON *json = cJSON_Parse(json_file);

    cJSON *top = cJSON_GetArrayItem(json, 0);
    cJSON *hello = cJSON_GetObjectItem(top, "name");
    printf("Hello: %s", cJSON_GetStringValue(hello));

    cJSON_Delete(json);
    free(json_file);
}


/*
const fs = require('fs');
const types = require('./components.json');

const baseTypes = ['float', 'vec2', 'vec3', 'vec4', 'mat4x4', 'Entity'];

const staticComponentCode = `
"static ECS *s_ecs;

"static int lcb_create_entity(lua_State *L)
"{
"    Entity ent = ecs_create_entity(s_ecs);
"    lua_pushnumber(L, (double)ent);
"    return 1;
"}

"static int lcb_destroy_entity(lua_State *L)
"{
"    double entity = luaL_checknumber(L, 1);
"    ecs_destroy_entity(s_ecs, (Entity)entity);
"    return 0;
"}

"static void lcb_push_Entity(lua_State *L, Entity *v) { lua_pushnumber(L, (double)(*v)); }
"static void lcb_pop_Entity(lua_State *L, Entity *v, int stack_index) { *v = (Entity)luaL_checknumber(L, stack_index); }
"static void lcb_push_float(lua_State *L, float *v) { lua_pushnumber(L, (double)(*v)); }
"static void lcb_pop_float(lua_State *L, float *v, int stack_index) { *v = (float)luaL_checknumber(L, stack_index); }
"static void lcb_push_vec2(lua_State *L, vec2 *v) { lml_push_vec2(L, *v); }
"static void lcb_pop_vec2(lua_State *L, vec2 *v, int stack_index) { lml_get_vec2(L, stack_index, *v); }
"static void lcb_push_vec3(lua_State *L, vec3 *v) { lml_push_vec3(L, *v); }
"static void lcb_pop_vec3(lua_State *L, vec3 *v, int stack_index) { lml_get_vec3(L, stack_index, *v); }
"static void lcb_push_vec4(lua_State *L, vec4 *v) { lml_push_vec4(L, *v); }
"static void lcb_pop_vec4(lua_State *L, vec4 *v, int stack_index) { lml_get_vec4(L, stack_index, *v); }
"static void lcb_push_quat(lua_State *L, quat *v) { lml_push_quat(L, *v); }
"static void lcb_pop_quat(lua_State *L, quat *v, int stack_index) { lml_get_quat(L, stack_index, *v); }
"extern void lcb_push_mat4x4(lua_State *L, mat4x4 v) { lml_push_mat4x4(L, *v); }
"static void lcb_pop_mat4x4(lua_State *L, mat4x4 *v, int stack_index) { lml_get_mat4x4(L, stack_index, *v); }
`;

const generateStruct = type => {
    const result = [`typedef struct ${type.name}`];
    result.push('{');

    for (let k in type.fields) {
        if (Array.isArray(type.fields[k])) {
            result.push(`    Vec ${k}; // of ${type.fields[k][0]}`);
        } else {
            result.push(`    ${type.fields[k]} ${k};`);
        }
    }

    result.push('}');
    result.push(`${type.name};`);
    return result.join('\n') + '\n';
};

const generateDefault = type =>
    [`static const ${type.name} ${type.name}_default = ${type.default};`];

const generateVecPushPop = typename => `
static void lcb_push_Vec_${typename}(lua_State *L, Vec *v) 
{
    lua_newtable(L);
    for (int i = 0; i < v->item_count; ++i)
    {
        lcb_push_${typename}(L, vec_at(v, i));
        lua_rawseti(L, -2, i + 1);
    }
}
static void lcb_pop_Vec_${typename}(lua_State *L, Vec *v, int stack_index)
{
    luaL_checktype(L, stack_index, LUA_TTABLE);
    size_t vec_len = lua_objlen(L, stack_index); 

    // TODO need to maybe have destructors inside of Vec rather than GenenrationalIndexArray
    // Lua is going to be manipulating Vecs on components directly.
    // This will work unless you have vecs of vecs, then memory will leak.
    vec_resize(v, vec_len);

    for (int i = 0; i < vec_len; ++i)
    {
        lua_rawgeti(L, stack_index, i + 1);
        lcb_pop_${typename}(L, vec_at(v, i), -1);
        lua_pop(L, 1);
    }
}
`;

const generateLuaPush = (type, proto) => {
    const result = [`static void lcb_push_${type.name}(lua_State *L, ${type.name} *v)`];
    if (proto) return result[0] + ';';
    result.push('{');
    result.push('    lua_newtable(L);');

    for (let k in type.fields) {
        const fieldRef = `&v->${k}`;
        const fieldType = (Array.isArray(type.fields[k]) ? 'Vec_' : '') + type.fields[k];

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
        const fieldRef = `&v->${k}`;
        const fieldType = (Array.isArray(type.fields[k]) ? 'Vec_' : '') + type.fields[k];

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
    result.push(`    ECS_GET_COMPONENT_DECL(${type.name}, x, s_ecs, e);`);
    result.push(`    lcb_push_${type.name}(L, x);`);
    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaSetComponent = type => {
    const result = [`static int lcb_set_component_${type.name}(lua_State *L)`];
    result.push('{');
    result.push('    Entity e = (Entity)luaL_checknumber(L, 1);');
    result.push(`    ECS_GET_COMPONENT_DECL(${type.name}, x, s_ecs, e);`);
    result.push(`    lcb_pop_${type.name}(L, x, 2);`);
    result.push('    return 0;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateLuaAddComponent = type => {
    const result = [`static int lcb_add_component_${type.name}(lua_State *L)`];
    result.push('{');
    result.push('    Entity e = (Entity)luaL_checknumber(L, 1);');
    result.push(`    ECS_GET_COMPONENT_DECL(${type.name}, x, s_ecs, e);`);
    result.push(`    if (!x) {`);
    result.push(`        ECS_ADD_COMPONENT_DEFAULT_DECL(${type.name}, x_new, s_ecs, e);`);
    result.push(`        x = x_new;`);
    result.push(`    }`);
    result.push(`    lcb_push_${type.name}(L, x);`);
    result.push('    return 1;');
    result.push('}');
    return result.join('\n') + '\n';
};

const generateInit = types => {
    const result = ['void components_init(lua_State *L, ECS *ecs)']
    result.push('{');
    result.push('    s_ecs = ecs;');

    for (let i = 0; i < types.length; ++i)
    {
        result.push(`    lua_pushcfunction(L, lcb_get_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "get_component_${types[i].name}");`);
        result.push(`    lua_pushcfunction(L, lcb_set_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "set_component_${types[i].name}");`);
        result.push(`    lua_pushcfunction(L, lcb_add_component_${types[i].name});`);
        result.push(`    lua_setglobal(L, "add_component_${types[i].name}");`);
        result.push(`    ECS_REGISTER_COMPONENT(${types[i].name}, ecs, NULL);`);
    }
    
    result.push('    lua_pushcfunction(L, lcb_create_entity);');
    result.push('    lua_setglobal(L, "create_entity");');
    result.push('    lua_pushcfunction(L, lcb_destroy_entity);');
    result.push('    lua_setglobal(L, "destroy_entity");');

    result.push('}');
    return result.join('\n') + '\n';
};

const generateFile_components_h = () => {
    const result = [`// Generated
#pragma once
#include <linmath.h>
#include <stdint.h>
#include <lua.h>
#include "vec.h"
#include "ecs.h"

extern void components_init(lua_State *L, ECS *ecs);
`];
    for (let i = 0; i < types.length; ++i)
        result.push(generateStruct(types[i]));

    for (let i = 0; i < types.length; ++i)
        result.push(generateDefault(types[i]));

    return result.join('\n');
};

const generateFile_components_c = () => {
    const result = [`// Generated
#include "components.h"
#include <linmath.lua.h>
#include <stdint.h>
#include <lauxlib.h>
#include <lualib.h>

${staticComponentCode}
`];
    for (let i = 0; i < types.length; ++i)
    {
        result.push(generateLuaPush(types[i], true));
        result.push(generateLuaPop(types[i], true));
    }

    baseTypes.forEach(t => result.push(generateVecPushPop(t)));

    result.push('\n');
    for (let i = 0; i < types.length; ++i)
    {
        result.push(generateLuaPush(types[i]));
        result.push(generateLuaPop(types[i]));
        result.push(generateLuaAddComponent(types[i]));
        result.push(generateLuaGetComponent(types[i]));
        result.push(generateLuaSetComponent(types[i]));
        result.push(generateVecPushPop(types[i].name));
    }
    result.push(generateInit(types));

    return result.join('\n');
}

fs.writeFileSync("src/components.h", generateFile_components_h());
fs.writeFileSync("src/components.c", generateFile_components_c());
*/
