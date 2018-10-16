#pragma once

#include <lua.h>
#include "ecs.h"

extern void ecs_lua_bind_ecs(ECS *ecs);
extern void ecs_lua_bind_functions(lua_State *L);