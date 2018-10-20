#pragma once

#include <stdio.h>

#define PANIC(...) do {  \
    printf(__VA_ARGS__); \
    exit(1);             \
} while (0)

#define FIND_INDEX_DECL(out_int, arr, len, val) \
int out_int = -1;                               \
{                                               \
    for (int i = 0; i < (len); ++i)             \
    {                                           \
        if ((arr)[i] == (val))                  \
        {                                       \
            out_int = i;                        \
            break;                              \
        }                                       \
    }                                           \
}