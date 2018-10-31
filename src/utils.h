#pragma once

#include <stdio.h>

#define PANIC(...) do {  \
    printf(__VA_ARGS__); \
    exit(1);             \
} while (0)

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

extern char *utils_read_file_alloc(const char *path_prefix, const char *path, int *file_length);
extern void utils_write_string_file(const char *path, const char *contents);
