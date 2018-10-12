#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "vec.h"

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
    return (GenerationalIndexAllocator) { vec_make_empty(sizeof(AllocatorEntry)), vec_make_empty(sizeof(uint32_t)) };
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

bool giallocator_is_index_live(const GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return false;

    AllocatorEntry *entry = vec_get(&gia->entries, index.index);
    
    return entry->is_live && entry->generation == index.generation;
}

void giallocator_deallocate(GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (! giallocator_is_index_live(gia, index)) return;

    AllocatorEntry *entry = vec_get(&gia->entries, index.index);
    entry->is_live = false;

    uint32_t x = index.index;
    vec_push(&gia->free_indices, &x);
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

GenerationalIndexArray giarray_new(size_t item_size)
{
    return (GenerationalIndexArray) { item_size, vec_make_empty(sizeof(GenerationalIndexArrayEntry) + item_size) };
}

void giarray_set(GenerationalIndexArray *gia, GenerationalIndex index, const void *value)
{
    if (gia->entries.item_count <= index.index)
        vec_resize(&gia->entries, index.index + 1);

    GenerationalIndexArrayEntry* entry = vec_get(&gia->entries, index.index);
    uint32_t prev_gen = entry->generation;

    if (prev_gen > index.generation)
        exit(1);

    entry->has_value = true;
    entry->generation = index.generation;
    memcpy(entry->entry, value, gia->item_size);
}

void *giarray_get(GenerationalIndexArray *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return NULL;

    GenerationalIndexArrayEntry* entry = vec_get(&gia->entries, index.index);

    return entry->has_value && entry->generation == index.generation
        ? entry->entry
        : NULL;
}

void giarray_remove(GenerationalIndexArray *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return;

    GenerationalIndexArrayEntry* entry = vec_get(&gia->entries, index.index);
    entry->has_value = false;
}

Vec/*<GenerationalIndex>*/ giarray_get_all_valid_indices(const GenerationalIndexArray *gia, const GenerationalIndexAllocator *allocator)
{
    Vec result;

    for (size_t i = 0; i < gia->entries.item_count; ++i)
    {
        GenerationalIndexArrayEntry* entry = vec_get(&gia->entries, i);
        if (!entry->has_value) continue;

        GenerationalIndex index = (GenerationalIndex) { entry->generation, i };

        if (giallocator_is_index_live(allocator, index))
            vec_push(&result, &index);
    }

    return result;
}

GenerationalIndex giarray_get_first_valid_index(const GenerationalIndexAllocator *allocator)
{
//  for (auto i = 0; i < m_entries.size(); ++i)
//  {
//      const auto& entry = m_entries[i];
//      if (!entry) continue;

//      GenerationalIndex index = { i, entry->generation };
//      
//      if (allocator.is_live(index))
//          return std::make_tuple(index, std::ref(entry->value));
//  }

//  return std::nullopt;
}


struct ECS 
{
    int inner;
};


#ifdef RUN_TESTS 
#include "testing.h"

TEST_RESULT ecs_test()
{
    /*
    GenerationalIndexArray<float> floaties;
    GenerationalIndexAllocator alloc;

    auto j = alloc.allocate();
    auto i = alloc.allocate();

    floaties.set(i, 2.0f);
    floaties.set(j, 4.0f);
    
    if (auto getty = floaties.get_first_valid_entry(alloc)) {
        cout << "Got " << std::get<1>(*getty).get();
    }

    return 0;
*/

    TEST_BEGIN("ECS!!! Vec should push and pop correctly");
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
