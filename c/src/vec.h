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

extern Vec vec_make_empty(size_t item_size);
extern void *vec_get(Vec *vec, size_t index);
extern void vec_set(Vec *vec, size_t index, const void *item_ref);
extern void vec_push(Vec *vec, const void *item_ref);
extern bool vec_pop(Vec *vec, void *result);
extern void vec_resize(Vec *vec, size_t new_item_count);
extern void vec_clear(Vec *vec);

#ifdef RUN_TESTS
#include "testing.h"
extern TEST_RESULT vec_test();
#endif