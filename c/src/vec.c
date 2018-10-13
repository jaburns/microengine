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
    vec->data = 0;
    vec->item_count = 0;
}

void vec_clear_with_callback(Vec *vec, VecCallback cb)
{
    for (size_t i = 0; i < vec->item_count; ++i)
        cb(vec_at(vec, i));

    vec_clear(vec);
}

#ifdef RUN_TESTS 
static int test_clear_callback_calls;
static uint8_t test_clear_callback_sum;

static void test_clear_callback(void *item)
{
    test_clear_callback_calls++;
    test_clear_callback_sum += *((uint8_t*)item);
}

TestResult vec_test(void)
{
    TEST_BEGIN("Vec at, push, and pop work correctly");

        Vec v = vec_empty(sizeof(float));

        float a = 4.0f;
        float b = 8.0f;

        vec_push_copy(&v, &a);
        vec_push_copy(&v, &a);
        vec_push_copy(&v, &b);

        TEST_ASSERT(*(float*)vec_at(&v, 0) == 4.0f);
        TEST_ASSERT(*(float*)vec_at(&v, 1) == 4.0f);
        TEST_ASSERT(*(float*)vec_at(&v, 2) == 8.0f);

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

        TEST_ASSERT(v.item_count == 0);
        TEST_ASSERT(v.data == 0);

    TEST_END();
    TEST_BEGIN("Vec clear with callback iterates the vec");

        Vec v = vec_empty(sizeof(uint8_t));

        uint8_t a;
        a = 2; vec_push_copy(&v, &a);
        a = 4; vec_push_copy(&v, &a);
        a = 8; vec_push_copy(&v, &a);

        test_clear_callback_calls = 0;
        test_clear_callback_sum = 0;
        vec_clear_with_callback(&v, &test_clear_callback);

        TEST_ASSERT(test_clear_callback_calls == 3);
        TEST_ASSERT(test_clear_callback_sum == 14);
        TEST_ASSERT(!vec_pop(&v, &a));

        TEST_ASSERT(v.item_count == 0);
        TEST_ASSERT(v.data == 0);

    TEST_END();
    TEST_BEGIN("Vec resize larger writes zeroes and smaller removes items");

        Vec v = vec_empty(sizeof(uint8_t));

        uint8_t a;
        a = 2; vec_push_copy(&v, &a);
        a = 4; vec_push_copy(&v, &a);

        vec_resize(&v, 4);

        TEST_ASSERT(v.item_count == 4);
        TEST_ASSERT(*(uint8_t*)vec_at(&v, 2) == 0);
        TEST_ASSERT(*(uint8_t*)vec_at(&v, 3) == 0);

        vec_resize(&v, 1);

        TEST_ASSERT(v.item_count == 1);
        TEST_ASSERT(*(uint8_t*)vec_at(&v, 0) == 2);

        vec_resize(&v, 0);

        TEST_ASSERT(v.item_count == 0);
        TEST_ASSERT(v.data == 0);

    TEST_END();
    return 0;
}
#endif