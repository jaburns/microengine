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
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, camera);
        *tr = Transform_default;
        tr->position[1] = 1.f;
        tr->position[2] = 5.f;
        ECS_ADD_COMPONENT_DECL(Camera, ca, ecs, camera);
        *ca = Camera_default;
    }

    Entity teapot = ecs_create_entity(ecs);
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, teapot);
        *tr = Transform_default;
        ECS_ADD_COMPONENT_DECL(Teapot, tp, ecs, teapot);
        *tp = Teapot_default;
    }

    Entity teapot2 = ecs_create_entity(ecs);
    {
        ECS_ADD_COMPONENT_DECL(Transform, tr, ecs, teapot2);
        *tr = Transform_default;
        tr->position[1] = 1.f;
        ECS_ADD_COMPONENT_DECL(Teapot, tp, ecs, teapot2);
        *tp = Teapot_default;
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