#pragma once

#include "../gl.h"
#include "../containers/vec.h"

typedef struct Material Material;

typedef enum MaterialPropertyType
{
    MATERIAL_PROPERTY_SHADER,
    MATERIAL_PROPERTY_FLOAT,
    MATERIAL_PROPERTY_VEC2,
    MATERIAL_PROPERTY_VEC3,
    MATERIAL_PROPERTY_VEC4,
    MATERIAL_PROPERTY_TEXTURE2D,
}
MaterialPropertyType;

typedef struct MaterialProperty
{
    MaterialPropertyType type;
    char *name;
    void *value;
}
MaterialProperty;

typedef struct MaterialShaderProperties
{
    char *shader_name;
    Vec properties; // of MaterialProperty
}
MaterialShaderProperties;

struct Material
{
    MaterialShaderProperties base_properties;
    Vec submaterials; // of MaterialShaderProperties
};

extern Material *material_load( const char *path );
extern void material_delete( Material *material );
