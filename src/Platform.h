#pragma once

// DirectXMath configuration - MUST be before including DirectXMath
// Required for macOS and MinGW (non-MSVC compilers)
#if defined(__APPLE__) || (defined(__GNUC__) && !defined(_MSC_VER))
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
// Note: PLATFORM_WINDOWS is defined by CMake for MSVC builds (Win32 API)
// For MinGW/SDL2 builds on Windows, PLATFORM_WINDOWS is NOT defined
#if defined(_WIN32) && defined(_MSC_VER)
    #ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS
    #endif
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
// Filesystem support
#include <filesystem>
namespace fs = std::filesystem;
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

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
#define VK_OEM_PLUS   SDLK_EQUALS   // = key (+ requires shift, but we want unshifted)
#define VK_OEM_MINUS  SDLK_MINUS

// Letter keys (action bindings use uppercase, SDL uses lowercase keycodes)
// Map uppercase ASCII to SDL lowercase keycodes
#define VK_A          SDLK_a
#define VK_B          SDLK_b
#define VK_H          SDLK_h
#define VK_M          SDLK_m
#define VK_N          SDLK_n
#define VK_P          SDLK_p
#define VK_Q          SDLK_q
#define VK_R          SDLK_r
#define VK_T          SDLK_t
#define VK_U          SDLK_u

// SDL mouse button mappings
#define VK_LBUTTON    (1000 + SDL_BUTTON_LEFT)
#define VK_RBUTTON    (1000 + SDL_BUTTON_RIGHT)
#define VK_MBUTTON    (1000 + SDL_BUTTON_MIDDLE)
#define VK_XBUTTON1   (1000 + SDL_BUTTON_X1)
#define VK_XBUTTON2   (1000 + SDL_BUTTON_X2)

// Modifier key mappings (used for exclusion from VK_ANY)
#define VK_LSHIFT     SDLK_LSHIFT
#define VK_RSHIFT     SDLK_RSHIFT
#define VK_LCONTROL   SDLK_LCTRL
#define VK_RCONTROL   SDLK_RCTRL
#define VK_LALT       SDLK_LALT
#define VK_RALT       SDLK_RALT
#define VK_LGUI       SDLK_LGUI    // Command key on macOS
#define VK_RGUI       SDLK_RGUI    // Command key on macOS
#endif

// Global resource path (set by Application, used for loading assets)
// On macOS app bundles this points to Contents/Resources/
// Otherwise it's the executable directory
extern std::string g_resourcePath;

// Include common game headers
#include "SharedConstants.h"
#include "Utils.h"
#include "Sentinel.h"
