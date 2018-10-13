#pragma once

#include <stdint.h>

typedef uint64_t Entity;
typedef struct ECS ECS;

#ifdef RUN_TESTS
#include "testing.h"
extern TestResult ecs_test();
#endif