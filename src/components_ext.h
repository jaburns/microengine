#pragma once

#include "components.h"

static const Teapot Teapot_default = { 0 };
static const Camera Camera_default = { 3.14159f / 2.f, 0.01f, 1024.f };
static const Transform Transform_default = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 1.0f}, {1.f, 1.f, 1.f} };

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