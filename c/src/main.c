#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_all_tests();
    #endif

    // game?
    getchar();

    return 0;
}