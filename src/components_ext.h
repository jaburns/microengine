#pragma once

#include "components.h"

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