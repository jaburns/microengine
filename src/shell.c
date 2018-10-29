#include "shell.h"

#include <string.h>
#include <imgui_impl.h>

#include "gl.h"
#include "utils.h"

struct ShellContext
{
    SDL_Window *sdl_window;
    SDL_GLContext sdl_gl_context;
    int window_width;
    int window_height;
    ShellInputs input_state;
};

ShellContext *shell_new(const char *title, int width, int height)
{
    ShellContext *context = malloc(sizeof(ShellContext));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) goto err;

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    context->window_width = width;
    context->window_height = height;

    context->sdl_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!context->sdl_window) goto err;

    context->sdl_gl_context = SDL_GL_CreateContext(context->sdl_window);

    if (!context->sdl_gl_context) goto err;

    #ifndef __APPLE__
        glewExperimental = GL_TRUE;
        const GLenum glewInitResult = glewInit();
        if (glewInitResult != GLEW_OK) goto err;
    #endif

    igCreateContext(NULL);
    ImGuiIO *io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplSdlGL3_Init(context->sdl_window, NULL);
    ImGui_ImplSdlGL3_NewFrame(context->sdl_window);

    context->input_state.keys_down = hashtable_empty(256, sizeof(bool), sizeof(SDL_Keycode));
    context->input_state.left_mouse = false;
    context->input_state.right_mouse = false;

    return context;

err:
    SDL_GL_DeleteContext(context->sdl_gl_context);
    SDL_DestroyWindow(context->sdl_window);
    SDL_Quit();
    free(context);

    return NULL;
}

static void update_keys_input_state(ShellInputs *state, const SDL_Event *event)
{
    if (event->type == SDL_KEYDOWN)
    {
        bool yes = true;
        hashtable_set_copy_i( &state->keys_down, event->key.keysym.sym, &yes );
    }
    else if (event->type == SDL_KEYUP)
    {
        hashtable_remove_i( &state->keys_down, event->key.keysym.sym );
    }
}

float shell_get_aspect(ShellContext *context)
{
    return (float)context->window_width / (float)context->window_height;
}

bool shell_flip_frame_poll_events(ShellContext *context)
{
    bool still_running = true;

    igRender();
    ImDrawData *ig_draw_data = igGetDrawData();
    ImGui_ImplSdlGL3_RenderDrawData(ig_draw_data);

    SDL_GL_SwapWindow(context->sdl_window);

    ImGui_ImplSdlGL3_NewFrame(context->sdl_window);

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        update_keys_input_state(&context->input_state, &event);

        ImGui_ImplSdlGL3_ProcessEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                still_running = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    still_running = false;
                break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                {
                    context->window_width = event.window.data1;
                    context->window_height = event.window.data2;
                    glViewport(0, 0, context->window_width, context->window_height);
                }
                break;

            case SDL_MOUSEMOTION:
                context->input_state.mouse_position[0] = 
                    2.0f * event.motion.x / (float)context->window_width - 1.0f;
                context->input_state.mouse_position[1] = 
                    1.0f - 2.0f * event.motion.y / (float)context->window_height;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                    context->input_state.left_mouse = true;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    context->input_state.right_mouse = true;
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    context->input_state.left_mouse = false;
                else if (event.button.button == SDL_BUTTON_RIGHT)
                    context->input_state.right_mouse = false;
                break;
        }
    }

    return still_running;
}

const ShellInputs *shell_view_input_state( ShellContext *context )
{
    return &context->input_state;
}

void shell_delete(ShellContext *context)
{
    if (!context) return;

    if (context->sdl_window)
    {
        ImGui_ImplSdlGL3_Shutdown();

        SDL_GL_DeleteContext(context->sdl_gl_context);
        SDL_DestroyWindow(context->sdl_window);
        SDL_Quit();

        context->sdl_gl_context = NULL;
        context->sdl_window = NULL;
    }
}