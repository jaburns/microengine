#include "geometry.h"

#define UNCONST_PTR( T, var ) ((T*)&(var))

float geometry_line_seg_intersects_triangle( const vec3 line0, const vec3 line1, const vec3 tri0, const vec3 tri1, const vec3 tri2 )
{
    const float EPSILON = 0.0000001f;

    vec3 *l0 = (vec3*)&line0;
    vec3 *l1 = (vec3*)&line1;
    vec3 *t0 = (vec3*)&tri0;
    vec3 *t1 = (vec3*)&tri1;
    vec3 *t2 = (vec3*)&tri2;

    //var rayOrigin = segment.a;
    //var rayLen = (segment.b - segment.a).magnitude;
    //var rayVector = (segment.b - segment.a) / rayLen;

    vec3 l1_sub_l0;
    glm_vec_sub( *l1, *l0, l1_sub_l0 );

    float ray_len = glm_vec_norm( l1_sub_l0 );
    vec3 ray_vector;
    glm_normalize_to( l1_sub_l0, ray_vector );

    //var edge0 = triangle.b - triangle.a;
    //var edge1 = triangle.c - triangle.a;

    vec3 edge0, edge1;
    glm_vec_sub( *t1, *t0, edge0 );
    glm_vec_sub( *t2, *t0, edge1 );

    //var h = Vector3.Cross(rayVector, edge1);
    //var a = Vector3.Dot(edge0, h);
    //if (a > -EPSILON && a < EPSILON) return false;

    vec3 h;
    glm_vec_cross( ray_vector, edge1, h );
    float a = glm_vec_dot( edge0, h );
    if( a > -EPSILON && a < EPSILON ) return false;

    //var f = 1f / a;
    //var s = rayOrigin - triangle.a;
    //var u = f * (Vector3.Dot(s, h));
    //if (u < 0.0f || u > 1.0f) return false;

    float f = 1.f / a;
    vec3 s;
    glm_vec_sub( *l0, *t0, s );
    float u = f * glm_vec_dot( s, h );
    if( u < 0.f || u > 1.f ) return false;

    //var q = Vector3.Cross(s, edge0);
    //var v = f * Vector3.Dot(rayVector, q);
    //if (v < 0.0f || u + v > 1.0f) return false;

    vec3 q;
    glm_vec_cross( s, edge0, q );
    float v = f * glm_vec_dot( ray_vector, q );
    if( v < 0.f || u + v > 1.f) return false;

    //var t = f * Vector3.Dot(edge1, q);
    //return t >= 0f && t <= rayLen;

    float t = f * glm_vec_dot( edge1, q );
    return t >= 0.f && t <= ray_len ? t : -1.f;
}
