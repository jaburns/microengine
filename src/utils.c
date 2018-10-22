#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include "utils.h"

#include <stdlib.h>

char *read_file_alloc(const char *path)
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
    fread(buffer, 1, length, f);
    buffer = malloc(length + 1);
    buffer[length] = 0;
    fclose(f);

    return buffer;
}