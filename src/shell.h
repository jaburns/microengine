#pragma once

#include <stdbool.h>
#include <SDL.h>

#include "vec.h"

typedef struct ShellContext ShellContext;

typedef struct ShellInputs
{
    Vec keys_down; // Vec of SDL_Keycode
}
ShellInputs;

extern ShellContext *shell_new(const char *title, int width, int height);
extern bool shell_flip_frame_poll_events(ShellContext *context);
extern void shell_delete(ShellContext *context);

extern ShellInputs *read_input_state_alloc(void);
extern void free_input_state(ShellInputs *state);