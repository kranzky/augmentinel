#pragma once

// Common header for the SDL2 + OpenGL build (the legacy Win32/D3D11 build uses stdafx.h).
// PLATFORM_WINDOWS is deliberately never defined here: it selects that legacy code.

// DirectXMath's intrinsics paths rely on MSVC; use its portable scalar path elsewhere.
#ifndef _MSC_VER
#define _XM_NO_INTRINSICS_
#endif

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#include <SDL.h>
#include <SDL_mixer.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

#include "Version.h"

// Map the Windows virtual key names used by the game's input bindings to SDL keycodes.
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
#define VK_OEM_PLUS   SDLK_EQUALS   // the unshifted + key
#define VK_OEM_MINUS  SDLK_MINUS

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

// Mouse buttons share the key namespace, offset clear of SDL's printable keycodes.
#define VK_MOUSE_BASE 1000
#define VK_LBUTTON    (VK_MOUSE_BASE + SDL_BUTTON_LEFT)
#define VK_RBUTTON    (VK_MOUSE_BASE + SDL_BUTTON_RIGHT)
#define VK_MBUTTON    (VK_MOUSE_BASE + SDL_BUTTON_MIDDLE)
#define VK_XBUTTON1   (VK_MOUSE_BASE + SDL_BUTTON_X1)
#define VK_XBUTTON2   (VK_MOUSE_BASE + SDL_BUTTON_X2)

// Directory holding 48.rom, sentinel.sna, shaders/ and sounds/: Contents/Resources in
// a macOS app bundle, otherwise the executable's directory. Set by Application::Init().
extern fs::path g_resourcePath;

#include "SharedConstants.h"
#include "Utils.h"
#include "Sentinel.h"
