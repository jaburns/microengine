#pragma once

#include "../ecs.h"
#include "../shell.h"

typedef struct RenderSystem RenderSystem;

extern RenderSystem *render_sys_new(void);
extern void render_sys_run(RenderSystem *sys, ECS *ecs);
extern void render_sys_delete(RenderSystem *sys);