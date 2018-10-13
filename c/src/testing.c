#ifdef RUN_TESTS

#include "testing.h"

#include "vec.h"
#include "ecs.h"
#include "hashtable.h"

void run_all_tests(void)
{
    TEST_RUN(vec_test);
    TEST_RUN(ecs_test);
    TEST_RUN(hashtable_test);
    printf("\nDone!\n");
}

#endif