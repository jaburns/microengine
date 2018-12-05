#include "ecs.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

    return (GenerationalIndex) { 0, (uint32_t)gia->entries.item_count - 1 };
}

bool giallocator_is_index_live(const GenerationalIndexAllocator *gia, GenerationalIndex index)
{
    if (index.index >= gia->entries.item_count) return false;

    const AllocatorEntry *entry = vec_at_const(&gia->entries, index.index);

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

GenerationalIndex *giallocator_get_all_allocated_indices_alloc(const GenerationalIndexAllocator *gia, size_t *result_length)
{
    Vec result = vec_empty(sizeof(GenerationalIndex));

    for (int i = 0; i < gia->entries.item_count; ++i)
    {
        const AllocatorEntry *entry = vec_at_const(&gia->entries, i);
        if (!entry->is_live) continue;

        GenerationalIndex index = { entry->generation, i };
        vec_push_copy(&result, &index);
    }

    *result_length = result.item_count;
    return result.data;
}

void giallocator_clear(GenerationalIndexAllocator *gia)
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
    ECSComponentDestructor destructor;
    Vec entries; // Vec of (GenerationalIndexArrayEntry + item)
}
GenerationalIndexArray;

GenerationalIndexArray giarray_empty(size_t item_size, ECSComponentDestructor destructor)
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

    for (uint32_t i = 0; i < gia->entries.item_count; ++i)
    {
        const GenerationalIndexArrayEntry* entry = vec_at_const(&gia->entries, i);
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
    for (uint32_t i = 0; i < gia->entries.item_count; ++i)
    {
        const GenerationalIndexArrayEntry* entry = vec_at_const(&gia->entries, i);
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
    ECSComponentDestructor destructor;
    GenerationalIndexArray components;
}
ECSComponent;

typedef struct EventListenerEntry
{
    ECSComponentEventType type;
    ECSComponentEventListener listener;
}
EventListenerEntry;

typedef struct BorrowedComponent
{
    Entity entity;
    const char *type; // Assumed to be a string with a static lifetime. TODO don't assume anything
    const void *component;
    const char *debug_file;
    int debug_line;
}
BorrowedComponent;

struct ECS
{
    GenerationalIndexAllocator allocator;
    HashTable components; // of ECSComponent keyed by component types
    Vec borrowed_components; // of BorrowedComponent
    HashTable event_listeners; // of Vec of EventListenerEntry keyed by component types
};

static GenerationalIndex entity_to_gi(Entity entity)
{
    if (entity == 0)
        PANIC("Attempted to convert empty entity in to generational index");

    entity -= 1;

    return (GenerationalIndex) {
        (uint32_t)((entity & 0x0000FFFFFF000000) >> 24),
        (uint32_t) (entity & 0x0000000000FFFFFF)
    };
}

static Entity gi_to_entity(GenerationalIndex index)
{
    return (((uint64_t)index.generation << 24) & 0x0000FFFFFF000000)
        |  (((uint64_t)index.index + 1)        & 0x0000000000FFFFFF);
}

ECS *ecs_new(void)
{
    ECS *ecs = malloc(sizeof(ECS));
    ecs->allocator = giallocator_empty();
    ecs->components = hashtable_empty(256, sizeof(ECSComponent));
    ecs->borrowed_components = vec_empty(sizeof(BorrowedComponent));
    ecs->event_listeners = hashtable_empty(256, sizeof(Vec));
    return ecs;
}

static void delete_components_hashtable_cb(void *context, ECSComponent *comp)
{
    giarray_clear(&comp->components);
}

static void delete_event_listeners_hashtable_cb(void *context, Vec *listeners)
{
    vec_clear(listeners);
}

void ecs_delete(ECS *ecs)
{
    if (!ecs) return;

    giallocator_clear(&ecs->allocator);
    hashtable_clear_with_callback(&ecs->components, NULL, delete_components_hashtable_cb);
    vec_clear(&ecs->borrowed_components);
    hashtable_clear_with_callback(&ecs->event_listeners, NULL, delete_event_listeners_hashtable_cb);

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

void ecs_register_component(ECS *ecs, const char *component_type, size_t component_size, ECSComponentDestructor destructor)
{
    if (hashtable_at(&ecs->components, component_type))
        PANIC("Tried to register the same component twice: '%s'\n", component_type);

    ECSComponent new_component = { component_size, destructor, giarray_empty(component_size, destructor) };
    hashtable_set_copy(&ecs->components, component_type, &new_component);
}

static bool check_borrowed_component_matches_ptr(const void *component, const BorrowedComponent *borrow_entry)
{
    return component == borrow_entry->component;
}

void *ecs_borrow_component(ECS *ecs, Entity entity, const char *component_type, const char *debug_file, int debug_line)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    void *result = comp ? giarray_at(&comp->components, entity_to_gi(entity)) : NULL;

    if (!result) return NULL;

    int found_index = vec_find_index(&ecs->borrowed_components, result, check_borrowed_component_matches_ptr);

    if (found_index >= 0)
    {
        BorrowedComponent *prev_borrow = vec_at(&ecs->borrowed_components, found_index);
        PANIC("A component of type '%s' was borrowed twice without being returned.\n"
            "Original borrow occurred at %s : %d\n" 
            "    This borrow occurred at %s : %d",
            component_type, prev_borrow->debug_file, prev_borrow->debug_line, debug_file, debug_line);
    }

    BorrowedComponent new_borrow = { 
        .component = result,
        .entity = entity,
        .type = component_type, // TODO don't assume that this string has a static lifetime
        .debug_file = debug_file,
        .debug_line = debug_line,
    };

    vec_push_copy(&ecs->borrowed_components, &new_borrow);

    return result;
}

void ecs_return_component(ECS *ecs, void *component, const char *debug_file, int debug_line)
{
    int found_index = vec_find_index(&ecs->borrowed_components, component, check_borrowed_component_matches_ptr);

    if (found_index < 0)
        PANIC("Attempted to return a component that was not borrowed.\n%s : %d", debug_file, debug_line);

    BorrowedComponent *borrowed = vec_at(&ecs->borrowed_components, found_index);

    Vec *listeners = hashtable_at(&ecs->event_listeners, borrowed->type);

    if (listeners) 
    for (int i = 0; i < listeners->item_count; ++i)
    {
        EventListenerEntry *entry = vec_at(listeners, i);
        if (entry->type == ECS_EVENT_COMPONENT_CHANGED)
            entry->listener(borrowed->entity, component);
    }

    vec_remove(&ecs->borrowed_components, found_index);
}

const void *ecs_view_component(const ECS *ecs, Entity entity, const char *component_type)
{
    const ECSComponent *comp = hashtable_at_const(&ecs->components, component_type);
    ECSComponent *comp_mut = (ECSComponent*)comp;
    return comp ? giarray_at(&comp_mut->components, entity_to_gi(entity)) : NULL;
}

void *ecs_add_component_zeroed(ECS *ecs, Entity entity, const char *component_type, const char *debug_file, int debug_line)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp)
        PANIC("Tried to add unregistered component: '%s'\n", component_type);

    void *result = giarray_set_copy_or_zeroed(&comp->components, entity_to_gi(entity), 0);

    BorrowedComponent new_borrow = { 
        .component = result,
        .entity = entity,
        .type = component_type, // TODO don't assume that this string has a static lifetime
        .debug_file = debug_file,
        .debug_line = debug_line,
    };

    vec_push_copy(&ecs->borrowed_components, &new_borrow);

    return result;
}

void ecs_remove_component(ECS *ecs, Entity entity, const char *component_type)
{
    ECSComponent *comp = hashtable_at(&ecs->components, component_type);
    if (!comp) return;

    giarray_remove(&comp->components, entity_to_gi(entity));
}

bool ecs_find_first_entity_with_component(const ECS *ecs, const char *component_type, Entity *out_entity)
{
    const ECSComponent *comp = hashtable_at_const(&ecs->components, component_type);
    if (!comp) return false;

    GenerationalIndex index;
    bool found_index = giarray_get_first_valid_index(&comp->components, &ecs->allocator, &index);
    if (!found_index) return false;

    *out_entity = gi_to_entity(index);
    return true;
}

Entity *ecs_find_all_entities_with_component_alloc(const ECS *ecs, const char *component_type, size_t *result_length)
{
    const ECSComponent *comp = hashtable_at_const(&ecs->components, component_type);
    if (!comp) return NULL;

    GenerationalIndex *result = giarray_get_all_valid_indices_alloc(&comp->components, &ecs->allocator, result_length);

    for (int i = 0; i < *result_length; ++i)
        ((Entity*)result)[i] = gi_to_entity(result[i]);

    return (Entity*)result;
}

Entity *ecs_find_all_entities_alloc(const ECS *ecs, size_t *result_length)
{
    const GenerationalIndex *result = giallocator_get_all_allocated_indices_alloc(&ecs->allocator, result_length);

    for (int i = 0; i < *result_length; ++i)
        ((Entity*)result)[i] = gi_to_entity(result[i]);

    return (Entity*)result;
}

static bool check_event_listeners_entries_match(EventListenerEntry *a, const EventListenerEntry *b)
{
    return a->listener == b->listener && a->type == b->type;
}

void ecs_add_component_event_listener(ECS *ecs, ECSComponentEventType event_type, const char *component_type, ECSComponentEventListener listener)
{
    Vec *entries = hashtable_at(&ecs->event_listeners, component_type);

    if (!entries)
    {
        Vec new_entries = vec_empty(sizeof(EventListenerEntry));
        entries = hashtable_set_copy(&ecs->event_listeners, component_type, &new_entries);
    }

    EventListenerEntry entry = { 
        .type = event_type,
        .listener = listener
    };

    int found_index = vec_find_index(entries, &entry, check_event_listeners_entries_match);

    if (found_index < 0)
        vec_push_copy(entries, &entry);
}

void ecs_remove_component_event_listener(ECS *ecs, ECSComponentEventType event_type, const char *component_type, ECSComponentEventListener listener)
{
    Vec *entries = hashtable_at(&ecs->event_listeners, component_type);
    if (!entries) return;

    EventListenerEntry entry = { 
        .type = event_type,
        .listener = listener
    };

    int found_index = vec_find_index(entries, &entry, check_event_listeners_entries_match);

    if (found_index >= 0)
        vec_remove(entries, found_index);
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
    TEST_BEGIN("GenerationalIndexAllocator get all indices works");

        GenerationalIndexAllocator alloc = giallocator_empty();

        giallocator_allocate(&alloc); // [(0,0)]
        GenerationalIndex j = giallocator_allocate(&alloc); // [(0,0), (0,1)]

        giallocator_deallocate(&alloc, j); // [(0,0), (x,1)]

        giallocator_allocate(&alloc); // [(0,0), (1,1)]
        GenerationalIndex l = giallocator_allocate(&alloc); // [(0,0), (1,1), (0,2)]
        giallocator_allocate(&alloc); // [(0,0), (1,1), (0,2), (0,3)]

        giallocator_deallocate(&alloc, l); // [(0,0), (1,1), (x,2), (0,3)]

        size_t num_indices;
        GenerationalIndex *indices = giallocator_get_all_allocated_indices_alloc(&alloc, &num_indices);

        TEST_ASSERT(num_indices == 3);
        TEST_ASSERT(indices[0].generation == 0 && indices[0].index == 0);
        TEST_ASSERT(indices[1].generation == 1 && indices[1].index == 1);
        TEST_ASSERT(indices[2].generation == 0 && indices[2].index == 3);

        free(indices);
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

        giarray_clear(&arr);
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

        giarray_clear(&arr);
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

        giarray_clear(&arr);
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

        ECS_VIEW_COMPONENT_DECL(float, get_float, ecs, e1);
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

        ECS_VIEW_COMPONENT_DECL(float, floaty0, ecs, e0);
        ECS_VIEW_COMPONENT_DECL(float, floaty2, ecs, e2);

        size_t result_count;
        Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(float, ecs, &result_count);

        TEST_ASSERT(result_count == 2);

        ECS_VIEW_COMPONENT_DECL(float, get0, ecs, entities[0]);
        ECS_VIEW_COMPONENT_DECL(float, get1, ecs, entities[1]);

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
        ECS_VIEW_COMPONENT_DECL(float, floaty_get, ecs, e0);
        TEST_ASSERT(floaty_get == floaty_set);

        ECS_REMOVE_COMPONENT(float, ecs, e0);
        ECS_VIEW_COMPONENT_DECL(float, floaty_get_again, ecs, e0);
        TEST_ASSERT(!floaty_get_again);

        ecs_delete(ecs);

    TEST_END();
    
    // TODO write tests for the borrow/return mutable pointer api
    // TODO event listener tests

    return 0;
}
#endif
