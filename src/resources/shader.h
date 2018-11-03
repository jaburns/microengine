#pragma once

#include "../gl.h"

enum
{
    SHADER_RENDER_QUEUE_GEOMETRY    = 2000,
    SHADER_RENDER_QUEUE_TRANSPARENT = 3000,
};

typedef struct Shader Shader;

extern Shader *shader_load( const char *path );
extern GLuint shader_get_handle( const Shader *shader );
extern int shader_get_render_queue( const Shader *shader );
extern void shader_use( const Shader *shader );
extern void shader_delete( Shader *shader );