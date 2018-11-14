#pragma once

#include <stdio.h>

#define PANIC(...) do {  \
    printf(__VA_ARGS__); \
    exit(1);             \
} while (0)


#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))


#ifdef _MSC_VER
    #define strtok_ctx strtok_s
#else
    #define strtok_ctx strtok_r
#endif


#define UTILS_STRTOK_FOR( str, split, ivar ) for( \
    char *tok_ctx_, *ivar = strtok_ctx( (str), (split), &tok_ctx_ ); \
    ivar != NULL; \
    ivar = strtok_ctx( NULL, (split), &tok_ctx_ ) \
)


#define UTILS_UNCONST_VEC( v ) ((float*)&(v)[0])


extern char *utils_read_file_alloc(const char *path_prefix, const char *path, size_t *file_length);
extern void utils_write_string_file(const char *path, const char *contents);
