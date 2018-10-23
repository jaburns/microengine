#include "editor_sys.h"

#include <imgui_impl.h>

#include "../ecs.h"
#include "../components.h"

struct EditorSystem
{
    bool inspector_open;
    Entity inspecting_entity;
};

EditorSystem *editor_sys_new(void)
{
    EditorSystem *sys = malloc(sizeof(EditorSystem));
    sys->inspector_open = false;
    sys->inspecting_entity = 0;
    return sys;
}

static void inspect_transform_tree(EditorSystem *sys, ECS *ecs, Entity parent_entity, Transform *parent)
{
    if (igTreeNodePtr(parent, "Entity"))
    {
        if (igButton("Inspect", (ImVec2) { 0.f, 0.f })) 
        {
            sys->inspecting_entity = parent_entity;
            sys->inspector_open = true;
        }

        for (int i = 0; i < parent->children_.item_count; ++i)
        {
            Entity e = *(Entity*)vec_at(&parent->children_, i);
            ECS_GET_COMPONENT_DECL(Transform, t, ecs, e);
            inspect_transform_tree(sys, ecs, e, t);
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
        inspect_transform_tree(sys, ecs, entities[i], t);
    }

    igEnd();

    if (sys->inspector_open)
    {
        igBegin("Inspector", &sys->inspector_open, 0);
        icb_inspect_all(sys->inspecting_entity);
        igEnd();
    }

    free(entities);
}

void editor_sys_delete(EditorSystem *sys)
{
    if (!sys) return;

    free(sys);
}