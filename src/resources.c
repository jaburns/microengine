#define _CRT_SECURE_NO_WARNINGS 1

#include "resources.h"

#include <stdlib.h>
#include <stdio.h>
#include <lodepng.h>

static GLchar* read_file(const char *path)
{
    char *buffer = 0;
    long length;
    FILE *f = fopen(path, "rb");

    if (! f) {
        printf("Read file error: %s", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = 0;
    fclose(f);

    return buffer;
}

static GLuint shader_compile_from_file(const char *shader_path, GLenum shader_type)
{
    const GLchar *const shader_contents = read_file(shader_path);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &shader_contents, NULL);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
        char *log = (char*)malloc(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, log);

        printf("Error in shader: %s\n", shader_path);
        printf("%s", log);

        free(log);
        free((void*)shader_contents);

        exit(EXIT_FAILURE);
    }

    free((void*)shader_contents);

    return shader;
}

ShaderProgramRef resources_load_shader(const char *vert_path, const char *frag_path)
{
    ShaderProgramRef ref = glCreateProgram();
    GLuint vert = shader_compile_from_file(vert_path, GL_VERTEX_SHADER);
    GLuint frag = shader_compile_from_file(frag_path, GL_FRAGMENT_SHADER);
    glAttachShader(ref, vert);
    glAttachShader(ref, frag);
    glLinkProgram(ref);
    glDetachShader(ref, vert);
    glDetachShader(ref, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
    return ref;
}

void resources_delete_shader(ShaderProgramRef ref)
{
    if (ref > 0) {
        glDeleteProgram(ref);
    }
}


TextureRef resources_load_texture(const char *png_path)
{
    TextureRef ref;

    unsigned char *image;
    unsigned int width, height;
    unsigned int error = lodepng_decode32_file(&image, &width, &height, png_path);

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

    return ref;
}

void resources_delete_texture(TextureRef ref)
{
    if (ref > 0) {
        glDeleteTextures(1, &ref);
    }
}

CubeMapRef resources_load_cubemap(const char *r, const char *l, const char *t, const char *bo, const char *ba, const char *f)
{
    CubeMapRef ref;

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

    return ref;
}

void resources_delete_cubemap(CubeMapRef ref)
{
    if (ref > 0) {
        glDeleteTextures(1, &ref);
    }
}
