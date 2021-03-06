#pragma once

#include <cglm/cglm.h>
#include <stdint.h>
#include "../gl.h"

typedef struct Submesh
{
    int num_indices;
    uint16_t *indices;
}
Submesh;

struct Mesh
{
    uint16_t num_vertices;
    vec3 *vertices;
    vec3 *normals;
    vec2 *uvs;

    uint16_t num_submeshes;
    Submesh *submeshes;
};

typedef struct Mesh Mesh;

extern Mesh *mesh_load( const char *path );
extern void mesh_delete( Mesh *mesh );
