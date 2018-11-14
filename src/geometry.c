#include "geometry.h"

#include "utils.h"

bool geometry_line_seg_intersects_triangle( const vec3 l0, const vec3 l1, const vec3 t0, const vec3 t1, const vec3 t2, vec3 out_intersection )
{
    const float EPSILON = 0.0000001f;

    vec3 l1_sub_l0;
    glm_vec_sub( UTILS_UNCONST_VEC( l1 ), UTILS_UNCONST_VEC( l0 ), l1_sub_l0 );

    float ray_len = glm_vec_norm( l1_sub_l0 );
    vec3 ray_vector;
    glm_normalize_to( l1_sub_l0, ray_vector );

    vec3 edge0, edge1;
    glm_vec_sub( UTILS_UNCONST_VEC( t1 ), UTILS_UNCONST_VEC( t0 ), edge0 );
    glm_vec_sub( UTILS_UNCONST_VEC( t2 ), UTILS_UNCONST_VEC( t0 ), edge1 );

    vec3 h;
    glm_vec_cross( ray_vector, edge1, h );
    float a = glm_vec_dot( edge0, h );
    if( a > -EPSILON && a < EPSILON ) return false;

    float f = 1.f / a;
    vec3 s;
    glm_vec_sub( UTILS_UNCONST_VEC( l0 ), UTILS_UNCONST_VEC( t0 ), s );
    float u = f * glm_vec_dot( s, h );
    if( u < 0.f || u > 1.f ) return false;

    vec3 q;
    glm_vec_cross( s, edge0, q );
    float v = f * glm_vec_dot( ray_vector, q );
    if( v < 0.f || u + v > 1.f) return false;

    float t = f * glm_vec_dot( edge1, q );
    if( t < 0.f || t > ray_len ) return false;

    vec3 x;
    glm_vec_scale( ray_vector, t, x );
    glm_vec_add( UTILS_UNCONST_VEC( l0 ), x, out_intersection );
    return true;
}
