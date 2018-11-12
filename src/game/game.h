#pragma once

#include "../containers/ecs.h"

typedef struct Game Game;

extern Game *game_new( ECS *ecs );
extern void game_update( Game *game, ECS *ecs );
extern void game_delete( Game *game );