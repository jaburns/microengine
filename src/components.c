#include "components.h"

#include <cglm/cglm.h>
#include <cJSON.h>
#include <string.h>

#include "utils.h"
#include "component_defs.h"


static ECS *s_ecs;
Entity *components_entity_to_change;

void components_bind_ecs( ECS *ecs )
{
    s_ecs = ecs;
}


static size_t calc_component_size( const ComponentInfo *info );

static size_t get_field_size( const ComponentField *info )
{
    if( info->flags & COMPONENT_FLAG_IS_VEC ) return sizeof( Vec );

    switch( info->type )
    {
    case COMPONENT_FIELD_TYPE_INT:    return sizeof( int );
    case COMPONENT_FIELD_TYPE_FLOAT:  return sizeof( float );
    case COMPONENT_FIELD_TYPE_BOOL:   return sizeof( bool );
    case COMPONENT_FIELD_TYPE_VEC2:   return sizeof( vec2 );
    case COMPONENT_FIELD_TYPE_VEC3:   return sizeof( vec3 );
    case COMPONENT_FIELD_TYPE_VEC4:   return sizeof( vec4 );
    case COMPONENT_FIELD_TYPE_VERSOR: return sizeof( versor );
    case COMPONENT_FIELD_TYPE_MAT4:   return sizeof( mat4 );
    case COMPONENT_FIELD_TYPE_ENTITY: return sizeof( Entity );
    case COMPONENT_FIELD_TYPE_STRING: return sizeof( char* );
    case COMPONENT_FIELD_TYPE_SUBCOMPONENT:
        for( int i = 0; i < COMPONENTS_ALL_INFOS_COUNT; ++i )
        {
            const ComponentInfo *component = COMPONENTS_ALL_INFOS[i];
            if( strcmp( component->name, info->subcomponent_name ) == 0 )
                return calc_component_size( component );
        }
    }

    PANIC("get_field_size called with unknown field type");
}

static size_t calc_component_size( const ComponentInfo *info )
{
    size_t result = 0;

    for( int i = 0; i < info->num_fields; ++i )
        result += get_field_size( &info->fields[i] );

    return result;
}


static cJSON *serialize_base_field( void *field, const ComponentField *info )
{
    switch( info->type )
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

    for( int i = 0; i < COMPONENTS_ALL_INFOS_COUNT; ++i )
    {
        const ComponentInfo *info = COMPONENTS_ALL_INFOS[i];
        ecs_register_component( ecs, info->name, calc_component_size( info ), info->destructor );
    }

    return ecs;
}

void components_generic_destruct( const ComponentInfo *info, void *component )
{
}

void components_inspect_entity( Entity e )
{
}

char *components_serialize_scene_alloc( void )
{
    return NULL;
}

ECS *components_deserialize_scene_alloc( const char *json_scene )
{
    return NULL;
}

const char *components_name_entity( Entity e )
{
    return NULL;
}