#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef int TEST_RESULT;

#define TEST_BEGIN(message) { int asserts__ = 0; printf("  %s : ", message);
#define TEST_END() printf("%d assertions passed\n", asserts__); }

#define TEST_ASSERT(test) do { if (test) asserts__++; else { printf("failed\n    Assertion failed at %s : %d\n\n", __FILE__, __LINE__); return -1; } } while (0)
#define TEST_RUN(test) do { TEST_RESULT result = test(); if (result) return result; } while (0)