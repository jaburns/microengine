#pragma once

#include <stdbool.h>

#include "../containers/ecs.h"
#include "../shell.h"
#include "../component_defs.h"

typedef struct InputSystem InputSystem;

extern InputSystem *input_sys_new( ShellContext *shell );
extern void input_sys_run( InputSystem *sys, ECS *ecs, SDL_GameController *controller );
extern void input_sys_delete( InputSystem *sys );

extern bool input_state_is_key_down( const InputState *inputs, SDL_Keycode key );
