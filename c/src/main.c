#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

#include "shell.h"

int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_all_tests();
    #endif

    shell_open("Hello world", 1024, 768);
    while (shell_flip_frame_poll_events()) { }
    shell_close();

    return 0;
}