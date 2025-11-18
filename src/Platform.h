#pragma once

// DirectXMath configuration - MUST be before including DirectXMath
#ifdef __APPLE__
    #ifndef _XM_NO_INTRINSICS_
        #define _XM_NO_INTRINSICS_
    #endif
    #ifndef _XM_NOSAL_
        #define _XM_NOSAL_
    #endif
    #ifndef _XM_NOCONCUR_
        #define _XM_NOCONCUR_
    #endif
#endif

// Platform detection
#if defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #ifndef PLATFORM_MACOS
        #define PLATFORM_MACOS
    #endif
    #include <TargetConditionals.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
#endif

// Common C++ includes
#include <array>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>
#include <fstream>
#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>
#include <locale>
#include <codecvt>

namespace fs = std::filesystem;

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// OpenGL
#ifdef PLATFORM_MACOS
    #ifndef GL_SILENCE_DEPRECATION
        #define GL_SILENCE_DEPRECATION
    #endif
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>  // Windows/Linux need GLEW
#endif

// DirectXMath (cross-platform math library)
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

// App constants
#define APP_NAME "Augmentinel"
#define APP_VERSION "v1.6.0-SDL2"

// SDL keycode mappings for non-Windows platforms
#ifndef PLATFORM_WINDOWS
// Map Windows VK_* constants to SDL keycodes
#define VK_ESCAPE     SDLK_ESCAPE
#define VK_RETURN     SDLK_RETURN
#define VK_LEFT       SDLK_LEFT
#define VK_RIGHT      SDLK_RIGHT
#define VK_UP         SDLK_UP
#define VK_DOWN       SDLK_DOWN
#define VK_HOME       SDLK_HOME
#define VK_END        SDLK_END
#define VK_PRIOR      SDLK_PAGEUP
#define VK_NEXT       SDLK_PAGEDOWN
#define VK_SPACE      SDLK_SPACE
#define VK_PAUSE      SDLK_PAUSE
#define VK_OEM_PLUS   SDLK_PLUS
#define VK_OEM_MINUS  SDLK_MINUS

// SDL mouse button mappings
#define VK_LBUTTON    (1000 + SDL_BUTTON_LEFT)
#define VK_RBUTTON    (1000 + SDL_BUTTON_RIGHT)
#define VK_MBUTTON    (1000 + SDL_BUTTON_MIDDLE)
#define VK_XBUTTON1   (1000 + SDL_BUTTON_X1)
#define VK_XBUTTON2   (1000 + SDL_BUTTON_X2)
#endif

// Include common game headers
#include "SharedConstants.h"
#include "Utils.h"
#include "Sentinel.h"
