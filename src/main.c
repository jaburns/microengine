#include <stdio.h>

#ifdef RUN_TESTS
#include "testing.h"
#endif

#include "ecs.h"
#include "shell.h"
#include "render_system.h"
#include "components.h"

int main(int argc, char **argv) 
{
    #ifdef RUN_TESTS
        run_all_tests();
    #endif

    ShellContext *ctx = shell_new("Hello world", 1024, 768);
    RenderSystem *rendersystem = rendersystem_new();
    ECS *ecs = ecs_new();

    Entity camera = ecs_create_entity(ecs);
    Entity teapot = ecs_create_entity(ecs);

    {
        ECS_ADD_COMPONENT_DECL(Transform, x, ecs, camera);
        *x = Transform_default;
        x->position[2] = 10.f;
    }{
        ECS_ADD_COMPONENT_DECL(Camera, x, ecs, camera);
        *x = Camera_default;
    }{
        ECS_ADD_COMPONENT_DECL(Transform, x, ecs, teapot);
        *x = Transform_default;
    }{
        ECS_ADD_COMPONENT_DECL(Teapot, x, ecs, teapot);
        *x = Teapot_default;
    }

    do 
    {
        rendersystem_run(rendersystem, ecs);
    } 
    while (shell_flip_frame_poll_events(ctx));

    ecs_delete(ecs);
    rendersystem_delete(rendersystem);
    shell_delete(ctx);

    return 0;
}