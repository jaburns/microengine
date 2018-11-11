#pragma once

#include "../containers/ecs.h"
#include "../containers/hashcache.h"
#include "../shell.h"

typedef struct RenderSystem RenderSystem;

extern RenderSystem *render_sys_new( HashCache *resources );
extern void render_sys_run( RenderSystem *sys, ECS *ecs, HashCache *resources, float aspect_ratio, bool play_mode );
extern void render_sys_delete( RenderSystem *sys );