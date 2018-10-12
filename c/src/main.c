#include <stdio.h>
#include "vec.h"
#include "ecs.h"

#ifdef RUN_TESTS
#include "testing.h"

static void run_tests()
{
    printf("Running tests...\n");
    TEST_RUN(vec_test);
    TEST_RUN(ecs_test);
}
#endif

int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_tests();
    #endif

    // game?

    return 0;
}