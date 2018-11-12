#pragma once

#include "../containers/ecs.h"

typedef struct CollisionSystem CollisionSystem;

extern CollisionSystem *collision_sys_new( void );
extern void collision_sys_run( CollisionSystem *sys, ECS *ecs );
extern void collision_sys_delete( CollisionSystem *sys );
