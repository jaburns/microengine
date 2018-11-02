#pragma once

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   include <SDL2/SDL.h>
#elif _WIN32
#   define HAVE_M_PI
#   include <GL/glew.h>
#   include <SDL.h>
#else
#   include <GL/glew.h>
#   include <SDL2/SDL.h>
#endif
