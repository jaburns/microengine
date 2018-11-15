#include "collision_sys.h"

#include <stdlib.h>
#include <cglm/cglm.h>

#include "../resources/mesh.h"
#include "../component_defs.h"
#include "../geometry.h"
#include "../utils.h"

typedef struct CachedCollider
{
    Hash transform_hash;
    Hash collider_hash;
    Vec triangles; // of Triangle
}
CachedCollider;

struct CollisionSystem
{
    Vec triangles; // of Triangle; TODO of CachedCollider and compute/compare hashes
};

CollisionSystem *collision_sys_new( void )
{
    CollisionSystem *result = malloc( sizeof( CollisionSystem ) );
    result->triangles = vec_empty( sizeof( Triangle ) );
    return result;
}

void collision_sys_run( CollisionSystem *sys, ECS *ecs, HashCache *resources )
{
    size_t collider_count;
    Entity *collider_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( MeshCollider, ecs, &collider_count );
    
    // TODO don't repopulate this every frame, only when meshcolliders move or pop in/out of existence
    vec_clear( &sys->triangles );

    for( int i = 0; i < collider_count; ++i )
    {
        ECS_GET_COMPONENT_CONST_DECL( MeshCollider, collider, ecs, collider_entities[i] );
        Mesh *mesh = hashcache_load( resources, collider->mesh );
        if( !mesh ) continue;

        mat4 world_matrix;
        ECS_GET_COMPONENT_DECL( Transform, transform, ecs, collider_entities[i] );
        if( transform )
            glm_mat4_copy( transform->worldMatrix_, world_matrix );
        else
            glm_mat4_identity( world_matrix );
        
        for( int j = 0; j < mesh->num_submeshes; ++j )
        for( int k = 0; k < mesh->submeshes[j].num_indices; k += 3 )
        {
            Triangle t;
            glm_mat4_mulv3( world_matrix, mesh->vertices[mesh->submeshes[j].indices[k + 0]], 1.f, t.a );
            glm_mat4_mulv3( world_matrix, mesh->vertices[mesh->submeshes[j].indices[k + 1]], 1.f, t.b );
            glm_mat4_mulv3( world_matrix, mesh->vertices[mesh->submeshes[j].indices[k + 2]], 1.f, t.c );

            vec_push_copy( &sys->triangles, &t );
        }
    }

    free( collider_entities );

    ECS_ENSURE_SINGLETON_DECL( WorldCollisionInfo, ecs, info );
    info->info = &sys->triangles;
}

void collision_sys_delete( CollisionSystem *sys )
{
    if( !sys ) return;
    vec_clear( &sys->triangles );
    free( sys );
}
