#pragma once

#include "gl.h"

// TODO Typedef resources as single elem structs

typedef GLuint ShaderProgramRef;
typedef GLuint TextureRef;
typedef GLuint CubeMapRef;

extern ShaderProgramRef resources_load_shader(const char *vert_path, const char *frag_path);
extern void resources_delete_shader(ShaderProgramRef ref);

extern TextureRef resources_load_texture(const char *png_path);
extern void resources_delete_texture(TextureRef ref);

extern CubeMapRef resources_load_cubemap(const char *r, const char *l, const char *t, const char *bo, const char *ba, const char *f);
extern void resources_delete_cubemap(CubeMapRef ref);