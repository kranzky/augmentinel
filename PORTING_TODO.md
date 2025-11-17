# Augmentinel Porting TODO List

**Status:** Not Started
**Current Phase:** Phase 1 (Preparation)

Use this file to track progress through the SDL2+OpenGL port. Check off items as you complete them.

---

## Phase 1: Build System & Foundation (2-3 days)

### 1.1: CMake Build System
- [ ] Install CMake 3.15+
- [ ] Install SDL2 via Homebrew: `brew install sdl2`
- [ ] Install SDL2_mixer via Homebrew: `brew install sdl2_mixer`
- [ ] Create `CMakeLists.txt` in project root
  - [ ] Set up project name and C++17 standard
  - [ ] Add `find_package(SDL2 REQUIRED CONFIG)`
  - [ ] Add `find_package(OpenGL REQUIRED)`
  - [ ] Set up FetchContent for DirectXMath
  - [ ] List all source files
  - [ ] Configure include directories
  - [ ] Link libraries
  - [ ] Add resource copy commands
- [ ] Test CMake configuration: `mkdir build && cd build && cmake ..`
- [ ] Verify CMake runs without errors

### 1.2: Platform Abstraction
- [ ] Create `src/Platform.h`
  - [ ] Add platform detection macros
  - [ ] Include SDL2 headers
  - [ ] Include OpenGL headers (gl3.h for macOS)
  - [ ] Include DirectXMath headers
  - [ ] Add common STL includes
  - [ ] Define APP_NAME and APP_VERSION
- [ ] Test: Verify Platform.h compiles standalone

### 1.3: Stub Application Class
- [ ] Modify `src/Application.h`
  - [ ] Remove Win32-specific members (HINSTANCE, HWND, etc.)
  - [ ] Add SDL_Window* member
  - [ ] Add SDL_GLContext member
  - [ ] Change View to OpenGLRenderer
  - [ ] Update method signatures
- [ ] Rewrite `src/Application.cpp`
  - [ ] Implement Init() with SDL_Init
  - [ ] Create SDL window with OpenGL context
  - [ ] Set OpenGL attributes (3.3 core profile)
  - [ ] Implement Run() with SDL event loop
  - [ ] Implement ProcessEvent() for SDL events
  - [ ] Implement Shutdown() to cleanup SDL
  - [ ] Remove all Win32 API calls
- [ ] Test: Application compiles (won't link yet)

### 1.4: Minimal main.cpp
- [ ] Rewrite `src/main.cpp`
  - [ ] Change from WinMain to main(int argc, char* argv[])
  - [ ] Create Application instance
  - [ ] Call Init() and Run()
  - [ ] Add try-catch for exceptions
- [ ] Test: main.cpp compiles

### 1.5: Stub OpenGL Renderer
- [ ] Create `src/OpenGLRenderer.h`
  - [ ] Inherit from View class
  - [ ] Add OpenGL member variables (VAO, programs, UBOs)
  - [ ] Declare Init(), BeginScene(), Render(), EndScene()
  - [ ] Declare DrawModel() override
  - [ ] Implement required View interface methods
- [ ] Create `src/OpenGLRenderer.cpp`
  - [ ] Implement constructor/destructor
  - [ ] Implement Init() - just clear color for now
  - [ ] Implement BeginScene() - glClear
  - [ ] Implement Render() - empty for now
  - [ ] Implement EndScene() - empty
  - [ ] Implement DrawModel() - stub
  - [ ] Implement View interface stubs
- [ ] Test: OpenGLRenderer compiles

### 1.6: Modify Portable Code
- [ ] Update `src/Spectrum.cpp` - replace `#include "stdafx.h"` with `#include "Platform.h"`
- [ ] Update `src/Augmentinel.cpp` - replace stdafx.h, remove VRView.h include
- [ ] Update `src/Model.cpp` - replace stdafx.h
- [ ] Update `src/Camera.cpp` - replace stdafx.h
- [ ] Update `src/Animate.cpp` - replace stdafx.h
- [ ] Update `src/Utils.cpp` - replace stdafx.h, fix Windows string functions
- [ ] Remove `src/stdafx.h` and `src/stdafx.cpp` from build
- [ ] Test: All files compile

### 1.7: Stub Audio and Settings
- [ ] Create stub `src/Audio.cpp` (empty implementations)
- [ ] Create stub `src/Settings.cpp` (empty implementations)
- [ ] Test: Full project compiles

### 1.8: First Build
- [ ] Run CMake build: `cd build && cmake --build .`
- [ ] Fix any compilation errors
- [ ] Run executable: `./Augmentinel`
- [ ] Verify: Window opens with clear color (blue/black)
- [ ] Verify: ESC key closes window
- [ ] Verify: No crashes

**Phase 1 Complete:** ✅ SDL2 window opens, OpenGL context created, program runs

---

## Phase 2: Shader Pipeline (2-3 days)

### 2.1: Convert Sentinel Vertex Shader
- [ ] Create `shaders/Sentinel.vert`
- [ ] Add `#version 330 core`
- [ ] Convert vertex input semantics to layout locations
  - [ ] `layout(location = 0) in vec3 a_position;`
  - [ ] `layout(location = 1) in vec3 a_normal;`
  - [ ] `layout(location = 2) in vec4 a_color;`
  - [ ] `layout(location = 3) in vec2 a_texcoord;`
- [ ] Convert cbuffer to uniform block
  - [ ] `layout(std140) uniform VertexConstants { ... }`
- [ ] Convert output semantics to out variables
  - [ ] `out vec4 v_color;`
  - [ ] `out vec2 v_texcoord;`
- [ ] Convert main() function
  - [ ] Change `mul(v, M)` to `M * v`
  - [ ] Change `saturate()` to `clamp(x, 0.0, 1.0)`
  - [ ] Change `lerp()` to `mix()`
- [ ] Copy lighting calculations from HLSL
- [ ] Test: Shader syntax check (will compile in Phase 2.5)

### 2.2: Convert Sentinel Fragment Shader
- [ ] Create `shaders/Sentinel.frag`
- [ ] Add `#version 330 core`
- [ ] Convert input from vertex shader: `in vec4 v_color;`
- [ ] Convert uniform block for PixelConstants
- [ ] Convert output: `out vec4 FragColor;`
- [ ] Port dissolve/noise logic
  - [ ] Copy `rnd()` function
  - [ ] Convert `clip()` to `discard`
- [ ] Test: Shader syntax check

### 2.3: Convert Effect Shaders
- [ ] Create `shaders/Effect.vert`
  - [ ] Use gl_VertexID to generate fullscreen quad
  - [ ] Output texture coordinates
- [ ] Create `shaders/Effect.frag`
  - [ ] Add `uniform sampler2D u_texture;`
  - [ ] Add PixelConstants uniform block
  - [ ] Port view_dissolve logic
  - [ ] Port view_desaturate logic
  - [ ] Port view_fade logic
- [ ] Test: Shader syntax check

### 2.4: Implement Shader Loading
- [ ] Add shader loading methods to OpenGLRenderer
  - [ ] `std::string LoadShaderFile(const char* filename)`
  - [ ] `GLuint CompileShader(const char* source, GLenum type, const char* name)`
  - [ ] `GLuint LinkProgram(GLuint vs, GLuint fs, const char* name)`
- [ ] Update OpenGLRenderer::Init()
  - [ ] Load Sentinel vertex shader
  - [ ] Load Sentinel fragment shader
  - [ ] Compile both shaders
  - [ ] Link into m_sentinelProgram
  - [ ] Check for errors, log if failed
- [ ] Test: Shaders compile and link without errors

### 2.5: Create Uniform Buffers
- [ ] In OpenGLRenderer::Init()
  - [ ] Create vertex constants UBO: `glGenBuffers(1, &m_vertexConstantsUBO)`
  - [ ] Allocate storage: `glBufferData(GL_UNIFORM_BUFFER, sizeof(VertexConstants), ...)`
  - [ ] Bind to binding point 0: `glBindBufferBase(GL_UNIFORM_BUFFER, 0, ...)`
  - [ ] Create pixel constants UBO similarly
  - [ ] Bind to binding point 1
- [ ] Verify VertexConstants and PixelConstants match between C++ and GLSL
  - [ ] Check alignment (std140 rules)
  - [ ] May need padding in C++ structs
- [ ] Test: UBOs created without errors

### 2.6: Render Test Triangle
- [ ] Create test triangle data in OpenGLRenderer::Init()
  - [ ] Array of 3 vertices with position, normal, color, uv
  - [ ] Create VBO and upload data
  - [ ] Create VAO
  - [ ] Set up vertex attributes (4 attributes)
- [ ] Update OpenGLRenderer::Render()
  - [ ] Create test view/projection matrices
  - [ ] Update vertex constants UBO
  - [ ] Set palette color (red for testing)
  - [ ] Bind shader program: `glUseProgram(m_sentinelProgram)`
  - [ ] Bind VAO
  - [ ] Draw triangle: `glDrawArrays(GL_TRIANGLES, 0, 3)`
- [ ] Test: Red triangle appears on screen
- [ ] Debug: Use glGetError() to check for OpenGL errors
- [ ] Debug: If black screen, check shader compilation logs

**Phase 2 Complete:** ✅ Shaders compile, test triangle renders correctly

---

## Phase 3: Model Rendering (3-4 days)

### 3.1: Modify Model Class
- [ ] Open `src/Model.h`
- [ ] Remove D3D11 members:
  - [ ] Comment out or delete `m_pHeapVertices`
  - [ ] Comment out or delete `m_pHeapIndices`
  - [ ] Comment out or delete `m_pVertexShader`
  - [ ] Comment out or delete `m_pPixelShader`
- [ ] Add OpenGL members:
  - [ ] `GLuint m_vbo{0};`
  - [ ] `GLuint m_ibo{0};`
  - [ ] `bool m_uploaded{false};`
- [ ] Remove BufferHeap.h include (D3D11-specific)
- [ ] Test: Model.h compiles

### 3.2: Remove D3D References from Model.cpp
- [ ] Open `src/Model.cpp`
- [ ] Remove any D3D11 buffer creation code
- [ ] Remove any D3D11 shader assignment code
- [ ] Keep all geometry/math logic intact
- [ ] Test: Model.cpp compiles

### 3.3: Implement Model Upload in Renderer
- [ ] Add `UploadModel(Model& model)` method to OpenGLRenderer
  - [ ] Check if already uploaded: `if (model.m_uploaded) return;`
  - [ ] Get vertex and index data from model
  - [ ] Create VBO: `glGenBuffers(1, &model.m_vbo)`
  - [ ] Upload vertex data: `glBufferData(GL_ARRAY_BUFFER, ...)`
  - [ ] Create IBO: `glGenBuffers(1, &model.m_ibo)`
  - [ ] Upload index data: `glBufferData(GL_ELEMENT_ARRAY_BUFFER, ...)`
  - [ ] Set `model.m_uploaded = true`
- [ ] Test: UploadModel compiles

### 3.4: Implement DrawModel
- [ ] Update `OpenGLRenderer::DrawModel(Model& model, const Model& linkedModel)`
  - [ ] Check if model is valid: `if (!model) return;`
  - [ ] Upload if needed: `if (!model.m_uploaded) UploadModel(model);`
  - [ ] Calculate world matrix: `auto world = model.GetWorldMatrix(linkedModel);`
  - [ ] Calculate WVP: `auto wvp = world * m_mViewProjection;`
  - [ ] Transpose matrices for GLSL: `m_vertexConstants.WVP = XMMatrixTranspose(wvp);`
  - [ ] Set lighting flag: `m_vertexConstants.lighting = model.lighting ? 1 : 0;`
  - [ ] Update vertex constants UBO
  - [ ] Set dissolved value: `m_pixelConstants.dissolved = model.dissolved;`
  - [ ] Update pixel constants UBO
  - [ ] Bind VBO and IBO
  - [ ] Set up vertex attributes (4 attributes, see PORTING_PLAN.md)
  - [ ] Draw: `glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);`
- [ ] Test: DrawModel compiles

### 3.5: Update BeginScene/Render
- [ ] Update `OpenGLRenderer::BeginScene()`
  - [ ] Clear color and depth: `glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);`
  - [ ] Enable depth test: `glEnable(GL_DEPTH_TEST)`
  - [ ] Enable backface culling: `glEnable(GL_CULL_FACE)`
  - [ ] Set up view matrix from camera
  - [ ] Set up projection matrix
  - [ ] Calculate m_mViewProjection
  - [ ] Set eye position in vertex constants
- [ ] Update `OpenGLRenderer::Render(IGame* pGame)`
  - [ ] Use sentinel shader: `glUseProgram(m_sentinelProgram)`
  - [ ] Call `pGame->Render(this)` to let game draw models
- [ ] Remove test triangle code
- [ ] Test: Compiles

### 3.6: Test with Game
- [ ] Run application
- [ ] Check console for Spectrum loading
- [ ] Verify landscape renders
- [ ] Verify game objects render
- [ ] Debug: Check for OpenGL errors
- [ ] Debug: Verify vertex data is correct
- [ ] Debug: Check matrix calculations

### 3.7: Fix Rendering Issues
- [ ] Check if geometry is inverted (winding order)
  - [ ] May need to flip triangle winding
  - [ ] Or change `glFrontFace(GL_CCW)` vs `GL_CW`
- [ ] Check if lighting looks correct
  - [ ] Verify normals are transformed correctly
  - [ ] Check light direction vectors
- [ ] Check depth testing
  - [ ] Verify depth buffer is cleared
  - [ ] Check near/far clip planes
- [ ] Verify colors match original
  - [ ] Check palette values
  - [ ] Verify color index is passed correctly

**Phase 3 Complete:** ✅ Landscape and game objects render correctly

---

## Phase 4: Game Integration (2-3 days)

### 4.1: Input Mapping
- [ ] Create `src/InputMapping.h`
- [ ] Implement `MapSDLKeyToVK()` function
  - [ ] Map SDLK_ESCAPE → VK_ESCAPE
  - [ ] Map SDLK_RETURN → VK_RETURN
  - [ ] Map arrow keys
  - [ ] Map letter keys (A-Z)
  - [ ] Map function keys
  - [ ] Map mouse buttons to VK_LBUTTON, VK_RBUTTON, etc.
- [ ] Update `Application::ProcessKeyEvent()`
  - [ ] Call MapSDLKeyToVK()
  - [ ] Call renderer->UpdateKey() with mapped value
  - [ ] Pass KeyState::DownEdge or UpEdge
- [ ] Update `Application::ProcessEvent()` for mouse buttons
  - [ ] Handle SDL_MOUSEBUTTONDOWN
  - [ ] Handle SDL_MOUSEBUTTONUP
  - [ ] Map to VK_ codes
- [ ] Test: Keyboard input works in game

### 4.2: Settings System
- [ ] Download SimpleIni.h from GitHub
- [ ] Place in `src/` directory
- [ ] Update `src/Settings.cpp`
  - [ ] Include SimpleIni.h
  - [ ] Remove all Win32 API calls
  - [ ] Implement `GetSettingsPath()` for macOS
    - [ ] Return ~/Library/Application Support/Augmentinel/
    - [ ] Create directory if doesn't exist
  - [ ] Implement `InitSettings()`
    - [ ] Create CSimpleIniA instance
    - [ ] Load from file
  - [ ] Implement `GetSetting()` string version
    - [ ] Use ini.GetValue()
  - [ ] Implement `GetSetting()` int version
    - [ ] Use ini.GetLongValue()
  - [ ] Implement `GetFlag()` bool version
    - [ ] Use ini.GetBoolValue()
  - [ ] Implement `SetSetting()` template
    - [ ] Use ini.SetValue()
    - [ ] Call ini.SaveFile()
  - [ ] Implement `RemoveSetting()`
  - [ ] Implement `GetSettingKeys()`
    - [ ] Use ini.GetAllKeys()
- [ ] Test: Settings save and load correctly
- [ ] Test: Landscape codes persist

### 4.3: Audio System
- [ ] Update `src/Audio.h`
  - [ ] Remove XAudio2 includes
  - [ ] Remove X3DAudio includes
  - [ ] Add SDL_mixer.h include
  - [ ] Replace member variables with SDL_mixer types
    - [ ] `std::map<std::wstring, Mix_Chunk*> m_sounds;`
    - [ ] `Mix_Music* m_music{nullptr};`
  - [ ] Simplify interface (remove spatial audio for now)
- [ ] Rewrite `src/Audio.cpp`
  - [ ] Implement constructor
    - [ ] Call Mix_OpenAudio()
    - [ ] Set m_initialized flag
  - [ ] Implement destructor
    - [ ] Free all Mix_Chunk*
    - [ ] Free Mix_Music*
    - [ ] Call Mix_CloseAudio()
  - [ ] Implement LoadWAV()
    - [ ] Use Mix_LoadWAV()
    - [ ] Store in map
  - [ ] Implement PlaySound()
    - [ ] Find chunk in map
    - [ ] Call Mix_PlayChannel()
    - [ ] Set volume with Mix_Volume()
  - [ ] Implement PlayMusic()
    - [ ] Use Mix_LoadMUS()
    - [ ] Call Mix_PlayMusic()
  - [ ] Implement SetMusicVolume()
    - [ ] Call Mix_VolumeMusic()
  - [ ] Implement Stop()
    - [ ] Call Mix_HaltChannel() and Mix_HaltMusic()
- [ ] Test: Sound effects play
- [ ] Test: Music plays
- [ ] Debug: Check audio file paths
- [ ] Debug: Verify WAV format compatibility

### 4.4: Remove VR References
- [ ] Open `src/Augmentinel.cpp`
  - [ ] Remove `#include "VRView.h"`
  - [ ] Remove `#include "OpenVR.h"`
  - [ ] Find all `IsVR()` checks
  - [ ] Replace with flat mode behavior
  - [ ] Remove VR sky view distance calculation
  - [ ] Remove controller drawing
- [ ] Open `src/View.h`
  - [ ] Remove `DrawControllers()` from IScene (or make it no-op)
  - [ ] Remove VR-specific methods
- [ ] Test: Game runs without VR code

### 4.5: Camera Controls
- [ ] Verify mouse look works
  - [ ] Test yaw (left/right)
  - [ ] Test pitch (up/down)
  - [ ] Check pitch limits
- [ ] Verify keyboard camera works
  - [ ] Arrow keys for look
  - [ ] Keys for player rotation in game
- [ ] Test mouse capture
  - [ ] Implement relative mouse mode: `SDL_SetRelativeMouseMode(SDL_TRUE)`
  - [ ] Disable when needed (menus, etc.)
- [ ] Fix mouse sensitivity
  - [ ] Tune m_mouse_divider value
  - [ ] Test with different mice

### 4.6: Fix Augmentinel Integration Issues
- [ ] Check that game state machine works
  - [ ] Title screen appears
  - [ ] Can select landscape
  - [ ] Can enter game
  - [ ] Can complete landscape
- [ ] Verify player controls
  - [ ] Can rotate left/right
  - [ ] Can look up/down
  - [ ] Can create robot/tree/boulder
  - [ ] Can absorb
  - [ ] Can transfer
- [ ] Check HUD/text rendering
  - [ ] Energy icons display
  - [ ] Text renders correctly
- [ ] Fix any timing issues
  - [ ] Spectrum frame rate correct (50 Hz)
  - [ ] Animations smooth

**Phase 4 Complete:** ✅ Game is fully playable, all features work

---

## Phase 5: Effects & Polish (2-3 days)

### 5.1: Framebuffer for Post-Processing
- [ ] Add members to OpenGLRenderer
  - [ ] `GLuint m_framebuffer{0};`
  - [ ] `GLuint m_sceneTexture{0};`
  - [ ] `GLuint m_depthbuffer{0};`
- [ ] Create FBO in Init()
  - [ ] `glGenFramebuffers(1, &m_framebuffer)`
  - [ ] Create color texture attachment
  - [ ] Create depth renderbuffer attachment
  - [ ] Check FBO completeness
- [ ] Update OnResize()
  - [ ] Recreate FBO textures at new size
- [ ] Test: FBO creates without errors

### 5.2: Implement Effect Pass
- [ ] Load and compile Effect shaders
  - [ ] Load Effect.vert and Effect.frag
  - [ ] Compile and link into m_effectProgram
- [ ] Update BeginScene()
  - [ ] If effects active, bind FBO
  - [ ] Otherwise, bind default framebuffer (0)
- [ ] Update EndScene()
  - [ ] If effects active:
    - [ ] Unbind FBO (bind 0)
    - [ ] Clear screen
    - [ ] Use effect shader program
    - [ ] Bind scene texture
    - [ ] Update pixel constants
    - [ ] Draw fullscreen quad
- [ ] Test: Effects render correctly

### 5.3: Implement View Effects
- [ ] Test dissolve effect
  - [ ] Trigger effect transition
  - [ ] Verify dissolve animation
- [ ] Test fade effect
  - [ ] Verify screen fades to black
- [ ] Test desaturate effect
  - [ ] Verify grayscale transition
- [ ] Fix any effect visual bugs
  - [ ] Check noise generation
  - [ ] Check blend modes

### 5.4: Implement MSAA
- [ ] Request MSAA in window creation
  - [ ] Already done: `SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4)`
- [ ] Enable MSAA: `glEnable(GL_MULTISAMPLE)`
- [ ] Test: Check if edges are smoother
- [ ] Make MSAA configurable in settings

### 5.5: Polish Visuals
- [ ] Verify fog rendering
  - [ ] Check fog density calculation
  - [ ] Test in different landscapes
- [ ] Verify z-fade (distance fade)
  - [ ] Test in sky view
- [ ] Check color accuracy
  - [ ] Compare screenshots with Windows version
  - [ ] Adjust palette if needed
- [ ] Test all visual effects
  - [ ] Robot creation/absorption
  - [ ] Tree creation/absorption
  - [ ] Transfer effect
  - [ ] Hyperspace
  - [ ] Death animation

### 5.6: Performance Optimization
- [ ] Profile with Instruments (macOS)
  - [ ] Check CPU usage
  - [ ] Check GPU usage
- [ ] Optimize if needed
  - [ ] Batch draw calls if possible
  - [ ] Reduce uniform buffer updates
  - [ ] Use glMapBuffer for large uploads
- [ ] Target 60+ FPS at 1920x1080
- [ ] Test on different Macs (Intel vs Apple Silicon)

**Phase 5 Complete:** ✅ All effects work, visuals polished, performance good

---

## Phase 6: Testing & Debug (2-3 days)

### 6.1: Gameplay Testing
- [ ] Test title screen
  - [ ] Music plays
  - [ ] Can start game
- [ ] Test landscape selection
  - [ ] Can browse landscapes
  - [ ] Can enter code
  - [ ] Last landscape remembered
- [ ] Play landscape 0000
  - [ ] Can complete successfully
  - [ ] All features work
- [ ] Play 10 different landscapes
  - [ ] Test variety of terrain
  - [ ] Test different difficulties
- [ ] Test death sequence
  - [ ] Verify death detection
  - [ ] Check restart

### 6.2: Input Testing
- [ ] Test all keyboard controls
  - [ ] Movement/rotation
  - [ ] Actions (A/T/B/R/Q/H/U)
  - [ ] Menus (arrows, enter, escape)
  - [ ] Pause
  - [ ] Music/tune toggles
- [ ] Test mouse controls
  - [ ] Free look
  - [ ] Object selection
  - [ ] Mouse buttons for actions
- [ ] Test window interactions
  - [ ] Resize window
  - [ ] Minimize/restore
  - [ ] Close window

### 6.3: Audio Testing
- [ ] Verify all sound effects
  - [ ] Dissolve
  - [ ] Turn
  - [ ] Absorb
  - [ ] Transfer
  - [ ] Hyperspace
  - [ ] Death
  - [ ] Meanie
  - [ ] Ping/pong
- [ ] Verify music
  - [ ] Title music
  - [ ] Background music
  - [ ] Music volume control
  - [ ] Music on/off toggle
- [ ] Check audio sync
  - [ ] Sounds play at correct times
  - [ ] No delay or lag

### 6.4: Settings Testing
- [ ] Test settings persistence
  - [ ] Close and reopen app
  - [ ] Verify settings retained
- [ ] Test landscape codes
  - [ ] Enter code for landscape
  - [ ] Close and reopen
  - [ ] Verify code remembered
- [ ] Test mouse speed setting
- [ ] Test MSAA setting
- [ ] Test sound pack selection

### 6.5: Stress Testing
- [ ] Play for extended period (30+ minutes)
  - [ ] Check for memory leaks
  - [ ] Monitor FPS stability
  - [ ] Watch for crashes
- [ ] Test edge cases
  - [ ] Very high landscapes
  - [ ] Very low landscapes
  - [ ] Complex terrain
- [ ] Test rapid input
  - [ ] Spam keys
  - [ ] Rapid mouse movement

### 6.6: Bug Fixing
- [ ] Fix any crashes found
- [ ] Fix any visual glitches
- [ ] Fix any audio issues
- [ ] Fix any input problems
- [ ] Fix any performance issues
- [ ] Test fixes

### 6.7: Code Cleanup
- [ ] Remove debug logging
- [ ] Remove commented-out D3D11 code
- [ ] Add comments where needed
- [ ] Check for TODO comments
- [ ] Run code formatter
- [ ] Check for compiler warnings

### 6.8: Final Validation
- [ ] Clean build from scratch
  - [ ] Delete build directory
  - [ ] Run cmake and build
  - [ ] Verify no warnings
- [ ] Test release build
  - [ ] Build in Release mode
  - [ ] Verify performance
  - [ ] Check optimization
- [ ] Create distributable
  - [ ] Copy executable
  - [ ] Copy resources (sounds, shaders, ROMs)
  - [ ] Test on fresh Mac (if possible)

**Phase 6 Complete:** ✅ Game is stable, polished, and ready to play

---

## Future Enhancements (Post-Port)

### Linux Support
- [ ] Test build on Linux
- [ ] Fix any platform-specific issues
- [ ] Update CMake for Linux libraries
- [ ] Create Linux package

### Windows Support (SDL2 version)
- [ ] Test build on Windows
- [ ] Install GLEW for Windows OpenGL
- [ ] Update CMake for Windows paths
- [ ] Create Windows installer

### Additional Features
- [ ] Vulkan renderer
- [ ] OpenAL for spatial audio
- [ ] Gamepad support
- [ ] Steam Deck optimization
- [ ] 4K/Retina support
- [ ] Shaders from file reload (for modding)
- [ ] Custom landscape editor

---

## Notes & Decisions

**Math Library:** Using DirectXMath (cross-platform)
**Settings:** Using SimpleIni
**Audio:** Using SDL2_mixer (2D audio only initially)
**OpenGL Version:** 3.3 Core Profile

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

---

## Common Issues & Solutions

**Issue:** "SDL2 not found"
**Solution:** `brew install sdl2 sdl2_mixer`

**Issue:** Black screen, no rendering
**Solution:** Check glGetError(), verify shader compilation logs, check FBO status

**Issue:** Geometry upside down
**Solution:** Flip Y coordinate or adjust projection matrix

**Issue:** Geometry inside out
**Solution:** Change winding order or glFrontFace()

**Issue:** Matrix math wrong
**Solution:** Remember to transpose matrices for GLSL (row-major to column-major)

**Issue:** Uniform buffer not updating
**Solution:** Check std140 alignment, verify binding points match

**Issue:** Audio not playing
**Solution:** Check file paths, verify SDL_mixer initialization, check Mix_OpenAudio() return value

**Issue:** Performance poor
**Solution:** Profile with Instruments, check for unnecessary state changes, verify VSync

---

## Progress Tracking

**Phase 1:** ⬜ Not Started
**Phase 2:** ⬜ Not Started
**Phase 3:** ⬜ Not Started
**Phase 4:** ⬜ Not Started
**Phase 5:** ⬜ Not Started
**Phase 6:** ⬜ Not Started

**Overall Progress:** 0% Complete

---

## Contact & Help

- SDL2 Documentation: https://wiki.libsdl.org/
- OpenGL Reference: https://docs.gl/
- DirectXMath: https://github.com/microsoft/DirectXMath
- SimpleIni: https://github.com/brofield/simpleini

For questions about original codebase, see PORTING_ANALYSIS.md
