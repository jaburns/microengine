#pragma once

#include "../gl.h"

typedef struct Material Material;

extern Material *material_load( const char *path );
extern void material_delete( Material *shader );
