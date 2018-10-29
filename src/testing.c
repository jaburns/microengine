#ifdef RUN_TESTS
#include "testing.h"

#include <ns_clock.h>

#include "containers/vec.h"
#include "containers/hashtable.h"
#include "containers/ecs.h"
#include "containers/hashcache.h"
#include "linmath_test.h"

int run_all_tests(void)
{
    uint64_t start = ns_clock();

    TEST_RUN(linmat_test);
    // TEST_RUN(vec_test);
    // TEST_RUN(hashtable_test);
    // TEST_RUN(ecs_test);
    // TEST_RUN(hashcache_test);

    uint64_t end = ns_clock();
    printf("\nDone! Tests completed in %d us.\n", (end - start) / 1000);
    return 0;
}

#endif
