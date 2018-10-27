#pragma once

#include "../gl.h"

typedef struct Shader Shader;

extern Shader *shader_load( const char *path );
extern GLuint shader_get_handle( const Shader *shader );
extern void shader_delete( Shader *shader );