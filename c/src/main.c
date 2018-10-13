#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

#include "shell.h"
#include "render_system.h"

int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_all_tests();
    #endif

    ShellContext *ctx = shell_new("Hello world", 1024, 768);
    RenderSystem *rendersystem = rendersystem_new();

    do 
    {
        // game?
    } 
    while (shell_flip_frame_poll_events(ctx));

    rendersystem_delete(rendersystem);
    shell_delete(ctx);

    return 0;
}