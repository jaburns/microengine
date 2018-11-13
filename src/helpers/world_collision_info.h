#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>

#include "../component_defs.h"

float world_collision_info_raycast( const WorldCollisionInfo *info, vec3 origin, vec3 ray_vec );
