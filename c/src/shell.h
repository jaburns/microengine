#pragma once

#include <stdbool.h>
#include <SDL.h>

typedef struct InputState
{
    int num_keys_down;
    SDL_Keycode *keys_down;
}
InputState;

extern bool shell_open(const char *title, int width, int height);
extern bool shell_flip_frame_poll_events(void);
extern void shell_close(void);

extern InputState *read_input_state(void);
extern void free_input_state(InputState *state);
