#pragma once

#include "../gl.h"

typedef struct Material Material;

typedef struct SubMaterial
{
    char *tex;
}
SubMaterial;

struct Material
{
    char *shader;

    int submaterials_count;
    SubMaterial *submaterials;
};

extern Material *material_load( const char *path );
extern void material_delete( Material *material );
