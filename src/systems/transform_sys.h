#pragma once

#include "../containers/ecs.h"
#include "../shell.h"

typedef struct TransformSystem TransformSystem;

extern TransformSystem *transform_sys_new(void);
extern void transform_sys_run(TransformSystem *sys, ECS *ecs);
extern void transform_sys_delete(TransformSystem *sys);
