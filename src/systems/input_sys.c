#include "input_sys.h"

#include "../component_defs.h"
#include "../containers/vec.h"

static InputFrame s_latest_inputs;

static bool compare_sdl_key_codes( const SDL_Keycode *a, const SDL_Keycode *b )
{
    return *a == *b;
}

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
        if( vec_find_index( &s_latest_inputs.keys, &event->key.keysym.sym, compare_sdl_key_codes ) < 0 )
            vec_push_copy( &s_latest_inputs.keys, &event->key.keysym.sym );
        break;

    case SDL_KEYUP: {}
        int index = vec_find_index( &s_latest_inputs.keys, &event->key.keysym.sym, compare_sdl_key_codes );
        if( index >= 0 )
            vec_remove( &s_latest_inputs.keys, index );
        break;
    }
}

InputSystem *input_sys_new( ShellContext *shell )
{
    s_latest_inputs.keys = vec_empty( sizeof( SDL_Keycode ) );
    s_latest_inputs.has_gamepad = false;
    s_latest_inputs.gamepad.buttons = vec_empty( sizeof( SDL_GameControllerButton ) );

    shell_bind_event_handler( shell, shell_event_callback );

    return NULL;
}

static float read_axis( int16_t val )
{
    return (float)val / 32767.f;
}

static void read_gamepad( SDL_GameController *controller, GamepadInputFrame *result )
{
    vec_clear( &result->buttons );

    for( int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i )
        if( SDL_GameControllerGetButton( controller, i ) )
            vec_push_copy( &result->buttons, &i );

    result-> left_stick[0] = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTX ) );
    result-> left_stick[1] = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTY ) );
    result->right_stick[0] = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTX ) );
    result->right_stick[1] = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTY ) );
}

void input_sys_run( InputSystem *sys, ECS *ecs, SDL_GameController *controller )
{
    if( controller )
    {
        s_latest_inputs.has_gamepad = true;
        read_gamepad( controller, &s_latest_inputs.gamepad );
    }
    else 
    {
        s_latest_inputs.has_gamepad = false;
    }

    ECS_ENSURE_SINGLETON_DECL( InputState, ecs, inputs );

    vec_clear( &inputs->prev.keys );
    vec_clear( &inputs->prev.gamepad.buttons );
    inputs->prev = inputs->cur;
    
    inputs->cur = s_latest_inputs;
    inputs->cur.keys = vec_clone( &s_latest_inputs.keys );
    inputs->cur.gamepad.buttons = vec_clone( &s_latest_inputs.gamepad.buttons );
}

void input_sys_delete( InputSystem *sys )
{
}

static bool find_sdl_key_index( const SDL_Keycode *a, const SDL_Keycode *b )
{
    return *a == *b;
}

bool input_state_is_key_down( const InputState *inputs, SDL_Keycode key )
{
    return vec_find_index( &inputs->cur.keys, &key, find_sdl_key_index ) >= 0;
}
