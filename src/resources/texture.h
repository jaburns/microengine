#pragma once

#include "../gl.h"

typedef struct Texture Texture;

extern Texture *texture_load(const char *png_path);
extern Texture *texture_load_cubemap(const char *r, const char *l, const char *t, const char *bo, const char *ba, const char *f);
extern GLuint texture_get_handle(const Texture *texture);
extern void texture_delete(Texture *texture);
