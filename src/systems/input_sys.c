#include "input_sys.h"

#include "../components.h"
#include "../containers/vec.h"

InputSystem *input_sys_new( void )
{
    return NULL;
}

void input_sys_run( InputSystem *sys, ECS *ecs, ShellContext *shell )
{
    const ShellInputs *shell_inputs = shell_view_input_state( shell );

    Entity inputs_entity;
    if( !ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( InputState, ecs, &inputs_entity ) )
    {
        inputs_entity = ecs_create_entity( ecs );
        ECS_ADD_COMPONENT_DEFAULT_DECL( InputState, inputs_, ecs, inputs_entity );
    }

    ECS_GET_COMPONENT_DECL( InputState, inputs, ecs, inputs_entity );

    // TODO keys down should be a vec
    // implement vec_find_index
    // have this system bind directly to the sdl input event stream instead of having the ShellInputs DTO

    inputs->prev.left_mouse   = inputs->cur.left_mouse;
    inputs->prev.right_mouse  = inputs->cur.right_mouse;
    inputs->prev.mouse_pos[0] = inputs->cur.mouse_pos[0];
    inputs->prev.mouse_pos[1] = inputs->cur.mouse_pos[1];
    vec_clear( &inputs->prev.keys );

    inputs->cur.left_mouse = shell_inputs->left_mouse;
    inputs->cur.right_mouse = shell_inputs->right_mouse;
    inputs->cur.mouse_pos[0] = shell_inputs->mouse_position[0];
    inputs->cur.mouse_pos[1] = shell_inputs->mouse_position[1];
    vec_clear( &inputs->cur.keys );
}

void input_sys_delete( InputSystem *sys )
{
}
