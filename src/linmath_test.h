#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <linmath.h>
#include "testing.h"

static bool quat_close(quat q, float x, float y, float z, float w)
{
    float dx = q[0] - x;
    float dy = q[1] - y;
    float dz = q[2] - z;
    float dw = q[3] - w;

    return dx*dx + dy*dy + dz*dz + dw*dw < 1e-6;
}

static bool vec3_close(vec3 v, float x, float y, float z)
{
    float dx = v[0] - x;
    float dy = v[1] - y;
    float dz = v[2] - z;

    return dx*dx + dy*dy + dz*dz < 1e-6;
}

static TestResult linmat_test( void )
{
    TEST_BEGIN("Quaternion multiplication works");

        quat a = {  1, -2, 3,  4 };
        quat b = { -5,  6, 7, -8 };
        quat r;

        quat_mul(r, a, b);

        TEST_ASSERT(quat_close(r, -60.f, 18.f, 0.f, -36.f));

    TEST_END();
    TEST_BEGIN("Quaternion mul vec3 works");

        quat q = { -0.6009133f, 0.3675318f, 0.06183508f, 0.7071068f };
        vec3 v = { -0.7591431f, -0.3433556f, 0.5529997f };
        vec3 r;

        quat_mul_vec3(r, q, v);

        TEST_ASSERT(vec3_close(r, -0.1202234f, 0.67126f, 0.7314072f));

    TEST_END();
    return 0;
}
