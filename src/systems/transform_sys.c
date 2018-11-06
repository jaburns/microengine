#include "transform_sys.h"
#include "../components.h"

#include <cglm/cglm.h>

struct TransformSystem
{
    uint8_t empty;
};

TransformSystem *transform_sys_new(void)
{
    return NULL;
}

static void Transform_to_matrix(Transform *transform, mat4 matrix)
{
    glm_mat4_identity(matrix);

    glm_translate(matrix, transform->position);

    mat4 rotation;
    glm_quat_mat4(transform->rotation, rotation);
    glm_mat4_mul(matrix, rotation, matrix);

    glm_scale(matrix, transform->scale);
}

void transform_sys_run(TransformSystem *sys, ECS *ecs)
{
    size_t num_transforms;
    Entity *transform_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(Transform, ecs, &num_transforms);

    for (int i = 0; i < num_transforms; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, t, ecs, transform_entities[i]);
        Transform_to_matrix(t, t->worldMatrix_);
        vec_clear(&t->children_);
    }

    for (int i = 0; i < num_transforms; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, t, ecs, transform_entities[i]);
        Entity parent = t->parent;

        if (parent)
        {
            ECS_GET_COMPONENT_DECL(Transform, p, ecs, parent);
            vec_push_copy(&p->children_, &transform_entities[i]);
        }

        while (parent)
        {
            ECS_GET_COMPONENT_DECL(Transform, p, ecs, parent);

            mat4 parent_matrix;
            Transform_to_matrix(p, parent_matrix);

            glm_mat4_mul(parent_matrix, t->worldMatrix_, t->worldMatrix_);

            parent = p->parent;
        }
    }

    free(transform_entities);
}

void transform_sys_delete(TransformSystem *sys)
{
}
