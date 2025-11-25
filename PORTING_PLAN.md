# Augmentinel SDL2+OpenGL Porting Plan

**Version:** 1.0
**Target Platform:** macOS (with cross-platform support)
**Timeline:** 1-2 weeks of focused development
**Last Updated:** 2025-11-25

---

## ✅ Phase 1 Status Update (2025-11-18)

**Phase 1 COMPLETE** - All deliverables met and tested successfully:

- ✅ CMake build system compiles successfully
- ✅ SDL2 window opens with OpenGL 3.3 context
- ✅ Executable builds (1.4 MB, all sources compile)
- ✅ Runtime tested: Window opens, ESC key closes application cleanly
- ✅ No crashes or errors during basic execution
- ✅ All platform abstractions in place (Platform.h)
- ✅ View base class implemented with stub methods
- ✅ Cross-platform compatibility established

**Key Implementation Notes:**
- DirectXMath required special macros on macOS: `_XM_NO_INTRINSICS_`, `_XM_NOSAL_`, `_XM_NOCONCUR_`
- Created stub `sal.h` header for DirectXMath SAL annotations
- All D3D11 code wrapped in `#ifdef PLATFORM_WINDOWS`
- VK_* keycode mappings created for SDL2 compatibility
- View.cpp base class implementation added (not in original plan, but necessary)

**Actual Time:** ~1 day (vs estimated 2-3 days)

**Next:** Phase 2 - Shader Pipeline

See PORTING_TODO.md for detailed completion checklist and learnings.

---

## ✅ Phase 2 Status Update (2025-11-20)

**Phase 2 COMPLETE** - Full 3D rendering pipeline operational! 🎉

- ✅ GLSL shaders converted from HLSL (Sentinel.vert/frag, Effect.vert/frag)
- ✅ Shader programs compile and link successfully
- ✅ Uniform buffer objects (UBOs) created and bound correctly
- ✅ Test triangle renders with correct RGB colors
- ✅ Camera and projection matrices working
- ✅ Perspective transformation functional
- ✅ Matrix transposition issues resolved (row-major → column-major)

**Key Implementation Notes:**
- GLSL syntax differs from HLSL: `mix()` vs `lerp()`, `clamp()` vs `saturate()`
- Uniform blocks use std140 layout with explicit padding for 16-byte alignment
- Column-major matrices in OpenGL (DirectXMath uses row-major, but memory layout compatible)
- No binding qualifiers in OpenGL 3.3 shaders (set via glUniformBlockBinding)
- Screenshot tool (`--screenshot` flag) implemented for automated testing

**Next:** Phase 3 - Model Rendering

---

## ✅ Phase 3 Status Update (2025-11-20)

**Phase 3 COMPLETE** - Full game rendering operational! 🎉

- ✅ Model vertex data uploaded to GPU (VBO/IBO)
- ✅ DrawModel() implemented with model caching system
- ✅ Landscape, trees, boulders, robots render correctly
- ✅ Game logic re-enabled and fully functional
- ✅ Camera movement, object selection working
- ✅ All 3D models render with proper lighting and colors

**Key Implementation Notes:**
- Model caching uses `m_modelVBOs` map with vertex/index pointers as keys
- UploadModel() creates separate VBO/IBO for each unique model
- DrawModel() binds VBO/IBO, updates uniforms, and issues glDrawElements call
- Game updates re-enabled after shader pipeline verification
- Full gameplay tested and working

**Next:** Phase 4 - Game Integration

---

## ✅ Phase 4 Status Update (2025-11-22)

**Phase 4.1 & 4.4 COMPLETE** - Audio and UI features implemented! ✅

**Phase 4.1: Audio System**
- ✅ SDL_mixer audio system fully implemented
- ✅ Amiga sound effects loading (13 WAV files)
- ✅ Background music playing automatically (amiga_pcm.wav converted from ADPCM to PCM)
- ✅ Audio initialization, playback, and cleanup working

**Phase 4.4: Energy Display UI**
- ✅ Orthographic projection support added to OpenGLRenderer
- ✅ Energy icons (3D models from Spectrum memory) render correctly
- ✅ Icons positioned in top-left corner (x=-795, y=420) matching PC version
- ✅ Icon scale (27) and spacing (15) calibrated to PC version
- ✅ Icons display gold robot, blue robot, boulder, tree based on player energy

**Key Implementation Notes:**
- Energy icons are 3D models extracted from Spectrum memory, not bitmaps
- Orthographic projection uses same depth range as perspective (NEAR_CLIP to FAR_CLIP)
- Icons render with `model.orthographic` flag set to true
- Flat mode keeps static icon positions from OnAddEnergySymbol()
- VR mode repositions icons dynamically relative to camera

**Next:** Phase 4.2, 4.3, 4.5 - Settings, game state, screen effects

---

## ✅ Phase 4.2 Partial Update (2025-11-25)

**Audio & Input Fixes COMPLETE** - Key bugs resolved! ✅

**Audio Fixes:**
- ✅ Music toggle (M key) now properly pauses/resumes instead of halting
  - Changed `Stop(AudioType::Music)` to use `Mix_PauseMusic()` instead of `Mix_HaltMusic()`
  - `SetMusicPlaying()` now detects halted music and signals caller to restart
- ✅ Volume control fixed
  - `VK_OEM_PLUS` now maps to `SDLK_EQUALS` (= key) instead of `SDLK_PLUS` (+ requires shift)
  - Volume up (=) and down (-) keys work correctly

**Input Fixes:**
- ✅ Modifier keys excluded from VK_ANY detection
  - Added VK_ mappings for SHIFT, CTRL, ALT, GUI (Command) keys in Platform.h
  - ALT-TAB, screenshots, and other system shortcuts no longer trigger game actions

**State Transition Refinement:**
- ✅ Sound stopping now only occurs on specific transitions:
  - LandscapePreview → Game (entering game)
  - Game → LandscapePreview (leaving to landscape select)
  - Game → Reset (ESC from game)
- ✅ Sounds continue playing during:
  - TitleScreen → LandscapePreview
  - U-turn within game
  - SkyView transitions

**Key Commits:**
- 2691b3e: Fix music toggle (pause vs halt) and modifier key exclusion
- b99b464: Fix volume up key (= vs +) and refine state change sound stopping

**Remaining for Phase 4.2:**
- ⏳ Fullscreen toggle (F11 key)
- ⏳ Settings persistence (SimpleIni integration)

**Next:** Phase 5 - Polish & Testing

---

## Overview

This document outlines the step-by-step plan for porting Augmentinel from Windows (Win32 + D3D11) to cross-platform (SDL2 + OpenGL). The plan is organized into 6 phases that can be executed incrementally with testing at each stage.

---

## Prerequisites

### Development Environment

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake
brew install sdl2
brew install sdl2_mixer
```

**CMake Version:** 3.15 or later
**C++ Standard:** C++17
**OpenGL Version:** 3.3 Core Profile (minimum)

### Library Decisions

| Component | Choice | Rationale |
|-----------|--------|-----------|
| Windowing | SDL2 | Cross-platform, well-documented |
| Rendering | OpenGL 3.3+ | Widely supported, mature |
| Audio | SDL2_mixer | Simple, sufficient for 2D audio |
| Math | DirectXMath | Keep existing (avoid large conversion) |
| Settings | SimpleIni | Header-only, easy integration |

---

## Phase 1: Build System & Foundation (2-3 days)

**Goal:** Set up CMake build, create SDL2 window with OpenGL context, render a clear color

### Tasks

#### 1.1: Create CMake Build System

**File:** `CMakeLists.txt` (create in root)

```cmake
cmake_minimum_required(VERSION 3.15)
project(Augmentinel VERSION 1.6.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform definitions
if(APPLE)
    add_definitions(-D__APPLE__)
elseif(WIN32)
    add_definitions(-DWIN32)
elseif(UNIX)
    add_definitions(-D__linux__)
endif()

# Find packages
find_package(SDL2 REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

# SDL2_mixer (may need different approach depending on installation)
find_library(SDL2_MIXER_LIBRARY SDL2_mixer REQUIRED)

# DirectXMath (download from GitHub if keeping)
include(FetchContent)
FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG main
)
FetchContent_MakeAvailable(directxmath)

# Source files (start with minimal set)
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/Augmentinel.cpp
    src/Spectrum.cpp
    src/Model.cpp
    src/Camera.cpp
    src/Animate.cpp
    src/Audio.cpp
    src/Settings.cpp
    src/Utils.cpp
    src/OpenGLRenderer.cpp  # New file
    z80/Z80.c
)

set(HEADERS
    src/Application.h
    src/Augmentinel.h
    src/Spectrum.h
    src/Model.h
    src/Camera.h
    src/Animate.h
    src/Audio.h
    src/Settings.h
    src/Utils.h
    src/OpenGLRenderer.h    # New file
    src/Game.h
    src/Sentinel.h
    src/Vertex.h
    src/Action.h
    src/SimpleHeap.h
    src/BufferHeap.h
    src/SharedConstants.h
    z80/Z80.h
    z80/Z80-support.h
)

add_executable(Augmentinel ${SOURCES} ${HEADERS})

target_include_directories(Augmentinel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/z80
    ${SDL2_INCLUDE_DIRS}
    ${directxmath_SOURCE_DIR}/Inc  # If using DirectXMath
)

target_link_libraries(Augmentinel
    SDL2::SDL2
    ${SDL2_MIXER_LIBRARY}
    OpenGL::GL
)

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/48.rom
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sentinel.sna
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sounds
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy shaders (GLSL versions)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
```

**Test:**
```bash
mkdir build
cd build
cmake ..
# Should configure successfully (won't build yet)
```

#### 1.2: Create Platform Abstraction Header

**File:** `src/Platform.h` (new)

```cpp
#pragma once

// Platform detection
#if defined(_WIN32)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #define PLATFORM_MACOS
    #include <TargetConditionals.h>
#elif defined(__linux__)
    #define PLATFORM_LINUX
#endif

// Common includes
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

namespace fs = std::filesystem;

// SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// OpenGL
#ifdef PLATFORM_MACOS
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>  // Windows/Linux need GLEW
#endif

// DirectXMath (cross-platform)
#include <DirectXMath.h>
#include <DirectXCollision.h>
using namespace DirectX;

// App constants
#define APP_NAME "Augmentinel"
#define APP_VERSION "v1.6.0-SDL2"

#include "SharedConstants.h"
#include "Utils.h"
#include "Sentinel.h"
```

#### 1.3: Stub Out Application Class

**File:** `src/Application.h` (modify)

```cpp
#pragma once
#include "Platform.h"
#include "Game.h"
#include "Audio.h"
#include "OpenGLRenderer.h"

class Application {
public:
    Application();
    ~Application();

    bool Init();
    void Run();
    void Shutdown();

private:
    void ProcessEvent(const SDL_Event& event);
    void ProcessKeyEvent(const SDL_KeyboardEvent& key, bool pressed);

    SDL_Window* m_window{nullptr};
    SDL_GLContext m_glContext{nullptr};

    std::shared_ptr<OpenGLRenderer> m_pRenderer;
    std::shared_ptr<Audio> m_pAudio;
    std::unique_ptr<Game> m_pGame;

    bool m_running{true};
    int m_windowWidth{1600};
    int m_windowHeight{900};
};
```

**File:** `src/Application.cpp` (rewrite)

```cpp
#include "Platform.h"
#include "Application.h"
#include "Augmentinel.h"
#include "Settings.h"

Application::Application() {
}

Application::~Application() {
    Shutdown();
}

bool Application::Init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) < 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // MSAA
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Create window
    m_window = SDL_CreateWindow(
        APP_NAME " " APP_VERSION,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_windowWidth,
        m_windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        SDL_Log("OpenGL context creation failed: %s", SDL_GetError());
        return false;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

#ifndef PLATFORM_MACOS
    // Initialize GLEW (Windows/Linux only)
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        SDL_Log("GLEW initialization failed: %s", glewGetErrorString(err));
        return false;
    }
#endif

    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    SDL_Log("Renderer: %s", glGetString(GL_RENDERER));

    // Initialize settings
    InitSettings(APP_NAME);

    // Create renderer
    m_pRenderer = std::make_shared<OpenGLRenderer>(m_windowWidth, m_windowHeight);
    if (!m_pRenderer->Init()) {
        SDL_Log("Renderer initialization failed");
        return false;
    }

    // Create audio
    m_pAudio = std::make_shared<Audio>();

    // Create game
    m_pGame = std::make_unique<Augmentinel>(m_pRenderer, m_pAudio);

    return true;
}

void Application::Run() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    constexpr float MAX_ACCUMULATED_TIME = 0.25f;

    while (m_running) {
        // Process events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ProcessEvent(event);
        }

        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsed = std::chrono::duration<float>(currentTime - lastTime).count();
        elapsed = std::min(elapsed, MAX_ACCUMULATED_TIME);
        lastTime = currentTime;

        // Update game
        m_pGame->Frame(elapsed);

        // Render
        m_pRenderer->BeginScene();
        m_pRenderer->Render(m_pGame.get());
        m_pRenderer->EndScene();

        // Swap buffers
        SDL_GL_SwapWindow(m_window);
    }
}

void Application::ProcessEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            m_running = false;
            break;

        case SDL_KEYDOWN:
            ProcessKeyEvent(event.key, true);
            break;

        case SDL_KEYUP:
            ProcessKeyEvent(event.key, false);
            break;

        case SDL_MOUSEMOTION:
            if (m_pRenderer) {
                m_pRenderer->MouseMove(event.motion.xrel, event.motion.yrel);
            }
            break;

        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                m_windowWidth = event.window.data1;
                m_windowHeight = event.window.data2;
                if (m_pRenderer) {
                    m_pRenderer->OnResize(m_windowWidth, m_windowHeight);
                }
            }
            break;
    }
}

void Application::ProcessKeyEvent(const SDL_KeyboardEvent& key, bool pressed) {
    // Map SDL keys to internal key states
    // TODO: Implement key mapping
    if (m_pRenderer) {
        // m_pRenderer->UpdateKey(mappedKey, pressed ? KeyState::DownEdge : KeyState::UpEdge);
    }
}

void Application::Shutdown() {
    m_pGame.reset();
    m_pAudio.reset();
    m_pRenderer.reset();

    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
```

#### 1.4: Create Minimal main.cpp

**File:** `src/main.cpp` (rewrite)

```cpp
#include "Platform.h"
#include "Application.h"

int main(int argc, char* argv[]) {
    try {
        Application app;

        if (!app.Init()) {
            SDL_Log("Application initialization failed");
            return 1;
        }

        app.Run();

        return 0;
    }
    catch (const std::exception& e) {
        SDL_Log("Exception: %s", e.what());
        return 1;
    }
}
```

#### 1.5: Create Stub OpenGL Renderer

**File:** `src/OpenGLRenderer.h` (new)

```cpp
#pragma once
#include "Platform.h"
#include "View.h"

class OpenGLRenderer : public View {
public:
    OpenGLRenderer(int width, int height);
    ~OpenGLRenderer() override;

    bool Init();

    // View interface
    void BeginScene() override;
    void Render(IGame* pGame) override;
    void EndScene() override;

    void DrawModel(Model& model, const Model& linkedModel = {}) override;
    bool IsPointerVisible() const override { return false; }

    void OnResize(uint32_t width, uint32_t height) override;

    XMVECTOR GetEyePositionVector() const override;
    XMVECTOR GetViewPositionVector() const override;
    XMVECTOR GetViewDirectionVector() const override;
    XMVECTOR GetViewUpVector() const override;
    XMMATRIX GetViewProjectionMatrix() const override;
    XMMATRIX GetOrthographicMatrix() const override;
    void GetSelectionRay(XMVECTOR& vPos, XMVECTOR& vDir) const override;

private:
    int m_width;
    int m_height;

    // OpenGL objects (will add in Phase 2)
    GLuint m_vao{0};
    GLuint m_shaderProgram{0};
};
```

**File:** `src/OpenGLRenderer.cpp` (new)

```cpp
#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(int width, int height)
    : m_width(width), m_height(height) {
}

OpenGLRenderer::~OpenGLRenderer() {
    // Cleanup OpenGL resources
}

bool OpenGLRenderer::Init() {
    // For Phase 1, just clear to a test color
    glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    return true;
}

void OpenGLRenderer::BeginScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::Render(IGame* pGame) {
    // Phase 1: Just clear screen
    // Phase 2+: Actually render
}

void OpenGLRenderer::EndScene() {
    // Nothing needed here, SDL_GL_SwapWindow called by Application
}

void OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel) {
    // Phase 2+: Implement
}

void OpenGLRenderer::OnResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
}

// Implement View interface methods (stubs for Phase 1)
XMVECTOR OpenGLRenderer::GetEyePositionVector() const {
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewPositionVector() const {
    return m_camera.GetPositionVector();
}

XMVECTOR OpenGLRenderer::GetViewDirectionVector() const {
    return m_camera.GetDirectionVector();
}

XMVECTOR OpenGLRenderer::GetViewUpVector() const {
    return m_camera.GetUpVector();
}

XMMATRIX OpenGLRenderer::GetViewProjectionMatrix() const {
    return m_mViewProjection;
}

XMMATRIX OpenGLRenderer::GetOrthographicMatrix() const {
    return XMMatrixIdentity();
}

void OpenGLRenderer::GetSelectionRay(XMVECTOR& vPos, XMVECTOR& vDir) const {
    vPos = m_camera.GetPositionVector();
    vDir = m_camera.GetDirectionVector();
}
```

**Test:** Build should succeed, window should open with dark blue background

```bash
cd build
cmake --build .
./Augmentinel  # Should show window with dark blue background
```

### Phase 1 Deliverables

- ✅ CMake build system compiles
- ✅ SDL2 window opens
- ✅ OpenGL context created
- ✅ Screen clears to blue color
- ✅ ESC key closes application
- ✅ No crashes

---

## Phase 2: Shader Pipeline (2-3 days)

**Goal:** Convert HLSL shaders to GLSL, compile and link shader programs, render a test triangle

### Tasks

#### 2.1: Convert Sentinel Shaders

**File:** `shaders/Sentinel.vert` (new, from Sentinel_VS.hlsl)

```glsl
#version 330 core

// Vertex attributes (match Vertex structure)
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_color;  // Note: uploaded as GL_UNSIGNED_BYTE normalized
layout(location = 3) in vec2 a_texcoord;

// Uniform buffer (matches VertexConstants in View.h)
layout(std140) uniform VertexConstants {
    mat4 WVP;
    mat4 W;
    vec4 Palette[16];
    vec3 EyePos;
    float z_fade;
    float fog_density;
    uint fog_colour_idx;
    uint lighting;
};

// Output to fragment shader
out vec4 v_color;
out vec2 v_texcoord;

// Constants
const float AMBIENT_INTENSITY = 0.35;
const float BACK_FACE_INTENSITY = AMBIENT_INTENSITY / 2.0;
const vec3 LIGHT1_DIR = normalize(vec3(1.0, 0.5, 0.5));
const float LIGHT1_INTENSITY = 0.8;
const vec3 LIGHT2_DIR = normalize(vec3(-1.0, 0.5, 0.5));
const float LIGHT2_INTENSITY = 0.2;
const float MAX_Z_FADE_DISTANCE = 32.0;

void main() {
    // Transform position
    gl_Position = WVP * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;

    float lightLevel = 1.0;

    if (lighting != 0u) {
        // Transform normal to world space
        vec3 transformedNormal = mat3(W) * a_normal;

        // Direction from vertex to eye
        vec3 vertexPos = (W * vec4(a_position, 1.0)).xyz;
        vec3 vertexDir = vertexPos - EyePos;

        // Front face check
        if (dot(vertexDir, transformedNormal) < 0.0) {
            // Front face - apply lighting
            lightLevel = AMBIENT_INTENSITY;

            // Light 1
            float intensity = dot(transformedNormal, LIGHT1_DIR);
            if (intensity > 0.0) {
                lightLevel += intensity * LIGHT1_INTENSITY;
            }

            // Light 2
            intensity = dot(transformedNormal, LIGHT2_DIR);
            if (intensity > 0.0) {
                lightLevel += intensity * LIGHT2_INTENSITY;
            }
        } else {
            // Back face
            lightLevel = BACK_FACE_INTENSITY;
        }
    }

    // Get color from palette
    uint colorIdx = uint(a_color.r * 255.0);  // Color index passed in red channel
    vec4 face_colour = clamp(lightLevel, 0.0, 1.0) * Palette[colorIdx];

    // Apply fog
    float fog_level = 1.0 / exp(length(gl_Position.xyz) * fog_density);
    v_color = mix(Palette[fog_colour_idx], face_colour, fog_level);

    // Apply z-fade
    if (z_fade > 0.0) {
        float z = (W * vec4(a_position, 1.0)).z;
        z = clamp(z, 0.0, MAX_Z_FADE_DISTANCE);
        float fade = 1.0 / exp(z * z_fade);
        v_color *= fade;
    }
}
```

**File:** `shaders/Sentinel.frag` (new, from Sentinel_PS.hlsl)

```glsl
#version 330 core

in vec4 v_color;
in vec2 v_texcoord;

// Uniform buffer (matches PixelConstants in View.h)
layout(std140) uniform PixelConstants {
    float dissolved;
    float time;
    float view_dissolve;
    float view_desaturate;
    float view_fade;
};

out vec4 FragColor;

float rnd(vec2 uv) {
    // Same random function as HLSL version
    return fract(cos(mod(123456780.0, 1024.0 * dot(uv, vec2(23.14069263277926, 2.6651441426902251)))));
}

void main() {
    if (dissolved > 0.0) {
        vec2 uv = fract(v_texcoord + vec2(time, time));
        if (rnd(uv) < dissolved) {
            discard;
        }
    }

    FragColor = v_color;
}
```

#### 2.2: Convert Effect Shaders

**File:** `shaders/Effect.vert` (new)

```glsl
#version 330 core

out vec2 v_texcoord;

void main() {
    // Generate fullscreen quad from vertex ID
    // Vertex IDs: 0, 1, 2, 3 for triangle strip
    float x = float(gl_VertexID / 2);
    float y = float(gl_VertexID % 2);

    v_texcoord = vec2(x, 1.0 - y);
    gl_Position = vec4(x * 2.0 - 1.0, y * 2.0 - 1.0, 0.0, 1.0);
}
```

**File:** `shaders/Effect.frag` (new)

```glsl
#version 330 core

in vec2 v_texcoord;

uniform sampler2D u_texture;

layout(std140) uniform PixelConstants {
    float dissolved;
    float time;
    float view_dissolve;
    float view_desaturate;
    float view_fade;
};

out vec4 FragColor;

float rnd(vec2 uv) {
    return fract(cos(mod(123456780.0, 1024.0 * dot(uv, vec2(23.14069263277926, 2.6651441426902251)))));
}

void main() {
    if (view_dissolve > 0.0) {
        vec2 uv = fract(v_texcoord + vec2(time, time));
        if (rnd(uv) < view_dissolve) {
            discard;
        }
    }

    vec4 color = texture(u_texture, v_texcoord);

    if (view_desaturate > 0.0) {
        float lum = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color.rgb = mix(color.rgb, vec3(lum), view_desaturate);
    }

    if (view_fade > 0.0) {
        color.rgb *= (1.0 - view_fade);
    }

    FragColor = color;
}
```

#### 2.3: Implement Shader Loading

**Add to OpenGLRenderer.h:**
```cpp
private:
    GLuint CompileShader(const char* source, GLenum type, const char* name);
    GLuint LinkProgram(GLuint vs, GLuint fs, const char* name);
    std::string LoadShaderFile(const char* filename);

    GLuint m_sentinelProgram{0};
    GLuint m_effectProgram{0};
    GLuint m_vertexConstantsUBO{0};
    GLuint m_pixelConstantsUBO{0};
```

**Add to OpenGLRenderer.cpp:**
```cpp
std::string OpenGLRenderer::LoadShaderFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SDL_Log("Failed to open shader file: %s", filename);
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint OpenGLRenderer::CompileShader(const char* source, GLenum type, const char* name) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        SDL_Log("Shader compilation failed (%s): %s", name, infoLog);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint OpenGLRenderer::LinkProgram(GLuint vs, GLuint fs, const char* name) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        SDL_Log("Program linking failed (%s): %s", name, infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

bool OpenGLRenderer::Init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Load and compile shaders
    auto sentinelVS = LoadShaderFile("shaders/Sentinel.vert");
    auto sentinelFS = LoadShaderFile("shaders/Sentinel.frag");

    if (sentinelVS.empty() || sentinelFS.empty()) {
        return false;
    }

    GLuint vs = CompileShader(sentinelVS.c_str(), GL_VERTEX_SHADER, "Sentinel.vert");
    GLuint fs = CompileShader(sentinelFS.c_str(), GL_FRAGMENT_SHADER, "Sentinel.frag");

    if (!vs || !fs) {
        return false;
    }

    m_sentinelProgram = LinkProgram(vs, fs, "Sentinel");
    if (!m_sentinelProgram) {
        return false;
    }

    // Create uniform buffers
    glGenBuffers(1, &m_vertexConstantsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VertexConstants), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_vertexConstantsUBO);

    glGenBuffers(1, &m_pixelConstantsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(PixelConstants), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_pixelConstantsUBO);

    SDL_Log("Shaders compiled successfully");
    return true;
}
```

#### 2.4: Render Test Triangle

**Add to OpenGLRenderer::Init():**
```cpp
// Create test triangle
float vertices[] = {
    // pos              // normal         // color  // uv
    0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,     0.5f, 1.0f,
   -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
    0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
};

glGenVertexArrays(1, &m_vao);
glBindVertexArray(m_vao);

GLuint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

// Position
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);

// Normal
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));

// Color (just 1 float for now, palette index)
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));

// Texcoord
glEnableVertexAttribArray(3);
glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
```

**Add to OpenGLRenderer::Render():**
```cpp
// Set up matrices
auto proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, (float)m_width / m_height, 0.1f, 100.0f);
auto view = XMMatrixLookAtLH(
    XMVectorSet(0, 0, -3, 1),
    XMVectorSet(0, 0, 0, 1),
    XMVectorSet(0, 1, 0, 0)
);
auto world = XMMatrixIdentity();
auto wvp = world * view * proj;

// Update constants
m_vertexConstants.WVP = XMMatrixTranspose(wvp);  // Transpose for GLSL
m_vertexConstants.W = XMMatrixTranspose(world);
m_vertexConstants.Palette[0] = XMFLOAT4(1, 0, 0, 1);  // Red triangle
m_vertexConstants.lighting = 0;

glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants), &m_vertexConstants);

// Draw triangle
glUseProgram(m_sentinelProgram);
glBindVertexArray(m_vao);
glDrawArrays(GL_TRIANGLES, 0, 3);
```

**Test:** Should see a red triangle on screen

### Phase 2 Deliverables

- ✅ All GLSL shaders compile
- ✅ Shader programs link successfully
- ✅ Uniform buffers working
- ✅ Test triangle renders
- ✅ No OpenGL errors

---

## Phase 3: Model Rendering (3-4 days)

**Goal:** Adapt Model class, implement VBO/IBO management, render landscape and game objects

### Tasks

#### 3.1: Modify Model Class

**In Model.h, replace D3D11 members:**
```cpp
// Remove:
// std::shared_ptr<D3D11HeapAllocation> m_pHeapVertices;
// std::shared_ptr<D3D11HeapAllocation> m_pHeapIndices;
// ComPtr<ID3D11VertexShader> m_pVertexShader;
// ComPtr<ID3D11PixelShader> m_pPixelShader;

// Add:
GLuint m_vbo{0};
GLuint m_ibo{0};
bool m_uploaded{false};
```

#### 3.2: Implement Model Upload

**Add to OpenGLRenderer:**
```cpp
void OpenGLRenderer::UploadModel(Model& model) {
    if (model.m_uploaded) return;

    auto& vertices = *model.m_pVertices;
    auto& indices = *model.m_pIndices;

    // Create VBO
    glGenBuffers(1, &model.m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, model.m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    // Create IBO
    glGenBuffers(1, &model.m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(uint32_t),
                 indices.data(),
                 GL_STATIC_DRAW);

    model.m_uploaded = true;
}

void OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel) {
    if (!model) return;

    // Upload if not already
    if (!model.m_uploaded) {
        UploadModel(model);
    }

    // Calculate world matrix
    auto world = model.GetWorldMatrix(linkedModel);
    auto wvp = world * m_mViewProjection;

    // Update constants
    m_vertexConstants.WVP = XMMatrixTranspose(wvp);
    m_vertexConstants.W = XMMatrixTranspose(world);
    m_vertexConstants.lighting = model.lighting ? 1 : 0;

    m_pixelConstants.dissolved = model.dissolved;

    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants), &m_vertexConstants);

    glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PixelConstants), &m_pixelConstants);

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, model.m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.m_ibo);

    // Set up vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, colour));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

    // Draw
    glDrawElements(GL_TRIANGLES, model.m_pIndices->size(), GL_UNSIGNED_INT, 0);
}
```

#### 3.3: Integrate Game Rendering

**Modify OpenGLRenderer::Render():**
```cpp
void OpenGLRenderer::Render(IGame* pGame) {
    // Set up view-projection matrix
    auto view = m_camera.GetViewMatrix();
    auto proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(45.0f),
        (float)m_width / m_height,
        NEAR_CLIP,
        FAR_CLIP
    );
    m_mViewProjection = view * proj;

    // Update vertex constants
    m_vertexConstants.EyePos = m_camera.GetPosition();
    SetPalette(/* get from game */);

    // Use Sentinel shader program
    glUseProgram(m_sentinelProgram);

    // Let game render its models
    pGame->Render(this);
}
```

**Test:** Should render landscape and game objects

### Phase 3 Deliverables

- ✅ Model class adapted for OpenGL
- ✅ VBO/IBO upload working
- ✅ Landscape renders
- ✅ Game objects render
- ✅ Transformations correct
- ✅ Lighting works

---

## Phase 4: Game Integration (2-3 days)

**Goal:** Hook up Spectrum emulator, implement input handling, integrate settings

### Tasks

#### 4.1: Remove D3D Dependencies from Portable Code

**Files to modify:**
- Remove `#include "stdafx.h"` from portable files
- Replace with `#include "Platform.h"`
- Check Spectrum.cpp, Augmentinel.cpp, Camera.cpp, etc.

#### 4.2: Implement Input Mapping

**Create:** `src/InputMapping.h`

```cpp
#pragma once
#include "Platform.h"
#include "Action.h"

// Map SDL keycodes to internal actions
inline int MapSDLKeyToVK(SDL_Keycode sdl_key) {
    switch (sdl_key) {
        case SDLK_ESCAPE: return VK_ESCAPE;
        case SDLK_RETURN: return VK_RETURN;
        case SDLK_SPACE: return VK_SPACE;
        case SDLK_LEFT: return VK_LEFT;
        case SDLK_RIGHT: return VK_RIGHT;
        case SDLK_UP: return VK_UP;
        case SDLK_DOWN: return VK_DOWN;
        case SDLK_HOME: return VK_HOME;
        case SDLK_END: return VK_END;
        case SDLK_PAGEUP: return VK_PRIOR;
        case SDLK_PAGEDOWN: return VK_NEXT;
        case SDLK_PAUSE: return VK_PAUSE;
        // Letters
        case SDLK_a: return 'A';
        case SDLK_b: return 'B';
        // ... etc
        default: return 0;
    }
}
```

**Update Application::ProcessKeyEvent:**
```cpp
void Application::ProcessKeyEvent(const SDL_KeyboardEvent& key, bool pressed) {
    int vk = MapSDLKeyToVK(key.keysym.sym);
    if (vk && m_pRenderer) {
        auto state = pressed ? KeyState::DownEdge : KeyState::UpEdge;
        m_pRenderer->UpdateKey(vk, state);
    }
}
```

#### 4.3: Implement Settings System

**Replace Settings.cpp with cross-platform version using SimpleIni:**

Download SimpleIni.h from: https://github.com/brofield/simpleini

**Rewrite Settings.cpp:**
```cpp
#include "Platform.h"
#include "Settings.h"
#include "SimpleIni.h"

static CSimpleIniA ini;
std::wstring settings_path;

void InitSettings(const std::string& app_name) {
    fs::path path;

#ifdef PLATFORM_MACOS
    const char* home = getenv("HOME");
    path = fs::path(home) / "Library/Application Support" / app_name;
#elif defined(PLATFORM_WINDOWS)
    // Use %APPDATA%
#else
    const char* config = getenv("XDG_CONFIG_HOME");
    if (!config) {
        const char* home = getenv("HOME");
        path = fs::path(home) / ".config" / app_name;
    } else {
        path = fs::path(config) / app_name;
    }
#endif

    fs::create_directories(path);
    path /= (app_name + ".ini");
    settings_path = path.wstring();

    ini.LoadFile(path.string().c_str());
}

// Implement Get/Set functions using SimpleIni API
// ... (see PORTING_TODO.md for details)
```

#### 4.4: Audio System

**Rewrite Audio.cpp/h using SDL2_mixer:**

```cpp
class Audio {
public:
    Audio();
    ~Audio();

    bool Available() const { return m_initialized; }
    bool LoadWAV(const fs::path& path);
    void PlaySound(const fs::path& path, float volume = 1.0f);
    void PlayMusic(const fs::path& path, bool loop = false);
    void SetMusicVolume(float volume);
    void Stop();

private:
    bool m_initialized{false};
    std::map<std::wstring, Mix_Chunk*> m_sounds;
    std::map<std::wstring, Mix_Music*> m_music;
};
```

### Phase 4 Deliverables

- ✅ Spectrum emulator runs
- ✅ Input controls work
- ✅ Settings load/save
- ✅ Audio plays
- ✅ Game is playable

---

## Phase 5: Effects & Polish (2-3 days)

**Goal:** Implement post-processing effects, camera controls, polish visuals, fix deprecation warnings

**Note:** Deprecation warnings from Phase 1 (codecvt_utf8 in Utils.h) will be addressed in this phase. Decision was made to defer these non-blocking warnings to focus on critical path (shaders, rendering, gameplay) in Phases 2-4.

### Tasks

#### 5.1: Fix Deprecation Warnings (Deferred from Phase 1)

Replace deprecated C++17 string conversion facilities in `Utils.h`:
- `std::codecvt_utf8` → platform-specific converters or C++20/library solution
- `std::wstring_convert` → platform-specific converters or C++20/library solution
- Functions affected: `to_wstring()`, `to_string()`
- Test file I/O after changes (48.rom, sentinel.sna, sounds loading)

#### 5.2: Implement Framebuffer for Effects

```cpp
// Create FBO for post-processing
glGenFramebuffers(1, &m_framebuffer);
glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

// Create texture
glGenTextures(1, &m_sceneTexture);
glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);

// Create depth renderbuffer
glGenRenderbuffers(1, &m_depthbuffer);
glBindRenderbuffer(GL_RENDERBUFFER, m_depthbuffer);
glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthbuffer);
```

#### 5.3: Implement Effect Pass

```cpp
void OpenGLRenderer::EndScene() {
    if (PixelShaderEffectsActive()) {
        // Unbind FBO, render to backbuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Use effect shader
        glUseProgram(m_effectProgram);
        glBindTexture(GL_TEXTURE_2D, m_sceneTexture);

        // Update pixel constants for effects
        glBindBuffer(GL_UNIFORM_BUFFER, m_pixelConstantsUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PixelConstants), &m_pixelConstants);

        // Draw fullscreen quad
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
```

#### 5.4: Mouse Look

```cpp
void OpenGLRenderer::MouseMove(int xrel, int yrel) {
    if (!m_freelook) return;

    float sensitivity = m_mouse_divider;
    float yaw = -xrel / sensitivity;
    float pitch = yrel / sensitivity;

    if (m_invert_mouse) pitch = -pitch;

    m_camera.Yaw(yaw);
    m_camera.Pitch(pitch);
}
```

### Phase 5 Deliverables

- ✅ Dissolve effect works
- ✅ Fade effects work
- ✅ Mouse look implemented
- ✅ Camera controls smooth
- ✅ Visuals match original

---

## Phase 6: Testing & Debug (2-3 days)

**Goal:** Fix bugs, optimize performance, verify gameplay

### Tasks

- Play through first 10 landscapes
- Test all input actions
- Verify audio timing
- Check for memory leaks
- Profile performance
- Fix any visual glitches
- Test window resizing
- Test settings persistence

### Phase 6 Deliverables

- ✅ No crashes
- ✅ Playable from start to finish
- ✅ Performance acceptable (60fps+)
- ✅ No obvious visual bugs
- ✅ Settings work correctly

---

## Dependencies & Installation

### SDL2
```bash
# macOS
brew install sdl2 sdl2_mixer

# Ubuntu/Debian
sudo apt install libsdl2-dev libsdl2-mixer-dev

# Windows (vcpkg)
vcpkg install sdl2 sdl2-mixer
```

### DirectXMath
```cmake
# In CMakeLists.txt
FetchContent_Declare(
    directxmath
    GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
    GIT_TAG main
)
FetchContent_MakeAvailable(directxmath)
```

### SimpleIni
```bash
# Download header file
wget https://raw.githubusercontent.com/brofield/simpleini/master/SimpleIni.h
# Place in src/
```

---

## Validation Checklist

After each phase:

- [ ] Code compiles without warnings
- [ ] No crashes during basic operation
- [ ] Phase goals met
- [ ] Git commit created with descriptive message

---

## Rollback Plan

If a phase fails:
1. Review error messages and logs
2. Check PORTING_ANALYSIS.md for design details
3. Compare with original D3D11 code for logic
4. Post questions to SDL2/OpenGL forums if stuck
5. Consider alternative approaches

---

## Performance Targets

- **Minimum:** 60 FPS at 1920x1080
- **Target:** 120+ FPS at 1920x1080
- **Memory:** < 200MB resident

---

## Success Criteria

Port is successful when:
1. ✅ Game runs on macOS
2. ✅ All gameplay features work
3. ✅ Visual fidelity matches original
4. ✅ No crashes or major bugs
5. ✅ Code is maintainable
6. ✅ Builds on macOS/Linux/Windows with same codebase

---

## Future Enhancements

After successful port:
- Vulkan renderer for better performance
- OpenAL for spatial audio
- Linux packaging
- Steam Deck support
- Gamepad support
- 4K/retina display support
