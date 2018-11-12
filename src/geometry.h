#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>

extern bool geometry_triangle_intersects_line_seg( vec3 t0, vec3 t1, vec3 t2, vec3 l0, vec3 l1 );
