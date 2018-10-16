// Generated
#pragma once
#include <linmath.h>
#include <stdint.h>

typedef struct Transform
{
    vec3 position;
    quat rotation;
    vec3 scale;
}
Transform;

typedef struct Teapot
{
    float nothing;
}
Teapot;

typedef struct Camera
{
    float fov;
    float near;
    float far;
}
Camera;