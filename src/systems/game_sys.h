#pragma once

#include "../containers/ecs.h"

typedef struct GameSystem GameSystem;

extern GameSystem *game_sys_new( void );
extern void game_sys_run( GameSystem *sys, ECS *ecs );
extern void game_sys_delete( GameSystem *sys );
