#pragma once

#include <stdbool.h>

#include "containers/hashtable.h"
#include "gl.h"

typedef struct ShellContext ShellContext;
typedef void (*ShellEventHandler)( float, float, SDL_Event* );

extern ShellContext *shell_new( const char *title, int width, int height );
extern void shell_bind_event_handler( ShellContext *context, ShellEventHandler handler );
extern bool shell_flip_frame_poll_events( ShellContext *context );
extern float shell_get_aspect( ShellContext *context );
extern SDL_GameController *shell_get_controller( ShellContext *context );
extern void shell_delete( ShellContext *context );