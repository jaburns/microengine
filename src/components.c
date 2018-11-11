#define _CRT_SECURE_NO_WARNINGS

#include "components.h"

#include <cglm/cglm.h>
#include <cJSON.h>
#include <string.h>
#include <imgui_impl.h>

#include "utils.h"
#include "component_defs.h"

static ECS *s_ecs; // DOESNT NEED TO BE STATIC SINCE WE DROPPED LUA

Entity *components_entity_to_change;

void components_bind_ecs( ECS *ecs )
{
    s_ecs = ecs;
}

static void inspect_int   ( const char *label, int    *v ) { igInputInt(label, v, 1, 10, 0); }
static void inspect_float ( const char *label, float  *v ) { igDragFloat(label, v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_bool  ( const char *label, bool   *v ) { igCheckbox(label, v); }
static void inspect_vec2  ( const char *label, vec2   *v ) { igDragFloat2(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_vec3  ( const char *label, vec3   *v ) { igDragFloat3(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_vec4  ( const char *label, vec4   *v ) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); }
static void inspect_versor( const char *label, versor *v ) { igDragFloat4(label, *v, 0.005f, -INFINITY, INFINITY, NULL, 1.0f); glm_quat_normalize(*v); }
static void inspect_mat4  ( const char *label, mat4   *v ) { igText("%s {Matrix}", label); }
static void inspect_Vec_T ( const char *label, void   *v ) { igText("%s {Vec<T>}", label); }

static void inspect_Entity( const char *label, Entity *v ) 
{
    bool name_from_transform;
    const char *entity_name = components_name_entity( *v, &name_from_transform );

    char name_buffer[256];
    snprintf( name_buffer, 256, name_from_transform ? "%s" : "(%s)", entity_name );

    if( igButton( name_buffer, (ImVec2){ 0, 0 } ) )
        components_entity_to_change = v;

    igSameLine( 0, -1 );
    igText( "%s", label );
}

static void inspect_string( const char *label, char **v )
{
    if( !*v )
    {
        *v = malloc(1); 
        (*v)[0] = 0;
    }

    char buf[1024] = "";
    buf[1023] = 0;
    strncpy( buf, *v, 1023 );
    igInputText( label, buf, 1024, 0, NULL, NULL );

    if( strcmp( buf, *v ) != 0 )
    {
        free( *v );
        size_t len = strlen( buf );
        *v = malloc( len + 1 );
        strncpy( *v, buf, len );
        (*v)[len] = 0;
    }
}

static const ComponentInfo *get_info_for_component_type( const char *type_name )
{
    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];
        if( strcmp( info->name, type_name ) == 0 ) return info;
    }

    PANIC("Unrecognized component type in get_info_for_component_type");
}

static void inspect_component( void *component, const char *label, const ComponentInfo *info );

static void inspect_field( void *field, const ComponentField *field_def )
{
    if( field_def->flags & COMPONENT_FLAG_IS_VEC )
    {
        inspect_Vec_T( field_def->name, NULL );
        return;
    }

    switch( field_def->type )
    {
    case COMPONENT_FIELD_TYPE_INT:    inspect_int( field_def->name, field );    return;
    case COMPONENT_FIELD_TYPE_FLOAT:  inspect_float( field_def->name, field );  return;
    case COMPONENT_FIELD_TYPE_BOOL:   inspect_bool( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC2:   inspect_vec2( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC3:   inspect_vec3( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VEC4:   inspect_vec4( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_VERSOR: inspect_versor( field_def->name, field ); return;
    case COMPONENT_FIELD_TYPE_MAT4:   inspect_mat4( field_def->name, field );   return;
    case COMPONENT_FIELD_TYPE_ENTITY: inspect_Entity( field_def->name, field ); return;
    case COMPONENT_FIELD_TYPE_STRING: inspect_string( field_def->name, field ); return;
    case COMPONENT_FIELD_TYPE_SUBCOMPONENT:
        inspect_component( field, field_def->name, get_info_for_component_type( field_def->subcomponent_name ) );
        return;
    }

    PANIC("Unhandled field type in inspect_field");
}

static void inspect_component( void *component, const char *label, const ComponentInfo *info )
{
    if (label) 
    { 
        igSeparator(); 
        igText(label); 
        igSeparator(); 
    }

    for( int i = 0; i < info->num_fields; ++ i )
    {
        const ComponentField *field = &info->fields[i];

        if( (field->flags & COMPONENT_FLAG_HIDDEN) == 0 ) 
        {
            igPushIDInt(i);
            inspect_field( (uint8_t*)component + field->offset, field );
            igPopID();
        }
    }
}

static cJSON *serialize_field( void *field, const ComponentField *field_def )
{
    switch( field_def->type )
    {
    case COMPONENT_FIELD_TYPE_INT:    return cJSON_CreateNumber(*(int*)field);
    case COMPONENT_FIELD_TYPE_FLOAT:  return cJSON_CreateNumber(*(float*)field);
    case COMPONENT_FIELD_TYPE_BOOL:   return cJSON_CreateBool  (*(bool*)field); 
    case COMPONENT_FIELD_TYPE_VEC2:   return cJSON_CreateFloatArray((float*)(*(vec2*)field), 2);
    case COMPONENT_FIELD_TYPE_VEC3:   return cJSON_CreateFloatArray((float*)(*(vec3*)field), 3);
    case COMPONENT_FIELD_TYPE_VEC4:   return cJSON_CreateFloatArray((float*)(*(vec4*)field), 4);
    case COMPONENT_FIELD_TYPE_VERSOR: return cJSON_CreateFloatArray((float*)(*(versor*)field), 4);
    case COMPONENT_FIELD_TYPE_MAT4:   return cJSON_CreateFloatArray((float*)(*(mat4*)field), 16);
    case COMPONENT_FIELD_TYPE_ENTITY: return cJSON_CreateNumber((double)(*(Entity*)field));
    case COMPONENT_FIELD_TYPE_STRING: return cJSON_CreateString(*(char**)field);
    }
    // TODO
}

static cJSON *serialize_component( void *component, const ComponentInfo *info )
{

}

ECS *components_ecs_new( void )
{
    ECS *ecs = ecs_new();

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];
        ecs_register_component( ecs, info->name, info->size, info->destructor );
    }

    return ecs;
}

void components_generic_destruct( const ComponentInfo *info, void *component )
{
    for( int i = 0; i < info->num_fields; ++i )
    {
        const ComponentField *field = &info->fields[i];
        void *field_ptr = (uint8_t*)component + field->offset;

        if( field->flags & COMPONENT_FLAG_IS_VEC )
        {
            Vec *vec = (Vec*)field_ptr;

            if( field->type == COMPONENT_FIELD_TYPE_SUBCOMPONENT )
            {
                for( int i = 0; i < vec->item_count; ++i )
                    components_generic_destruct( get_info_for_component_type( field->subcomponent_name ), vec_at( vec, i ) );
            }
            else if( field->type == COMPONENT_FIELD_TYPE_STRING )
            {
                for( int i = 0; i < vec->item_count; ++i )
                    free( *(char**)vec_at( vec, i ) );
            }

            vec_clear( vec );
        }
        else if( field->type == COMPONENT_FIELD_TYPE_SUBCOMPONENT )
        {
            components_generic_destruct( get_info_for_component_type( field->subcomponent_name ), field_ptr );
        }
        else if( field->type == COMPONENT_FIELD_TYPE_STRING )
        {
            char **string = (char**)field_ptr;
            free( *string );
            *string = NULL;
        }
    }
}

void components_inspect_entity( Entity e )
{
    int visible_component_count = 0;
    const char *visible_components[COMPONENTS_TOTAL_COUNT];

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        if( COMPONENTS_ALL_INFOS[i]->flags & COMPONENT_FLAG_HIDDEN ) continue;

        visible_components[visible_component_count] = COMPONENTS_ALL_INFOS[i]->name;
        visible_component_count++;
    }

    static int selected_component = 0;

    igCombo( "", &selected_component, visible_components, visible_component_count, 0 );
    igSameLine( 0, -1 );

    if( igButton( "Add Component", (ImVec2){ 0, 0 } ) )
    {
        const char *comp = visible_components[selected_component];
        
        // TODO use defaults not zero
        if( !ecs_get_component( s_ecs, e, comp ) )
            ecs_add_component_zeroed( s_ecs, e, comp );
    }

    igSeparator();

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
    {
        igPushIDInt(0);

        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];

        bool keep_alive = true;
        void *component = ecs_get_component( s_ecs, e, info->name );

        if( component && igCollapsingHeaderBoolPtr( info->name, &keep_alive, 0 ) )
            inspect_component( component, NULL, info );

        if (! keep_alive)
            ecs_remove_component( s_ecs, e, info->name );

        igPopID();
    }
}

char *components_serialize_scene_alloc( void )
{
    return NULL;
}

ECS *components_deserialize_scene_alloc( const char *json_scene )
{
    return NULL;
}

const char *components_name_entity( Entity e, bool *name_from_transform )
{
    ECS_GET_COMPONENT_DECL( Transform, t, s_ecs, e );
    if( t && t->name && strlen( t->name ) ) 
    {
        *name_from_transform = true;
        return t->name;
    }

    *name_from_transform = false;

    for( int i = 0; i < COMPONENTS_TOTAL_COUNT; ++i )
        if( ecs_get_component( s_ecs, e, COMPONENTS_ALL_INFOS[i]->name ) )
            return COMPONENTS_ALL_INFOS[i]->name;

    return "empty";
}