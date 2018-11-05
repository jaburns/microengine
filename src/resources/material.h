#pragma once

#include "../gl.h"
#include "../containers/vec.h"

typedef struct Material Material;

typedef enum MaterialPropertyType
{
    MATERIAL_PROPERTY_SHADER,
    MATERIAL_PROPERTY_FLOAT,
    MATERIAL_PROPERTY_VEC2,
    MATERIAL_PROPERTY_VEC3,
    MATERIAL_PROPERTY_VEC4,
    MATERIAL_PROPERTY_TEXTURE2D,
}
MaterialPropertyType;

typedef struct MaterialProperty
{
    MaterialPropertyType type;
    char *name;
    void *value;
}
MaterialProperty;

typedef struct MaterialShaderProperties
{
    char *shader_name;
    Vec properties; // of MaterialProperty
}
MaterialShaderProperties;

struct Material
{
    MaterialShaderProperties base_properties;
    Vec submaterials; // of MaterialShaderProperties
};

extern Material *material_load( const char *path );
extern void material_delete( Material *material );


/*

void deserialize_Transform( Entity entity, ECS *ecs, HashTable *entities_for_ids, cJSON *component_obj )
{
    ECS_ADD_COMPONENT_DEFAULT_DECL( Transform, comp, ecs, entity );
//  comp->name = deserialize_field_string( component_obj, "name" );
//  comp->position = deserialize_field_vec3( component_obj, "position" );

//strdup( cJSON_GetStringValue( cJSON_GetObjectItem( component_obj, "name" ) ) );
//cJSON_GetStringValue( cJSON_GetObjectItem( component_obj, "name" ) ) );
        // for each field
}

ECS *components_deserialize_scene_alloc( const char *json_scene )
{
    ECS *result = ecs_new();
    cJSON *json = cJSON_Parse( json_scene );
    HashTable entities_for_ids = hashtable_empty( 1024, sizeof( Entity ) );
    char id_as_str_buf[128];
    cJSON *entity_obj;

    cJSON_ArrayForEach( entity_obj, json )
    {
        sprintf( id_as_str_buf, "%f", cJSON_GetObjectItem( entity_obj, "_id" )->valuedouble );
        Entity new_entity = ecs_create_entity( result );
        hashtable_set_copy( &entities_for_ids, id_as_str_buf, &new_entity );
    }

    cJSON_ArrayForEach( entity_obj, json )
    {
        sprintf( id_as_str_buf, "%f", cJSON_GetObjectItem( entity_obj, "_id" )->valuedouble );
        Entity e = *(Entity*)hashtable_at( &entities_for_ids, id_as_str_buf );

        cJSON *component_obj;
        cJSON_ArrayForEach( component_obj, entity_obj )
        {
            char *type_name = component_obj->string;
            if( strcmp( "_id", type_name ) == 0 ) continue;

            if( strcmp( "Transform", type_name ) == 0 ) 
                deserialize_Transform( e, result, &entities_for_ids, component_obj );
            // Generate for each type.

            printf( "    %s\n", type_name );
        }
    }

    hashtable_clear( &entities_for_ids );
    cJSON_Delete( json );
    return result;
}
*/
