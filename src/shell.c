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

    glewExperimental = GL_TRUE;
    const GLenum glewInitResult = glewInit();

    if (glewInitResult != GLEW_OK) goto err;

    igCreateContext(NULL);
    ImGuiIO *io = igGetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplSdlGL3_Init(context->sdl_window, NULL);
    ImGui_ImplSdlGL3_NewFrame(context->sdl_window);

    context->input_state.keys_down = vec_empty(sizeof(SDL_Keycode));

    return context;

err:
    SDL_GL_DeleteContext(context->sdl_gl_context);
    SDL_DestroyWindow(context->sdl_window);
    SDL_Quit();
    free(context);

    return NULL;
}

static void update_input_state(ShellInputs *state, const SDL_Event *event)
{
    const SDL_Keycode key = event->key.keysym.sym;
    FIND_INDEX_DECL(index, (SDL_Keycode*)state->keys_down.data, state->keys_down.item_count, key);

    if (index < 0 && event->type == SDL_KEYDOWN)
        vec_push_copy(&state->keys_down, &key);
    else if (index >= 0 && event->type == SDL_KEYUP)
        vec_remove(&state->keys_down, index);
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
        update_input_state(&context->input_state, &event);

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
        }
    }

    return still_running;
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

ShellInputs *read_input_state_alloc(ShellContext *context)
{
    ShellInputs *result = malloc(sizeof(ShellInputs));
    result->keys_down = vec_clone(&context->input_state.keys_down);
    return result;
}

void free_input_state(ShellInputs *state)
{
    vec_clear(&state->keys_down);
    free(state);
}