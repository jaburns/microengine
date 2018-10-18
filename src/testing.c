#ifdef RUN_TESTS
#include "testing.h"

#include "vec.h"
#include "hashtable.h"
#include "ecs.h"
#include "ns_clock.h"

int run_all_tests(void)
{
    uint64_t start = ns_clock();

    TEST_RUN(vec_test);
    TEST_RUN(hashtable_test);
    TEST_RUN(ecs_test);

    uint64_t end = ns_clock();
    printf("\nDone! Tests completed in %d us.\n", (end - start) / 1000);
    return 0;
}

#endif
