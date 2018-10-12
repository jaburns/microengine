#include "vec.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

Vec vec_make_empty(size_t item_size)
{
    return (Vec){ NULL, item_size, 0 };
}

void *vec_get(Vec *vec, size_t index)
{
    return (uint8_t*)vec->data + (vec->item_size * index);
}

void vec_set(Vec *vec, size_t index, const void *item_ref)
{
    memcpy((uint8_t*)vec->data + vec->item_size * index, item_ref, vec->item_size);
}

void vec_push(Vec *vec, const void *item_ref)
{
    vec->item_count++;
    vec->data = realloc(vec->data, vec->item_size * vec->item_count);
    vec_set(vec, vec->item_count - 1, item_ref);
}

bool vec_pop(Vec *vec, void *result)
{
    if (vec->item_count == 0) return false;

    vec->item_count--;
    memcpy(result, vec_get(vec, vec->item_count), vec->item_size);
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

#ifdef RUN_TESTS 
#include "testing.h"

TEST_RESULT vec_test()
{
    TEST_BEGIN("Vec should push and pop correctly");
        Vec v = vec_make_empty(sizeof(float));

        const float a = 4.0f;
        const float b = 8.0f;

        vec_push(&v, &a);
        vec_push(&v, &a);
        vec_push(&v, &b);

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

    return 0;
}

#endif