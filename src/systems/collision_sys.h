#pragma once

#include "../containers/ecs.h"
#include "../containers/hashcache.h"

typedef struct CollisionSystem CollisionSystem;

extern CollisionSystem *collision_sys_new( void );
extern void collision_sys_run( CollisionSystem *sys, ECS *ecs, HashCache *resources );
extern void collision_sys_delete( CollisionSystem *sys );

extern bool world_collision_info_raycast( const WorldCollisionInfo *info, const vec3 origin, const vec3 ray_vec, vec3 out_intersection );
