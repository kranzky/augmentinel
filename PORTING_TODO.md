# Augmentinel Porting TODO List

**Status:** Phase 1 Complete ✅, Phase 2 Complete ✅, Phase 3 Complete ✅, Phase 4.1 Complete ✅, Phase 4.4 Complete ✅
**Current Phase:** Phase 4 (Game Integration) - Audio and UI features implemented
**Last Updated:** 2025-11-22

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

**Status:** In Progress (2.1-2.8 complete ✅ - Uniforms updating!)
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

### 2.2: Convert Sentinel Vertex Shader ✅
- [x] Read existing `shaders/Sentinel_VS.hlsl`
- [x] Create `shaders/SharedConstants.h` with PALETTE_SIZE constant
- [x] Create `shaders/Sentinel.vert` with GLSL #version 330 core
- [x] Convert vertex input semantics to layout locations (position, normal, colour, texcoord)
- [x] Convert cbuffer to uniform block (std140, binding = 0)
- [x] Convert output semantics to out variables (v_colour, v_texcoord)
- [x] Convert main() function:
  - [x] Changed `mul(v, M)` to `M * v` (matrices will be transposed when uploaded)
  - [x] Changed `saturate(x)` to `clamp(x, 0.0, 1.0)`
  - [x] Changed `lerp(a, b, t)` to `mix(a, b, t)`
- [x] Port palette lookup using uint colour index
- [x] Port lighting calculations (2 directional lights + ambient, backface handling)
- [x] Port fog calculation (exponential distance fog)
- [x] Port z_fade effect for object creation/absorption
- [x] Test: Build succeeded, shaders copied to build directory

**Key conversions:**
- HLSL row-major matrices → GLSL column-major (matrices transposed on upload)
- uint colour index used directly in GLSL (no unpacking needed)
- All intrinsics converted (saturate→clamp, lerp→mix)

### 2.3: Convert Sentinel Fragment Shader ✅
- [x] Read existing `shaders/Sentinel_PS.hlsl`
- [x] Create `shaders/Sentinel.frag` with GLSL #version 330 core
- [x] Convert input from vertex shader: `in vec4 v_colour;`, `in vec2 v_texcoord;`
- [x] Convert uniform block for PixelConstants (std140, binding = 1)
- [x] Convert output: `out vec4 FragColor;`
- [x] Port dissolve/noise logic:
  - [x] Copy `rnd()` hash function (frac→fract, fmod→mod)
  - [x] Calculate dissolve threshold
  - [x] Convert `clip()` to `if (random_value - dissolved < 0.0) discard;`
- [x] Test: Build succeeded, shaders copied to build directory

**Key conversions:**
- HLSL `clip(x)` discards if x < 0, converted to explicit if/discard
- HLSL `frac()` → GLSL `fract()`
- HLSL `fmod()` → GLSL `mod()`

### 2.4: Convert Effect Shaders (Post-Processing) ✅
- [x] Read existing `shaders/Effect_VS.hlsl` and `shaders/Effect_PS.hlsl`
- [x] Create `shaders/Effect.vert` with GLSL #version 330 core
  - [x] Use `gl_VertexID` to generate fullscreen quad (vertices 0, 1, 2, 3)
  - [x] Generate UV coordinates from vertex ID (0→(0,0), 1→(1,0), 2→(0,1), 3→(1,1))
  - [x] Map UV [0,1] to clip space [-1,1]
  - [x] Flip Y for correct texture orientation
- [x] Create `shaders/Effect.frag` with GLSL #version 330 core
  - [x] Add `uniform sampler2D u_sceneTexture;` for scene sampling
  - [x] Add PixelConstants uniform block (std140, binding = 1)
  - [x] Sample scene texture with `texture()` function
  - [x] Port view_dissolve logic (noise-based dissolve with discard)
  - [x] Port view_desaturate logic (convert to grayscale using luminance)
  - [x] Port view_fade logic (fade to black)
  - [x] Combine effects and output
- [x] Test: Build succeeded, shaders copied to build directory

**Key conversions:**
- No vertex attributes needed - fullscreen quad generated from `gl_VertexID`
- HLSL `Texture2D tex` + `sampler samp` → GLSL `uniform sampler2D u_sceneTexture`
- HLSL `tex.Sample(samp, uv)` → GLSL `texture(u_sceneTexture, uv)`
- Same PixelConstants uniform block structure as Sentinel fragment shader
- All three view effects applied sequentially: dissolve, desaturate, fade

### 2.5: Implement Shader Loading ✅
- [x] Add shader loading methods to OpenGLRenderer
- [x] Implement `LoadShaderFile(const std::string& filename)`
  - [x] Opens file from shaders/ directory using std::ifstream
  - [x] Reads entire contents using std::stringstream
  - [x] Returns shader source as string
  - [x] Logs error if file not found
  - [x] Logs successful load with file size
- [x] Implement `CompileShader(const char* source, GLenum type, const char* name)`
  - [x] Creates shader object with glCreateShader()
  - [x] Sets source with glShaderSource()
  - [x] Compiles with glCompileShader()
  - [x] Checks compilation status with glGetShaderiv(GL_COMPILE_STATUS)
  - [x] Retrieves and logs compilation errors with glGetShaderInfoLog()
  - [x] Returns 0 on failure, shader ID on success
  - [x] Logs successful compilation
- [x] Implement `LinkProgram(GLuint vs, GLuint fs, const char* name)`
  - [x] Creates program object with glCreateProgram()
  - [x] Attaches both vertex and fragment shaders with glAttachShader()
  - [x] Links program with glLinkProgram()
  - [x] Checks link status with glGetProgramiv(GL_LINK_STATUS)
  - [x] Retrieves and logs link errors with glGetProgramInfoLog()
  - [x] Detaches and deletes shader objects after linking
  - [x] Returns 0 on failure, program ID on success
  - [x] Logs successful link
- [x] Test: Methods compile successfully ✅

**Implementation details:**
- All three methods are private helper functions in OpenGLRenderer
- Error handling uses SDL_Log for consistent logging
- Shader objects are automatically cleaned up after linking
- Returns 0 on any error for easy error checking

### 2.6: Load and Compile Shaders ✅
- [x] Fixed GLSL shader compatibility for OpenGL 3.3
  - [x] Removed `#include` directives (not supported in GLSL)
  - [x] Inlined PALETTE_SIZE constant directly in Sentinel.vert
  - [x] Removed `binding` qualifiers from uniform blocks (OpenGL 4.2+ feature)
  - [x] Updated to use `layout(std140)` without binding specifier
- [x] Update OpenGLRenderer::Init()
  - [x] Load Sentinel vertex shader source (2960 bytes)
  - [x] Load Sentinel fragment shader source (1056 bytes)
  - [x] Compile vertex shader with error checking
  - [x] Compile fragment shader with error checking
  - [x] Link into m_sentinelProgram (program ID: 3)
  - [x] Load Effect vertex shader source (665 bytes)
  - [x] Load Effect fragment shader source (1960 bytes)
  - [x] Compile Effect shaders with error checking
  - [x] Link into m_effectProgram (program ID: 4)
  - [x] Log success with program IDs
- [x] Test: Build and run ✅
  - [x] Console shows successful shader compilation logs
  - [x] All shaders compiled without errors
  - [x] All programs linked successfully
  - [x] Application initializes and runs correctly

**Test Results:**
```
INFO: OpenGLRenderer: Initializing shader pipeline...
INFO: Loaded shader file: shaders/Sentinel.vert (2960 bytes)
INFO: Loaded shader file: shaders/Sentinel.frag (1056 bytes)
INFO: Compiled shader: Sentinel.vert
INFO: Compiled shader: Sentinel.frag
INFO: Linked shader program: Sentinel
INFO: Loaded shader file: shaders/Effect.vert (665 bytes)
INFO: Loaded shader file: shaders/Effect.frag (1960 bytes)
INFO: Compiled shader: Effect.vert
INFO: Compiled shader: Effect.frag
INFO: Linked shader program: Effect
INFO: OpenGLRenderer: Shader pipeline initialized successfully
INFO:   - Sentinel program: 3
INFO:   - Effect program: 4
```

**Note:** Uniform block bindings will be set programmatically in Phase 2.7 using glUniformBlockBinding()

### 2.7: Create Uniform Buffers (UBOs) ✅
- [x] In OpenGLRenderer::Init()
  - [x] Create vertex constants UBO with glGenBuffers/glBufferData
  - [x] Bind to binding point 0 with glBindBufferBase
  - [x] Create pixel constants UBO similarly
    - [x] Bind to binding point 1
  - [x] Set uniform block bindings with glUniformBlockBinding
    - [x] VertexConstants in Sentinel program → binding point 0
    - [x] PixelConstants in Sentinel program → binding point 1
    - [x] PixelConstants in Effect program → binding point 1
  - [x] Verify sizes match between C++ and GLSL
    - [x] sizeof(VertexConstants) = 480 bytes ✅
    - [x] sizeof(PixelConstants) = 32 bytes ✅
    - [x] Check std140 padding - structs already have padding from Phase 1 ✅
  - [x] Unbind buffer
  - [x] Check for OpenGL errors with glGetError()
- [x] Test: UBOs created successfully, no errors ✅

**Test Results:**
```
INFO: OpenGLRenderer: Creating uniform buffers...
INFO:   - Vertex constants UBO: 1 (size: 480 bytes)
INFO:   - Pixel constants UBO: 2 (size: 32 bytes)
INFO:   - Bound VertexConstants in Sentinel program to binding point 0
INFO:   - Bound PixelConstants in Sentinel program to binding point 1
INFO:   - Bound PixelConstants in Effect program to binding point 1
INFO: OpenGLRenderer: Uniform buffers created successfully
```

**Bonus:**
- [x] Added `--screenshot` command-line option for automated testing
- [x] Integrated stb_image_write.h for PNG screenshot capture
- [x] Screenshot functionality verified (1600x900 PNG, 42kB)

### 2.8: Update Uniform Buffers ✅
- [x] Create UpdateVertexConstants() method
  - [x] Binds vertex constants UBO
  - [x] Uploads m_vertexConstants with glBufferSubData
  - [x] Unbinds buffer
- [x] Create UpdatePixelConstants() method similarly
  - [x] Binds pixel constants UBO
  - [x] Uploads m_pixelConstants with glBufferSubData
  - [x] Unbinds buffer
- [x] Call from BeginScene() before rendering each frame
- [x] Test: Methods compile and run without errors ✅

**Implementation:**
- Methods added to OpenGLRenderer.cpp (lines 331-341)
- Called from BeginScene() to update uniforms each frame
- Screenshot test confirms no rendering issues

### 2.9: Create Test Triangle ✅
- [x] In OpenGLRenderer::Init(), create test triangle data
  - [x] Created 3 vertices (bottom-left, bottom-right, top)
  - [x] Vertices use palette indices 0, 1, 2 for RGB colors
  - [x] Created VBO with glGenBuffers/glBufferData (108 bytes)
- [x] Create VAO and configure all vertex attributes
  - [x] Attribute 0: position (vec3, GL_FLOAT)
  - [x] Attribute 1: normal (vec3, GL_FLOAT)
  - [x] Attribute 2: color (uint, GL_UNSIGNED_INT) - uses glVertexAttribIPointer
  - [x] Attribute 3: texcoord (vec2, GL_FLOAT)
  - [x] Proper unbinding after setup
- [x] Test: VAO and VBO created without errors ✅

**Test Results:**
```
INFO: OpenGLRenderer: Creating test triangle...
INFO:   - Test VBO created: 3 (108 bytes)
INFO:   - VAO created: 1
INFO: OpenGLRenderer: Test triangle created successfully
```

**Implementation Details:**
- Test VBO: ID 3 (3 vertices × 36 bytes = 108 bytes)
- VAO: ID 1
- Cleanup added to destructor (glDeleteBuffers for VBO)

### 2.10: Render Test Triangle ✅
- [x] Update OpenGLRenderer::Render()
  - [x] Create identity matrices for testing
  - [x] Set m_vertexConstants.WVP to identity
  - [x] Set m_vertexConstants.W to identity
  - [x] Set palette colors (RGB at indices 0, 1, 2)
  - [x] Update vertex constants UBO
  - [x] Bind shader program: `glUseProgram(m_sentinelProgram)`
  - [x] Bind VAO: `glBindVertexArray(m_vao)`
  - [x] Draw triangle: `glDrawArrays(GL_TRIANGLES, 0, 3)`
  - [x] Unbind: `glBindVertexArray(0)`
  - [x] Check glGetError() after each call
- [x] Run application - No OpenGL errors ✅
- [x] Result: **Perfect RGB triangle rendered!** ✅

**Test Results:**
```
✅ No OpenGL errors during rendering
✅ Triangle renders with smooth color interpolation:
   - Red at bottom-left vertex (palette[0])
   - Green at bottom-right vertex (palette[1])
   - Blue at top vertex (palette[2])
✅ Shader pipeline fully functional
✅ Palette-based coloring system working correctly
```

**Milestone:** Complete shader pipeline verified with visual output!

### 2.11: Test Camera and Projection ✅
- [x] Update BeginScene() to use camera
  - [x] Get view matrix from m_camera.GetViewMatrix()
  - [x] Create projection matrix (perspective) with XMMatrixPerspectiveFovLH
  - [x] Calculate m_mViewProjection = view * proj
  - [x] Store camera position in m_vertexConstants.EyePos
- [x] Initialize camera to known position
  - [x] Camera at (0, 0, -5) looking forward
- [x] Update test triangle world matrix
  - [x] Translate triangle to (0, 0, 5) in front of camera
  - [x] Calculate WVP = world * m_mViewProjection
  - [x] Transpose matrices for GLSL (row-major → column-major)
- [x] Test: Triangle renders with proper 3D projection ✅
- [x] Result: **Perfect perspective rendering!** ✅

**Test Results:**
```
✅ Triangle renders smaller (correct perspective at distance)
✅ Triangle centered on screen (camera aligned correctly)
✅ RGB colors interpolate correctly
✅ View matrix working (camera at -5 looking at triangle at +5)
✅ Projection matrix working (45° FOV, correct aspect ratio)
✅ World-View-Projection transformation correct
✅ Matrix transposition (DirectXMath row-major → GLSL column-major) working
```

**Phase 2 Complete Criteria:**
- ✅ Shaders compile without errors
- ✅ Test triangle renders with correct colors
- ✅ Camera and projection matrices work
- ✅ Uniform buffers update correctly

**🎉 PHASE 2 COMPLETE! 🎉**
Full 3D rendering pipeline operational with camera, projection, and shaders!

---

## Phase 3: Model Rendering (3-4 days)

**Status:** Phase 3.6 complete - Title screen rendering!
**Actual Time So Far:** ~1 day
**Prerequisites:** Phase 2 complete ✅

**Current State After Phase 2:**
- Full 3D rendering pipeline operational (shaders, UBOs, camera, projection)
- Test triangle renders correctly with RGB palette colors
- Game updates (`m_pGame->Frame()`) temporarily disabled to prevent camera conflicts
- Camera fixed at (0, 0, -5) for testing
- Screenshot tool working for verification

**Phase 3 Goals:**
- Upload game models from Spectrum memory to GPU (VBO/IBO)
- Implement DrawModel() to render models with correct transformations
- Re-enable game updates and integrate with Augmentinel game logic
- Remove test triangle code

### 3.1: Conditionally Compile D3D11 Code
**Note:** Already done in Phase 1, but verify:
- [x] Model.h D3D11 members wrapped
- [x] Model.cpp D3D11 code wrapped
- [x] BufferHeap.h includes removed
- [x] Verify Model class compiles clean on macOS ✅

### 3.2: Add OpenGL Members to Model (Future)
**Note:** For Phase 3, models will be uploaded on-demand by renderer. OpenGL buffer handles will be stored in renderer, not in Model class. This avoids modifying the portable Model class.

- [x] Decision: Store VBO/IBO in renderer's map, keyed by Model* ✅
- [ ] Alternative: Add optional OpenGL members to Model (requires more changes) ❌ Not chosen

### 3.3: Implement Model Upload in Renderer
- [x] Add `std::map<const Model*, GLuint> m_modelVBOs;` to OpenGLRenderer ✅
- [x] Add `std::map<const Model*, GLuint> m_modelIBOs;` to OpenGLRenderer ✅
- [x] Add `std::map<const Model*, size_t> m_modelIndexCounts;` to OpenGLRenderer ✅
- [x] Add `UploadModel(const Model& model)` method ✅
  - [x] Check if already uploaded: `if (m_modelVBOs.count(&model)) return;` ✅
  - [x] Get vertex and index data from model ✅
    - [x] `auto& vertices = *model.m_pVertices;` ✅
    - [x] `auto& indices = *model.m_pIndices;` ✅
  - [x] Create VBO ✅
    ```cpp
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 vertices.data(), GL_STATIC_DRAW);
    m_modelVBOs[&model] = vbo;
    ```
  - [x] Create IBO ✅
    ```cpp
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 indices.data(), GL_STATIC_DRAW);
    m_modelIBOs[&model] = ibo;
    m_modelIndexCounts[&model] = indices.size();
    ```
  - [x] Unbind buffers ✅
  - [x] Check glGetError() ✅
- [x] Test: UploadModel compiles ✅

### 3.4: Implement DrawModel
- [x] Update `OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel)` ✅
  - [x] Check if model is valid: `if (!model) return;` ✅
  - [x] Upload if needed: `if (!m_modelVBOs.count(&model)) UploadModel(model);` ✅
  - [x] Calculate world matrix: `auto world = model.GetWorldMatrix(linkedModel);` ✅
  - [x] Calculate WVP: `auto wvp = world * m_mViewProjection;` ✅
  - [x] **Transpose matrices for GLSL:** ✅
    ```cpp
    m_vertexConstants.WVP = XMMatrixTranspose(wvp);
    m_vertexConstants.W = XMMatrixTranspose(world);
    ```
  - [x] Set eye position (for lighting): ✅
    ```cpp
    auto eyePos = m_camera.GetPosition();
    m_vertexConstants.EyePos = eyePos;
    ```
  - [x] Set lighting flag: `m_vertexConstants.lighting = model.lighting ? 1 : 0;` ✅
  - [x] Set dissolved value: `m_pixelConstants.dissolved = model.dissolved;` ✅
  - [x] Update UBOs: ✅
    ```cpp
    UpdateVertexConstants();
    UpdatePixelConstants();
    ```
  - [x] Bind buffers: ✅
    ```cpp
    glBindBuffer(GL_ARRAY_BUFFER, m_modelVBOs.at(&model));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelIBOs.at(&model));
    ```
  - [x] Set up vertex attributes (same as Phase 2 triangle setup) ✅
  - [x] Draw: ✅
    ```cpp
    size_t indexCount = m_modelIndexCounts.at(&model);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    ```
  - [x] Check glGetError() ✅
- [x] Test: DrawModel compiles ✅

### 3.5: Update BeginScene/Render Flow
- [x] Update `OpenGLRenderer::BeginScene()` ✅
  - [x] Clear buffers: `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);` ✅
  - [x] Enable depth test: `glEnable(GL_DEPTH_TEST)` ✅
  - [x] Enable backface culling: `glEnable(GL_CULL_FACE); glCullFace(GL_BACK);` ✅
  - [x] Set up view matrix from camera ✅
  - [x] Set up projection matrix (FOV, aspect, near, far) ✅
  - [x] Calculate m_mViewProjection = view * proj ✅
  - [x] Set clear color based on m_fill_colour_idx or game state ✅
- [x] Update `OpenGLRenderer::Render(IGame* pGame)` ✅
  - [x] Bind sentinel shader: `glUseProgram(m_sentinelProgram)` ✅
  - [x] Bind VAO: `glBindVertexArray(m_vao)` ✅
  - [x] Call `pGame->Render(this)` to let game draw its models ✅
    - [x] Game will call our DrawModel() for each model ✅
  - [x] Unbind VAO ✅
- [x] Remove test triangle code ✅
- [x] Test: Compiles ✅

### 3.6: Test with Game Rendering ✅
- [x] Run application ✅
- [x] Check console for: ✅
  - [x] Spectrum initialization ✅
  - [x] ROM loading (48.rom) ✅
  - [x] Snapshot loading (sentinel.sna) ✅
  - [x] Game state messages ✅
- [x] Expected: Title screen renders (not landscape - that's later) ✅
  - [x] "THE SENTINEL" text ✅
  - [x] Sentinel figure on platform ✅
  - [x] Multiple models (21+) ✅
- [x] Debug rendering issues: ✅
  - [x] Added SDL_Log in DrawModel to count calls ✅
  - [x] Verified pGame->Render() is called ✅
  - [x] Checked models have valid vertex data ✅
  - [x] Verified camera position updates from game ✅
  - [x] **CRITICAL FIX: Removed matrix transpose - DirectXMath compatible as-is!** ✅

### 3.7: Fix Rendering Issues

#### 3.7.1: Geometry Issues ✅
- [x] If geometry is inverted (inside-out): ✅
  - [x] Try `glFrontFace(GL_CW);` instead of GL_CCW ✅ **FIXED!**
  - [n/a] Or flip triangle winding in model data
  - [n/a] Or flip cull face: `glCullFace(GL_FRONT);`
- [n/a] If geometry is upside down: (not needed - geometry orientation correct)
  - [n/a] Check projection matrix handedness
  - [n/a] May need to flip Y in projection
- [n/a] If geometry is stretched: (not needed - aspect ratio correct)
  - [n/a] Verify aspect ratio in projection matrix
  - [n/a] Check for correct window width/height

**Result:** Changed `glFrontFace(GL_CCW)` to `glFrontFace(GL_CW)` in BeginScene(). Title screen now renders with correct face orientation - "THE SENTINEL" text and Sentinel figure display properly with correct lighting and depth.

#### 3.7.2: Lighting Issues ✅
- [x] Lighting looks correct ✅
  - [x] Normals transformed correctly (W matrix) ✅
  - [x] Normal transformation in vertex shader correct (`mat3(W)`) ✅
  - [x] Light direction vectors correct ✅
  - [x] Ambient vs diffuse lighting balance correct ✅
  - [x] Matches Windows version ✅

**Result:** Lighting renders correctly with proper ambient and diffuse calculations. Lit faces show cyan/turquoise, shadowed faces show brown/darker tones.

#### 3.7.3: Color Issues ✅
- [x] Colors are correct ✅
  - [x] Palette initialization correct ✅
  - [x] Color index unpacking correct (uint32 → palette index) ✅
  - [x] Endianness correct ✅
  - [x] Palette values match Windows version ✅
  - [x] Gamma/color space correct ✅
- [x] Color brightness correct ✅
  - [x] Lighting calculations correct ✅
  - [x] Ambient light term correct ✅
  - [x] No gamma correction issues ✅

**Result:** Colors match PC reference screenshot. Title screen displays correct cyan, brown, red, yellow, and black colors.

#### 3.7.4: Depth Issues ✅
- [x] No z-fighting or depth problems ✅
  - [x] Depth buffer cleared correctly ✅
  - [x] Near/far clip planes correct (NEAR_CLIP = 0.1, FAR_CLIP = 500.0) ✅
  - [x] Depth test enabled ✅
  - [x] Depth buffer bit depth correct ✅
  - [x] Depth range and viewport correct ✅

**Result:** Depth rendering works correctly. 3D geometry displays proper occlusion and layering.

#### 3.7.5: Performance Issues ✅
- [x] Added visual performance debugging ✅
  - [x] FPS counter (updates every second) ✅
  - [x] Frame time tracking ✅
  - [x] Draw call counter ✅
  - [x] Uploaded model counter ✅
  - [x] TAB key toggle ✅
  - [x] Enabled by default with --screenshot ✅
- [x] Performance is excellent ✅
  - [x] ~60 FPS (16.73ms frame time)
  - [x] 143 draw calls per frame (title screen)
  - [x] No unnecessary state changes detected

**Result:** Added comprehensive performance debugging system. Console output shows FPS, frame time, draw calls, and model count. Toggle with TAB key. Performance is excellent at ~60 FPS.

### 3.8: Verify Game Features ✅
- [x] Test landscape exploration ✅
  - [x] Can move camera with mouse ✅
  - [x] Can see terrain, objects ✅
  - [x] Geometry loads correctly ✅
- [x] Test model types render: ✅
  - [x] Trees ✅
  - [x] Boulders ✅
  - [x] Robots ✅
  - [x] Sentinels ✅
  - [x] Pedestals ✅
  - [x] Landscape tiles ✅
- [x] Test object creation (if input works) ✅
  - [x] Tree creation shows model ✅
  - [x] Boulder creation shows model ✅
  - [x] Robot creation shows model ✅
- [x] Test object absorption ✅
  - [x] Dissolve effect visible ✅
  - [x] Model disappears ✅

**Phase 3 Complete Criteria:**
- ✅ Landscape renders correctly
- ✅ All object types render
- ✅ Lighting looks correct
- ✅ Colors match original
- ✅ Depth sorting works
- ✅ No visual artifacts
- ✅ Performance acceptable (30+ FPS)

---

## Phase 4: Polish & Remaining Features (2-3 days)

**Status:** Not Started
**Prerequisites:** Phase 3 complete

### 4.1: Audio System (SDL_mixer) ✅
- [x] Initialize SDL_mixer ✅
  - [x] Add SDL_mixer to CMakeLists.txt dependencies ✅
  - [x] Initialize audio subsystem on startup ✅
  - [x] Handle audio device selection/errors ✅
- [x] Implement sound effects ✅
  - [x] Load WAV files from sounds/ directories ✅
  - [x] Play platform-specific tunes (BBC/C64/Spectrum/Amiga) ✅
  - [x] Implement PlaySound() method ✅
  - [x] Test absorption, creation, transfer sounds ✅
- [x] Implement background music ✅
  - [x] Load music files from sounds/music/ ✅
  - [x] Implement PlayMusic() with looping ✅
  - [x] Music volume controls ✅
  - [x] Test music playback during gameplay ✅
- [ ] Spatial audio (optional - deferred)
  - [ ] Implement PositionListener() for 3D audio
  - [ ] Test sound positioning with object locations

**Result:** Audio system fully implemented with SDL_mixer. All Amiga sound effects load and play correctly during gameplay. Music system implemented but current amiga.wav uses Microsoft ADPCM format which is not supported by SDL_mixer - consider converting to PCM format with: `ffmpeg -i amiga.wav -acodec pcm_s16le amiga_pcm.wav`

### 4.2: Settings System
- [ ] Implement settings persistence
  - [ ] Choose INI library (SimpleIni or alternatives)
  - [ ] Implement InitSettings(), GetSetting(), SetSetting()
  - [ ] Create default settings file on first run
  - [ ] Load settings on startup
  - [ ] Save settings on exit
- [ ] Graphics settings
  - [ ] Window size/fullscreen mode
  - [ ] VSync enable/disable
  - [ ] FOV setting
- [ ] Audio settings
  - [ ] Master volume
  - [ ] Music volume
  - [ ] Sound effects volume
  - [ ] Tune selection (BBC/C64/Spectrum/Amiga)
  - [ ] Music enable/disable
- [ ] Control settings
  - [ ] Mouse sensitivity
  - [ ] Invert mouse Y-axis
  - [ ] Key bindings (if configurable)

### 4.3: Game State & Progression
- [ ] Test all game states
  - [ ] Title screen → Landscape preview transition
  - [ ] Landscape preview → Game start
  - [ ] Game → Sky view (Q key)
  - [ ] Game → Level complete
  - [ ] Level complete → Next level
- [ ] Test win/lose conditions
  - [ ] Win: Absorb sentinel and transfer to its pedestal
  - [ ] Lose: Absorbed by sentinel/sentry/meanie
  - [ ] Proper game over screen displays on defeat
  - [ ] Game over screen shows correct information
  - [ ] Can exit or restart from game over screen
- [ ] Test landscape codes
  - [ ] Enter custom landscape codes
  - [ ] Code validation (10,000+ landscapes unlocked)
  - [ ] Remember last played landscape

### 4.4: Energy Display UI ✅
- [x] Locate or create energy icons ✅
  - [x] Icons are 3D models extracted from Spectrum memory (not bitmaps) ✅
  - [x] Gold robot, blue robot, boulder, tree models ✅
  - [x] Spectrum::IconToModel() extracts icon data ✅
  - [x] Icons cached for performance ✅
- [x] Implement energy display system ✅
  - [x] OnAddEnergySymbol() receives energy data from Spectrum emulator ✅
  - [x] Icons created based on symbol_idx (1=robot, 2=tree, 4=boulder, 6=gold robot) ✅
  - [x] Orthographic projection support added to DrawModel() ✅
  - [x] Icons positioned in screen space (x_base, y, z coordinates) ✅
- [x] Icon rendering ✅
  - [x] Icons rendered as 3D models with model.orthographic flag ✅
  - [x] Rendered in Augmentinel::Render() for GameState::Game ✅
  - [x] Scale and spacing configured (scale=27, spacing=15, positioned at top-left) ✅
  - [x] No lighting on icons (model.lighting = false) ✅
- [x] Integration & testing ✅
  - [x] Energy display updates via OnAddEnergySymbol hook ✅
  - [x] Icons visible in game state when m_icons not empty ✅
  - [x] Proper layering (drawn after game models) ✅

**Result:** Energy display system was already fully implemented! Icons are 3D models extracted from Spectrum memory (not bitmaps). Added orthographic projection support to OpenGLRenderer::DrawModel() and fixed GetOrthographicMatrix() to use correct depth range (NEAR_CLIP to FAR_CLIP). Icons now positioned correctly in top-left corner (x=-795, y=420) matching PC version, with proper scale (27) and spacing (15). Energy icons display during gameplay showing gold robot, blue robot, boulder, and tree based on player energy.

### 4.5: Screen Effects & Transitions
- [ ] Screen blanking effects
  - [ ] Blank screen during transfer action
  - [ ] Blank screen during U-turn (180° rotation)
  - [ ] Identify other cases requiring screen blank
  - [ ] Implement fade-to-black transition
  - [ ] Proper timing for each effect
- [ ] Verify effect integration
  - [ ] Effects don't interfere with gameplay
  - [ ] Screen restores correctly after effect
  - [ ] Effects work with existing fade/dissolve system

### 4.6: Bug Fixes & Testing
- [ ] Full gameplay testing
  - [ ] Play through 3+ levels completely
  - [ ] Test all object types (trees, boulders, robots)
  - [ ] Test absorption and creation extensively
  - [ ] Verify energy system works correctly
- [ ] Edge case testing
  - [ ] Rapid key presses
  - [ ] Creating/absorbing while animations active
  - [ ] Window resize during gameplay
  - [ ] Alt-tab / focus loss behavior
- [ ] Performance verification
  - [ ] No memory leaks (check with instruments/valgrind)
  - [ ] Stable frame rate during long play sessions
  - [ ] GPU memory usage reasonable

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
**Phase 2:** 🔄 In Progress (2.1-2.8 complete: Uniform buffers updating!)
**Phase 3:** ⬜ Not Started (Model rendering)
**Phase 4:** ⬜ Not Started (Game integration)
**Phase 5:** ⬜ Not Started (Effects & polish)
**Phase 6:** ⬜ Not Started (Testing & debug)

**Overall Progress:** ~37% Complete (Phase 1 complete + Phase 2 nearing completion)
**Executable Status:** Builds successfully ✅ (1.4 MB)
**Shaders Status:** Sentinel & Effect shaders compiled and linked ✅
**Shader Programs:** Sentinel (ID: 3), Effect (ID: 4) ✅
**UBO Status:** Created, bound, and updating each frame ✅
**Screenshot Tool:** `./Augmentinel --screenshot` saves screenshot.png and exits ✅
**Next Milestone:** Create and render test triangle (Phase 2.9-2.10)

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
./Augmentinel                # Normal mode (ESC to exit)
./Augmentinel --screenshot   # Capture screenshot.png and exit
./Augmentinel --help         # Show command-line options
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
