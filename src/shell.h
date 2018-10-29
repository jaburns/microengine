#pragma once

#include <stdbool.h>

#include "containers/hashtable.h"

typedef struct ShellContext ShellContext;

typedef struct ShellInputs
{
    HashTable keys_down; // int keys of SDL_Keycode, if there's a value then key is down.
    bool left_mouse;
    bool right_mouse;
    float mouse_position[2];
}
ShellInputs;

extern ShellContext *shell_new( const char *title, int width, int height );
extern bool shell_flip_frame_poll_events( ShellContext *context );
extern float shell_get_aspect( ShellContext *context );
extern const ShellInputs *shell_view_input_state( ShellContext *context );
extern void shell_delete( ShellContext *context );