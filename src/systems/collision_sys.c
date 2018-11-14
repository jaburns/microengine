#include "collision_sys.h"

#include <stdlib.h>
#include <cglm/cglm.h>

#include "../resources/mesh.h"
#include "../component_defs.h"
#include "../geometry.h"

struct CollisionSystem
{
    Vec triangles; // of Triangle
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
    // TODO multiply the triangles by the world matrix of the collider if it has one
    vec_clear( &sys->triangles );

    for( int i = 0; i < collider_count; ++i )
    {
        ECS_GET_COMPONENT_DECL( MeshCollider, collider, ecs, collider_entities[i] );
        Mesh *mesh = hashcache_load( resources, collider->mesh );
        if( !mesh ) continue;
        
        for( int j = 0; j < mesh->num_submeshes; ++j )
        for( int k = 0; k < mesh->submeshes[j].num_indices; k += 3 )
        {
            Triangle t;
            glm_vec_copy( mesh->vertices[mesh->submeshes[j].indices[k + 0]], t.a );
            glm_vec_copy( mesh->vertices[mesh->submeshes[j].indices[k + 1]], t.b );
            glm_vec_copy( mesh->vertices[mesh->submeshes[j].indices[k + 2]], t.c );
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
