#pragma once

typedef struct RenderSystem RenderSystem;

extern RenderSystem *rendersystem_new(void);
extern void rendersystem_delete(RenderSystem *sys);