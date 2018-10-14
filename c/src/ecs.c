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
    Vec entries; // Vec of AllocatorEntry
    Vec free_indices; // Vec of uint32_t
}
GenerationalIndexAllocator;

GenerationalIndexAllocator giallocator_empty(void)
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
    vec_push_copy(&gia->entries, &new_entry);

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
    vec_push_copy(&gia->free_indices, &x);

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
    Vec entries; // Vec of (GenerationalIndexArrayEntry + item)
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

void *giarray_set_copy_or_zeroed(GenerationalIndexArray *gia, GenerationalIndex index, const void *value)
{
    if (gia->entries.item_count <= index.index)
        vec_resize(&gia->entries, index.index + 1);

    GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, index.index);
    entry->has_value = true;
    entry->generation = index.generation;

    if (value)
        memcpy(entry->entry, value, gia->item_size);
    else
        memset(entry->entry, 0, gia->item_size);

    return entry->entry;
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
){
    Vec result = vec_empty(sizeof(GenerationalIndex));

    for (size_t i = 0; i < gia->entries.item_count; ++i)
    {
        GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, i);
        if (!entry->has_value) continue;

        GenerationalIndex index = (GenerationalIndex) { entry->generation, i };

        if (giallocator_is_index_live(allocator, index))
            vec_push_copy(&result, &index);
    }

    *result_length = result.item_count;
    return result.data;
}

Maybe_GenerationalIndex giarray_get_first_valid_index(
    const GenerationalIndexArray *gia, const GenerationalIndexAllocator *allocator
){
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
#define GI_TO_ENTITY(gi) (*(Entity*)(&gi))

struct ECS 
{
    GenerationalIndexAllocator allocator;
    HashTable component_arrays; // HashTable of GenerationalIndexArray for each component type
};


ECS *ecs_new(void)
{
    ECS *ecs = malloc(sizeof(ECS));
    ecs->allocator = giallocator_empty();
    ecs->component_arrays = hashtable_empty(1024, sizeof(GenerationalIndexArray));
    return ecs;
}

static void ecs_delete_hashtable_cb(GenerationalIndexArray *arr)
{
    giarray_clear(arr);
}

void ecs_delete(ECS *ecs)
{
    if (!ecs) return;

    giallocator_clear(&ecs->allocator);
    hashtable_clear_with_callback(&ecs->component_arrays, ecs_delete_hashtable_cb);
    free(ecs);
}


Entity ecs_create_entity(ECS *ecs)
{
    GenerationalIndex gi = giallocator_allocate(&ecs->allocator);
    return GI_TO_ENTITY(gi);
}

void ecs_destroy_entity(ECS *ecs, Entity entity)
{
    giallocator_deallocate(&ecs->allocator, ENTITY_TO_GI(entity));
}

bool ecs_is_entity_valid(const ECS *ecs, Entity entity)
{
    return giallocator_is_index_live(&ecs->allocator, ENTITY_TO_GI(entity));
}


void *ecs_get_component(ECS *ecs, Entity entity, const char *component_type)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component_type);
    return arr ? giarray_at(arr, ENTITY_TO_GI(entity)) : NULL;
}

void *ecs_add_component_zeroed(ECS *ecs, Entity entity, const char *component_type, size_t component_size)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component_type);

    if (!arr)
    {
        GenerationalIndexArray new_arr = giarray_empty(component_size);
        arr = hashtable_set_copy(&ecs->component_arrays, component_type, &new_arr);
    }

    return giarray_set_copy_or_zeroed(arr, ENTITY_TO_GI(entity), 0);
}

void ecs_remove_component(ECS *ecs, Entity entity, const char *component_type)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component_type);
    if (!arr) return;

    giarray_remove(arr, ENTITY_TO_GI(entity));
}


bool ecs_find_first_entity_with_component(const ECS *ecs, const char *component_type, Entity *out_entity)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component_type);
    if (!arr) return false;

    Maybe_GenerationalIndex maybe_index = giarray_get_first_valid_index(arr, &ecs->allocator);
    if (!maybe_index.has_value) return false;

    *out_entity = GI_TO_ENTITY(maybe_index.value);
    return true;
}

Entity *ecs_find_all_entities_with_component_alloc(const ECS *ecs, const char *component_type, size_t *result_length)
{
    GenerationalIndexArray *arr = hashtable_at(&ecs->component_arrays, component_type);
    if (!arr) return;

    return giarray_get_all_valid_indices_alloc(arr, &ecs->allocator, result_length);
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
        giarray_set_copy_or_zeroed(&arr, i, &val_in);

        TestArrayElem *val_out = giarray_at(&arr, i);

        TEST_ASSERT(val_out);
        TEST_ASSERT(val_out != &val_in);
        TEST_ASSERT(val_out->x == 4.0f && val_out->y == 19);

        giallocator_deallocate(&alloc, i);

        GenerationalIndex j = giallocator_allocate(&alloc);
        TestArrayElem *val_out2 = giarray_set_copy_or_zeroed(&arr, j, 0);

        TEST_ASSERT(!giarray_at(&arr, i));

        TEST_ASSERT(val_out2);
        TEST_ASSERT(val_out2->x == 0.0f && val_out->y == 0);

        giarray_clear(&arr);
        giallocator_clear(&alloc);

    TEST_END();
    TEST_BEGIN("GenerationalIndexArray find all valid indices works");
        
        GenerationalIndexAllocator alloc = giallocator_empty();
        GenerationalIndexArray arr = giarray_empty(sizeof(float));
        GenerationalIndex i0 = giallocator_allocate(&alloc);
        GenerationalIndex i1 = giallocator_allocate(&alloc);
        GenerationalIndex i2 = giallocator_allocate(&alloc);

        float *f0 = giarray_set_copy_or_zeroed(&arr, i0, 0);
        float *f1 = giarray_set_copy_or_zeroed(&arr, i1, 0);
        float *f2 = giarray_set_copy_or_zeroed(&arr, i2, 0);

        giarray_remove(&arr, i1);

        size_t result_count;
        GenerationalIndex *results = giarray_get_all_valid_indices_alloc(&arr, &alloc, &result_count);

        TEST_ASSERT(result_count == 2);
        TEST_ASSERT(results[0].generation == i0.generation && results[0].index == i0.index);
        TEST_ASSERT(results[1].generation == i2.generation && results[1].index == i2.index);

        free(results);

        giarray_clear(&arr);
        giallocator_clear(&alloc);

    TEST_END();
    TEST_BEGIN("GenerationalIndexArray remove element works");

        GenerationalIndexAllocator alloc = giallocator_empty();
        GenerationalIndexArray arr = giarray_empty(sizeof(float));
        GenerationalIndex i = giallocator_allocate(&alloc);

        giarray_set_copy_or_zeroed(&arr, i, 0);
        TEST_ASSERT(giarray_at(&arr, i));

        giarray_remove(&arr, i);
        TEST_ASSERT(!giarray_at(&arr, i));

        giarray_clear(&arr);
        giallocator_clear(&alloc);

    TEST_END();
    TEST_BEGIN("ECS add component and get component work");

        ECS *ecs = ecs_new();

        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);

        ECS_ADD_COMPONENT_DECL(float, set_float0, ecs, e0);
        ECS_ADD_COMPONENT_DECL(float, set_float1, ecs, e1);

        ECS_GET_COMPONENT_DECL(float, get_float, ecs, e1);
        TEST_ASSERT(set_float1 == get_float);

        ecs_delete(ecs);

    TEST_END();
    TEST_BEGIN("ECS find all entities with component works");

        ECS *ecs = ecs_new();

        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);
        Entity e2 = ecs_create_entity(ecs);

        ECS_ADD_COMPONENT(float, ecs, e0);
        ECS_ADD_COMPONENT(float, ecs, e2);
        ECS_ADD_COMPONENT(uint32_t, ecs, e0);
        ECS_ADD_COMPONENT(uint32_t, ecs, e1);
        ECS_ADD_COMPONENT(uint32_t, ecs, e2);

        ECS_GET_COMPONENT_DECL(float, floaty0, ecs, e0);
        ECS_GET_COMPONENT_DECL(float, floaty2, ecs, e2);

        size_t result_count;
        Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(float, ecs, &result_count);

        TEST_ASSERT(result_count == 2);

        ECS_GET_COMPONENT_DECL(float, get0, ecs, entities[0]);
        ECS_GET_COMPONENT_DECL(float, get1, ecs, entities[1]);

        TEST_ASSERT(entities[0] == e0 && get0 == floaty0);
        TEST_ASSERT(entities[1] == e2 && get1 == floaty2);

        free(entities);

        ecs_delete(ecs);

    TEST_END();
    TEST_BEGIN("ECS find first entity with component works");

        ECS *ecs = ecs_new();

        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);
        Entity entity;
        bool did_find;

        ECS_ADD_COMPONENT(float, ecs, e0);
        ECS_ADD_COMPONENT(float, ecs, e1);
        ECS_ADD_COMPONENT(uint32_t, ecs, e1);

        did_find = ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(uint32_t, ecs, &entity);
        TEST_ASSERT(did_find);
        TEST_ASSERT(entity == e1);

        did_find = ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(int16_t, ecs, &entity);
        TEST_ASSERT(!did_find);

        ecs_delete(ecs);

    TEST_END();
    TEST_BEGIN("ECS remove component works");

        ECS *ecs = ecs_new();
        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);
        Entity e2 = ecs_create_entity(ecs);
        
        ECS_ADD_COMPONENT_DECL(float, floaty_set, ecs, e0);
        ECS_GET_COMPONENT_DECL(float, floaty_get, ecs, e0);
        TEST_ASSERT(floaty_get == floaty_set);

        ECS_REMOVE_COMPONENT(float, ecs, e0);
        ECS_GET_COMPONENT_DECL(float, floaty_get_again, ecs, e0);
        TEST_ASSERT(!floaty_get_again);

        ecs_delete(ecs);

    TEST_END();
    return 0;
}
#endif