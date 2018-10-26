#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include "utils.h"

#include <stdlib.h>

char *utils_read_file_alloc(const char *path, size_t *file_length)
{
    size_t length;
    char *buffer = 0;
    FILE *f = fopen(path, "rb");

    if (! f) PANIC("Read file error: %s", path);

    fseek(f, 0, SEEK_END);
    length = (size_t)ftell(f);
    rewind(f);
    buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = 0;
    fclose(f);

    if (file_length) *file_length = length;

    return buffer;
}

void utils_write_string_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");

    if (! f) PANIC("Write file error: %s", path);

    fputs(contents, f);
    fclose(f);
}