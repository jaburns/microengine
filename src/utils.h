#pragma once

#define DECLARE_MAYBE(T) typedef struct Maybe_##T \
{                                                 \
    bool has_value;                               \
    T value;                                      \
}                                                 \
Maybe_##T

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