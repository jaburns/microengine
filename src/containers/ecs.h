#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint64_t Entity;
typedef struct ECS ECS;
typedef void (*ComponentDestructor)(void*);

extern ECS *ecs_new(void);
extern void ecs_delete(ECS *ecs);

extern Entity ecs_create_entity(ECS *ecs);
extern void ecs_destroy_entity(ECS *ecs, Entity entity);
extern bool ecs_is_entity_valid(const ECS *ecs, Entity entity);

extern void ecs_register_component(ECS *ecs, const char *component_type, size_t component_size, ComponentDestructor destructor);
extern void *ecs_get_component(ECS *ecs, Entity entity, const char *component_type);
extern const void *ecs_get_component_const(const ECS *ecs, Entity entity, const char *component_type);
extern void *ecs_add_component_zeroed(ECS *ecs, Entity entity, const char *component_type);
extern void ecs_remove_component(ECS *ecs, Entity entity, const char *component_type);

extern bool ecs_find_first_entity_with_component(ECS *ecs, const char *component_type, Entity *out_entity);
extern Entity *ecs_find_all_entities_with_component_alloc(ECS *ecs, const char *component_type, size_t *result_length);
extern Entity *ecs_find_all_entities_alloc(ECS *ecs, size_t *result_length);

#define ECS_REGISTER_COMPONENT(T, ecs_ptr, destructor) \
    ecs_register_component((ecs_ptr), #T, sizeof(T), destructor)

#define ECS_GET_COMPONENT_DECL(T, var_name, ecs_ptr, entity) \
    T *var_name = ecs_get_component((ecs_ptr), (entity), #T)

#define ECS_GET_COMPONENT_CONST_DECL(T, var_name, ecs_ptr, entity) \
    const T *var_name = ecs_get_component_const((ecs_ptr), (entity), #T)

#define ECS_ADD_COMPONENT_ZEROED_DECL(T, var_name, ecs_ptr, entity) \
    T *var_name = ecs_add_component_zeroed((ecs_ptr), (entity), #T)

#define ECS_ADD_COMPONENT_DEFAULT_DECL(T, var_name, ecs_ptr, entity) \
    T *var_name = ecs_add_component_zeroed((ecs_ptr), (entity), #T); \
    *var_name = T##_default;

#define ECS_REMOVE_COMPONENT(T, ecs_ptr, entity) \
    ecs_remove_component((ecs_ptr), (entity), #T)

#define ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(T, ecs_ptr, out_entity_ptr) \
    ecs_find_first_entity_with_component((ecs_ptr), #T, out_entity_ptr)

#define ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(T, ecs_ptr, result_length) \
    ecs_find_all_entities_with_component_alloc((ecs_ptr), #T, (result_length))

#define ECS_ENSURE_SINGLETON_DECL(T, ecs_ptr, var_name) \
    T *var_name; \
    { \
        Entity entity_; \
        if (!ecs_find_first_entity_with_component((ecs_ptr), #T, &entity_)) { \
            entity_ = ecs_create_entity(ecs_ptr); \
            var_name = ecs_add_component_zeroed((ecs_ptr), entity_, #T); \
            *var_name = T##_default; \
        } else { \
            var_name = ecs_get_component((ecs_ptr), entity_, #T); \
        } \
    }

#define ECS_GET_SINGLETON_DECL(T, ecs_ptr, var_name) \
    T *var_name = NULL; \
    { \
        Entity entity_; \
        if (ecs_find_first_entity_with_component((ecs_ptr), #T, &entity_)) { \
            var_name = ecs_get_component((ecs_ptr), entity_, #T); \
        } \
    }

#ifdef RUN_TESTS
#include "../testing.h"
extern TestResult ecs_test(void);
#endif
