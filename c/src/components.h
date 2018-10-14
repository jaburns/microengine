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

static const Transform Transform_default = { {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.1f}, {1.f, 1.f, 1.f} };

typedef struct Teapot
{
    uint8_t nothing;
}
Teapot;

static const Teapot Teapot_default = { 0 };

typedef struct Camera
{
    float fov; 
    float near;
    float far;
}
Camera;

static const Camera Camera_default = { 3.14159f / 2.f, 0.01f, 1024.f };