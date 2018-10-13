#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t Entity;
typedef struct ECS ECS;

extern ECS *ecs_new(void);
extern void ecs_delete(ECS *ecs);

extern Entity ecs_create_entity(ECS *ecs);
extern void ecs_destroy_entity(ECS *ecs, Entity entity);
extern bool ecs_is_entity_valid(const ECS *ecs, Entity entity);

extern void *ecs_get_component(ECS *ecs, Entity entity, const char *component_type);
extern void *ecs_add_component_zeroed(ECS *ecs, Entity entity, const char *component_type, size_t component_size);
extern void ecs_remove_component(ECS *ecs, Entity entity, const char *component_type);

extern Entity ecs_find_first_entity_with_component(const ECS *ecs, const char *component_type);
extern Entity *ecs_find_all_entities_with_component_alloc(const ECS *ecs, const char *component_type, size_t *result_length);

#define ECS_GET_COMPONENT_DECL(T, var_name, ecs_ptr, entity) \
    T *var_name = ecs_get_component((ecs_ptr), (entity), #T)

#define ECS_ADD_COMPONENT(T, ecs_ptr, entity) \
    ecs_add_component_zeroed((ecs_ptr), (entity), #T, sizeof(T))

#define ECS_ADD_COMPONENT_DECL(T, var_name, ecs_ptr, entity) \
    T *var_name = ecs_add_component_zeroed((ecs_ptr), (entity), #T, sizeof(T))

#define ECS_REMOVE_COMPONENT(T, ecs_ptr, entity) \
    ecs_remove_component((ecs_ptr), (entity), #T)

#define ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(T, ecs_ptr) \
    ecs_find_first_entity_with_component((ecs_ptr), #T)

#define ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(T, ecs_ptr, result_length) \
    ecs_find_all_entities_with_component_alloc((ecs_ptr), #T, (result_length))

#ifdef RUN_TESTS
#include "testing.h"
extern TestResult ecs_test(void);
#endif