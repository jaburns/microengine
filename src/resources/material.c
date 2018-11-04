#include "material.h"

#include "../utils.h"

#include <string.h>
#include <cJSON.h>

#ifdef _MSC_VER
  #define strdup _strdup
#endif

static MaterialProperty parse_material_property( char *key, cJSON *value )
{
    MaterialProperty result;
    result.name = strdup( key );

    const char *type_name = cJSON_GetStringValue( cJSON_GetObjectItem( value, "type" ) );

    // TODO add non-texture properties
    if( strcmp( "texture2d", type_name ) == 0 )
    {
        result.type = MATERIAL_PROPERTY_TEXTURE2D;
        result.value = strdup( cJSON_GetStringValue( cJSON_GetObjectItem( value, "value" ) ) );
    }

    return result;
}

static MaterialShaderProperties parse_material_shader_properties( cJSON *properties )
{
    MaterialShaderProperties result;
    result.properties = vec_empty( sizeof( MaterialProperty ) ); 
    result.shader_name = NULL;

    cJSON *current_element = NULL;
    cJSON_ArrayForEach( current_element, properties )
    {
        char *current_key = current_element->string;

        if( strcmp( "shader", current_key ) == 0 )
        {
            result.shader_name = strdup( cJSON_GetStringValue( current_element ) );
            continue;
        }
        else if( strcmp( "submaterials", current_key ) == 0 )
        {
            continue;
        }

        MaterialProperty prop = parse_material_property( current_key, current_element );
        vec_push_copy( &result.properties, &prop );
    }

    return result;
}

Material *material_load( const char *path )
{
    char *file = utils_read_file_alloc( "resources/", path, NULL );

    if( !file ) return NULL;

    cJSON *json = cJSON_Parse( file );
    free( file );

    Material *mat = malloc( sizeof( Material ) );
    mat->base_properties = parse_material_shader_properties( json );
    mat->submaterials = vec_empty( sizeof( MaterialShaderProperties ) );

    cJSON *submaterials = cJSON_GetObjectItem( json, "submaterials" );
    if( submaterials )
    {
        cJSON *current_element = NULL;
        cJSON_ArrayForEach( current_element, submaterials )
        {
            MaterialShaderProperties submat_props = parse_material_shader_properties( current_element );
            vec_push_copy( &mat->submaterials, &submat_props );
        }
    }

    cJSON_Delete( json );
    return mat;
}

static void free_material_prop_callback( void *ctx, MaterialProperty *prop )
{
    free( prop->name );
    free( prop->value );
}

static void free_material_props( MaterialShaderProperties *props )
{
    free( props->shader_name );
    vec_clear_with_callback( &props->properties, NULL, free_material_prop_callback );
}

static void free_material_submats_callback( void *ctx, MaterialShaderProperties *props )
{
    free_material_props( props );
}

void material_delete( Material *mat )
{
    if( !mat ) return;

    free_material_props( &mat->base_properties );
    vec_clear_with_callback( &mat->submaterials, NULL, free_material_submats_callback );
    free( mat );
}
