#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Vec
{
    size_t item_size;
    size_t item_count;
    void *data;
}
Vec;

typedef void (*VecCallback)(void*);

extern Vec vec_empty(size_t item_size);

extern void *vec_at(Vec *vec, size_t index);
extern void vec_set_copy(Vec *vec, size_t index, const void *item_ref);

extern void vec_insert_copy(Vec *vec, size_t index, const void *item_ref);
extern void vec_remove(Vec *vec, size_t index);

extern void vec_push_copy(Vec *vec, const void *item_ref);
extern bool vec_pop(Vec *vec, void *result);

extern Vec vec_clone(Vec *vec);
extern void vec_resize(Vec *vec, size_t new_item_count);
extern void vec_clear(Vec *vec);
extern void vec_clear_with_callback(Vec *vec, VecCallback cb);

#ifdef RUN_TESTS
#include "testing.h"
extern TestResult vec_test(void);
#endif
