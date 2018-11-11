#define _CRT_SECURE_NO_WARNINGS 1

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cJSON.h>

#include "../src/utils.h"

const char *BASE_TYPES[] = { "int", "float", "bool", "vec2", "vec3", "vec4", "versor", "mat4", "Entity", "string" };
const size_t NUM_BASE_TYPES = sizeof(BASE_TYPES) / sizeof(char*);

const char *COMPONENTS_H_HEADER =
  "#pragma once"
"\n#include <cglm/cglm.h>"
"\n#include <stdint.h>"
"\n#include <lua.h>"
"\n#include \"containers/vec.h\""
"\n#include \"containers/ecs.h\""
"\n"
"\nextern ECS *components_ecs_new(void);"
"\nextern void components_bind_ecs(ECS *ecs);"
"\nextern void components_init_lua(lua_State *L);"
"\nextern void components_inspect_entity(Entity e);"
"\nextern char *components_serialize_scene_alloc(void);"
"\nextern ECS *components_deserialize_scene_alloc(const char *json_scene);"
"\nextern const char *components_name_entity(Entity e);"
"\n"
"\nextern Entity *components_entity_to_change;"
"\n";

const char *COMPONENTS_C_HEADER = 
  "#define _CRT_SECURE_NO_WARNINGS 1"
"\n#include \"components.h\""
"\n#include <cglm.lua.h>"
"\n#include <stdint.h>"
"\n#include <stdbool.h>"
"\n#include <lauxlib.h>"
"\n#include <lualib.h>"
"\n#include <imgui_impl.h>"
"\n#include <string.h>"
"\n#include <cJSON.h>"
"\n#include \"utils.h\""
"\n#include \"containers/hashtable.h\""
"\n#ifdef _MSC_VER"
"\n#define strdup _strdup"
"\n#endif"
"\n"
"\nstatic ECS *s_ecs;"
"\n"
"\nEntity *components_entity_to_change;"
"\n"
"\nvoid components_bind_ecs(ECS *ecs)"
"\n{"
"\n    s_ecs = ecs;"
"\n}"
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
"\nstatic void icb_inspect_int    (const char *label, int    *v) { igInputInt(label, v, 1, 10, 0); }"
"\nstatic void icb_inspect_float  (const char *label, float  *v) { igDragFloat(label, v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_bool   (const char *label, bool   *v) { igCheckbox(label, v); }"
"\nstatic void icb_inspect_vec2   (const char *label, vec2   *v) { igDragFloat2(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_vec3   (const char *label, vec3   *v) { igDragFloat3(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_vec4   (const char *label, vec4   *v) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }"
"\nstatic void icb_inspect_versor (const char *label, versor *v) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); glm_quat_normalize(*v); }"
"\nstatic void icb_inspect_mat4   (const char *label, mat4   *v) { igText(\"%s {Matrix}\", label); }"
"\nstatic void icb_inspect_Entity (const char *label, Entity *v) {"
"\n    if(igButton(*v ? components_name_entity(*v) : \"[no entity]\", (ImVec2){0,0}))"
"\n        components_entity_to_change = v;"
"\n    igSameLine(0, -1);"
"\n    igText(\"%s\", label);"
"\n}"
"\nstatic void icb_inspect_Vec_T  (const char *label, void   *v) { igText(\"%s {Vec<T>}\", label); }"
"\nstatic void icb_inspect_string (const char *label, char  **v) {"
"\n    if (!*v) { *v = malloc(1); (*v)[0] = 0; }"
"\n    char buf[1024] = \"\";"
"\n    buf[1023] = 0;"
"\n    strncpy(buf, *v, 1023);"
"\n    igInputText(label, buf, 1024, 0, NULL, NULL);"
"\n    if (strcmp(buf, *v) == 0) return;"
"\n    free(*v);"
"\n    size_t len = strlen(buf);"
"\n    *v = malloc(len + 1);"
"\n    strncpy(*v, buf, len);"
"\n    (*v)[len] = 0;"
"\n}"
"\n"
"\nstatic cJSON *serialize_field_int    (int    *c) { return cJSON_CreateNumber(*c); }"
"\nstatic cJSON *serialize_field_float  (float  *c) { return cJSON_CreateNumber(*c); }"
"\nstatic cJSON *serialize_field_bool   (bool   *c) { return cJSON_CreateBool  (*c); }"
"\nstatic cJSON *serialize_field_vec2   (vec2   *c) { return cJSON_CreateFloatArray((float*)(*c), 2); }"
"\nstatic cJSON *serialize_field_vec3   (vec3   *c) { return cJSON_CreateFloatArray((float*)(*c), 3); }"
"\nstatic cJSON *serialize_field_vec4   (vec4   *c) { return cJSON_CreateFloatArray((float*)(*c), 4); }"
"\nstatic cJSON *serialize_field_versor (versor *c) { return cJSON_CreateFloatArray((float*)(*c), 4); }"
"\nstatic cJSON *serialize_field_mat4   (mat4   *c) { return cJSON_CreateFloatArray((float*)(*c), 16); }"
"\nstatic cJSON *serialize_field_Entity (Entity *c) { return cJSON_CreateNumber((double)(*c)); }"
"\nstatic cJSON *serialize_field_string (char  **c) { return cJSON_CreateString(*c); }"
"\n"
"\nstatic void deserialize_field_int    ( cJSON *item, int    *out, void *x ) { *out = (int)item->valuedouble; }"
"\nstatic void deserialize_field_float  ( cJSON *item, float  *out, void *x ) { *out = (float)item->valuedouble; }"
"\nstatic void deserialize_field_bool   ( cJSON *item, bool   *out, void *x ) { *out = cJSON_IsTrue(item); }"
"\nstatic void deserialize_field_vec2   ( cJSON *item, vec2   *out, void *x ) { for( int i = 0; i < 2; ++i ) (*out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; }"
"\nstatic void deserialize_field_vec3   ( cJSON *item, vec3   *out, void *x ) { for( int i = 0; i < 3; ++i ) (*out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; }"
"\nstatic void deserialize_field_vec4   ( cJSON *item, vec4   *out, void *x ) { for( int i = 0; i < 4; ++i ) (*out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; }"
"\nstatic void deserialize_field_versor ( cJSON *item, versor *out, void *x ) { for( int i = 0; i < 4; ++i ) (*out)[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; }"
"\nstatic void deserialize_field_mat4   ( cJSON *item, mat4   *out, void *x ) { for( int i = 0; i < 16; ++i ) ((float*)(*out))[i] = (float)cJSON_GetArrayItem( item, i )->valuedouble; }"
"\nstatic void deserialize_field_string ( cJSON *item, char  **out, void *x ) { *out = strdup(cJSON_GetStringValue(item)); }"
"\nstatic void deserialize_field_Entity ( cJSON *item, Entity *out, HashTable *entities_for_ids ) {"
"\n    char id_as_str_buf[128];"
"\n    sprintf( id_as_str_buf, \"%f\", item->valuedouble );"
"\n    *out = *(Entity*)hashtable_at( entities_for_ids, id_as_str_buf );"
"\n}"
"\n"
"\nstatic void lcb_push_int    (lua_State *L, int    *v)                  { lua_pushnumber(L, (int)(*v)); }"
"\nstatic void  lcb_pop_int    (lua_State *L, int    *v, int stack_index) { *v = (int)luaL_checknumber(L, stack_index); }"
"\nstatic void lcb_push_float  (lua_State *L, float  *v)                  { lua_pushnumber(L, (double)(*v)); }"
"\nstatic void  lcb_pop_float  (lua_State *L, float  *v, int stack_index) { *v = (float)luaL_checknumber(L, stack_index); }"
"\nstatic void lcb_push_bool   (lua_State *L, bool   *v)                  { lua_pushboolean(L, *v); }"
"\nstatic void  lcb_pop_bool   (lua_State *L, bool   *v, int stack_index) { *v = (bool)lua_toboolean(L, stack_index); }"
"\nstatic void lcb_push_vec2   (lua_State *L, vec2   *v)                  { glmlua_push_vec2(L, *v); }"
"\nstatic void  lcb_pop_vec2   (lua_State *L, vec2   *v, int stack_index) { glmlua_get_vec2(L, stack_index, *v); }"
"\nstatic void lcb_push_vec3   (lua_State *L, vec3   *v)                  { glmlua_push_vec3(L, *v); }"
"\nstatic void  lcb_pop_vec3   (lua_State *L, vec3   *v, int stack_index) { glmlua_get_vec3(L, stack_index, *v); }"
"\nstatic void lcb_push_vec4   (lua_State *L, vec4   *v)                  { glmlua_push_vec4(L, *v); }"
"\nstatic void  lcb_pop_vec4   (lua_State *L, vec4   *v, int stack_index) { glmlua_get_vec4(L, stack_index, *v); }"
"\nstatic void lcb_push_versor (lua_State *L, versor *v)                  { glmlua_push_quat(L, *v); }"
"\nstatic void  lcb_pop_versor (lua_State *L, versor *v, int stack_index) { glmlua_get_quat(L, stack_index, *v); }"
"\nstatic void lcb_push_mat4   (lua_State *L, mat4   *v)                  { glmlua_push_mat4(L, *v); }"
"\nstatic void  lcb_pop_mat4   (lua_State *L, mat4   *v, int stack_index) { glmlua_get_mat4(L, stack_index, *v); }"
"\nstatic void lcb_push_Entity (lua_State *L, Entity *v)                  { lua_pushnumber(L, (double)(*v)); }"
"\nstatic void  lcb_pop_Entity (lua_State *L, Entity *v, int stack_index) { *v = (Entity)luaL_checknumber(L, stack_index); }"
"\nstatic void lcb_push_string (lua_State *L, char  **v)                  { lua_pushstring(L, *v ? *v : \"\"); }"
"\nstatic void  lcb_pop_string (lua_State *L, char  **v, int stack_index) { if(*v) free(*v); *v = strdup(luaL_checkstring(L, stack_index)); }"
"\n";


#define W(output, ...) output += sprintf(output, "\n/*generated*/" __VA_ARGS__)
#define WL(output, ...) output += sprintf(output, __VA_ARGS__)

static bool is_type_basic(const char *type_name)
{
    for (int i = 0; i < NUM_BASE_TYPES; ++i)
        if (strcmp(type_name, BASE_TYPES[i]) == 0) 
            return true;

    return false;
}

static bool should_serialize(cJSON *obj)
{
    cJSON *item = cJSON_GetObjectItem(obj, "serialize");
    if (!item) return true;
    return !cJSON_IsFalse(item);
}

static void write_struct_def(char **output, cJSON *type)
{
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
    W(*output, "typedef struct %s", name);
    W(*output, "{");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);
        const char *field_name = cJSON_GetStringValue(cJSON_GetObjectItem(field, "name"));
        const char *field_type = cJSON_GetStringValue(cJSON_GetObjectItem(field, "type"));

        if (cJSON_GetObjectItem(field, "vec"))
            W(*output, "    Vec %s; // of %s", field_name, field_type);
        else if (strcmp("string", field_type) == 0)
            W(*output, "    char *%s;", field_name);
        else
            W(*output, "    %s %s;", field_type, field_name);
    }

    W(*output, "}");
    W(*output, "%s;\n", name);
}

static void write_default_for_type(char **output, cJSON *types, cJSON *type);

static void write_default_for_type_name(char **output, cJSON *types, const char *type_name)
{
    if (strcmp(type_name, "float") == 0) WL(*output, "0.f");
    else if (strcmp(type_name, "bool") == 0) WL(*output, "0");
    else if (strcmp(type_name, "vec2") == 0) WL(*output, "{0.f,0.f}");
    else if (strcmp(type_name, "vec3") == 0) WL(*output, "{0.f,0.f,0.f}");
    else if (strcmp(type_name, "vec4") == 0) WL(*output, "{0.f,0.f,0.f,0.f}");
    else if (strcmp(type_name, "versor") == 0) WL(*output, "{0.f,0.f,0.f,1.f}");
    else if (strcmp(type_name, "Entity") == 0) WL(*output, "0");
    else if (strcmp(type_name, "string") == 0) WL(*output, "0");
    else if (strcmp(type_name, "mat4") == 0) 
        WL(*output, "{{1.f,0.f,0.f,0.f},{0.f,1.f,0.f,0.f},{0.f,0.f,1.f,0.f},{0.f,0.f,0.f,1.f}}");

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *iter_type = cJSON_GetArrayItem(types, i);
        const char *iter_type_name = cJSON_GetStringValue(cJSON_GetObjectItem(iter_type, "name"));

        if (strcmp(iter_type_name, type_name) == 0)
        {
            write_default_for_type(output, types, iter_type);
            break;
        }
    }
}

static void write_default_for_field( char **output, cJSON *types, cJSON *field )
{
    const char *type_name = cJSON_GetStringValue( cJSON_GetObjectItem( field, "type" ) );
    cJSON *default_override_item = cJSON_GetObjectItem( field, "default" );
    cJSON *is_vec = cJSON_GetObjectItem( field, "vec" );

    if( default_override_item ) 
        WL( *output, "%s", cJSON_GetStringValue( default_override_item ) );
    else if( is_vec ) 
        WL( *output, "{sizeof(%s),0,0}", strcmp( "string", type_name ) == 0 ? "char*" : type_name );
    else 
        write_default_for_type_name( output, types, type_name );
}

static void write_default_for_type(char **output, cJSON *types, cJSON *type)
{
    WL(*output, "{");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);
        write_default_for_field(output, types, field);
        if (i < max - 1) WL(*output, ",");
    }

    WL(*output, "}");
}

static void write_default_def(char **output, cJSON *types, cJSON *type)
{
    const char *name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    WL(*output, "static const %s %s_default = ", name, name);
    write_default_for_type(output, types, type);
    WL(*output, ";\n");
}

static void write_vec_push_pop(char **output, const char *type_name)
{
    bool is_string = strcmp( "string", type_name ) == 0;
    bool clear_with_callback = !is_type_basic( type_name ) || is_string;

    if( clear_with_callback )
    {
        if( is_string )
            W(*output, "static void lcb_vec_clear_callback_string(void *ctx, char **v) { free(*v); }");
        else
            W(*output, "static void lcb_vec_clear_callback_%s(void *ctx, %s *v) { destruct_%s(v); }", type_name, type_name, type_name);
    }

    W(*output, "static void lcb_push_Vec_%s(lua_State *L, Vec *v)", type_name);
    W(*output, "{");
    W(*output, "    lua_newtable(L);");
    W(*output, "    for (int i = 0; i < v->item_count; ++i)");
    W(*output, "    {");
    W(*output, "        lcb_push_%s(L, vec_at(v, i));", type_name);
    W(*output, "        lua_rawseti(L, -2, i + 1);");
    W(*output, "    }");
    W(*output, "}");
    W(*output, "static void lcb_pop_Vec_%s(lua_State *L, Vec *v, int stack_index)", type_name);
    W(*output, "{");
    W(*output, "    luaL_checktype(L, stack_index, LUA_TTABLE);");
    W(*output, "    size_t vec_len = lua_objlen(L, stack_index); ");
    W(*output, "");

    if( clear_with_callback )
        W(*output, "    vec_clear_with_callback(v, NULL, lcb_vec_clear_callback_%s);", type_name);

    W(*output, "    vec_resize(v, vec_len);");
    W(*output, "");
    W(*output, "    for (int i = 0; i < vec_len; ++i)");
    W(*output, "    {");
    W(*output, "        lua_rawgeti(L, stack_index, i + 1);");
    W(*output, "        lcb_pop_%s(L, vec_at(v, i), -1);", type_name);
    W(*output, "        lua_pop(L, 1);");
    W(*output, "    }");
    W(*output, "}");
}

static void write_lua_push(char **output, cJSON *type)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void lcb_push_%s(lua_State *L, %s *v)", type_name, type_name);
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

static void write_lua_pop(char **output, cJSON *type)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void lcb_pop_%s(lua_State *L, %s *v, int stack_index)", type_name, type_name);
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

static void write_find_entity_with(char **output, const char *type_name)
{
    W(*output, "static int lcb_find_entity_with_%s(lua_State *L)", type_name);
    W(*output, "{");
    W(*output, "    Entity e = 0;");
    W(*output, "    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(%s, s_ecs, &e);", type_name);
    W(*output, "    lcb_push_Entity(L, &e);");
    W(*output, "    return 1;");
    W(*output, "}");
}

static void write_serialize(char **output, cJSON *type)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void serialize_%s(%s *c, cJSON *obj, bool nested)", type_name, type_name);
    W(*output, "{");
    W(*output, "    cJSON *comp_obj = nested ? obj : cJSON_AddObjectToObject(obj, \"%s\");", type_name);

    cJSON *fields = cJSON_GetObjectItem( type, "fields" );
    for( int i = 0, max = cJSON_GetArraySize( fields ); i < max; ++i )
    {
        cJSON *field = cJSON_GetArrayItem( fields, i );

        if( !should_serialize( field ) ) continue;

        const char *field_name = cJSON_GetStringValue( cJSON_GetObjectItem( field, "name" ) ); 
        const char *field_type = cJSON_GetStringValue( cJSON_GetObjectItem( field, "type" ) ); 
        bool type_basic = is_type_basic( field_type );

        if( cJSON_GetObjectItem( field, "vec" ) ) 
        {
            if( type_basic )
            {
               W(*output, "  { cJSON *arr = cJSON_AddArrayToObject(comp_obj, \"%s\");", field_name);
               W(*output, "    for (int i = 0; i < c->%s.item_count; ++i)", field_name);
               W(*output, "        cJSON_AddItemToArray(arr, serialize_field_%s(vec_at(&c->%s, i))); }", field_type, field_name);
            }
            else
            {
                // TODO
            }
        }
        else if( type_basic )
        {
            W(*output, "    cJSON_AddItemToObject(comp_obj, \"%s\", serialize_field_%s(&c->%s));", field_name, field_type, field_name);
        }
        else
        {
            W(*output, "  { cJSON *sub_obj = cJSON_AddObjectToObject(comp_obj, \"%s\");", field_name);
            W(*output, "    serialize_%s(&c->%s, sub_obj, true); }", field_type, field_name);
        }
    }

    W(*output, "}");
}

static void write_deserialize(char **output, cJSON *type)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void deserialize_nested_%s( cJSON *item, %s *out, HashTable *entities_for_ids )", type_name, type_name);
    W(*output, "{");

    cJSON *field;
    cJSON_ArrayForEach( field, cJSON_GetObjectItem( type, "fields" ) )
    {
        if( !should_serialize( field ) ) continue;

        const char *field_name = cJSON_GetStringValue( cJSON_GetObjectItem( field, "name" ) ); 
        const char *field_type = cJSON_GetStringValue( cJSON_GetObjectItem( field, "type" ) ); 

        if( cJSON_GetObjectItem( field, "vec" ) ) 
        {
            // TODO
        }
        else if( is_type_basic( field_type ) )
        {
            W(*output, "    deserialize_field_%s( cJSON_GetObjectItem( item, \"%s\" ), &out->%s, entities_for_ids );", field_type, field_name, field_name);
        }
        else
        {
            W(*output, "    deserialize_nested_%s( cJSON_GetObjectItem( item, \"%s\" ), &out->%s, entities_for_ids );", field_type, field_name, field_name);
        }
    }

    W(*output, "}");

    W(*output, "static void deserialize_%s(Entity entity, ECS *ecs, HashTable *entities_for_ids, cJSON *component_obj)", type_name);
    W(*output, "{");
    W(*output, "    ECS_ADD_COMPONENT_DEFAULT_DECL( %s, comp, ecs, entity );", type_name);
    W(*output, "    deserialize_nested_%s( component_obj, comp, entities_for_ids );", type_name);
    W(*output, "}");
}

static void write_destructor( char **output, cJSON *type )
{
    const char *type_name = cJSON_GetStringValue( cJSON_GetObjectItem( type, "name" ) );

    W( *output, "static void destruct_%s( %s *v )", type_name, type_name );
    W( *output, "{" );

    cJSON *fields = cJSON_GetObjectItem( type, "fields" );
    for( int i = 0, max = cJSON_GetArraySize( fields ); i < max; ++i )
    {
        cJSON *field = cJSON_GetArrayItem( fields, i );
        const char *field_name = cJSON_GetStringValue( cJSON_GetObjectItem( field, "name" ) ); 
        const char *field_type = cJSON_GetStringValue( cJSON_GetObjectItem( field, "type" ) ); 
        bool type_basic = is_type_basic( field_type );
        bool type_string = strcmp( "string", field_type ) == 0;

        if( cJSON_GetObjectItem( field, "vec" ) ) 
        {
            if( !type_basic )
            {
                W( *output, "    for ( int i = 0; i < v->%s.item_count; ++i )", field_name );
                W( *output, "        destruct_%s( vec_at( &v->%s, i ) );", field_type, field_name );
            }
            else if( type_string )
            {
                W( *output, "    for ( int i = 0; i < v->%s.item_count; ++i )", field_name );
                W( *output, "        free( *(char**)vec_at( &v->%s, i ) );", field_name );
            }
            W( *output, "    vec_clear( &v->%s );", field_name );
        }
        else if( type_string )
        {
            W( *output, "    free( v->%s );", field_name );
            W( *output, "    v->%s = 0;", field_name );
        }
        else if( !type_basic )
        {
            W( *output, "    destruct_%s( &v->%s );", field_type, field_name );
        }
    }
    W( *output, "}" );
}

static void write_inspector(char **output, cJSON *type)
{
    const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

    W(*output, "static void icb_inspect_%s(const char *label, %s *v)", type_name, type_name);
    W(*output, "{");
    W(*output, "    if (label) { igSeparator(); igText(label); igSeparator(); }");

    cJSON *fields = cJSON_GetObjectItem(type, "fields");
    for (int i = 0, max = cJSON_GetArraySize(fields); i < max; ++i)
    {
        cJSON *field = cJSON_GetArrayItem(fields, i);

        if (cJSON_GetObjectItem(field, "hide")) continue;

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

static void write_component_names_array(char **output, cJSON *types)
{
    W(*output, "static const char *TOP_LEVEL_COMPONENT_NAMES[] = {");
    
    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        if (cJSON_GetObjectItem(type, "hide")) continue;

        WL(*output, "\"%s\"", type_name);
        if (i < max - 1) WL(*output, ",");
    }

    WL(*output, "};\n");
}

static void write_inspect_all(char **output, cJSON *types)
{
    W(*output, "void components_inspect_entity(Entity e)");
    W(*output, "{");
    W(*output, "    static int selected_component = 0;");
    W(*output, "    igCombo(\"\", &selected_component, TOP_LEVEL_COMPONENT_NAMES, COUNT_OF(TOP_LEVEL_COMPONENT_NAMES), 0);");
    W(*output, "    igSameLine(0, -1);");
    W(*output, "    bool do_add_component = igButton(\"Add Component\", (ImVec2){0,0});");
    W(*output, "    igSeparator();");

    int top_level_index = -1;
    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

        if (! cJSON_GetObjectItem(type, "hide"))
        {
            top_level_index++;
            W(*output, "    if (do_add_component && selected_component == %d) {", top_level_index);
            W(*output, "        ECS_ADD_COMPONENT_DEFAULT_DECL(%s, c, s_ecs, e);", type_name);
            W(*output, "    }");
        }

        W(*output, "    {");
        W(*output, "        igPushIDInt(%d);", i);
        W(*output, "        bool keep_alive = true;");
        W(*output, "        ECS_GET_COMPONENT_DECL(%s, v, s_ecs, e);", type_name);
        W(*output, "        if (v && igCollapsingHeaderBoolPtr(\"%s\", &keep_alive, 0))", type_name);
        W(*output, "            icb_inspect_%s(NULL, v);", type_name);
        W(*output, "        if (! keep_alive)");
        W(*output, "            ECS_REMOVE_COMPONENT(%s, s_ecs, e);", type_name);
        W(*output, "        igPopID();");
        W(*output, "    }");
    }

    W(*output, "}");
}


static void write_components_init(char **output, cJSON *types)
{
    cJSON *type;

    W(*output, "ECS *components_ecs_new(void)");
    W(*output, "{");
    W(*output, "    ECS *ecs = ecs_new();");

    cJSON_ArrayForEach(type, types)
    {
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        W(*output, "    ECS_REGISTER_COMPONENT(%s, ecs, destruct_%s);", type_name, type_name);
    }

    W(*output, "    return ecs;");
    W(*output, "}");
    W(*output, "void components_init_lua(lua_State *L)");
    W(*output, "{");

    cJSON_ArrayForEach(type, types)
    {
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        W(*output, "    lua_pushcfunction(L, lcb_get_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"get_component_%s\");", type_name);
        W(*output, "    lua_pushcfunction(L, lcb_set_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"set_component_%s\");", type_name);
        W(*output, "    lua_pushcfunction(L, lcb_add_component_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"add_component_%s\");", type_name);
        W(*output, "    lua_pushcfunction(L, lcb_find_entity_with_%s);", type_name);
        W(*output, "    lua_setglobal(L, \"find_entity_with_%s\");", type_name);
    }
    
    W(*output, "    lua_pushcfunction(L, lcb_create_entity);");
    W(*output, "    lua_setglobal(L, \"create_entity\");");
    W(*output, "    lua_pushcfunction(L, lcb_destroy_entity);");
    W(*output, "    lua_setglobal(L, \"destroy_entity\");");
    W(*output, "}");
}

static void write_components_name_entity(char **output, cJSON *types)
{
    W(*output, "const char *components_name_entity(Entity e)");
    W(*output, "{");
    W(*output, "    ECS_GET_COMPONENT_DECL(Transform, t, s_ecs, e);");
    W(*output, "    if (t && t->name && strlen(t->name)) return t->name;");

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        W(*output, "    { ECS_GET_COMPONENT_DECL(%s, c, s_ecs, e);", type_name);
        W(*output, "    if (c) return \"(%s)\"; }", type_name);
    }

    W(*output, "    return \"(empty)\";");
    W(*output, "}");
}

static void write_components_serialize_scene(char **output, cJSON *types)
{
    W(*output, "char *components_serialize_scene_alloc(void)");
    W(*output, "{");
    W(*output, "    cJSON *json = cJSON_CreateArray();");
    W(*output, "    size_t num_entities;");
    W(*output, "    Entity *entities = ecs_find_all_entities_alloc(s_ecs, &num_entities);");

    W(*output, "    for (int i = 0; i < num_entities; ++i) {");
    W(*output, "        bool empty = true;");
    W(*output, "        cJSON *obj = cJSON_CreateObject();");
    W(*output, "        cJSON_AddNumberToObject(obj, \"_id\", (double)entities[i]);");
    
    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        if (!should_serialize(type)) continue;
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        W(*output, "        { ECS_GET_COMPONENT_DECL(%s, c, s_ecs, entities[i]);", type_name);
        W(*output, "        if (c) { serialize_%s(c, obj, false); empty = false; } }", type_name);
    }

    W(*output, "        if (empty)");
    W(*output, "            cJSON_Delete(obj);");
    W(*output, "        else");
    W(*output, "            cJSON_AddItemToArray(json, obj);");
    W(*output, "    }");

    W(*output, "    char *result = cJSON_PrintUnformatted(json);");
    W(*output, "    free(entities);");
    W(*output, "    cJSON_Delete(json);");
    W(*output, "    return result;");
    W(*output, "}");
}

static void write_components_deserialize_scene(char **output, cJSON *types)
{
    W(*output, "ECS *components_deserialize_scene_alloc( const char *json_scene )");
    W(*output, "{");
    W(*output, "    ECS *result = components_ecs_new();");
    W(*output, "    cJSON *json = cJSON_Parse( json_scene );");
    W(*output, "    HashTable entities_for_ids = hashtable_empty( 1024, sizeof( Entity ) );");
    W(*output, "    char id_as_str_buf[128];");
    W(*output, "    cJSON *entity_obj;");
    W(*output, "");
    W(*output, "    Entity empty_entity = 0;");
    W(*output, "    sprintf( id_as_str_buf, \"%%f\", 0.0 );");
    W(*output, "    hashtable_set_copy( &entities_for_ids, id_as_str_buf, &empty_entity );");
    W(*output, "");
    W(*output, "    cJSON_ArrayForEach( entity_obj, json )");
    W(*output, "    {");
    W(*output, "        sprintf( id_as_str_buf, \"%%f\", cJSON_GetObjectItem( entity_obj, \"_id\" )->valuedouble );");
    W(*output, "        Entity new_entity = ecs_create_entity( result );");
    W(*output, "        hashtable_set_copy( &entities_for_ids, id_as_str_buf, &new_entity );");
    W(*output, "    }");
    W(*output, "");
    W(*output, "    cJSON_ArrayForEach( entity_obj, json )");
    W(*output, "    {");
    W(*output, "        sprintf( id_as_str_buf, \"%%f\", cJSON_GetObjectItem( entity_obj, \"_id\" )->valuedouble );");
    W(*output, "        Entity e = *(Entity*)hashtable_at( &entities_for_ids, id_as_str_buf );");
    W(*output, "");
    W(*output, "        cJSON *component_obj;");
    W(*output, "        cJSON_ArrayForEach( component_obj, entity_obj )");
    W(*output, "        {");
    W(*output, "            char *type_name = component_obj->string;");
    W(*output, "            if( strcmp( \"_id\", type_name ) == 0 ) continue;");
    W(*output, "");

    cJSON *type;
    cJSON_ArrayForEach( type, types )
    {
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));
        W(*output, "            if( strcmp( \"%s\", type_name ) == 0 ) ", type_name);
        W(*output, "                deserialize_%s( e, result, &entities_for_ids, component_obj );", type_name);
    }

    W(*output, "        }");
    W(*output, "    }");
    W(*output, "");
    W(*output, "    hashtable_clear( &entities_for_ids );");
    W(*output, "    cJSON_Delete( json );");
    W(*output, "    return result;");
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
        write_default_def(&output, types, type);
    }

    return output_start;
}

static char *generate_components_c_alloc(cJSON *types)
{
    char *output = malloc(1024 * 1024);
    char *output_start = output;

    W(output, "%s", COMPONENTS_C_HEADER);

    write_component_names_array(&output, types);

    for (int i = 0; i < NUM_BASE_TYPES; ++i)
        write_vec_push_pop(&output, BASE_TYPES[i]);

    for (int i = 0, max = cJSON_GetArraySize(types); i < max; ++i)
    {
        cJSON *type = cJSON_GetArrayItem(types, i);
        const char *type_name = cJSON_GetStringValue(cJSON_GetObjectItem(type, "name"));

        write_inspector(&output, type);
        write_lua_push(&output, type);
        write_lua_pop(&output, type);
        write_add_component(&output, type_name);
        write_get_component(&output, type_name);
        write_set_component(&output, type_name);
        write_find_entity_with(&output, type_name);
        write_destructor(&output, type);
        write_vec_push_pop(&output, type_name);
        write_serialize(&output, type);
        write_deserialize(&output, type);
    }

    write_inspect_all(&output, types);
    write_components_init(&output, types);
    write_components_name_entity(&output, types);
    write_components_serialize_scene(&output, types);
    write_components_deserialize_scene(&output, types);

    return output_start;
}

void generate_components(void)
{
    char *json_file = utils_read_file_alloc("", "components.json", NULL);
    cJSON *json = cJSON_Parse(json_file);
    free(json_file);

    char *components_h = generate_components_h_alloc(json);
    utils_write_string_file("src/components.h", components_h);
    free(components_h);

    char *components_c = generate_components_c_alloc(json);
    utils_write_string_file("src/components.c", components_c);
    free(components_c);

    cJSON_Delete(json);

    getchar();
}