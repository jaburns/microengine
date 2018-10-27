#include "editor_sys.h"

#include <string.h>
#include <imgui_impl.h>

#include "../containers/ecs.h"
#include "../components.h"

struct EditorSystem
{
    Entity inspecting_entity;
};

EditorSystem *editor_sys_new( void )
{
    EditorSystem *sys = malloc( sizeof( EditorSystem ) );
    sys->inspecting_entity = 0;
    return sys;
}

static void inspect_transform_tree( EditorSystem *sys, ECS *ecs, Entity parent_entity, Transform *parent )
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if( sys->inspecting_entity == parent_entity )
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if( parent->children_.item_count == 0 )
        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    const char *name = parent->name && strlen( parent->name ) ? parent->name : "( entity )";
    bool node_open = igTreeNodeExPtr( parent, node_flags, name );

    if( igIsItemClicked( 0 ) )
        sys->inspecting_entity = parent_entity;

    if( parent->children_.item_count > 0 && node_open )
    {
        for( int i = 0; i < parent->children_.item_count; ++i )
        {
            Entity e = *( Entity* )vec_at( &parent->children_, i );
            ECS_GET_COMPONENT_DECL( Transform, t, ecs, e );
            inspect_transform_tree( sys, ecs, e, t );
        }

        igTreePop();
    }
}

void editor_sys_run( EditorSystem *sys, ECS *ecs )
{
    size_t num_transforms;
    Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( Transform, ecs, &num_transforms );

    igBegin( "Scene", NULL, 0 );

    for( int i = 0; i < num_transforms; ++i )
    {
        ECS_GET_COMPONENT_DECL( Transform, t, ecs, entities[i] );

        if( t->parent ) continue;
        inspect_transform_tree( sys, ecs, entities[i], t );
    }

    igEnd();

    if( sys->inspecting_entity )
    {
        bool keep_open = true;
        igBegin( "Inspector", &keep_open, 0 );
        components_inspect_entity( sys->inspecting_entity );
        igEnd();

        if( !keep_open ) sys->inspecting_entity = 0;
    }

    free( entities );
}

void editor_sys_delete( EditorSystem *sys )
{
    if( !sys ) return;

    free( sys );
}