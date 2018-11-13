#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>

typedef struct Triangle
{
    vec3 a, b, c;
}
Triangle;

extern float geometry_line_seg_intersects_triangle( const vec3 line0, const vec3 line1, const vec3 tri0, const vec3 tri1, const vec3 tri2 );
