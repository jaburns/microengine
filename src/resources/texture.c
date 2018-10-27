#define _CRT_SECURE_NO_WARNINGS 1

#include "texture.h"

#include <stdlib.h>
#include <string.h>

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

    if (error != 0) {
        printf("PNG Error: %s", lodepng_error_text(error));
        return 0;
    }

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
    GLuint ref;

    glGenTextures(1, &ref);
    glActiveTexture(GL_TEXTURE0);

    const char* sides[6] = { r, l, t, bo, ba, f };

    glBindTexture(GL_TEXTURE_CUBE_MAP, ref);
    for (int i = 0; i < 6; ++i) {
        unsigned char *image;
        unsigned int width, height;
        unsigned int error = lodepng_decode32_file(&image, &width, &height, sides[i]);
        if (error != 0) {
            printf("PNG Error: %s", lodepng_error_text(error));
            return 0;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        free(image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

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
