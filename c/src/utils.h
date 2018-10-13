#pragma once

#define DECLARE_MAYBE(T) typedef struct Maybe_##T { \
    bool has_value; \
    T value; \
} Maybe_##T