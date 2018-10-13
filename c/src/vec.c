#include "vec.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

Vec vec_empty(size_t item_size)
{
    return (Vec){ item_size, 0, NULL };
}

void *vec_at(Vec *vec, size_t index)
{
    return (uint8_t*)vec->data + (vec->item_size * index);
}

void vec_set_copy(Vec *vec, size_t index, const void *item_ref)
{
    memcpy((uint8_t*)vec->data + vec->item_size * index, item_ref, vec->item_size);
}

void vec_push_copy(Vec *vec, const void *item_ref)
{
    vec->item_count++;
    vec->data = realloc(vec->data, vec->item_size * vec->item_count);
    vec_set_copy(vec, vec->item_count - 1, item_ref);
}

bool vec_pop(Vec *vec, void *result)
{
    if (vec->item_count == 0) return false;

    vec->item_count--;
    memcpy(result, vec_at(vec, vec->item_count), vec->item_size);
    vec->data = realloc(vec->data, vec->item_size * vec->item_count);
    return true;
}

void vec_resize(Vec *vec, size_t new_item_count)
{
    size_t additional_item_count = new_item_count > vec->item_count
        ? new_item_count - vec->item_count
        : 0;

    size_t old_item_count = vec->item_count;
    vec->item_count = new_item_count;
    vec->data = realloc(vec->data, vec->item_size * vec->item_count);

    if (additional_item_count > 0)
        memset((uint8_t*)vec->data + old_item_count * vec->item_size, 0, additional_item_count);
}

void vec_clear(Vec *vec)
{
    free(vec->data);
    vec->item_count = 0;
}

void vec_clear_with_callback(Vec *vec, VecCallback cb)
{
    for (size_t i = 0; i < vec->item_count; ++i)
        cb(vec_at(vec, i));

    vec_clear(vec);
}

#ifdef RUN_TESTS 
TestResult vec_test(void)
{
    TEST_BEGIN("Vec push and pop work correctly");
        Vec v = vec_empty(sizeof(float));

        const float a = 4.0f;
        const float b = 8.0f;

        vec_push_copy(&v, &a);
        vec_push_copy(&v, &a);
        vec_push_copy(&v, &b);

        float popped;
        bool didPop;

        didPop = vec_pop(&v, &popped);
        TEST_ASSERT(didPop && popped == 8.0f);
        didPop = vec_pop(&v, &popped);
        TEST_ASSERT(didPop && popped == 4.0f);
        didPop = vec_pop(&v, &popped);
        TEST_ASSERT(didPop && popped == 4.0f);
        didPop = vec_pop(&v, &popped);
        TEST_ASSERT(!didPop);
    TEST_END();

    // TODO test vec_resize, vec_at, clear with callback

    return 0;
}
#endif