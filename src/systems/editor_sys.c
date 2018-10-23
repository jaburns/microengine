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

static void inspect_transform_tree(ECS *ecs, Transform *parent)
{
    if (igTreeNodePtr(parent, "Transform"))
    {
        icb_inspect_Transform(parent);

        for (int i = 0; i < parent->children_.item_count; ++i)
        {
            ECS_GET_COMPONENT_DECL(Transform, t, ecs, *(Entity*)vec_at(&parent->children_, i));
            inspect_transform_tree(ecs, t);
        }

        igTreePop();
    }
}

void editor_sys_run(EditorSystem *sys, ECS *ecs)
{
    size_t num_transforms;
    Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(Transform, ecs, &num_transforms);

    igBegin("Scene", NULL, 0);

    for (int i = 0; i < num_transforms; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, t, ecs, entities[i]);

        if (t->parent) continue;
        inspect_transform_tree(ecs, t);
    }

    igEnd();

    free(entities);
}

void editor_sys_delete(EditorSystem *sys)
{
    if (!sys) return;

    free(sys);
}
