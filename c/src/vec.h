#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct Vec
{
    void *data;
    size_t item_size;
    size_t item_count;
}
Vec;

extern Vec vec_new(size_t item_size);
extern void *vec_get(Vec *vec, size_t index);
extern void vec_set(Vec *vec, size_t index, void *item_ref);
extern void vec_push(Vec *vec, void *item_ref);
extern bool vec_pop(Vec *vec, void *result);