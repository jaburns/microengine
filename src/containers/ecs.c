#include "ecs.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../utils.h"
#include "vec.h"
#include "hashtable.h"


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
    ComponentDestructor destructor;
    Vec entries; // Vec of (GenerationalIndexArrayEntry + item)
}
GenerationalIndexArray;

GenerationalIndexArray giarray_empty(size_t item_size, ComponentDestructor destructor)
{
    return (GenerationalIndexArray) { item_size, destructor, vec_empty(sizeof(GenerationalIndexArrayEntry) + item_size) };
}

static void giarray_clear_callback(GenerationalIndexArray *context, GenerationalIndexArrayEntry *entry)
{
    if (entry->has_value)
        context->destructor(entry->entry);
}

void giarray_clear(GenerationalIndexArray *gia)
{
    if (gia->destructor)
        vec_clear_with_callback(&gia->entries, gia, giarray_clear_callback);
    else
        vec_clear(&gia->entries);
}

void *giarray_set_copy_or_zeroed(GenerationalIndexArray *gia, GenerationalIndex index, const void *value)
{
    if (gia->entries.item_count <= index.index)
        vec_resize(&gia->entries, index.index + 1);

    GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, index.index);

    if (gia->destructor && entry->has_value)
        gia->destructor(entry->entry);

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

    if (gia->destructor && entry->has_value)
        gia->destructor(entry->entry);

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

bool giarray_get_first_valid_index(
    const GenerationalIndexArray *gia, const GenerationalIndexAllocator *allocator, GenerationalIndex *result
){
    for (size_t i = 0; i < gia->entries.item_count; ++i)
    {
        GenerationalIndexArrayEntry* entry = vec_at(&gia->entries, i);
        if (!entry->has_value) continue;

        GenerationalIndex index = (GenerationalIndex) { entry->generation, i };

        if (giallocator_is_index_live(allocator, index))
        {
            *result = index;
            return true;
        }
    }

    return false;
}


typedef struct ECSComponent
{
    size_t size;
    ComponentDestructor destructor;
    GenerationalIndexArray components;
}
ECSComponent;

struct ECS
{
    GenerationalIndexAllocator allocator;
    HashTable components; // of ECSComponent 
};

static GenerationalIndex entity_to_gi(Entity entity)
{
    if (entity == 0)
        PANIC("Attempted to convert empty entity in to generational index");

    entity -= 1;

    return (GenerationalIndex) {
        (entity & 0x0000FFFFFF000000) >> 24,
        (entity & 0x0000000000FFFFFF)
    };
}

static Entity gi_to_entity(GenerationalIndex index)
{
    return ((index.generation << 24) & 0x0000FFFFFF000000)
        |  ((index.index + 1)        & 0x0000000000FFFFFF);
}

ECS *ecs_new(void)
{
    ECS *ecs = malloc(sizeof(ECS));
    ecs->allocator = giallocator_empty();
    ecs->components = hashtable_empty(1024, sizeof(ECSComponent));
    return ecs;
}

static void ecs_delete_hashtable_cb(void *context, ECSComponent *comp)
{
    giarray_clear(&comp->components);
}

void ecs_delete(ECS *ecs)
{
    if (!ecs) return;

    giallocator_clear(&ecs->allocator);
    hashtable_clear_with_callback(&ecs->components, NULL, ecs_delete_hashtable_cb);
    free(ecs);
}


Entity ecs_create_entity(ECS *ecs)
{
    GenerationalIndex gi = giallocator_allocate(&ecs->allocator);
    return gi_to_entity(gi);
}

void ecs_destroy_entity(ECS *ecs, Entity entity)
{
    giallocator_deallocate(&ecs->allocator, entity_to_gi(entity));
}

bool ecs_is_entity_valid(const ECS *ecs, Entity entity)
{
    return giallocator_is_index_live(&ecs->allocator, entity_to_gi(entity));
}


void ecs_register_component(ECS *ecs, const char *component_type, size_t component_size, ComponentDestructor destructor)
{
    if (hashtable_at(&ecs->components, component_type))
        PANIC("Tried to register the same component twice: '%s'\n", component_type);

    ECSComponent new_component = { component_size, destructor, giarray_empty(component_size, destructor) };
    hashtable_set_copy(&ecs->components, component_type, &new_component);
}

void *ecs_get_component(ECS *ecs, Entity entity, const char *component_type)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    return comp ? giarray_at(&comp->components, entity_to_gi(entity)) : NULL;
}

void *ecs_add_component_zeroed(ECS *ecs, Entity entity, const char *component_type)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp)
        PANIC("Tried to add unregistered component: '%s'\n", component_type);

    return giarray_set_copy_or_zeroed(&comp->components, entity_to_gi(entity), 0);
}

void ecs_remove_component(ECS *ecs, Entity entity, const char *component_type)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp) return;

    giarray_remove(&comp->components, entity_to_gi(entity));
}


bool ecs_find_first_entity_with_component(const ECS *ecs, const char *component_type, Entity *out_entity)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp) return false;

    GenerationalIndex index;
    bool found_index = giarray_get_first_valid_index(&comp->components, &ecs->allocator, &index);
    if (!found_index) return false;

    *out_entity = gi_to_entity(index);
    return true;
}

Entity *ecs_find_all_entities_with_component_alloc(const ECS *ecs, const char *component_type, size_t *result_length)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp) return NULL;

    GenerationalIndex *result = giarray_get_all_valid_indices_alloc(&comp->components, &ecs->allocator, result_length);

    for (int i = 0; i < *result_length; ++i)
        ((Entity*)result)[i] = gi_to_entity(result[i]);

    return (Entity*)result;
}



#ifdef RUN_TESTS
static int test_destructor_call_count = 0;
static void test_destructor(void *value)
{
    test_destructor_call_count++;
}

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
        GenerationalIndexArray arr = giarray_empty(sizeof(TestArrayElem), test_destructor);

        test_destructor_call_count = 0;

        GenerationalIndex i = giallocator_allocate(&alloc);
        TestArrayElem val_in = { 4.0f, 19 };
        giarray_set_copy_or_zeroed(&arr, i, &val_in);

        TestArrayElem *val_out = giarray_at(&arr, i);

        TEST_ASSERT(val_out);
        TEST_ASSERT(val_out != &val_in);
        TEST_ASSERT(val_out->x == 4.0f && val_out->y == 19);
        TEST_ASSERT(test_destructor_call_count == 0);

        giallocator_deallocate(&alloc, i);

        GenerationalIndex j = giallocator_allocate(&alloc);
        TestArrayElem *val_out2 = giarray_set_copy_or_zeroed(&arr, j, 0);

        TEST_ASSERT(test_destructor_call_count == 1);

        TEST_ASSERT(!giarray_at(&arr, i));

        TEST_ASSERT(val_out2);
        TEST_ASSERT(val_out2->x == 0.0f && val_out->y == 0);

        giarray_clear(&arr, NULL);
        giallocator_clear(&alloc);

        TEST_ASSERT(test_destructor_call_count == 2);

    TEST_END();
    TEST_BEGIN("GenerationalIndexArray find all valid indices works");

        GenerationalIndexAllocator alloc = giallocator_empty();
        GenerationalIndexArray arr = giarray_empty(sizeof(float), NULL);
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

        giarray_clear(&arr, NULL);
        giallocator_clear(&alloc);

    TEST_END();
    TEST_BEGIN("GenerationalIndexArray remove element works, destructor not called twice");

        GenerationalIndexAllocator alloc = giallocator_empty();
        GenerationalIndexArray arr = giarray_empty(sizeof(float), test_destructor);
        GenerationalIndex i = giallocator_allocate(&alloc);

        test_destructor_call_count = 0;

        giarray_set_copy_or_zeroed(&arr, i, 0);
        TEST_ASSERT(giarray_at(&arr, i));

        giarray_remove(&arr, i);
        TEST_ASSERT(!giarray_at(&arr, i));

        TEST_ASSERT(test_destructor_call_count == 1);

        giarray_clear(&arr, NULL);
        giallocator_clear(&alloc);

        TEST_ASSERT(test_destructor_call_count == 1);

    TEST_END();
    TEST_BEGIN("Entity to GenerationalIndex conversion reverses");

        {
            GenerationalIndex i = { 29, 119 };
            GenerationalIndex j = entity_to_gi(gi_to_entity(i));
            TEST_ASSERT(i.index == j.index && i.generation == j.generation);
        } {
            GenerationalIndex i = { 0, 0 };
            GenerationalIndex j = entity_to_gi(gi_to_entity(i));
            TEST_ASSERT(i.index == j.index && i.generation == j.generation);
        }

    TEST_END();
    TEST_BEGIN("ECS add component and get component work");

        ECS *ecs = ecs_new();

        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);

        ECS_REGISTER_COMPONENT(float, ecs, NULL);

        ECS_ADD_COMPONENT_ZEROED_DECL(float, set_float0, ecs, e0);
        ECS_ADD_COMPONENT_ZEROED_DECL(float, set_float1, ecs, e1);

        ECS_GET_COMPONENT_DECL(float, get_float, ecs, e1);
        TEST_ASSERT(set_float1 == get_float);

        ecs_delete(ecs);

    TEST_END();
    TEST_BEGIN("ECS find all entities with component works");

        ECS *ecs = ecs_new();

        Entity e0 = ecs_create_entity(ecs);
        Entity e1 = ecs_create_entity(ecs);
        Entity e2 = ecs_create_entity(ecs);

        ECS_REGISTER_COMPONENT(float, ecs, NULL);
        ECS_REGISTER_COMPONENT(uint32_t, ecs, NULL);

        ECS_ADD_COMPONENT_ZEROED_DECL(float, _0, ecs, e0);
        ECS_ADD_COMPONENT_ZEROED_DECL(float, _1, ecs, e2);
        ECS_ADD_COMPONENT_ZEROED_DECL(uint32_t, _2, ecs, e0);
        ECS_ADD_COMPONENT_ZEROED_DECL(uint32_t, _3, ecs, e1);
        ECS_ADD_COMPONENT_ZEROED_DECL(uint32_t, _4, ecs, e2);

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

        ECS_REGISTER_COMPONENT(float, ecs, NULL);
        ECS_REGISTER_COMPONENT(uint32_t, ecs, NULL);

        ECS_ADD_COMPONENT_ZEROED_DECL(float, _0, ecs, e0);
        ECS_ADD_COMPONENT_ZEROED_DECL(float, _1, ecs, e1);
        ECS_ADD_COMPONENT_ZEROED_DECL(uint32_t, _2, ecs, e1);

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

        ECS_REGISTER_COMPONENT(float, ecs, NULL);

        ECS_ADD_COMPONENT_ZEROED_DECL(float, floaty_set, ecs, e0);
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
