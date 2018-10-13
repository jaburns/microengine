#pragma once

typedef uint64_t Entity;
typedef struct ECS ECS;

#ifdef RUN_TESTS
#include "testing.h"
extern TEST_RESULT ecs_test();
#endif