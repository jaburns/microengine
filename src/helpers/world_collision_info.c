#include "world_collision_info.h"

#include "../geometry.h"

float world_collision_info_raycast( const WorldCollisionInfo *info, vec3 origin, vec3 ray_vec )
{
    if( !info || !info->info ) return false;
    const Vec *triangles = info->info;

    vec3 end_pt;
    glm_vec_add( origin, ray_vec, end_pt );

    for( int i = 0; i < triangles->item_count; ++i )
    {
        const Triangle *t = vec_at_const( triangles, i );
        float intersection = geometry_line_seg_intersects_triangle( origin, end_pt, t->a, t->b, t->c );
        if( intersection > 0.f ) return intersection;
    }

    return -1.f;
}