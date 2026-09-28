#include "GL.h"
#include <SDL.h>

#ifdef __APPLE__

bool LoadGL()
{
    return true;
}

#else

#define AUGMENTINEL_GL_DEFINE(type, name) type p_##name{ nullptr };
AUGMENTINEL_GL_FUNCTIONS(AUGMENTINEL_GL_DEFINE)
#undef AUGMENTINEL_GL_DEFINE

bool LoadGL()
{
    // Stringize before the name is macro-expanded to its p_ pointer.
#define AUGMENTINEL_GL_LOAD(type, name) \
    p_##name = reinterpret_cast<type>(SDL_GL_GetProcAddress(#name)); \
    if (!p_##name) \
    { \
        SDL_Log("OpenGL function %s is unavailable", #name); \
        return false; \
    }
    AUGMENTINEL_GL_FUNCTIONS(AUGMENTINEL_GL_LOAD)
#undef AUGMENTINEL_GL_LOAD

    return true;
}

#endif
