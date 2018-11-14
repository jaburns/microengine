#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>

typedef struct Triangle
{
    vec3 a, b, c;
}
Triangle;

extern bool geometry_line_seg_intersects_triangle( const vec3 l0, const vec3 l1, const vec3 t0, const vec3 t1, const vec3 t2, vec3 out_intersection );
