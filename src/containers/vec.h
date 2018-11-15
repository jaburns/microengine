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

// First arg is arbitrary context, second arg is the element operated on
typedef void (*VecCallback)(void*, void*); 
typedef bool (*VecItemChecker)(void*, const void*);

extern Vec vec_empty(size_t item_size);

extern void *vec_at(Vec *vec, size_t index);
extern const void *vec_at_const(const Vec *vec, size_t index);
extern void vec_set_copy(Vec *vec, size_t index, const void *item_ref);

extern void vec_insert_copy(Vec *vec, size_t index, const void *item_ref);
extern void vec_remove(Vec *vec, size_t index);

extern void *vec_push_copy(Vec *vec, const void *item_ref);
extern bool vec_pop(Vec *vec, void *result);

extern int vec_find_index(const Vec *vec, void *context, VecItemChecker check);
extern Vec vec_clone(const Vec *vec);
extern void vec_resize(Vec *vec, size_t new_item_count);
extern void vec_clear(Vec *vec);
extern void vec_clear_with_callback(Vec *vec, void *context, VecCallback cb);

#define VEC_FOREACH(T, vec_ptr) for(                                                                \
    struct ForIter_ { size_t index; T *item; } iter = { 0, 0 };                                     \
    (size_t)(iter.item = (T*)vec_at((vec_ptr), iter.index)) && iter.index < (vec_ptr)->item_count;  \
    ++iter.index                                                                                    \
)

#ifdef RUN_TESTS
#include "../testing.h"
extern TestResult vec_test(void);
#endif
