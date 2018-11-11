#pragma once
#include "containers/ecs.h"

typedef enum ComponentFieldType
{
    COMPONENT_FIELD_TYPE_INT,
    COMPONENT_FIELD_TYPE_FLOAT,
    COMPONENT_FIELD_TYPE_BOOL,
    COMPONENT_FIELD_TYPE_VEC2,
    COMPONENT_FIELD_TYPE_VEC3,
    COMPONENT_FIELD_TYPE_VEC4,
    COMPONENT_FIELD_TYPE_VERSOR,
    COMPONENT_FIELD_TYPE_MAT4,
    COMPONENT_FIELD_TYPE_ENTITY,
    COMPONENT_FIELD_TYPE_STRING,
    COMPONENT_FIELD_TYPE_SUBCOMPONENT,
}
ComponentFieldType;

typedef enum ComponentFlags
{
    COMPONENT_FLAG_IS_VEC         = 0x01,
    COMPONENT_FLAG_HIDDEN         = 0x02,
    COMPONENT_FLAG_DONT_SERIALIZE = 0x04,
}
ComponentFlags;

typedef struct ComponentField 
{
    const char *name;
    ComponentFieldType type;
    ComponentFlags flags;
    const char *subcomponent_name;
}
ComponentField;

typedef struct ComponentInfo
{
    const char *name;
    ComponentDestructor destructor;
    ComponentFlags flags;
    size_t num_fields;
    ComponentField fields[];
}
ComponentInfo;

extern ECS *components_ecs_new( void );
extern void components_bind_ecs( ECS *ecs );

extern void components_inspect_entity( Entity e );
extern const char *components_name_entity( Entity e );
extern Entity *components_entity_to_change; // TODO use a function instead of a global var

extern char *components_serialize_scene_alloc( void );
extern ECS *components_deserialize_scene_alloc( const char *json_scene );

extern void components_generic_destruct( const ComponentInfo *info, void *component );
