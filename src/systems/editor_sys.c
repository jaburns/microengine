#include "editor_sys.h"

#include <string.h>
#include <imgui_impl.h>
#include <math.h>

#include <cglm/cglm.h>

#include "../utils.h"
#include "../containers/ecs.h"
#include "../containers/hashtable.h"
#include "../components.h"

struct EditorSystem
{
    Entity selected_entity;
    Entity reparenting_entity;

    Entity active_camera;
    float camera_pitch;
    float camera_yaw;
};

EditorSystem *editor_sys_new( void )
{
    EditorSystem *sys = malloc( sizeof( EditorSystem ) );
    sys->selected_entity = 0;
    sys->reparenting_entity = 0;
    sys->active_camera = 0;
    sys->camera_pitch = 0;
    sys->camera_yaw = 0;
    return sys;
}

static void inspect_transform_tree( EditorSystem *sys, ECS *ecs, Entity entity, Transform *transform )
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if( sys->selected_entity == entity )
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if( !transform || transform->children_.item_count == 0 )
        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    bool node_open = igTreeNodeExPtr( (void*)entity, node_flags, components_name_entity( entity ) );

    if( igIsItemClicked( 0 ) )
        sys->selected_entity = entity;

    if( transform && transform->children_.item_count > 0 && node_open )
    {
        for( int i = 0; i < transform->children_.item_count; ++i )
        {
            Entity e = *(Entity*)vec_at( &transform->children_, i );
            ECS_GET_COMPONENT_DECL( Transform, t, ecs, e );
            inspect_transform_tree( sys, ecs, e, t );
        }

        igTreePop();
    }
}

// TODO delta_millis should be available on a component along with total time elapsed and other time info.

static void update_view_drag( EditorSystem *sys, const InputState *inputs, Camera *cam, Transform *transform, float delta_millis )
{
    if( inputs->cur.right_mouse )
    {
        float dx = inputs->cur.mouse_pos[0] - inputs->prev.mouse_pos[0];
        float dy = inputs->cur.mouse_pos[1] - inputs->prev.mouse_pos[1];

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

//  #define X(pos_key, neg_key, pos_vec) do { \
//      if( hashtable_at_i( &inputs->cur.keys, pos_key ) ) \
//      { \
//          glm_vec_add( pos_vec, drive, drive ); \
//      } \
//      else if( hashtable_at_i( &inputs->cur.keys, neg_key ) ) \
//      { \
//          glm_vec_flipsign( pos_vec ); \
//          glm_vec_add( pos_vec, drive, drive ); \
//      } \
//  } while (0)

//      X( SDLK_w, SDLK_s, fwd );
//      X( SDLK_d, SDLK_a, right );
//      X( SDLK_e, SDLK_q, up );

//  #undef X

    glm_vec_scale( drive, 0.02f * delta_millis, drive );
    glm_vec_add( transform->position, drive, transform->position );
}

static void reparent_entity( ECS *ecs, Entity this_entity, Entity to_entity )
{
    if( !this_entity || this_entity == to_entity ) return;

    ECS_GET_COMPONENT_DECL( Transform, this_t, ecs, this_entity );
    if( !this_t ) return;

    if( !to_entity )
    {
        this_t->parent = 0;
        return;
    }

    ECS_GET_COMPONENT_DECL( Transform, to_t, ecs, to_entity );
    if( !to_t ) return;

    Entity parent = to_t->parent;
    while( parent )
    {
        if( parent == this_entity ) return;
        ECS_GET_COMPONENT_DECL( Transform, parent_t, ecs, parent );
        parent = parent_t->parent;
    }

    this_t->parent = to_entity;
}

void editor_sys_run( EditorSystem *sys, ECS *ecs, float delta_millis )
{
    size_t num_entities;
    Entity *entities = ecs_find_all_entities_alloc( ecs, &num_entities );

    igBegin( "Scene", NULL, 0 );

    igText( "%.1f fps", 1000.f / delta_millis );

    if( igButton( "Create", (ImVec2){ 0, 0 } ) )
        ecs_create_entity( ecs );

    igSameLine( 0, -1 );

    if( sys->reparenting_entity )
    {
        const char *name = components_name_entity( sys->reparenting_entity );
        if( igButton( name, (ImVec2){ 0, 0 } ) ) 
        {
            reparent_entity( ecs, sys->reparenting_entity, sys->selected_entity );
            sys->reparenting_entity = 0;
        }
    }
    else if( igButton( "Reparent", (ImVec2){ 0, 0 } ) )
    {
        sys->reparenting_entity = sys->selected_entity;
    }

    igSameLine( 0, -1 );

    if( igButton( "No Parent", (ImVec2){ 0, 0 } ) )
    {
        reparent_entity( ecs, sys->selected_entity, 0 );
        sys->reparenting_entity = 0;
    }

    igSameLine( 0, -1 );

    if( igButton( "Delete", (ImVec2){ 0, 0 } ) )
    {
        ecs_destroy_entity( ecs, sys->selected_entity );
        sys->reparenting_entity = 0;
        sys->selected_entity = 0;
    }

    igSeparator();

    for( int i = 0; i < num_entities; ++i )
    {
        ECS_GET_COMPONENT_DECL( Transform, t, ecs, entities[i] );
        if( t && t->parent ) continue;
        inspect_transform_tree( sys, ecs, entities[i], t );
    }

    igEnd();
    free( entities );

    if( sys->selected_entity )
    {
        bool keep_open = true;
        igBegin( "Inspector", &keep_open, 0 );
        components_inspect_entity( sys->selected_entity );
        igEnd();

        if( !keep_open ) sys->selected_entity = 0;
    }

    Entity inputs_entity;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( InputState, ecs, &inputs_entity );
    ECS_GET_COMPONENT_DECL( InputState, inputs, ecs, inputs_entity );

    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( Camera, ecs, &sys->active_camera );
    ECS_GET_COMPONENT_DECL( Camera, active_cam, ecs, sys->active_camera );
    ECS_GET_COMPONENT_DECL( Transform, active_cam_transform, ecs, sys->active_camera );

    update_view_drag( sys, inputs, active_cam, active_cam_transform, delta_millis );
}

void editor_sys_delete( EditorSystem *sys )
{
    if( !sys ) return;

    free( sys );
}
