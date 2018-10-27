#include "material.h"

#include "../utils.h"

#include <string.h>
#include <cJSON.h>

#ifdef _MSC_VER
  #define strdup _strdup
#endif

Material *material_load( const char *path )
{
    char *file = utils_read_file_alloc( "resources/", path, NULL );
    cJSON *json = cJSON_Parse( file );
    free( file );

    Material *mat = malloc( sizeof( Material ) );
    
    mat->shader = strdup( cJSON_GetStringValue( cJSON_GetObjectItem( json, "shader" ) ) );

    cJSON *submats = cJSON_GetObjectItem( json, "submaterials" );
    mat->submaterials_count = cJSON_GetArraySize( submats ); 
    mat->submaterials = malloc( sizeof( SubMaterial ) * mat->submaterials_count );

    for( int i = 0; i < mat->submaterials_count; ++i )
    {
        cJSON *submat_item = cJSON_GetArrayItem(submats, i);
        mat->submaterials[i].tex = strdup( cJSON_GetStringValue( cJSON_GetObjectItem( submat_item, "tex" ) ) );
    }

    cJSON_Delete( json );
    return mat;
}

void material_delete( Material *mat )
{
    if( !mat ) return;

    free( mat->shader );

    for( int i = 0; i < mat->submaterials_count; ++i )
        free( mat->submaterials[i].tex );

    free( mat->submaterials );
    free( mat );
}
