#include "transform_sys.h"
#include "../components.h"

struct TransformSystem
{
    uint8_t empty;
};

extern TransformSystem *transform_sys_new(void)
{
    return NULL;
}

static void Transform_to_matrix(Transform *transform, mat4x4 matrix)
{
    mat4x4_identity(matrix);
    mat4x4 m;

    mat4x4_translate(m, transform->position[0], transform->position[1], transform->position[2]);
    mat4x4_mul(matrix, matrix, m);

    mat4x4_from_quat(m, transform->rotation);
    mat4x4_mul(matrix, matrix, m);

    mat4x4_scale_aniso(m, m, transform->scale[0], transform->scale[1], transform->scale[2]);
    mat4x4_mul(matrix, matrix, m);
}

extern void transform_sys_run(TransformSystem *sys, ECS *ecs)
{
    size_t num_transforms;
    Entity *transform_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(Transform, ecs, &num_transforms);

    for (int i = 0; i < num_transforms; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, t, ecs, transform_entities[i]);
        Transform_to_matrix(t, t->worldMatrix_);

        vec_clear(&t->children_);
        Entity parent = t->parent;

        while (parent)
        {
            ECS_GET_COMPONENT_DECL(Transform, p, ecs, parent);

            mat4x4 parent_matrix;
            Transform_to_matrix(p, parent_matrix);

            mat4x4_mul(t->worldMatrix_, parent_matrix, t->worldMatrix_);

            vec_push_copy(&p->children_, &transform_entities[i]);

            parent = p->parent;
        }
    }

    free(transform_entities);
}


extern void transform_sys_delete(TransformSystem *sys)
{
}