#include "editor_sys.h"

#include <imgui_impl.h>

#include "../ecs.h"
#include "../components.h"

struct EditorSystem
{
    float empty;
};

EditorSystem *editor_sys_new(void)
{
    return malloc(sizeof(EditorSystem));
}

void editor_sys_run(EditorSystem *sys, ECS *ecs)
{
    size_t num_transforms;
    Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(Transform, ecs, &num_transforms);

    igBegin("Transform Editor", NULL, 0);

    for (int i = 0; i < num_transforms; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, t, ecs, entities[i]);

        igPushIDInt(i);
        igDragFloat3("Position", t, 0.005f, -INFINITY, INFINITY, NULL, 1.0f);
        igPopID();
    }

    igEnd();

    free(entities);
}

void editor_sys_delete(EditorSystem *sys)
{
    if (!sys) return;

    free(sys);
}
