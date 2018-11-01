#include "input_sys.h"

#include "../components.h"
#include "../containers/vec.h"

static InputFrame s_latest_inputs;

static void shell_event_callback( float window_width, float window_height, SDL_Event *event )
{
    switch (event->type)
    {
    case SDL_MOUSEMOTION:
        s_latest_inputs.mouse_pos[0] = 2.0f * event->motion.x / window_width - 1.0f;
        s_latest_inputs.mouse_pos[1] = 1.0f - 2.0f * event->motion.y / window_height;
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
            s_latest_inputs.left_mouse = true;
        if (event->button.button == SDL_BUTTON_RIGHT)
            s_latest_inputs.right_mouse = true;
        break;

    case SDL_MOUSEBUTTONUP:
        if (event->button.button == SDL_BUTTON_LEFT)
            s_latest_inputs.left_mouse = false;
        if (event->button.button == SDL_BUTTON_RIGHT)
            s_latest_inputs.right_mouse = false;
        break;

    case SDL_KEYDOWN:
        break;

    case SDL_KEYUP:
        break;
    }
}

InputSystem *input_sys_new( ShellContext *shell )
{
    s_latest_inputs.keys = vec_empty( sizeof( SDL_Keycode ) );
    shell_bind_event_handler( shell, shell_event_callback );
    return NULL;
}

void input_sys_run( InputSystem *sys, ECS *ecs )
{
    Entity inputs_entity;
    if( !ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( InputState, ecs, &inputs_entity ) )
    {
        inputs_entity = ecs_create_entity( ecs );
        ECS_ADD_COMPONENT_DEFAULT_DECL( InputState, inputs_, ecs, inputs_entity );
    }

    ECS_GET_COMPONENT_DECL( InputState, inputs, ecs, inputs_entity );

    // TODO keys down should be a vec
    // implement vec_find_index

    inputs->prev.left_mouse   = inputs->cur.left_mouse;
    inputs->prev.right_mouse  = inputs->cur.right_mouse;
    inputs->prev.mouse_pos[0] = inputs->cur.mouse_pos[0];
    inputs->prev.mouse_pos[1] = inputs->cur.mouse_pos[1];

    inputs->cur.left_mouse   = s_latest_inputs.left_mouse;
    inputs->cur.right_mouse  = s_latest_inputs.right_mouse;
    inputs->cur.mouse_pos[0] = s_latest_inputs.mouse_pos[0];
    inputs->cur.mouse_pos[1] = s_latest_inputs.mouse_pos[1];
}

void input_sys_delete( InputSystem *sys )
{
}
