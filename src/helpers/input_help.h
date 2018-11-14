#pragma once

#include <stdbool.h>

#include "../component_defs.h"
#include "../gl.h"

extern bool input_state_is_key_down( const InputState *inputs, SDL_Keycode key );
