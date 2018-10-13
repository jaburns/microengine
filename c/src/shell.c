#pragma once

#include "shell.h"
#include "utils.h"

#include <string.h>
#include <GL/glew.h>
#include <SDL.h>

typedef struct MediaContext
{
    SDL_Window *sdl_window;
    SDL_GLContext sdl_gl_context;
    int window_width;
    int window_height;
    InputState input_state;
}
MediaContext;

static MediaContext s_context = { NULL };

bool shell_open(const char *title, int width, int height)
{
    if (s_context.sdl_window) return false;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetSwapInterval(1);

    s_context.window_width = width;
    s_context.window_height = height;

    s_context.sdl_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    s_context.sdl_gl_context = SDL_GL_CreateContext(s_context.sdl_window);

    if (!s_context.sdl_gl_context)
    {
        shell_close();
        return false;
    }

    glewExperimental = GL_TRUE;
    const GLenum glewInitResult = glewInit();

    if (glewInitResult != GLEW_OK)
    {
        shell_close();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    return true;
}

static void update_input_state(InputState *state, const SDL_Event *event)
{
    const SDL_Keycode key = event->key.keysym.sym;

    int index;
    FIND_INDEX(index, state->keys_down, state->num_keys_down, key);

    switch (event->type) 
    {
        case SDL_KEYDOWN:
            if (index >= 0) break;

            state->num_keys_down++;

            if (state->num_keys_down == 1)
                state->keys_down = malloc(sizeof(SDL_Keycode));
            else
                state->keys_down = realloc(state->keys_down, state->num_keys_down*sizeof(SDL_Keycode));

            state->keys_down[state->num_keys_down - 1] = key;

            break;

        case SDL_KEYUP:
            if (index < 0) break;

            state->keys_down[index] = state->keys_down[state->num_keys_down - 1];
            state->num_keys_down--;
            state->keys_down = realloc(state->keys_down, state->num_keys_down*sizeof(SDL_Keycode));

            break;
    }
}

bool shell_flip_frame_poll_events(void)
{
    bool still_running = true;

    SDL_GL_SwapWindow(s_context.sdl_window);
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        update_input_state(&s_context.input_state, &event);

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
                    s_context.window_width = event.window.data1;
                    s_context.window_height = event.window.data2;
                    glViewport(0, 0, s_context.window_width, s_context.window_height);
                }
                break;
        }
    }

    return still_running;
}

void shell_close(void)
{
    if (s_context.sdl_window)
    {
        SDL_GL_DeleteContext(s_context.sdl_gl_context);
        SDL_DestroyWindow(s_context.sdl_window);
        SDL_Quit();

        s_context.sdl_window = NULL;
    }
}

InputState *copy_input_state(const InputState *from)
{
    InputState *result = malloc(sizeof(InputState));
    result->num_keys_down = from->num_keys_down;

    if (result->num_keys_down > 0)
    {
        result->keys_down = malloc(result->num_keys_down * sizeof(SDL_Keycode));
        memcpy(result->keys_down, from->keys_down, result->num_keys_down * sizeof(SDL_Keycode));
    }
    else
        result->keys_down = NULL;

    return result;
}

InputState *read_input_state(void)
{
    return copy_input_state(&s_context.input_state);
}

void free_input_state(InputState *state)
{
    if (state->num_keys_down > 0)
        free(state->keys_down);

    free(state);
}
