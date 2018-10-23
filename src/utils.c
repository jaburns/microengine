#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include "utils.h"

#include <stdlib.h>

char *read_file_alloc(const char *path)
{
    char *buffer = 0;
    long length;
    FILE *f = fopen(path, "rb");

    if (! f) PANIC("Read file error: %s", path);

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = 0;
    fclose(f);

    return buffer;
}

void write_file(const char *path, const char *contents)
{
    FILE *f = fopen(path, "wb");

    if (! f) PANIC("Write file error: %s", path);

    fputs(contents, f);
    fclose(f);
}