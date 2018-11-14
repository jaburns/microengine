#include "input_help.h"

#include "../containers/vec.h"

static bool find_sdl_key_index( const SDL_Keycode *a, const SDL_Keycode *b )
{
    return *a == *b;
}

bool input_state_is_key_down( const InputState *inputs, SDL_Keycode key )
{
    return vec_find_index( &inputs->cur.keys, &key, find_sdl_key_index ) >= 0;
}
