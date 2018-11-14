#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>

#include "../component_defs.h"

extern bool world_collision_info_raycast( const WorldCollisionInfo *info, const vec3 origin, const vec3 ray_vec, vec3 out_intersection );
