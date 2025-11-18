# Augmentinel Porting TODO List

**Status:** Phase 1 Complete ✅
**Current Phase:** Phase 2 (Shader Pipeline)
**Last Updated:** 2025-11-18

Use this file to track progress through the SDL2+OpenGL port. Check off items as you complete them.

---

## Phase 1: Build System & Foundation ✅ COMPLETE

**Estimated:** 2-3 days
**Actual:** ~1 day
**Status:** All objectives met, executable builds and runs

### 1.1: CMake Build System ✅
- [x] Install CMake 3.15+
- [x] Install SDL2 via Homebrew: `brew install sdl2` (v2.32.10 installed)
- [x] Install SDL2_mixer via Homebrew: `brew install sdl2_mixer` (v2.8.1 installed)
- [x] Create `CMakeLists.txt` in project root
  - [x] Set up project name and C++17 standard
  - [x] Add `find_package(SDL2 REQUIRED CONFIG)`
  - [x] Add `find_package(OpenGL REQUIRED)`
  - [x] Set up FetchContent for DirectXMath (dec2022 tag)
  - [x] List all source files (added View.cpp to list)
  - [x] Configure include directories
  - [x] Link libraries (SDL2::SDL2, SDL2_mixer, OpenGL::GL)
  - [x] Add resource copy commands (48.rom, sentinel.sna, sounds/)
  - [x] Add macOS-specific compile definitions
- [x] Test CMake configuration: `mkdir build && cd build && cmake ..`
- [x] Verify CMake runs without errors ✅

**Additional work done:**
- Added `CPU_Z80_DEPENDENCIES_H="Z80-support.h"` define for Z80 emulator
- Added `CPU_Z80_USE_LOCAL_HEADER` define to use local Z80.h
- Added `_XM_NO_INTRINSICS_` for DirectXMath macOS compatibility

### 1.2: Platform Abstraction ✅
- [x] Create `src/Platform.h`
  - [x] Add platform detection macros (PLATFORM_MACOS, PLATFORM_WINDOWS, PLATFORM_LINUX)
  - [x] Include SDL2 headers (<SDL2/SDL.h>, <SDL2/SDL_mixer.h>)
  - [x] Include OpenGL headers (OpenGL/gl3.h for macOS, GL/glew.h for Windows/Linux)
  - [x] Include DirectXMath headers with proper configuration
  - [x] Add common STL includes (vector, map, string, filesystem, etc.)
  - [x] Define APP_NAME and APP_VERSION
  - [x] Add VK_* keycode mappings for SDL (SDLK_ESCAPE → VK_ESCAPE, etc.)
  - [x] Add VK_LBUTTON/RBUTTON/MBUTTON mouse button mappings (offset +1000 from SDL codes)
- [x] Test: Verify Platform.h compiles standalone ✅

**Additional discoveries:**
- DirectXMath requires `_XM_NO_INTRINSICS_`, `_XM_NOSAL_`, `_XM_NOCONCUR_` on macOS
- Created stub `sal.h` file for DirectXMath Windows annotation headers
- VK_* constants needed for ~30 different keys/buttons

### 1.3: Stub Application Class ✅
- [x] Modify `src/Application.h`
  - [x] Remove Win32-specific members (HINSTANCE, HWND, etc.)
  - [x] Add SDL_Window* member
  - [x] Add SDL_GLContext member
  - [x] Change OpenGLRenderer to View base class (for polymorphism)
  - [x] Update method signatures (ProcessEvent, ProcessKeyEvent, ProcessMouseButton)
- [x] Rewrite `src/Application.cpp`
  - [x] Implement Init() with SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)
  - [x] Create SDL window with OpenGL context (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)
  - [x] Set OpenGL attributes (3.3 core profile, depth 24, stencil 8, MSAA 4x)
  - [x] Implement Run() with SDL event loop (SDL_PollEvent)
  - [x] Implement ProcessEvent() for SDL events (quit, key, mouse, resize)
  - [x] Implement ProcessKeyEvent() and ProcessMouseButton()
  - [x] Implement Shutdown() to cleanup SDL (SDL_DestroyWindow, SDL_GL_DeleteContext, SDL_Quit)
  - [x] Remove all Win32 API calls (CreateWindowEx, GetMessage, etc.)
  - [x] Add high_resolution_clock for delta time calculation
- [x] Test: Application compiles ✅

**Implementation notes:**
- Used chrono for frame timing instead of QueryPerformanceCounter
- Added VSync with SDL_GL_SetSwapInterval(1)
- Logged OpenGL version/vendor info for debugging

### 1.4: Minimal main.cpp ✅
- [x] Rewrite `src/main.cpp`
  - [x] Change from WinMain to main(int argc, char* argv[])
  - [x] Create Application instance
  - [x] Call Init() and Run()
  - [x] Add try-catch for exceptions
  - [x] Use SDL_Log instead of MessageBox for errors
- [x] Test: main.cpp compiles ✅

### 1.5: Stub OpenGL Renderer ✅
- [x] Create `src/OpenGLRenderer.h`
  - [x] Inherit from View class
  - [x] Add OpenGL member variables (VAO, programs, UBOs)
  - [x] Declare Init(), BeginScene(), Render(), EndScene()
  - [x] Declare DrawModel() override
  - [x] Implement required View interface methods (GetEyePositionVector, etc.)
- [x] Create `src/OpenGLRenderer.cpp`
  - [x] Implement constructor/destructor
  - [x] Implement Init() - glClearColor, glEnable(GL_DEPTH_TEST/GL_CULL_FACE)
  - [x] Implement BeginScene() - glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
  - [x] Implement Render() - stub (will be filled in Phase 2)
  - [x] Implement EndScene() - stub
  - [x] Implement DrawModel() - stub
  - [x] Implement View interface stubs (return default values)
- [x] Test: OpenGLRenderer compiles ✅

### 1.6: View Base Class Implementation ✅
**Note:** This was not in the original plan but was necessary for linking

- [x] Create `src/View.cpp` with non-virtual method implementations
  - [x] Implement ~View() destructor
  - [x] Implement SetPalette() - copies to m_vertexConstants.Palette
  - [x] Implement SetFillColour(), SetFogColour(), SetMouseSpeed()
  - [x] Implement EnableFreeLook()
  - [x] Implement InputAction(), OutputAction() - stubs for Phase 4
  - [x] Implement GetEyePosition(), GetViewPosition(), GetCameraPosition(), etc.
  - [x] Implement SetCameraPosition(), SetCameraRotation() - delegate to Camera class
  - [x] Implement IsVR() - returns false
  - [x] Implement GetEffect(), SetEffect(), TransitionEffect() - manipulate pixel/vertex constants
  - [x] Implement EnableAnimatedNoise(), PixelShaderEffectsActive()
  - [x] Implement SetPitchLimits() - delegate to Camera
  - [x] Implement UpdateKey(), GetKeyState(), AnyKeyPressed(), ReleaseKeys()
  - [x] Implement DrawModel(), DrawControllers() - stubs
- [x] Add View.cpp to CMakeLists.txt source list
- [x] Test: View.cpp compiles and links ✅

**Key learning:** View has many non-pure-virtual methods that need implementations even for stub renderer

### 1.7: Modify Portable Code ✅
- [x] Update `src/Spectrum.cpp`
  - [x] Replace `#include "stdafx.h"` with `#include "Platform.h"`
  - [x] Replace `std::exception(msg)` with `std::runtime_error(msg)` (3 instances)
  - [x] Replace `DWORD` with `uint32_t`
  - [x] Wrap `DebugBreak()` in `#ifdef PLATFORM_WINDOWS`
- [x] Update `src/Augmentinel.cpp`
  - [x] Replace stdafx.h with Platform.h
  - [x] Remove VRView.h include
  - [x] Replace `std::exception(msg)` with `std::runtime_error(msg)` (3 instances)
  - [x] Wrap `PostQuitMessage()` in `#ifdef PLATFORM_WINDOWS`
  - [x] Wrap `DebugBreak()` in `#ifdef PLATFORM_WINDOWS` (3 instances)
  - [x] Fix `p.path().filename()` → `p.path().filename().wstring()` for music files
  - [x] Wrap AddToolTip() and OptionsDialogProc() Windows UI code in `#ifdef PLATFORM_WINDOWS`
- [x] Update `src/Model.h`
  - [x] Wrap `#include "BufferHeap.h"` in `#ifdef PLATFORM_WINDOWS`
  - [x] Wrap D3D11-specific members in `#ifdef PLATFORM_WINDOWS`
    - [x] m_pHeapVertices, m_pHeapIndices
    - [x] m_pVertexShader, m_pPixelShader (ComPtr types)
  - [x] Fix Model() default constructor - explicit initialization (macOS Clang issue with XMFLOAT3)
- [x] Update `src/Model.cpp`
  - [x] Replace stdafx.h
  - [x] Wrap `m_pHeapVertices.reset()` in `#ifdef PLATFORM_WINDOWS`
- [x] Update `src/View.h`
  - [x] Wrap `#include "BufferHeap.h"` and `#include "StateTracker.h"` in `#ifdef PLATFORM_WINDOWS`
  - [x] Wrap D3D11-specific members (ComPtr, D3D11VertexHeap, etc.) in `#ifdef PLATFORM_WINDOWS`
  - [x] Wrap Init(), DisableAltEnter(), UpdateConstants() in `#ifdef PLATFORM_WINDOWS`
  - [x] Wrap GetDXGIAdapterIndex() in `#ifdef PLATFORM_WINDOWS`
  - [x] Add padding to VertexConstants struct (alignment fix for macOS)
- [x] Update `src/Augmentinel.h`
  - [x] Wrap Options() static method in `#ifdef PLATFORM_WINDOWS`
- [x] Update `src/Vertex.h`
  - [x] Replace `UINT32 colour` with `uint32_t colour`
- [x] Update `src/Camera.cpp` - replace stdafx.h
- [x] Update `src/Animate.cpp` - replace stdafx.h
- [x] Update `src/Utils.h`
  - [x] Wrap Windows-specific functions in `#ifdef PLATFORM_WINDOWS`
    - [x] Fail(), ModulePath(), ModuleDirectory(), WorkingDirectory(), WindowText()
  - [x] Keep FileContents() declaration outside #ifdef (cross-platform)
- [x] Update `src/Utils.cpp`
  - [x] Replace stdafx.h
  - [x] Wrap Windows implementations in `#ifdef PLATFORM_WINDOWS`
  - [x] Add cross-platform FileContents() using std::ifstream
  - [x] Keep random_source() and random_uint32() outside #ifdef
- [x] Remove `src/stdafx.h` and `src/stdafx.cpp` from build ✅
- [x] Test: All portable files compile ✅

**Major learnings:**
- macOS Clang doesn't allow `Model() = default` with XMFLOAT3 brace initializers - needed explicit constructor
- VertexConstants needs padding for 16-byte alignment on macOS
- std::exception(msg) is MSVC-specific, standard C++ requires std::runtime_error
- DirectXMath sal.h inclusion is unconditional - needed stub header

### 1.8: Stub Audio and Settings ✅
- [x] Create `src/Audio.h` with stub class
  - [x] Add AudioType enum (Tune, Music, LoopingEffect, Effect)
  - [x] Add stub methods: LoadWAV(), Play() (multiple overloads), Stop(), IsPlaying()
  - [x] Add stub methods: SetMusicPlaying(), SetMusicVolume(), PositionListener()
  - [x] All methods return false/empty/do nothing
- [x] Create `src/Audio.cpp` - empty file (all inline in .h)
- [x] Create `src/Settings.h` with inline stub functions
  - [x] InitSettings() - SDL_Log stub message
  - [x] GetSetting() - returns default value
  - [x] Other setting functions - stubs
- [x] Create `src/Settings.cpp` - empty file (all inline in .h)
- [x] Test: Stubs compile and link ✅

**Implementation notes:**
- Audio needs 4 AudioType values (not just Tune/Music as originally planned)
- Play() has 3 overloads: (filename, type), (filename, type, position), (filename)
- SetMusicPlaying() returns bool (not void as initially thought)
- PositionListener() takes XMFLOAT3 parameters for 3D audio positioning

### 1.9: Fix Z80 Emulator Dependencies ✅
**Note:** Not in original plan, discovered during build

- [x] Create stub `build/_deps/directxmath-src/Inc/sal.h`
  - [x] Define empty SAL annotation macros (_In_, _Out_, _Check_return_, etc.)
  - [x] ~40 macro definitions needed
- [x] Add `CPU_Z80_DEPENDENCIES_H="Z80-support.h"` to CMakeLists.txt
  - [x] Points Z80.h to use local Z80-support.h instead of external Z library
- [x] Add `CPU_Z80_USE_LOCAL_HEADER` to CMakeLists.txt
  - [x] Makes Z80.c use local "Z80.h" instead of <emulation/CPU/Z80.h>
- [x] Test: Z80 emulator compiles ✅

**Key discovery:** Z80 emulator has two #ifdef checks for header paths - both needed configuration

### 1.10: First Build ✅
- [x] Run CMake build: `cd build && cmake --build .`
- [x] Fix compilation errors:
  - [x] DirectXMath sal.h - created stub
  - [x] BufferHeap.h D3D11 dependencies - wrapped in #ifdef
  - [x] HRESULT, LPCWSTR, HWND types - wrapped in #ifdef
  - [x] std::exception → std::runtime_error
  - [x] UINT32, DWORD → uint32_t
  - [x] DebugBreak, PostQuitMessage - wrapped in #ifdef
  - [x] Model() constructor - explicit initialization
  - [x] VertexConstants alignment - added padding
  - [x] FileContents() - cross-platform implementation
  - [x] VK_* keycodes - mapped in Platform.h
  - [x] AudioType enum - added all values
  - [x] Z80 header paths - added defines
  - [x] View methods - created View.cpp
  - [x] Camera member access - used public methods
- [x] Fix linker errors:
  - [x] Created View.cpp with base class implementations
  - [x] Added View.cpp to CMakeLists.txt
- [x] **BUILD SUCCESS:** Executable created at `build/Augmentinel` (1.4 MB) ✅
- [x] Compilation warnings: Only deprecation warnings for codecvt_utf8 (acceptable)
  - **Decision:** Defer to Phase 5 - warnings don't block functionality, won't interfere with shader work
  - Source: `Utils.h` lines 52-62 - `std::codecvt_utf8` and `std::wstring_convert` deprecated in C++17
  - Impact: None - code works correctly, warnings are noise but manageable

**Testing:**
- [x] Run executable: `./Augmentinel` ✅
- [x] Verify: Window opens with clear color ✅
- [x] Verify: ESC key closes window ✅
- [x] Verify: No crashes ✅

**Result:** Resources auto-copy correctly via CMake, basic execution confirmed working

**Phase 1 Status:** ✅ COMPLETE
**Phase 1 Result:** SDL2 window and OpenGL context infrastructure complete, program builds successfully, ready for shader implementation

---

## Phase 2: Shader Pipeline (2-3 days)

**Status:** In Progress (2.1 complete)
**Prerequisites:** Phase 1 complete ✅

### 2.1: Test Basic Execution ✅
- [x] Resources confirmed auto-copied by CMake ✅
  - [x] 48.rom exists in build/
  - [x] sentinel.sna exists in build/
  - [x] shaders/ directory copied to build/
- [x] Run `./Augmentinel` from build directory ✅
- [x] Verify window opens with clear color ✅
- [x] Verify ESC key closes application ✅
- [x] Check console output for errors ✅
- [x] Test: No crashes ✅

**Result:** Basic execution fully working, ready for shader implementation

**Discovered shaders available:**
- `shaders/Sentinel_VS.hlsl` / `Sentinel_PS.hlsl` - Main game rendering (PRIORITY)
- `shaders/Effect_VS.hlsl` / `Effect_PS.hlsl` - Post-processing effects (PRIORITY)
- `shaders/Pointer_VS.hlsl` / `Pointer_PS.hlsl` - VR pointer (defer to Phase 6)
- `shaders/Mirror_VS.hlsl` / `Mirror_PS.hlsl` - VR mirror view (defer to Phase 6)
- `shaders/OpenVR_VS.hlsl` / `OpenVR_PS.hlsl` - VR rendering (defer to Phase 6)

**Phase 2 Strategy:** Focus on Sentinel and Effect shaders first (non-VR gameplay)

### 2.2: Convert Sentinel Vertex Shader
- [ ] Read existing `shaders/Sentinel_VS.hlsl`
  - [ ] Document cbuffer structure
  - [ ] Note input/output semantics
- [ ] Create `shaders/Sentinel.vert`
- [ ] Add `#version 330 core`
- [ ] Convert vertex input semantics to layout locations
  - [ ] `layout(location = 0) in vec3 a_position;`
  - [ ] `layout(location = 1) in vec3 a_normal;`
  - [ ] `layout(location = 2) in vec4 a_color;`  // Note: uint32 color needs unpacking
  - [ ] `layout(location = 3) in vec2 a_texcoord;`
- [ ] Convert cbuffer to uniform block
  - [ ] `layout(std140, binding = 0) uniform VertexConstants { ... }`
  - [ ] Verify alignment matches C++ struct (std140 padding rules)
  - [ ] Add 64-byte aligned matrices
  - [ ] Add palette array
- [ ] Convert output semantics to out variables
  - [ ] `out vec4 v_color;`
  - [ ] `out vec3 v_position;` (if needed for lighting)
  - [ ] `out vec3 v_normal;` (if needed)
  - [ ] `out vec2 v_texcoord;`
- [ ] Convert main() function
  - [ ] Change `mul(v, M)` to `M * v` (column-major)
  - [ ] Change `saturate(x)` to `clamp(x, 0.0, 1.0)`
  - [ ] Change `lerp(a, b, t)` to `mix(a, b, t)`
  - [ ] Change `frac()` to `fract()`
- [ ] Port palette lookup and color indexing
  - [ ] Unpack uint32 color to index
  - [ ] Look up in Palette array
- [ ] Port lighting calculations from HLSL
  - [ ] Directional light (sun)
  - [ ] Ambient term
  - [ ] Per-vertex vs per-pixel decision
- [ ] Test: Shader syntax check with `glslangValidator` if available

### 2.3: Convert Sentinel Fragment Shader
- [ ] Locate original pixel shader
- [ ] Create `shaders/Sentinel.frag`
- [ ] Add `#version 330 core`
- [ ] Convert input from vertex shader: `in vec4 v_color;`, etc.
- [ ] Convert uniform block for PixelConstants
  - [ ] `layout(std140, binding = 1) uniform PixelConstants { ... }`
  - [ ] dissolved, time, view_dissolve, view_desaturate, view_fade
- [ ] Convert output: `out vec4 FragColor;`
- [ ] Port dissolve/noise logic
  - [ ] Copy `rnd()` hash function
  - [ ] Calculate dissolve threshold
  - [ ] Convert `clip()` to `if (alpha < threshold) discard;`
- [ ] Port view effects (if in pixel shader)
  - [ ] Dissolve noise
  - [ ] Desaturation
  - [ ] Fade to black
- [ ] Final color output: `FragColor = color;`
- [ ] Test: Shader syntax check

**Note:** If Sentinel shaders are combined or simple, may be minimal work. Check original implementation first.

### 2.4: Convert Effect Shaders (Post-Processing)
- [ ] Create `shaders/Effect.vert`
  - [ ] Use gl_VertexID to generate fullscreen quad:
    ```glsl
    vec2 positions[3] = vec2[](
        vec2(-1, -1), vec2(3, -1), vec2(-1, 3)
    );
    gl_Position = vec4(positions[gl_VertexID], 0, 1);
    v_texcoord = positions[gl_VertexID] * 0.5 + 0.5;
    ```
  - [ ] Output texture coordinates
- [ ] Create `shaders/Effect.frag`
  - [ ] Add `uniform sampler2D u_sceneTexture;`
  - [ ] Add PixelConstants uniform block (binding = 1)
  - [ ] Sample scene texture
  - [ ] Port view_dissolve logic (noise-based dissolve)
  - [ ] Port view_desaturate logic (convert to grayscale)
  - [ ] Port view_fade logic (fade to black)
  - [ ] Combine effects and output
- [ ] Test: Shader syntax check

### 2.5: Implement Shader Loading
- [ ] Add shader loading methods to OpenGLRenderer
  - [ ] `std::string LoadShaderFile(const std::string& filename)`
    - [ ] Open file in shaders/ directory
    - [ ] Read entire contents
    - [ ] Return as string
    - [ ] Handle file not found
  - [ ] `GLuint CompileShader(const char* source, GLenum type, const char* name)`
    - [ ] Call glCreateShader(type)
    - [ ] Call glShaderSource()
    - [ ] Call glCompileShader()
    - [ ] Check glGetShaderiv(GL_COMPILE_STATUS)
    - [ ] If failed, get log with glGetShaderInfoLog()
    - [ ] Log error and return 0
  - [ ] `GLuint LinkProgram(GLuint vs, GLuint fs, const char* name)`
    - [ ] Call glCreateProgram()
    - [ ] Call glAttachShader() for both shaders
    - [ ] Call glLinkProgram()
    - [ ] Check glGetProgramiv(GL_LINK_STATUS)
    - [ ] If failed, get log and report error
    - [ ] Call glDeleteShader() for temporary shader objects
    - [ ] Return program
- [ ] Test: Methods compile

### 2.6: Load and Compile Shaders
- [ ] Update OpenGLRenderer::Init()
  - [ ] Load Sentinel vertex shader source
  - [ ] Load Sentinel fragment shader source
  - [ ] Compile vertex shader
  - [ ] Check for compilation errors
  - [ ] Compile fragment shader
  - [ ] Check for compilation errors
  - [ ] Link into m_sentinelProgram
  - [ ] Check for link errors
  - [ ] Store program ID
  - [ ] Repeat for Effect shaders → m_effectProgram
- [ ] Test: Build and run
  - [ ] Check console for shader compilation logs
  - [ ] Verify no errors
  - [ ] If errors, fix shader syntax

### 2.7: Create Uniform Buffers (UBOs)
- [ ] In OpenGLRenderer::Init()
  - [ ] Create vertex constants UBO:
    ```cpp
    glGenBuffers(1, &m_vertexConstantsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(VertexConstants), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_vertexConstantsUBO);
    ```
  - [ ] Create pixel constants UBO similarly
    - [ ] Bind to binding point 1
  - [ ] Verify sizes match between C++ and GLSL
    - [ ] sizeof(VertexConstants) should match shader
    - [ ] Check std140 padding (vec3 → vec4 alignment)
    - [ ] May need padding fields in C++ struct (already added in Phase 1)
  - [ ] Unbind buffer
- [ ] Test: UBOs created, check glGetError()

### 2.8: Update Uniform Buffers
- [ ] Create UpdateVertexConstants() method
  ```cpp
  void UpdateVertexConstants() {
      glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants), &m_vertexConstants);
  }
  ```
- [ ] Create UpdatePixelConstants() method similarly
- [ ] Call from BeginScene() and/or before draw calls
- [ ] Test: Methods compile

### 2.9: Create Test Triangle
- [ ] In OpenGLRenderer::Init(), create test data
  ```cpp
  Vertex testVerts[3] = {
      Vertex(-0.5f, -0.5f, 0.0f, 0xFF0000FF), // Red
      Vertex( 0.5f, -0.5f, 0.0f, 0xFF00FF00), // Green
      Vertex( 0.0f,  0.5f, 0.0f, 0xFFFF0000), // Blue
  };
  glGenBuffers(1, &m_testVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_testVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(testVerts), testVerts, GL_STATIC_DRAW);
  ```
- [ ] Create VAO and set up attributes
  ```cpp
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);
  glEnableVertexAttribArray(0); // position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(1); // normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(2); // color
  glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, colour));
  glEnableVertexAttribArray(3); // texcoord
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));
  glBindVertexArray(0);
  ```
- [ ] Test: VAO created without errors

### 2.10: Render Test Triangle
- [ ] Update OpenGLRenderer::Render()
  - [ ] Create identity matrices for testing
  - [ ] Set m_vertexConstants.WVP to identity (or simple ortho)
  - [ ] Set m_vertexConstants.W to identity
  - [ ] Set palette colors (red in palette[0])
  - [ ] Update vertex constants UBO
  - [ ] Bind shader program: `glUseProgram(m_sentinelProgram)`
  - [ ] Bind VAO: `glBindVertexArray(m_vao)`
  - [ ] Draw triangle: `glDrawArrays(GL_TRIANGLES, 0, 3)`
  - [ ] Unbind: `glBindVertexArray(0)`
  - [ ] Check glGetError() after each call
- [ ] Run application
- [ ] Expected: Colored triangle on screen
- [ ] Debug if black screen:
  - [ ] Check shader compilation logs again
  - [ ] Verify vertex data uploaded correctly
  - [ ] Check glGetError() for OpenGL errors
  - [ ] Use graphics debugger (RenderDoc if available)
  - [ ] Verify matrices are correct
  - [ ] Check clear color is visible

### 2.11: Test Camera and Projection
- [ ] Update BeginScene() to use camera
  - [ ] Get view matrix from m_camera.GetViewMatrix()
  - [ ] Create projection matrix (perspective or ortho)
    ```cpp
    auto proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, NEAR_CLIP, FAR_CLIP);
    m_mViewProjection = view * proj;
    ```
  - [ ] Store in m_vertexConstants
- [ ] Update test triangle world matrix
  - [ ] Place triangle in front of camera
  - [ ] E.g., translate(0, 0, 5)
- [ ] Test: Triangle renders with proper 3D projection
- [ ] Test: Moving camera changes view

**Phase 2 Complete Criteria:**
- ✅ Shaders compile without errors
- ✅ Test triangle renders with correct colors
- ✅ Camera and projection matrices work
- ✅ Uniform buffers update correctly

---

## Phase 3: Model Rendering (3-4 days)

**Status:** Not Started
**Prerequisites:** Phase 2 complete

### 3.1: Conditionally Compile D3D11 Code
**Note:** Already done in Phase 1, but verify:
- [x] Model.h D3D11 members wrapped
- [x] Model.cpp D3D11 code wrapped
- [x] BufferHeap.h includes removed
- [ ] Verify Model class compiles clean on macOS ✅

### 3.2: Add OpenGL Members to Model (Future)
**Note:** For Phase 3, models will be uploaded on-demand by renderer. OpenGL buffer handles will be stored in renderer, not in Model class. This avoids modifying the portable Model class.

- [ ] Decision: Store VBO/IBO in renderer's map, keyed by Model*
- [ ] Alternative: Add optional OpenGL members to Model (requires more changes)

### 3.3: Implement Model Upload in Renderer
- [ ] Add `std::map<const Model*, GLuint> m_modelVBOs;` to OpenGLRenderer
- [ ] Add `std::map<const Model*, GLuint> m_modelIBOs;` to OpenGLRenderer
- [ ] Add `std::map<const Model*, size_t> m_modelIndexCounts;` to OpenGLRenderer
- [ ] Add `UploadModel(const Model& model)` method
  - [ ] Check if already uploaded: `if (m_modelVBOs.count(&model)) return;`
  - [ ] Get vertex and index data from model
    - [ ] `auto& vertices = *model.m_pVertices;`
    - [ ] `auto& indices = *model.m_pIndices;`
  - [ ] Create VBO
    ```cpp
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);
    m_modelVBOs[&model] = vbo;
    ```
  - [ ] Create IBO
    ```cpp
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);
    m_modelIBOs[&model] = ibo;
    m_modelIndexCounts[&model] = indices.size();
    ```
  - [ ] Unbind buffers
  - [ ] Check glGetError()
- [ ] Test: UploadModel compiles

### 3.4: Implement DrawModel
- [ ] Update `OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel)`
  - [ ] Check if model is valid: `if (!model) return;`
  - [ ] Upload if needed: `if (!m_modelVBOs.count(&model)) UploadModel(model);`
  - [ ] Calculate world matrix: `auto world = model.GetWorldMatrix(linkedModel);`
  - [ ] Calculate WVP: `auto wvp = world * m_mViewProjection;`
  - [ ] **Transpose matrices for GLSL:**
    ```cpp
    m_vertexConstants.WVP = XMMatrixTranspose(wvp);
    m_vertexConstants.W = XMMatrixTranspose(world);
    ```
  - [ ] Set eye position (for lighting):
    ```cpp
    auto eyePos = m_camera.GetPosition();
    m_vertexConstants.EyePos = eyePos;
    ```
  - [ ] Set lighting flag: `m_vertexConstants.lighting = model.lighting ? 1 : 0;`
  - [ ] Set dissolved value: `m_pixelConstants.dissolved = model.dissolved;`
  - [ ] Update UBOs:
    ```cpp
    UpdateVertexConstants();
    UpdatePixelConstants();
    ```
  - [ ] Bind buffers:
    ```cpp
    glBindBuffer(GL_ARRAY_BUFFER, m_modelVBOs.at(&model));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelIBOs.at(&model));
    ```
  - [ ] Set up vertex attributes (same as Phase 2 triangle setup)
  - [ ] Draw:
    ```cpp
    size_t indexCount = m_modelIndexCounts.at(&model);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    ```
  - [ ] Check glGetError()
- [ ] Test: DrawModel compiles

### 3.5: Update BeginScene/Render Flow
- [ ] Update `OpenGLRenderer::BeginScene()`
  - [ ] Clear buffers: `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);`
  - [ ] Enable depth test: `glEnable(GL_DEPTH_TEST)`
  - [ ] Enable backface culling: `glEnable(GL_CULL_FACE); glCullFace(GL_BACK);`
  - [ ] Set up view matrix from camera
  - [ ] Set up projection matrix (FOV, aspect, near, far)
  - [ ] Calculate m_mViewProjection = view * proj
  - [ ] Set clear color based on m_fill_colour_idx or game state
- [ ] Update `OpenGLRenderer::Render(IGame* pGame)`
  - [ ] Bind sentinel shader: `glUseProgram(m_sentinelProgram)`
  - [ ] Bind VAO: `glBindVertexArray(m_vao)`
  - [ ] Call `pGame->Render(this)` to let game draw its models
    - [ ] Game will call our DrawModel() for each model
  - [ ] Unbind VAO
- [ ] Remove test triangle code
- [ ] Test: Compiles

### 3.6: Test with Game Rendering
- [ ] Run application
- [ ] Check console for:
  - [ ] Spectrum initialization
  - [ ] ROM loading (48.rom)
  - [ ] Snapshot loading (sentinel.sna)
  - [ ] Game state messages
- [ ] Expected: Landscape should render
  - [ ] Terrain geometry
  - [ ] Trees, rocks, sentinels
  - [ ] Player perspective
- [ ] Debug if nothing renders:
  - [ ] Add SDL_Log in DrawModel to count calls
  - [ ] Verify pGame->Render() is called
  - [ ] Check if models have valid vertex data
  - [ ] Verify matrices aren't NaN or inf
  - [ ] Use graphics debugger to inspect draw calls

### 3.7: Fix Rendering Issues

#### 3.7.1: Geometry Issues
- [ ] If geometry is inverted (inside-out):
  - [ ] Try `glFrontFace(GL_CW);` instead of GL_CCW
  - [ ] Or flip triangle winding in model data
  - [ ] Or flip cull face: `glCullFace(GL_FRONT);`
- [ ] If geometry is upside down:
  - [ ] Check projection matrix handedness
  - [ ] May need to flip Y in projection
- [ ] If geometry is stretched:
  - [ ] Verify aspect ratio in projection matrix
  - [ ] Check for correct window width/height

#### 3.7.2: Lighting Issues
- [ ] If lighting looks wrong:
  - [ ] Verify normals are transformed correctly (W matrix)
  - [ ] Check normal transformation in vertex shader
    - [ ] Should use `mat3(W)` or `transpose(inverse(W))`
  - [ ] Verify light direction vectors
  - [ ] Check ambient vs diffuse lighting balance
  - [ ] Compare with Windows version

#### 3.7.3: Color Issues
- [ ] If colors are wrong:
  - [ ] Check palette initialization
  - [ ] Verify color index unpacking (uint32 → palette index)
  - [ ] Check endianness of color values
  - [ ] Compare palette values with Windows version
  - [ ] Verify gamma/color space
- [ ] If colors are too dark/bright:
  - [ ] Check lighting calculations
  - [ ] Verify ambient light term
  - [ ] Check gamma correction

#### 3.7.4: Depth Issues
- [ ] If z-fighting or depth problems:
  - [ ] Verify depth buffer is cleared
  - [ ] Check near/far clip planes (NEAR_CLIP = 0.1, FAR_CLIP = 500.0)
  - [ ] Verify depth test is enabled
  - [ ] Check depth buffer bit depth (should be 24-bit)
  - [ ] Verify depth range and viewport

#### 3.7.5: Performance Issues
- [ ] If FPS is low:
  - [ ] Check for unnecessary state changes
  - [ ] Minimize UBO updates (only when changed)
  - [ ] Batch similar models if possible
  - [ ] Profile with Instruments (macOS)
  - [ ] Check for synchronous GPU operations

### 3.8: Verify Game Features
- [ ] Test landscape exploration
  - [ ] Can move camera with mouse
  - [ ] Can see terrain, objects
  - [ ] Geometry loads correctly
- [ ] Test model types render:
  - [ ] Trees
  - [ ] Boulders
  - [ ] Robots
  - [ ] Sentinels
  - [ ] Pedestals
  - [ ] Landscape tiles
- [ ] Test object creation (if input works)
  - [ ] Tree creation shows model
  - [ ] Boulder creation shows model
  - [ ] Robot creation shows model
- [ ] Test object absorption
  - [ ] Dissolve effect visible
  - [ ] Model disappears

**Phase 3 Complete Criteria:**
- ✅ Landscape renders correctly
- ✅ All object types render
- ✅ Lighting looks correct
- ✅ Colors match original
- ✅ Depth sorting works
- ✅ No visual artifacts
- ✅ Performance acceptable (30+ FPS)

---

## Phase 4: Game Integration (2-3 days)

**Status:** Not Started
**Prerequisites:** Phase 3 complete

*(Content unchanged from original, as Phase 4 hasn't been started yet)*

---

## Phase 5: Effects & Polish (2-3 days)

**Status:** Not Started
**Prerequisites:** Phase 4 complete

### 5.1: Fix Deprecation Warnings
**Deferred from Phase 1** - Fix codecvt_utf8 warnings in Utils.h

- [ ] Replace deprecated `std::codecvt_utf8` and `std::wstring_convert` in Utils.h
  - [ ] Option 1: Use platform-specific converters (mbstowcs/wcstombs)
  - [ ] Option 2: Use C++20 std::format with char8_t (if upgrading to C++20)
  - [ ] Option 3: Use third-party library (ICU, boost::locale)
- [ ] Update `to_wstring()` function (line 50-55)
- [ ] Update `to_string()` function (line 57-62)
- [ ] Test file I/O still works (loading 48.rom, sentinel.sna, sounds)
- [ ] Verify clean build with zero warnings

**Why deferred:** Warnings don't block functionality; focus on critical path (shaders, rendering, gameplay) first.

### 5.2: Post-Processing Effects

*(Content from original Phase 5)*

---

## Phase 6: Testing & Debug (2-3 days)

**Status:** Not Started
**Prerequisites:** Phase 5 complete

*(Content unchanged from original)*

---

## Future Enhancements (Post-Port)

*(Content unchanged from original)*

---

## Key Learnings from Phase 1

### Build System
- DirectXMath requires special configuration on macOS (`_XM_NO_INTRINSICS_`, `_XM_NOSAL_`, `_XM_NOCONCUR_`)
- DirectXMath unconditionally includes `sal.h` - needed stub header with empty SAL macros
- Z80 emulator needs two separate defines for header paths
- CMake FetchContent works well for DirectXMath dependency

### Platform Differences
- macOS Clang doesn't allow `= default` constructors with XMFLOAT3 brace initializers
- Struct alignment differs: VertexConstants needed explicit padding for 16-byte alignment
- `std::exception(message)` is MSVC-specific, standard is `std::runtime_error(message)`
- Windows types: UINT32 → uint32_t, DWORD → uint32_t, HRESULT/HWND/LPCWSTR need #ifdef

### Code Architecture
- View base class needs non-virtual method implementations for linking
- Polymorphism pattern: Application holds View*, constructs OpenGLRenderer
- VK_* keycodes needed mapping to SDLK_* constants (30+ mappings)
- AudioType enum needs 4 values: Tune, Music, LoopingEffect, Effect
- Audio has multiple Play() overloads including 3D positional variant

### Best Practices
- Wrap all Windows-specific code in `#ifdef PLATFORM_WINDOWS` from the start
- Use cross-platform types (uint32_t, std::filesystem) instead of Windows types
- Implement stubs for all base class virtual methods
- Keep portable code (Model, Camera, Spectrum) clean of platform-specific dependencies
- Use SDL_Log instead of platform-specific logging

---

## Progress Tracking

**Phase 1:** ✅ Complete (Build system, foundation, first successful build)
**Phase 2:** ⬜ Not Started (Shader pipeline)
**Phase 3:** ⬜ Not Started (Model rendering)
**Phase 4:** ⬜ Not Started (Game integration)
**Phase 5:** ⬜ Not Started (Effects & polish)
**Phase 6:** ⬜ Not Started (Testing & debug)

**Overall Progress:** ~15% Complete (Phase 1 of 6)
**Executable Status:** Builds successfully ✅ (1.4 MB)
**Next Milestone:** Get test triangle rendering with shaders (Phase 2.10)

---

## Notes & Decisions

**Math Library:** DirectXMath (cross-platform, kept from Windows version)
**Settings:** Using SimpleIni (to be implemented Phase 4)
**Audio:** Using SDL2_mixer (2D audio only initially, to be implemented Phase 4)
**OpenGL Version:** 3.3 Core Profile
**Shader Language:** GLSL 330 core
**Build System:** CMake 3.15+
**SDL Version:** SDL2 2.32.10, SDL2_mixer 2.8.1

---

## Useful Commands

**Build:**
```bash
cd build
cmake --build .
```

**Clean build:**
```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

**Run:**
```bash
cd build
./Augmentinel
```

**Debug with lldb:**
```bash
lldb ./Augmentinel
(lldb) run
```

**Check OpenGL version:**
```cpp
SDL_Log("OpenGL: %s", glGetString(GL_VERSION));
SDL_Log("GLSL: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
```

**Validate GLSL shader:**
```bash
glslangValidator shaders/Sentinel.vert
glslangValidator shaders/Sentinel.frag
```

---

## Common Issues & Solutions

**Issue:** "SDL2 not found"
**Solution:** `brew install sdl2 sdl2_mixer`

**Issue:** DirectXMath compile errors
**Solution:** Ensure `_XM_NO_INTRINSICS_` is defined, create sal.h stub if needed

**Issue:** Model() constructor errors on macOS
**Solution:** Use explicit initialization list, not `= default` with XMFLOAT3

**Issue:** VertexConstants size assertion fails
**Solution:** Add padding field to make size multiple of 16 bytes

**Issue:** Undefined symbols for View methods
**Solution:** Create View.cpp with implementations, add to CMakeLists.txt

**Issue:** std::exception doesn't accept message parameter
**Solution:** Use std::runtime_error(message) instead

**Issue:** VK_* constants undefined
**Solution:** Map to SDLK_* in Platform.h (e.g., VK_ESCAPE = SDLK_ESCAPE)

**Issue:** Black screen, no rendering
**Solution:** Check glGetError(), verify shader compilation logs, check FBO status, verify matrices

**Issue:** Geometry upside down
**Solution:** Flip Y coordinate or adjust projection matrix handedness

**Issue:** Geometry inside out
**Solution:** Change winding order with glFrontFace() or flip culling

**Issue:** Matrix math wrong
**Solution:** Remember to transpose matrices for GLSL (DirectXMath is row-major, GLSL is column-major)

**Issue:** Uniform buffer not updating
**Solution:** Check std140 alignment, verify binding points match shader, use glBufferSubData

**Issue:** Audio not playing
**Solution:** Check file paths, verify SDL_mixer initialization, check Mix_OpenAudio() return value

**Issue:** Performance poor
**Solution:** Profile with Instruments, check for unnecessary state changes, verify VSync settings

---

## Contact & Help

- **SDL2 Documentation:** https://wiki.libsdl.org/
- **OpenGL Reference:** https://docs.gl/
- **DirectXMath:** https://github.com/microsoft/DirectXMath
- **SimpleIni:** https://github.com/brofield/simpleini
- **GLSL Reference:** https://www.khronos.org/opengl/wiki/OpenGL_Shading_Language

For questions about original codebase, see `PORTING_ANALYSIS.md`
For implementation guidance, see `PORTING_PLAN.md`
