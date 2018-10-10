
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); if (message) return message; } while (0)

typedef struct Vec
{
    void *data;
    size_t item_size;
    size_t item_count;
}
Vec;

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

static char *vec_test()
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

int main() {
    vec_test();
}

typedef struct GenerationalIndex 
{
    uint32_t generation;
    uint32_t index;
}
GenerationalIndex;

typedef struct AllocatorEntry
{
    bool is_live;
    uint32_t generation;
}
AllocatorEntry;

typedef struct GenerationalIndexAllocator
{
    Vec entries; // of AllocatorEntry
    Vec free_indices; // of uint32_t
}
GenerationalIndexAllocator;

GenerationalIndexAllocator giallocator_new()
{
    return (GenerationalIndexAllocator) { vec_new(sizeof(AllocatorEntry)), vec_new(sizeof(uint32_t)) };
}

GenerationalIndex giallocator_allocate(GenerationalIndexAllocator *gia)
{
    if (gia->entries.item_count > 0)
    {
        uint32_t index;
        vec_pop(&gia->entries, &index);

        AllocatorEntry *entry = vec_get(&gia->entries, index);
        entry->generation += 1;
        entry->is_live = true;

        return (GenerationalIndex) { entry->generation, index };
    }

    AllocatorEntry new_entry = (AllocatorEntry) { true, 0 };
    vec_push(&gia->entries, &new_entry);

    return (GenerationalIndex) { gia->entries.item_count - 1, 0 };
}

void giallocator_deallocate(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    //
}

bool giallocator_is_index_live(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    //
}