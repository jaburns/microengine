#include "collision_sys.h"

#include <stdlib.h>
#include <cglm/cglm.h>

#include "../resources/mesh.h"
#include "../component_defs.h"
#include "../geometry.h"
#include "../utils.h"

typedef struct CachedCollider
{
    Entity entity;
    Hash transform_hash;
    Hash collider_hash;
    Vec triangles; // of Triangle
}
CachedCollider;

struct CollisionSystem
{
    Vec cached_colliders; // of CachedCollider
};

CollisionSystem *collision_sys_new( void )
{
    CollisionSystem *result = malloc( sizeof( CollisionSystem ) );
    result->cached_colliders = vec_empty( sizeof( CachedCollider ) );
    return result;
}

static CachedCollider *find_or_add_cached( Vec *cached_colliders, Entity entity, const Transform *transform, const MeshCollider *collider, bool *stale )
{
    CachedCollider *cached = NULL;
    int i;
    for( i = 0; i < cached_colliders->item_count; ++i )
    {
        cached = vec_at( cached_colliders, i );
        if( cached->entity == entity ) break;
    }
    if( i == cached_colliders->item_count ) cached = NULL;

    Hash transform_hash = utils_hash( transform, sizeof( Transform ) );
    Hash collider_hash = utils_hash( collider, sizeof( MeshCollider ) );

    if( !cached )
    {
        CachedCollider new;
        new.transform_hash = transform_hash;
        new.collider_hash = collider_hash;
        new.entity = entity;
        new.triangles = vec_empty( sizeof( Triangle ) );
        *stale = true;
        return vec_push_copy( cached_colliders, &new );
    }

    *stale = cached->collider_hash != collider_hash || cached->transform_hash != transform_hash;
    cached->collider_hash = collider_hash;
    cached->transform_hash = transform_hash;

    if( *stale ) vec_clear( &cached->triangles );

    return cached;
}

void collision_sys_run( CollisionSystem *sys, ECS *ecs, HashCache *resources )
{
    size_t collider_count;
    Entity *collider_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( MeshCollider, ecs, &collider_count );
    
    for( int i = 0; i < collider_count; ++i )
    {
        ECS_GET_COMPONENT_CONST_DECL( MeshCollider, collider, ecs, collider_entities[i] );
        ECS_GET_COMPONENT_DECL( Transform, transform, ecs, collider_entities[i] );

        bool cache_stale;
        CachedCollider *cached = find_or_add_cached( &sys->cached_colliders, collider_entities[i], transform, collider, &cache_stale );

        if( !cache_stale ) continue;

        printf( "Recalculating collision info for: %s\n", transform->name );

        Mesh *mesh = hashcache_load( resources, collider->mesh );
        if( !mesh ) {
            printf( "No mesh to add\n" );
            continue;
        }

        mat4 world_matrix;
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
        
            vec_push_copy( &cached->triangles, &t );
        }
    }

    free( collider_entities );

    ECS_ENSURE_SINGLETON_DECL( WorldCollisionInfo, ecs, info );
    info->info = &sys->cached_colliders;
}

static void clear_cached_collider( void *x, CachedCollider *c )
{
    vec_clear( &c->triangles );
}

void collision_sys_delete( CollisionSystem *sys )
{
    if( !sys ) return;
    vec_clear_with_callback( &sys->cached_colliders, NULL, clear_cached_collider );
    free( sys );
}

bool world_collision_info_raycast( const WorldCollisionInfo *info, const vec3 origin, const vec3 ray_vec, vec3 out_intersection )
{
    if( !info || !info->info ) return false;
    const Vec *cached_colliders = info->info;

    vec3 end_pt;
    glm_vec_add( UTILS_UNCONST_VEC( origin ), UTILS_UNCONST_VEC( ray_vec ), end_pt );

    for( int i = 0; i < cached_colliders->item_count; ++i )
    {
        const CachedCollider *collider = vec_at_const( cached_colliders, i );

        for( int j = 0; j < collider->triangles.item_count; ++j )
        {
            const Triangle *t = vec_at_const( &collider->triangles, j );

            if( geometry_line_seg_intersects_triangle( origin, end_pt, t->a, t->b, t->c, out_intersection ) ) 
                return true;
        }
    }

    return false;
}
