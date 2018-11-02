#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef int TestResult;

//#define FAST_TESTS 1
#ifdef FAST_TESTS
    #define TEST_PRINT()
#else
    #include <stdio.h>
    #define TEST_PRINT printf
#endif

#define TEST_BEGIN(message) \
do { \
    int asserts__ = 0; \
    TEST_PRINT("  %s : ", message);

#define TEST_END() \
    TEST_PRINT("%d assertions passed\n", asserts__); \
} while (0)

#define TEST_ASSERT(test) do { \
    if (test) { \
        asserts__++; \
    } else { \
        TEST_PRINT("failed\n    Assertion failed at %s : %d\n\n", __FILE__, __LINE__); \
        return -1; \
    } \
} while (0)

#define TEST_RUN(test) do { \
    TEST_PRINT("\nRunning test \"%s\"...\n", #test); \
    TestResult result = test(); \
    if (result) return result; \
} while (0)

extern int run_all_tests(void);
