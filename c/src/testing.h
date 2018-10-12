#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef int TEST_RESULT;

#define TEST_BEGIN(message) do { printf("  %s : ", message); } while(0)
#define TEST_END() do { printf("passed\n"); } while(0)

#define TEST_ASSERT(test) do { if (!(test)) { printf("failed\n    Assertion failed at %s : %d\n\n", __FILE__, __LINE__); return -1; } } while (0)
#define TEST_RUN(test) do { TEST_RESULT result = test(); if (result) return result; } while (0)