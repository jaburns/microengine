#pragma once

#include "ecs.h"
#include "shell.h"

typedef struct RenderSystem RenderSystem;

extern RenderSystem *rendersystem_new(void);
extern void rendersystem_run(RenderSystem *sys, ECS *ecs);
extern void rendersystem_delete(RenderSystem *sys);