#include "world_collision_info.h"

#include "../geometry.h"
#include "../utils.h"

bool world_collision_info_raycast( const WorldCollisionInfo *info, const vec3 origin, const vec3 ray_vec, vec3 out_intersection )
{
    if( !info || !info->info ) return false;
    const Vec *triangles = info->info;

    vec3 end_pt;
    glm_vec_add( UTILS_UNCONST_VEC( origin ), UTILS_UNCONST_VEC( ray_vec ), end_pt );

    for( int i = 0; i < triangles->item_count; ++i )
    {
        const Triangle *t = vec_at_const( triangles, i );
        if( geometry_line_seg_intersects_triangle( origin, end_pt, t->a, t->b, t->c, out_intersection ) ) 
            return true;
    }

    return false;
}