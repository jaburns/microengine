#pragma once

#include <stdio.h>
#include <stdbool.h>

typedef int TestResult;

#define TEST_BEGIN(message) \
do { \
    int asserts__ = 0; \
    printf("  %s : ", message);

#define TEST_END() \
    printf("%d assertions passed\n", asserts__); \
} while (0)

#define TEST_ASSERT(test) do { \
    if (test) { \
        asserts__++; \
    } else { \
        printf("failed\n    Assertion failed at %s : %d\n\n", __FILE__, __LINE__); \
        return -1; \
    } \
} while (0)

#define TEST_RUN(test) do { \
    printf("\nRunning test \"%s\"...\n", #test); \
    TestResult result = test(); \
    if (result) return result; \
} while (0)

extern void run_all_tests(void);