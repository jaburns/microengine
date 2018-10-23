#define _CRT_SECURE_NO_WARNINGS 1

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cJSON.h>

#include "../src/utils.h"

const char *BASE_TYPES[] = { "float", "vec2", "vec3", "vec4", "mat4x4", "Entity" };
const size_t NUM_BASE_TYPES = sizeof(BASE_TYPES) / sizeof(char*);

const char *COMPONENTS_H_HEADER =
"// Generated"
"\n#pragma once"
"\n#include <linmath.h>"
"\n#include <stdint.h>"
"\n#include <lua.h>"
"\n#include \"vec.h\""
"\n#include \"ecs.h\""
"\n"
"\nextern void components_init(lua_State *L, ECS *ecs);";

const char *COMPONENTS_C_HEADER = 
"// Generated"
"\n#include \"components.h\""
"\n#include <linmath.lua.h>"
"\n#include <stdint.h>"
"\n#include <lauxlib.h>"
"\n#include <lualib.h>"
"\n#include <imgui_impl.h>"
"\n"
"\nstatic ECS *s_ecs;"
"\n"
"\nstatic int lcb_create_entity(lua_State *L)"
"\n{"
"\n    Entity ent = ecs_create_entity(s_ecs);"
"\n    lua_pushnumber(L, (double)ent);"
"\n    return 1;"
"\n}"
"\n"
"\nstatic int lcb_destroy_entity(lua_State *L)"
"\n{"
"\n    double entity = luaL_checknumber(L, 1);"
"\n    ecs_destroy_entity(s_ecs, (Entity)entity);"
"\n    return 0;"
"\n}"
"\n"
"\nstatic void icb_inspect_Entity(const char *label, Entity *v) { igText(\"%s {Entity}\", label); }"
"\nstatic void icb_inspect_float(const char *label, float *v) { igDragFloat(label, v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_vec2(const char *label, vec2 *v) { igDragFloat2(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_vec3(const char *label, vec3 *v) { igDragFloat3(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_vec4(const char *label, vec4 *v) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_quat(const char *label, quat *v) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); quat_norm(*v, *v); }"
"\nstatic void icb_inspect_mat4x4(const char *label, quat *v) { igText(\"%s {Matrix}\", label); }"
"\nstatic void icb_inspect_Vec_T(const char *label, void *v) { igText(\"%s {Vec<T>}\", label); }"
"\n"
"\nstatic void lcb_push_Entity(lua_State *L, Entity *v) { lua_pushnumber(L, (double)(*v)); }"
"\nstatic void lcb_pop_Entity(lua_State *L, Entity *v, int stack_index) { *v = (Entity)luaL_checknumber(L, stack_index); }"
"\nstatic void lcb_push_float(lua_State *L, float *v) { lua_pushnumber(L, (double)(*v)); }"
"\nstatic void lcb_pop_float(lua_State *L, float *v, int stack_index) { *v = (float)luaL_checknumber(L, stack_index); }"
"\nstatic void lcb_push_vec2(lua_State *L, vec2 *v) { lml_push_vec2(L, *v); }"
"\nstatic void lcb_pop_vec2(lua_State *L, vec2 *v, int stack_index) { lml_get_vec2(L, stack_index, *v); }"
"\nstatic void lcb_push_vec3(lua_State *L, vec3 *v) { lml_push_vec3(L, *v); }"
"\nstatic void lcb_pop_vec3(lua_State *L, vec3 *v, int stack_index) { lml_get_vec3(L, stack_index, *v); }"
"\nstatic void lcb_push_vec4(lua_State *L, vec4 *v) { lml_push_vec4(L, *v); }"
"\nstatic void lcb_pop_vec4(lua_State *L, vec4 *v, int stack_index) { lml_get_vec4(L, stack_index, *v); }"
"\nstatic void lcb_push_quat(lua_State *L, quat *v) { lml_push_quat(L, *v); }"
"\nstatic void lcb_pop_quat(lua_State *L, quat *v, int stack_index) { lml_get_quat(L, stack_index, *v); }"
"\nstatic void lcb_push_mat4x4(lua_State *L, mat4x4 v) { lml_push_mat4x4(L, *v); }"
"\nstatic void lcb_pop_mat4x4(lua_State *L, mat4x4 *v, int stack_index) { lml_get_mat4x4(L, stack_index, *v); }"
"\n";

#define W(output, ...) output += sprintf(output, "\n" __VA_ARGS__)
#define WL(output, ...) output += sprintf(output, __VA_ARGS__)

static void write_struct_def(char **output, cJSON *type)
{
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
    W(*output, "typedef struct %s", name);
    W(*output, "{");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);

        if (cJSON_GetObjectItem(field, "vec"))
            W(*output, "    Vec %s; // of %s", 
                cJSON_GetStringValue(cJSON_GetObjectItem(field, "name")),
                cJSON_GetStringValue(cJSON_GetObjectItem(field, "type"))
            );
        else
            W(*output, "    %s %s;", 
                cJSON_GetStringValue(cJSON_GetObjectItem(field, "type")),
                cJSON_GetStringValue(cJSON_GetObjectItem(field, "name"))
            );
    }

    W(*output, "}");
    W(*output, "%s;\n", name);
}

static void write_default_for_field(char **output, cJSON *field)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(field, "type"));
    cJSON *default_override_item = cJSON_GetObjectItem(field, "default");
    cJSON *is_vec = cJSON_GetObjectItem(field, "vec");

    if (default_override_item) WL(*output, "%s", cJSON_GetStringValue(default_override_item));
    else if (is_vec) WL(*output, "{sizeof(%s),0,0}", type_name);
    else if (strcmp(type_name, "float") == 0) WL(*output, "0.f");
    else if (strcmp(type_name, "vec2") == 0) WL(*output, "{0.f,0.f}");
    else if (strcmp(type_name, "vec3") == 0) WL(*output, "{0.f,0.f,0.f}");
    else if (strcmp(type_name, "vec4") == 0) WL(*output, "{0.f,0.f,0.f,0.f}");
    else if (strcmp(type_name, "quat") == 0) WL(*output, "{0.f,0.f,0.f,1.f}");
    else if (strcmp(type_name, "Entity") == 0) WL(*output, "0");
    else if (strcmp(type_name, "mat4x4") == 0) 
        WL(*output, "{{1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},{0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}");
}

static void write_default_def(char **output, cJSON *type)
{
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    WL(*output, "static const %s %s_default = {", name, name);

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);
        write_default_for_field(output, field);
        if (i < max - 1) WL(*output, ",");
    }

    WL(*output, "};\n");
}

static void write_vec_push_pop(char **output, const char *typename)
{
    W(*output, "static void lcb_push_Vec_%s(lua_State *L, Vec *v)", typename);
    W(*output, "{");
    W(*output, "    lua_newtable(L);");
    W(*output, "    for (int i = 0; i < v->item_count; ++i)");
    W(*output, "    {");
    W(*output, "        lcb_push_%s(L, vec_at(v, i));", typename);
    W(*output, "        lua_rawseti(L, -2, i + 1);");
    W(*output, "    }");
    W(*output, "}");
    W(*output, "static void lcb_pop_Vec_%s(lua_State *L, Vec *v, int stack_index)", typename);
    W(*output, "{");
    W(*output, "    luaL_checktype(L, stack_index, LUA_TTABLE);");
    W(*output, "    size_t vec_len = lua_objlen(L, stack_index); ");
    W(*output, "");
    // TODO need to maybe have destructors inside of Vec rather than GenenrationalIndexArray
    // Lua is going to be manipulating Vecs on components directly.
    // This will work unless you have vecs of vecs, then memory will leak.
    W(*output, "    vec_resize(v, vec_len);");
    W(*output, "");
    W(*output, "    for (int i = 0; i < vec_len; ++i)");
    W(*output, "    {");
    W(*output, "        lua_rawgeti(L, stack_index, i + 1);");
    W(*output, "        lcb_pop_%s(L, vec_at(v, i), -1);", typename);
    W(*output, "        lua_pop(L, 1);");
    W(*output, "    }");
    W(*output, "}");
}

static void write_lua_push(char **output, cJSON *type, bool with_body)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    // TODO this code for maybe writing a prototype is in like 5 places.
    W(*output, "static void lcb_push_%s(lua_State *L, %s *v)", type_name, type_name);

    if (! with_body) 
    {
        WL(*output, ";\n");
        return;
    }

    W(*output, "{");
    W(*output, "    lua_newtable(L);");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);
        const char *field_type_prefix = cJSON_GetObjectItem(field, "vec") ? "Vec_" : "";
        const char *field_name = cJSON_GetStringValue(cJSON_GetObjectItem(field, "name")); 
        const char *field_type = cJSON_GetStringValue(cJSON_GetObjectItem(field, "type")); 

        W(*output, "    lua_pushstring(L, \"%s\");", field_name);
        W(*output, "    lcb_push_%s%s(L, &v->%s);", field_type_prefix, field_type, field_name);
        W(*output, "    lua_settable(L, -3);");
    }

    W(*output, "}");
}

static void write_lua_pop(char **output, cJSON *type, bool with_body)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void lcb_pop_%s(lua_State *L, %s *v, int stack_index)", type_name, type_name);

    if (! with_body) 
    {
        WL(*output, ";\n");
        return;
    }

    W(*output, "{");
    W(*output, "    luaL_checktype(L, stack_index, LUA_TTABLE);");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);
        const char *field_type_prefix = cJSON_GetObjectItem(field, "vec") ? "Vec_" : "";
        const char *field_name = cJSON_GetStringValue(cJSON_GetObjectItem(field, "name")); 
        const char *field_type = cJSON_GetStringValue(cJSON_GetObjectItem(field, "type")); 

        W(*output, "    lua_getfield(L, stack_index, \"%s\");", field_name);
        W(*output, "    lcb_pop_%s%s(L, &v->%s, -1);", field_type_prefix, field_type, field_name);
        W(*output, "    lua_pop(L, 1);");
    }

    W(*output, "}");
}

static void write_add_component(char **output, const char *type_name)
{
    W(*output, "static int lcb_add_component_%s(lua_State *L)", type_name);
    W(*output, "{");
    W(*output, "    Entity e = (Entity)luaL_checknumber(L, 1);");
    W(*output, "    ECS_GET_COMPONENT_DECL(%s, x, s_ecs, e);", type_name);
    W(*output, "    if (!x) {");
    W(*output, "        ECS_ADD_COMPONENT_DEFAULT_DECL(%s, x_new, s_ecs, e);", type_name);
    W(*output, "        x = x_new;");
    W(*output, "    }");
    W(*output, "    lcb_push_%s(L, x);", type_name);
    W(*output, "    return 1;");
    W(*output, "}");
}

static void write_get_component(char **output, const char *type_name)
{
    W(*output, "static int lcb_get_component_%s(lua_State *L)", type_name);
    W(*output, "{");
    W(*output, "    Entity e = (Entity)luaL_checknumber(L, 1);");
    W(*output, "    ECS_GET_COMPONENT_DECL(%s, x, s_ecs, e);", type_name);
    W(*output, "    lcb_push_%s(L, x);", type_name);
    W(*output, "    return 1;");
    W(*output, "}");
}

static void write_set_component(char **output, const char *type_name)
{
    W(*output, "static int lcb_set_component_%s(lua_State *L)", type_name);
    W(*output, "{");
    W(*output, "    Entity e = (Entity)luaL_checknumber(L, 1);");
    W(*output, "    ECS_GET_COMPONENT_DECL(%s, x, s_ecs, e);", type_name);
    W(*output, "    lcb_pop_%s(L, x, 2);", type_name);
    W(*output, "    return 0;");
    W(*output, "}");
}

static void write_inspector(char **output, cJSON *type, bool body_decl)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "%s", body_decl ? "" : "extern ");
    WL(*output, "void icb_inspect_%s(%s *v)", type_name, type_name);

    if (! body_decl) 
    {
        WL(*output, ";\n");
        return;
    }

    W(*output, "{");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);

        if (cJSON_GetObjectItem(field, "hideInInspector")) continue;

        const char *field_name = cJSON_GetStringValue(cJSON_GetObjectItem(field, "name")); 
        const char *field_type = cJSON_GetStringValue(cJSON_GetObjectItem(field, "type")); 

        W(*output, "    igPushIDInt(%d);", i);

        if (cJSON_GetObjectItem(field, "vec"))
            W(*output, "    icb_inspect_Vec_T(\"%s\", NULL);", field_name);
        else
            W(*output, "    icb_inspect_%s(\"%s\", &v->%s);", field_type, field_name, field_name);

        W(*output, "    igPopID();");
    }

    W(*output, "}");
}

static void write_inspect_all(char **output, cJSON *types, bool body_decl)
{
    W(*output, "%s", body_decl ? "" : "extern ");
    WL(*output, "void icb_inspect_all(Entity e)");

    if (! body_decl) 
    {
        WL(*output, ";\n");
        return;
    }

    W(*output, "{");

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

        W(*output, "    {");
        W(*output, "        ECS_GET_COMPONENT_DECL(%s, v, s_ecs, e);", type_name);
        W(*output, "        if (v && igCollapsingHeader(\"%s\", 0))", type_name);
        W(*output, "            icb_inspect_%s(v);", type_name);
        W(*output, "    }");
    }

    W(*output, "}");
}

static void write_components_init(char **output, cJSON *types)
{
    W(*output, "void components_init(lua_State *L, ECS *ecs)");
    W(*output, "{");
    W(*output, "    s_ecs = ecs;");

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

        W(*output, "    lua_pushcfunction(L, lcb_get_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"get_component_%s\");", type_name);
        W(*output, "    lua_pushcfunction(L, lcb_set_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"set_component_%s\");", type_name);
        W(*output, "    lua_pushcfunction(L, lcb_add_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"add_component_%s\");", type_name);
        W(*output, "    ECS_REGISTER_COMPONENT(%s, ecs, NULL);", type_name);
    }
    
    W(*output, "    lua_pushcfunction(L, lcb_create_entity);");
    W(*output, "    lua_setglobal(L, \"create_entity\");");
    W(*output, "    lua_pushcfunction(L, lcb_destroy_entity);");
    W(*output, "    lua_setglobal(L, \"destroy_entity\");");
    W(*output, "}");
}

static char *generate_components_h_alloc(cJSON *types)
{
    char *output = malloc(1024 * 1024);
    char *output_start = output;

    W(output, "%s", COMPONENTS_H_HEADER);

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        write_struct_def(&output, type);
        write_default_def(&output, type);
        write_inspector(&output, type, false);
    }

    write_inspect_all(&output, types, false);

    return output_start;
}

static char *generate_components_c_alloc(cJSON *types)
{
    char *output = malloc(1024 * 1024);
    char *output_start = output;

    W(output, "%s", COMPONENTS_C_HEADER);

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        write_lua_push(&output, type, false);
        write_lua_pop(&output, type, false);
    }

    for (int i = 0; i < NUM_BASE_TYPES; ++i)
        write_vec_push_pop(&output, BASE_TYPES[i]);

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

        write_inspector(&output, type, true);
        write_lua_push(&output, type, true);
        write_lua_pop(&output, type, true);
        write_add_component(&output, type_name);
        write_get_component(&output, type_name);
        write_set_component(&output, type_name);
        write_vec_push_pop(&output, type_name);
    }

    write_inspect_all(&output, types, true);
    write_components_init(&output, types);

    return output_start;
}

void generate_components(void)
{
    const char *json_file = read_file_alloc("components.json");
    cJSON *json = cJSON_Parse(json_file);

    const char *components_h = generate_components_h_alloc(json);
    write_file("src/components.h", components_h);
    free(components_h);

    const char *components_c = generate_components_c_alloc(json);
    write_file("src/components.c", components_c);
    free(components_c);

    cJSON_Delete(json);
    free(json_file);

    getchar();
}