#include "vec.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "minunit.h"

Vec vec_new(size_t item_size)
{
    return (Vec){ NULL, item_size, 0 };
}

void *vec_get(Vec *vec, size_t index)
{
    return (uint8_t*)vec->data + (vec->item_size * index);
}

void vec_set(Vec *vec, size_t index, void *item_ref)
{
    memcpy((uint8_t*)vec->data + vec->item_size * index, item_ref, vec->item_size);
}

void vec_push(Vec *vec, void *item_ref)
{
    vec->item_count++;

    if (vec->item_count == 1)
        vec->data = malloc(vec->item_size);
    else
        vec->data = realloc(vec->data, vec->item_size * vec->item_count);

    vec_set(vec, vec->item_count - 1, item_ref);
}

bool vec_pop(Vec *vec, void *result)
{
    if (vec->item_count == 0) return false;

    vec->item_count--;

    memcpy(result, vec_get(vec, vec->item_count), vec->item_size);

    vec->data = realloc(vec->data, vec->item_size * vec->item_count);
}

#if RUNNING_TESTS
const char *vec_test()
{
    Vec v = vec_new(sizeof(float));

    const float a = 4.0f;
    const float b = 8.0f;

    vec_push(&v, &a);
    vec_push(&v, &a);
    vec_push(&v, &b);

    float popped;
    bool didPop;

    didPop = vec_pop(&v, &popped);
    mu_assert("vec_pop behaves correctly", didPop && popped == 8.0f);
    didPop = vec_pop(&v, &popped);
    mu_assert("vec_pop behaves correctly", didPop && popped == 4.0f);
    didPop = vec_pop(&v, &popped);
    mu_assert("vec_pop behaves correctly", didPop && popped == 4.0f);
    didPop = vec_pop(&v, &popped);
    mu_assert("vec_pop behaves correctly", !didPop);

    return 0;
}
#endif