#define _CRT_SECURE_NO_WARNINGS 1

#include "texture.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <lodepng.h>

#include "../utils.h"


struct Texture
{
    GLuint handle;
};

GLuint texture_get_handle(const Texture *texture)
{
    return texture->handle;
}

Texture *texture_load(const char *png_path)
{
    char path[1024] = "resources/";
    strcat( path, png_path );

    GLuint ref;
    unsigned char *image;
    unsigned int width, height;
    unsigned int error = lodepng_decode32_file(&image, &width, &height, path);

    if (error != 0) return NULL;

    glGenTextures(1, &ref);
    glBindTexture(GL_TEXTURE_2D, ref);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    free(image);

    Texture *result = malloc(sizeof(Texture));
    result->handle = ref;
    return result;
}

Texture *texture_load_cubemap(const char *r, const char *l, const char *t, const char *bo, const char *ba, const char *f)
{
    const char* sides[6] = { r, l, t, bo, ba, f };
    uint8_t* images[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
    int widths[6], heights[6];

    for (int i = 0; i < 6; ++i) 
    {
        uint32_t error = lodepng_decode32_file(&images[i], &widths[i], &heights[i], sides[i]);

        if (error != 0) 
        {
            for (int j = 0; j <= i; ++j)
                free(images[j]);

            return NULL;
        }
    }

    GLuint ref;
    glGenTextures(1, &ref);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ref);

    for (int i = 0; i < 6; ++i) 
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, widths[i], heights[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, images[i]);
        free(images[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Texture *result = malloc(sizeof(Texture));
    result->handle = ref;
    return result;
}

void texture_delete(Texture *texture)
{
    if (!texture || texture->handle == 0) return;

    glDeleteTextures(1, &texture->handle);
    free(texture);
}
