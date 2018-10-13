#include "ecs.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils.h"
#include "vec.h"
#include "hashtable.h"

typedef struct GenerationalIndex 
{
    uint32_t generation;
    uint32_t index;
}
GenerationalIndex;

DECLARE_MAYBE(GenerationalIndex);

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

GenerationalIndexAllocator giallocator_empty()
{
    return (GenerationalIndexAllocator) { vec_empty(sizeof(AllocatorEntry)), vec_empty(sizeof(uint32_t)) };
}

GenerationalIndex giallocator_allocate(GenerationalIndexAllocator *gia)
{
    if (gia->free_indices.item_count > 0)
    {
        uint32_t index;
        vec_pop(&gia->free_indices, &index);

        AllocatorEntry *entry = vec_at(&gia->entries, index);
        entry->generation += 1;
        entry->is_live = true;

        return (GenerationalIndex) { entry->generation, index };
    }

    AllocatorEntry new_entry = (AllocatorEntry) { true, 0 };
    vec_push(&gia->entries, &new_entry);

    return (GenerationalIndex) { 0, gia->entries.item_count - 1 };
}

bool giallocator_is_index_live(const GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return false;

    AllocatorEntry *entry = vec_at(&gia->entries, index.index);
    
    return entry->is_live && entry->generation == index.generation;
}

bool giallocator_deallocate(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (!giallocator_is_index_live(gia, index)) return false;

    AllocatorEntry *entry = vec_at(&gia->entries, index.index);
    entry->is_live = false;

    uint32_t x = index.index;
    vec_push(&gia->free_indices, &x);

    return true;
}

bool giallocator_clear(GenerationalIndexAllocator *gia)
{
    vec_clear(&gia->entries);
    vec_clear(&gia->free_indices);
}

typedef struct GenerationalIndexArrayEntry
{
    bool has_value;
    uint32_t generation;
    uint8_t entry[];
}
GenerationalIndexArrayEntry;

typedef struct GenerationalIndexArray
{
    size_t item_size;
    Vec entries; // of (GenerationalIndexArrayEntry + item)
}
GenerationalIndexArray;

GenerationalIndexArray giarray_empty(size_t item_size)
{
    return (GenerationalIndexArray) { item_size, vec_empty(sizeof(GenerationalIndexArrayEntry) + item_size) };
}

void giarray_clear(GenerationalIndexArray *gia)
{
    vec_clear(&gia->entries);
}

void giarray_set_copy(GenerationalIndexArray *gia, GenerationalIndex index, const void *value)
{
    if (gia->entries.item_count <= index.index)
        vec_resize(&gia->entries, index.index + 1);

    GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, index.index);
    entry->has_value = true;
    entry->generation = index.generation;
    memcpy(entry->entry, value, gia->item_size);
}

void *giarray_at(GenerationalIndexArray *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return NULL;

    GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, index.index);

    return entry->has_value && entry->generation == index.generation
        ? entry->entry
        : NULL;
}

void giarray_remove(GenerationalIndexArray *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return;

    GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, index.index);
    entry->has_value = false;
}

GenerationalIndex *giarray_get_all_valid_indices_alloc(
    const GenerationalIndexArray *gia, const GenerationalIndexAllocator *allocator, size_t *result_length
) {
    Vec result = vec_empty(sizeof(GenerationalIndex));

    for (size_t i = 0; i < gia->entries.item_count; ++i)
    {
        GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, i);
        if (!entry->has_value) continue;

        GenerationalIndex index = (GenerationalIndex) { entry->generation, i };

        if (giallocator_is_index_live(allocator, index))
            vec_push(&result, &index);
    }

    *result_length = result.item_count;
    return result.data;
}

Maybe_GenerationalIndex giarray_get_first_valid_index(
    const GenerationalIndexArray *gia, const GenerationalIndexAllocator *allocator
) {
    for (size_t i = 0; i < gia->entries.item_count; ++i)
    {
        GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, i);
        if (!entry->has_value) continue;

        GenerationalIndex index = (GenerationalIndex) { entry->generation, i };

        if (giallocator_is_index_live(allocator, index))
            return (Maybe_GenerationalIndex) { true, index };
    }

    return (Maybe_GenerationalIndex) { false };
}

#define ENTITY_TO_GI(entity) (*(GenerationalIndex*)(&entity))

struct ECS 
{
    GenerationalIndexAllocator allocator;
    HashTable component_arrays;
};

void *get_component(ECS *ecs, Entity entity, const char *component)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component);
    return arr ? giarray_at(arr, ENTITY_TO_GI(entity)) : NULL;
}






#ifdef RUN_TESTS 
TestResult ecs_test()
{
    TEST_BEGIN("GenerationalIndexAllocator works");

        GenerationalIndexAllocator alloc = giallocator_empty();

        GenerationalIndex i = giallocator_allocate(&alloc);
        GenerationalIndex j = giallocator_allocate(&alloc);

        TEST_ASSERT(i.generation == 0 && i.index == 0);
        TEST_ASSERT(j.generation == 0 && j.index == 1);

        TEST_ASSERT( giallocator_deallocate(&alloc, i));
        TEST_ASSERT(!giallocator_deallocate(&alloc, i));

        GenerationalIndex k = giallocator_allocate(&alloc);

        TEST_ASSERT(k.generation == 1 && k.index == 0);

        TEST_ASSERT(!giallocator_is_index_live(&alloc, i));
        TEST_ASSERT( giallocator_is_index_live(&alloc, j));
        TEST_ASSERT( giallocator_is_index_live(&alloc, k));

        giallocator_clear(&alloc);

    TEST_END();
    TEST_BEGIN("GenerationalIndexArray works with multiple generations");

        typedef struct TestArrayElem
        {
            float x;
            int16_t y;
        }
        TestArrayElem;

        GenerationalIndexAllocator alloc = giallocator_empty();
        GenerationalIndexArray arr = giarray_empty(sizeof(TestArrayElem));

        GenerationalIndex i = giallocator_allocate(&alloc);
        TestArrayElem val_in = { 4.0f, 19 };
        giarray_set_copy(&arr, i, &val_in);

        TestArrayElem *val_out = giarray_at(&arr, i);

        TEST_ASSERT(val_out);
        TEST_ASSERT(val_out != &val_in);
        TEST_ASSERT(val_out->x == 4.0f && val_out->y == 19);

        giallocator_deallocate(&alloc, i);

        GenerationalIndex j = giallocator_allocate(&alloc);
        TestArrayElem val_in2 = { 8.0f, 29 };
        giarray_set_copy(&arr, j, &val_in2);

        TEST_ASSERT(!giarray_at(&arr, i));

        TestArrayElem *val_out2 = giarray_at(&arr, j);

        TEST_ASSERT(val_out2);
        TEST_ASSERT(val_out2 != &val_in2);
        TEST_ASSERT(val_out2->x == 8.0f && val_out->y == 29);

    TEST_END();
    return 0;
}
#endif
