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
    bool hierarchy_reset;
    bool hierarchy_open;
    bool fps_reset;
    bool fps_open;

    ECS *pre_play_ecs;

    Entity selected_entity;
    Entity reparenting_entity;
};

EditorSystem *editor_sys_new( void )
{
    EditorSystem *sys = malloc( sizeof( EditorSystem ) );
    sys->selected_entity = 0;
    sys->reparenting_entity = 0;
    sys->hierarchy_open = false;
    sys->hierarchy_reset = false;
    sys->fps_open = false;
    sys->fps_reset = false;
    sys->pre_play_ecs = NULL;
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
        VEC_FOREACH( Entity, &transform->children_ )
        {
            ECS_GET_COMPONENT_DECL( Transform, t, ecs, *iter.item );
            inspect_transform_tree( sys, ecs, *iter.item, t );
        }

        igTreePop();
    }
}

static bool find_sdl_key_index( SDL_Keycode *a, SDL_Keycode *b )
{
    return *a == *b;
}

static void update_view_drag( EditorSystem *sys, InputState *inputs, Camera *cam, Transform *transform, float delta_millis )
{
// Camera rotation

    if( inputs->cur.right_mouse )
    {
        float dx = 3 * (inputs->cur.mouse_pos[0] - inputs->prev.mouse_pos[0]);
        float dy = 3 * (inputs->cur.mouse_pos[1] - inputs->prev.mouse_pos[1]);

        versor yaw;
        glm_quatv( yaw, dx, (vec3){ 0, 1, 0 });
        glm_quat_mul( yaw, transform->rotation, transform->rotation );

        vec3 pitch_axis;
        versor pitch;
        glm_quat_rotatev( transform->rotation, (vec3){ 1, 0, 0 }, pitch_axis );
        glm_quatv( pitch, -dy, pitch_axis );
        glm_quat_mul( pitch, transform->rotation, transform->rotation );
    }

// Camera movement

    vec3 fwd, up, right, drive;
    glm_quat_rotatev( transform->rotation, (vec3){ 0.f, 0.f, 1.f }, fwd );
    glm_quat_rotatev( transform->rotation, (vec3){ 0.f, 1.f, 0.f }, up );
    glm_quat_rotatev( transform->rotation, (vec3){ 1.f, 0.f, 0.f }, right );
    glm_vec_zero( drive );

    #define X(pos_key, neg_key, pos_vec) do { \
        SDL_Keycode p = pos_key; \
        SDL_Keycode n = neg_key; \
        if( vec_find_index( &inputs->cur.keys, &p, find_sdl_key_index ) >= 0 ) \
        { \
            glm_vec_add( pos_vec, drive, drive ); \
        } \
        else if( vec_find_index( &inputs->cur.keys, &n, find_sdl_key_index ) >= 0 ) \
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

static bool find_editor_camera( ECS *ecs, Camera **out_camera, Transform **out_transform )
{
    bool result = false;
    size_t num_cameras;
    Entity *camera_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( Camera, ecs, &num_cameras );

    if( num_cameras == 0 ) goto exit;

    for( int i = 0; i < num_cameras; ++i )
    {
        ECS_GET_COMPONENT_DECL( Camera, camera, ecs, camera_entities[i] );
        ECS_GET_COMPONENT_DECL( Transform, transform, ecs, camera_entities[i] );

        if( transform && camera->is_editor )
        {
            *out_camera = camera;
            *out_transform = transform;
            result = true;
            break;
        }
    }

exit:
    free( camera_entities );
    return result;
}


EditorSystemUpdateResult editor_sys_run( EditorSystem *sys, ECS *ecs )
{
    ECS *maybe_new_ecs = NULL;

    igBeginMainMenuBar();

        if( igBeginMenu( "Scene", true ) )
        {
            if( igMenuItemBool( "Show Hierarchy", NULL, false, true ) )
            {
                if( sys->hierarchy_open )
                    sys->hierarchy_reset = true;
                sys->hierarchy_open = true;
            }

            if( igMenuItemBool( "Show FPS", NULL, false, true ) )
            {
                if( sys->fps_open )
                    sys->fps_reset = true;
                sys->fps_open = true;
            }

            if( igMenuItemBool( "Save Scene", NULL, false, true ) )
            {
                char *json_scene = components_serialize_scene_alloc();
                utils_write_string_file( "resources/scenes/saved.jscene", json_scene );
                free( json_scene );
            }

            if( igMenuItemBool( "Load Scene", NULL, false, true ) )
            {
                char *json_scene = utils_read_file_alloc( "", "resources/scenes/saved.jscene", NULL );
                maybe_new_ecs = components_deserialize_scene_alloc( json_scene );
                free( json_scene );
            }

            igEndMenu();
        }

        if( igBeginMenu( "Game", true ) )
        {
            if( sys->pre_play_ecs )
            {
                if( igMenuItemBool( "Pause", NULL, false, true ) ) {}
                if( igMenuItemBool( "Stop", NULL, false, true ) )
                {
                    maybe_new_ecs = sys->pre_play_ecs;
                    sys->pre_play_ecs = NULL;
                }
            }
            else
            {
                if( igMenuItemBool( "Play", NULL, false, true ) )
                {
                    char *json_scene = components_serialize_scene_alloc();
                    sys->pre_play_ecs = components_deserialize_scene_alloc( json_scene );
                    free( json_scene );
                }
            }

            igEndMenu();
        }

    igEndMainMenuBar();

    Entity clock_entity;
    ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( ClockInfo, ecs, &clock_entity );
    ECS_GET_COMPONENT_DECL( ClockInfo, clock, ecs, clock_entity );
    size_t num_entities;
    Entity *entities = ecs_find_all_entities_alloc( ecs, &num_entities );

    if( sys->fps_open )
    {
        if( sys->fps_reset )
        {
            igSetNextWindowPos( (ImVec2){ 0.f, 20.f }, 0, (ImVec2){ 0.f, 0.f } );
            sys->fps_reset = false;
        }

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoNav;

        igBegin( "", &sys->fps_open, flags );
            igText( "%.1f fps", 1000.f / clock->delta_millis );
        igEnd();
    }

    if( sys->hierarchy_open )
    {
        if( sys->hierarchy_reset )
        {
            igSetNextWindowPos( (ImVec2){ 0.f, 20.f }, 0, (ImVec2){ 0.f, 0.f } );
            sys->hierarchy_reset = false;
        }

        igBegin( "Hierarchy", &sys->hierarchy_open, 0 );

        if( igButton( "Create", (ImVec2){ 0, 0 } ) )
            ecs_create_entity( ecs );

        igSameLine( 0, -1 );

        if( sys->reparenting_entity )
        {
            if( igButton( "[Reparent]", (ImVec2){ 0, 0 } ) ) 
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

        if( components_entity_to_change )
        {
            igSameLine( 0, -1 );
            if( igButton( "Assign", (ImVec2){ 0, 0 } ) )
            {
                *components_entity_to_change = sys->selected_entity;
                components_entity_to_change = NULL;
            }
        }

        igSeparator();

        for( int i = 0; i < num_entities; ++i )
        {
            ECS_GET_COMPONENT_DECL( Transform, t, ecs, entities[i] );
            if( t && t->parent ) continue;
            inspect_transform_tree( sys, ecs, entities[i], t );
        }

        igEnd();
    }

    if( sys->selected_entity )
    {
        bool keep_open = true;
        igBegin( "Inspector", &keep_open, 0 );
            components_inspect_entity( sys->selected_entity );
        igEnd();
        if( !keep_open ) sys->selected_entity = 0;
    }

    free( entities );

    if( !sys->pre_play_ecs )
    {
        Camera *camera;
        Transform *camera_transform;
        if( find_editor_camera( ecs, &camera, &camera_transform ) )
        {
            Entity inputs_entity;
            ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( InputState, ecs, &inputs_entity );
            ECS_GET_COMPONENT_DECL( InputState, inputs, ecs, inputs_entity );

            if( inputs )
                update_view_drag( sys, inputs, camera, camera_transform, clock->delta_millis );
        }
    }

    return (EditorSystemUpdateResult) {
        .new_ecs = maybe_new_ecs,
        .in_play_mode = sys->pre_play_ecs != NULL
    };
}

void editor_sys_delete( EditorSystem *sys )
{
    if( !sys ) return;

    free( sys );
}
