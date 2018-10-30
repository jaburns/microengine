#include "editor_sys.h"

#include <string.h>
#include <imgui_impl.h>
#include <math.h>

#include <cglm/cglm.h>

#include "../containers/ecs.h"
#include "../containers/hashtable.h"
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

static void update_view_drag( EditorSystem *sys, const ShellInputs *inputs, Camera *cam, Transform *transform, float delta_millis )
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

    vec3 look_dir = { 
        cosf(sys->camera_yaw), 
        sys->camera_pitch,
        sinf(sys->camera_yaw) 
    };

    mat4 tmp;
    glm_lookat( (vec3){ 0.f, 0.f, 0.f }, look_dir, (vec3){ 0.f, 1.f, 0.f }, tmp );
    glm_mat4_inv( tmp, tmp );
    glm_mat4_quat( tmp, transform->rotation );

    vec3 fwd, up, right, drive;
    glm_quat_rotatev( transform->rotation, (vec3){ 0.f, 0.f, 1.f }, fwd );
    glm_quat_rotatev( transform->rotation, (vec3){ 0.f, 1.f, 0.f }, up );
    glm_quat_rotatev( transform->rotation, (vec3){ 1.f, 0.f, 0.f }, right );
    glm_vec_zero( drive );

    #define X(pos_key, neg_key, pos_vec) do { \
        if( hashtable_at_i( &inputs->keys_down, pos_key ) ) \
        { \
            glm_vec_add( pos_vec, drive, drive ); \
        } \
        else if( hashtable_at_i( &inputs->keys_down, neg_key ) ) \
        { \
            glm_vec_flipsign( pos_vec ); \
            glm_vec_add( pos_vec, drive, drive ); \
        } \
    } while (0)

        X( SDLK_w, SDLK_s, fwd );
        X( SDLK_d, SDLK_a, right );
        X( SDLK_e, SDLK_q, up );

    #undef X

    glm_vec_scale( drive, 0.02f * delta_millis, drive );
    glm_vec_add( transform->position, drive, transform->position );
}

void editor_sys_run( EditorSystem *sys, ECS *ecs, const ShellInputs *inputs, float delta_millis )
{
    size_t num_transforms;
    Entity *entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( Transform, ecs, &num_transforms );

    igBegin( "Scene", NULL, 0 );
    igText( "%.1f fps", 1000.f / delta_millis );
    igSeparator();

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

        update_view_drag( sys, inputs, active_cam, active_cam_transform, delta_millis );
    }
}

void editor_sys_delete( EditorSystem *sys )
{
    if( !sys ) return;

    free( sys );
}