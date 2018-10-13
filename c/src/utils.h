#pragma once

#define DECLARE_MAYBE(T) typedef struct Maybe_##T \
{                                                 \
    bool has_value;                               \
    T value;                                      \
}                                                 \
Maybe_##T

#define FIND_INDEX(out_int, arr, len, val) do \
{                                             \
    out_int = -1;                             \
                                              \
    for (int i = 0; i < len; ++i)             \
    {                                         \
        if (arr[i] == val)                    \
        {                                     \
            out_int = i;                      \
            break;                            \
        }                                     \
    }                                         \
}                                             \
while (0)