#include "mesh.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "../utils.h"

static void *allocate_and_load_data( uint8_t **file_ptr, uint16_t num_verts, size_t elem_size )
{
    size_t size = elem_size * num_verts;
    void *result = malloc( size );
    memcpy( result, *file_ptr, size );
    *file_ptr += size;
    return result;
}

Mesh *mesh_load( const char *path )
{
    const uint8_t *file = utils_read_file_alloc( "resources/", path, NULL );

    if( !file ) return NULL;

    const uint8_t *p = file;
    Mesh *mesh = malloc( sizeof( Mesh ) );

    mesh->num_vertices = *(uint16_t*)p; p += 2;
    uint16_t mesh_flags = *(uint16_t*)p; p += 2;

    mesh->vertices = allocate_and_load_data( &p, mesh->num_vertices, sizeof( vec3 ) );
    mesh->normals = allocate_and_load_data( &p, mesh->num_vertices, sizeof( vec3 ) );
    mesh->uvs = allocate_and_load_data( &p, mesh->num_vertices, sizeof( vec2 ) );

    mesh->num_submeshes = *(uint16_t*)p; p += 2;
    mesh->submeshes = malloc( sizeof( Submesh ) * mesh->num_submeshes );
    for( int i = 0; i < mesh->num_submeshes; ++i )
    {
        mesh->submeshes[i].num_indices = *(uint16_t*)p; p += 2;
        mesh->submeshes[i].indices = allocate_and_load_data( &p, mesh->submeshes[i].num_indices, sizeof( uint16_t ) );
    }

    free( file );
    return mesh;
}

void mesh_delete( Mesh *mesh )
{
    if( !mesh ) return;

    free( mesh->vertices );
    free( mesh->normals );
    free( mesh->uvs );

    for( int i = 0; i < mesh->num_submeshes; ++i )
        free( mesh->submeshes[i].indices );
    free( mesh->submeshes );

    free( mesh );
}
