#pragma once

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   include <SDL2/SDL.h>
#elif _WIN32
#   include <GL/glew.h>
#   include <SDL.h>
#else
#   include <GL/glew.h>
#   include <SDL2/SDL.h>
#endif
