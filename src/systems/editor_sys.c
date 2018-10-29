#include "editor_sys.h"

#include <string.h>
#include <imgui_impl.h>
#include <math.h>

#include <cglm/cglm.h>

#include "../containers/ecs.h"
#include "../components.h"

struct EditorSystem
{
    Entity inspecting_entity;

    Entity active_camera;
    bool view_dragging;
    vec2 last_mouse_pos;
    float camera_pitch;
    float camera_yaw;
};

EditorSystem *editor_sys_new( void )
{
    EditorSystem *sys = malloc( sizeof( EditorSystem ) );
    sys->inspecting_entity = 0;
    sys->active_camera = 0;
    sys->view_dragging = false;
    sys->camera_pitch = 0;
    sys->camera_yaw = 0;
    sys->last_mouse_pos[0] = 0;
    sys->last_mouse_pos[1] = 0;
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

int i = 0;

static void update_view_drag( EditorSystem *sys, const ShellInputs *inputs, Camera *cam, Transform *transform )
{
    sys->view_dragging = inputs->right_mouse;

    float dx = inputs->mouse_position[0] - sys->last_mouse_pos[0];
    float dy = inputs->mouse_position[1] - sys->last_mouse_pos[1];

    sys->last_mouse_pos[0] = inputs->mouse_position[0];
    sys->last_mouse_pos[1] = inputs->mouse_position[1];

    if( sys->view_dragging )
    {
        sys->camera_yaw   -= 3 * dx;
        sys->camera_pitch -= 3 * dy;
    }

    cam->x = sys->camera_yaw;
    cam->y = sys->camera_pitch;

//  vec3 look_dir = {
//      cosf(sys->camera_yaw), // - 3.14159265f / 2.0f),
//      sys->camera_pitch,
//      sinf(sys->camera_yaw), //  - 3.14159265f / 2.0f)
//  };

//  glm_normalize(look_dir);

//  printf("%f, %f, %f\n", look_dir[0], look_dir[1], look_dir[2]);

//  glm_quat_for( look_dir, (vec3){ 0, 0, -1 }, (vec3){ 1, 0, 0 }, transform->rotation );

    return;

    /*
    printf("%f\t%f\t%f\t%f\t\n", dx, dy, sys->camera_yaw, sys->last_mouse_pos[0]);


    vec3_norm( look_dir, look_dir );

    i = (i + 1) % 2;

    mat4x4 look_mat;
    mat4x4_look_at(look_mat, (vec3){0,0,0}, look_dir, (vec3){0,1,0});
    mat4x4 dorp;

    mat4x4_invert( dorp, look_mat );

    if (i)
        mat4x4_dup( transform->worldMatrix_, dorp );
    else
        quat_from_mat4x4( transform->rotation, dorp );
*/
}

void editor_sys_run( EditorSystem *sys, ECS *ecs, const ShellInputs *inputs )
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
    free( entities );

    if( sys->inspecting_entity )
    {
        bool keep_open = true;
        igBegin( "Inspector", &keep_open, 0 );
        components_inspect_entity( sys->inspecting_entity );
        igEnd();

        if( !keep_open ) sys->inspecting_entity = 0;
    }

    if( igGetIO()->WantCaptureMouse )
    {
        sys->view_dragging = false;
    }
    else
    {
        ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( Camera, ecs, &sys->active_camera );
        ECS_GET_COMPONENT_DECL( Camera, active_cam, ecs, sys->active_camera );
        ECS_GET_COMPONENT_DECL( Transform, active_cam_transform, ecs, sys->active_camera );

        update_view_drag( sys, inputs, active_cam, active_cam_transform );
    }
}

void editor_sys_delete( EditorSystem *sys )
{
    if( !sys ) return;

    free( sys );
}