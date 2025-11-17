# Augmentinel macOS Porting Analysis

**Goal:** Port Augmentinel from Windows (Win32 + D3D11) to cross-platform (SDL2 + OpenGL)

**Scope:** Non-VR version only, targeting macOS initially but with cross-platform compatibility

**Analysis Date:** 2025-11-17

---

## Executive Summary

Porting Augmentinel to SDL2+OpenGL is **highly feasible**. Approximately 32% of the codebase (~2,500 lines) is already portable and can be reused with minimal changes. The remaining work involves replacing platform-specific layers (windowing, rendering, audio) with cross-platform alternatives.

**Estimated Effort:** 1-2 weeks of focused development
**Risk Level:** Low-Medium (well-trodden path, clean architecture)

---

## Codebase Statistics

- **Total C++ code:** ~7,827 lines
- **Shader code:** ~347 lines (HLSL)
- **Win32 API usage:** 122 occurrences across 22 files
- **D3D11 API usage:** 193 occurrences across 14 files
- **XAudio2 usage:** 14 occurrences across 2 files

---

## Component-by-Component Analysis

### ✅ FULLY PORTABLE (Reuse As-Is)

#### 1. Z80 Emulation (`z80/Z80.c`, `z80/Z80.h`, `z80/Z80-support.h`)
- **Lines:** ~800
- **Platform Dependencies:** None
- **Action Required:** None - pure C implementation
- **Notes:** Licensed under GPL v3 by Manuel Sainz de Baranda y Goñi

#### 2. Spectrum Emulation (`Spectrum.cpp/h`)
- **Lines:** ~600
- **Platform Dependencies:** Uses DirectXMath (XMFLOAT3, XMVECTOR)
- **Action Required:**
  - Option A: Keep DirectXMath (it's cross-platform)
  - Option B: Convert to GLM (OpenGL Mathematics)
- **Key Functions:**
  - `LoadSnapshot()` - loads ZX Spectrum snapshot
  - `RunFrame()` - executes one frame of Z80 code
  - `ExtractLandscape()` - extracts 3D geometry from memory
  - `ExtractModels()` - extracts game objects
- **Dependencies:** `ISentinelEvents` interface for callbacks

#### 3. Core Game Constants (`Sentinel.h`)
- **Lines:** ~200
- **Platform Dependencies:** Uses XMFLOAT4 for palette colors
- **Action Required:** Minimal - just the math types
- **Key Data:**
  - Game constants (FOV, map size, etc.)
  - EGA color palettes
  - Face color mappings
  - `ISentinelEvents` interface definition (pure virtual)
- **Notes:** `ISentinelEvents` is the callback interface from Spectrum → Game

#### 4. Game State Machine (`Augmentinel.cpp/h`)
- **Lines:** ~1,500
- **Platform Dependencies:**
  - References to `VRView` (can be removed)
  - Windows VK_ key codes in input bindings
- **Action Required:**
  - Remove VR-specific code paths
  - Replace VK_ codes with SDL2 keycodes
  - Settings system integration (minor)
- **Key State Machine:**
  ```
  Unknown → Reset → TitleScreen → LandscapePreview → Game
  → SkyView → PlayerDead → ShowKiller → Complete
  ```
- **Implements:**
  - `IGame` interface (Render method)
  - `ISentinelEvents` interface (receives Spectrum callbacks)
  - `IModelSource` interface (model lookups)

#### 5. Animation System (`Animate.cpp/h`)
- **Lines:** ~200
- **Platform Dependencies:** Uses DirectXMath
- **Action Required:** Same as math library decision
- **Features:** Smooth transitions for model transformations

#### 6. Utilities (`Utils.cpp/h`)
- **Lines:** ~150
- **Platform Dependencies:** Some Windows-specific string conversion
- **Action Required:** Replace wide string helpers with cross-platform versions
- **Key Functions:** String utilities, random number helpers

---

### ⚠️ NEEDS ADAPTATION (Moderate Changes)

#### 1. Model System (`Model.cpp/h`)

**Current Structure:**
```cpp
class Model {
    // ✅ Portable data
    std::shared_ptr<std::vector<Vertex>> m_pVertices;
    std::shared_ptr<std::vector<uint32_t>> m_pIndices;
    XMFLOAT3 pos, rot;
    float scale, dissolved;

    // ❌ D3D11-specific (must remove)
    std::shared_ptr<D3D11HeapAllocation> m_pHeapVertices;
    std::shared_ptr<D3D11HeapAllocation> m_pHeapIndices;
    ComPtr<ID3D11VertexShader> m_pVertexShader;
    ComPtr<ID3D11PixelShader> m_pPixelShader;
};
```

**Changes Required:**
1. Remove D3D11HeapAllocation members
2. Remove shader ComPtr members
3. Add OpenGL equivalents:
   ```cpp
   GLuint m_vbo{0};  // Vertex buffer object
   GLuint m_ibo{0};  // Index buffer object
   ```
4. Shader programs will be managed by Renderer, not Model

**Portable Parts:**
- Vertex data structure (XMFLOAT3 pos/normal, UINT32 color, XMFLOAT2 texcoord)
- Geometry generation (`CreateBlock`, normal calculation)
- Ray testing and collision detection
- Transform calculations (`GetWorldMatrix`)
- Bounding box (uses DirectXCollision::BoundingBox - need to replace or keep)

**Line Numbers:**
- D3D11 members defined: `Model.h:54-59`
- Vertex structure: `Vertex.h:3-12` (fully portable)

#### 2. Camera System (`Camera.cpp/h`)

**Platform Dependencies:**
- Uses DirectXMath extensively (XMVECTOR, XMMATRIX, XMFLOAT3)
- Functions: `XMMatrixRotationRollPitchYaw`, `XMVector4Transform`, `XMMatrixLookToLH`

**Decision Required:**
- **Option A:** Keep DirectXMath (Microsoft open-sourced it, works on macOS)
  - Pro: No changes needed
  - Con: Additional dependency
- **Option B:** Convert to GLM
  - Pro: More "native" to OpenGL ecosystem
  - Con: ~135 lines to convert, math function mapping needed

**Recommendation:** Keep DirectXMath initially for speed, consider GLM conversion later

**Portable Parts:**
- All camera logic (pitch/yaw/roll, movement, limits)
- View matrix calculation

---

### ❌ MUST REPLACE (Complete Rewrite)

#### 1. Application Layer (`Application.cpp/h`)

**Current:** Win32-based application (~400 lines)

**Uses:**
- `WinMain` entry point
- `WNDCLASSEX` window class registration
- `CreateWindowEx` for window creation
- `MSG` / `PeekMessage` / `DispatchMessage` message loop
- `CoInitializeEx` for COM
- `DialogBox` for launcher dialog
- Window management (activate, position, cursor clipping)

**Replace With:** SDL2-based application

**New Structure:**
```cpp
class Application {
    SDL_Window* m_window{nullptr};
    SDL_GLContext m_glContext{nullptr};
    std::shared_ptr<OpenGLRenderer> m_pRenderer;
    std::shared_ptr<Audio> m_pAudio;
    std::unique_ptr<Game> m_pGame;

    bool Init();
    void Run();  // SDL event loop
    void ProcessEvent(const SDL_Event& event);
};
```

**Key Changes:**
```cpp
// OLD (Win32)
MSG msg{};
while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

// NEW (SDL2)
SDL_Event event;
while (SDL_PollEvent(&event)) {
    ProcessEvent(event);
}
```

**Estimated Lines:** ~300 lines new code

#### 2. Rendering System (`View.cpp/h`, `FlatView.cpp/h`)

**Current:** D3D11-based renderer (~1,200 lines total)

**D3D11 Components to Replace:**

| D3D11 Component | OpenGL Equivalent | Notes |
|----------------|-------------------|-------|
| `ID3D11Device` | OpenGL context | Created via SDL |
| `ID3D11DeviceContext` | Current GL state | Implicit in OpenGL |
| `IDXGISwapChain1` | SDL_GL_SwapWindow | SDL manages swap |
| `ID3D11Buffer` (vertex) | `glGenBuffers` / VBO | Per-model |
| `ID3D11Buffer` (index) | `glGenBuffers` / IBO | Per-model |
| `ID3D11Buffer` (constant) | `glGenBuffers` / UBO | Uniform buffer |
| `ID3D11VertexShader` | `glCreateShader(GL_VERTEX_SHADER)` | |
| `ID3D11PixelShader` | `glCreateShader(GL_FRAGMENT_SHADER)` | |
| `ID3D11InputLayout` | `glVertexAttribPointer` | VAO setup |
| `ID3D11RenderTargetView` | FBO + texture | For effects |
| `ID3D11DepthStencilView` | Depth renderbuffer | Attached to FBO |
| `ID3D11RasterizerState` | `glCullFace`, etc. | State calls |
| `ID3D11SamplerState` | `glSamplerParameter` | Texture sampling |

**View.h Interface (Keep):**
```cpp
struct IScene {
    virtual void DrawModel(Model& model, const Model& linkedModel = {}) = 0;
    virtual void DrawControllers() = 0;  // Can remove (VR only)
    virtual bool IsPointerVisible() const = 0;
};

class View : public IScene {
    virtual void BeginScene();
    virtual void Render(IGame* pRender) = 0;
    virtual void EndScene() = 0;
    // ... camera, effects, etc.
};
```

**New OpenGLRenderer Class:**
```cpp
class OpenGLRenderer : public View {
    GLuint m_vao{0};
    GLuint m_shaderProgram{0};
    GLuint m_effectProgram{0};
    GLuint m_vertexConstantsUBO{0};
    GLuint m_pixelConstantsUBO{0};
    GLuint m_framebuffer{0};  // For post-processing

    void InitGL();
    void CompileShaders();
    void CreateBuffers();

    void BeginScene() override;
    void Render(IGame* pRender) override;
    void EndScene() override;
    void DrawModel(Model& model, const Model& linkedModel) override;
};
```

**Critical Structures (Keep Compatible):**
```cpp
// View.h:31-41 - These are used in shaders
struct VertexConstants {
    XMMATRIX WVP;           // World-View-Projection
    XMMATRIX W;             // World
    XMFLOAT4 Palette[16];   // Color palette
    XMFLOAT3 EyePos;
    float z_fade;
    float fog_density;
    uint32_t fog_colour_idx;
    uint32_t lighting;
};

struct PixelConstants {
    float dissolved;
    float time;
    float view_dissolve;
    float view_desaturate;
    float view_fade;
};
```

**Estimated Lines:** ~600-800 lines new code

**Key Implementation Points:**

1. **Shader Compilation:**
   ```cpp
   GLuint CompileShader(const char* source, GLenum type);
   GLuint LinkProgram(GLuint vs, GLuint fs);
   ```

2. **Buffer Management:**
   ```cpp
   // Create vertex buffer for model
   glGenBuffers(1, &model.m_vbo);
   glBindBuffer(GL_ARRAY_BUFFER, model.m_vbo);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                vertices.data(), GL_STATIC_DRAW);
   ```

3. **Uniform Buffers:**
   ```cpp
   // Update constant buffers
   glBindBuffer(GL_UNIFORM_BUFFER, m_vertexConstantsUBO);
   glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(VertexConstants),
                   &m_vertexConstants);
   ```

4. **Drawing:**
   ```cpp
   glBindVertexArray(vao);
   glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
   ```

#### 3. Shaders (~347 lines HLSL → GLSL)

**Files to Convert:**
- `shaders/Sentinel_VS.hlsl` → `Sentinel.vert`
- `shaders/Sentinel_PS.hlsl` → `Sentinel.frag`
- `shaders/Effect_VS.hlsl` → `Effect.vert`
- `shaders/Effect_PS.hlsl` → `Effect.frag`

**VR Shaders (Skip):**
- OpenVR_VS/PS.hlsl
- Mirror_VS/PS.hlsl
- Pointer_VS/PS.hlsl (may keep for selection pointer)

**Conversion Guide:**

| HLSL | GLSL | Notes |
|------|------|-------|
| `cbuffer name : register(b0)` | `layout(std140) uniform name` | Uniform block |
| `float4x4` | `mat4` | Matrix type |
| `float4` | `vec4` | Vector type |
| `mul(v, M)` | `M * v` | Matrix multiply (note order!) |
| `POSITION` semantic | `layout(location=0) in vec3` | Vertex attribute |
| `SV_POSITION` | `gl_Position` | Built-in |
| `SV_TARGET` | `layout(location=0) out vec4` | Fragment output |
| `saturate(x)` | `clamp(x, 0.0, 1.0)` | |
| `lerp(a, b, t)` | `mix(a, b, t)` | |
| `frac(x)` | `fract(x)` | |
| `Texture2D tex` | `uniform sampler2D tex` | |
| `tex.Sample(samp, uv)` | `texture(tex, uv)` | |

**Matrix Ordering:**
HLSL uses row-major by default, GLSL uses column-major. Either:
- Transpose matrices before sending to shader, OR
- Use `layout(row_major)` in GLSL (less common)

**Example Conversion (Sentinel_VS.hlsl:43-46):**
```hlsl
// HLSL
VS_OUTPUT output;
output.pos = mul(float4(input.pos.xyz, 1.0f), WVP);
output.uv = input.uv;
```

```glsl
// GLSL
out vec2 v_uv;
void main() {
    gl_Position = WVP * vec4(a_position, 1.0);
    v_uv = a_texcoord;
}
```

**Estimated Effort:** 2-3 hours per shader pair, ~8-12 hours total

#### 4. Audio System (`Audio.cpp/h`)

**Current:** XAudio2 + X3DAudio (~400 lines)

**Uses:**
- `XAudio2Create` for engine initialization
- `IXAudio2MasteringVoice` for output
- `IXAudio2SourceVoice` for playback
- `X3DAudioInitialize` for spatial audio
- HRTF support via hook DLL

**Replace With:** SDL2_mixer (simpler) or OpenAL (3D audio)

**Recommendation:** Start with SDL2_mixer

**New Structure:**
```cpp
class Audio {
    std::map<std::wstring, Mix_Chunk*> m_sounds;
    std::map<std::wstring, Mix_Music*> m_music;

    bool LoadWAV(const fs::path& path);
    void PlaySound(const fs::path& path, float volume = 1.0f);
    void PlayMusic(const fs::path& path, bool loop = false);
    void SetMusicVolume(float volume);
    void Stop();
};
```

**Key Changes:**
- Remove all XAudio2/X3DAudio code
- Remove HRTF spatial audio (can add back with OpenAL later)
- Simpler interface, same functionality for 2D audio

**Estimated Lines:** ~200 lines new code

**Note:** Spatial audio (3D positioning) would require OpenAL instead of SDL2_mixer. Since game uses limited spatial effects, SDL2_mixer should suffice initially.

#### 5. Settings System (`Settings.cpp/h`)

**Current:** Windows INI file API (~77 lines)

**Uses:**
- `GetModuleFileName` - get exe path
- `SHGetFolderPath` - get AppData folder
- `GetPrivateProfileString` - read INI values
- `WritePrivateProfileString` - write INI values
- `GetPrivateProfileInt` - read integer values

**Replace With:** Cross-platform INI parser

**Options:**
1. **inih library** (https://github.com/benhoyt/inih) - simple, public domain
2. **SimpleIni** (https://github.com/brofield/simpleini) - header-only
3. **Custom implementation** - ~100 lines for basic key=value parsing

**Recommendation:** SimpleIni (header-only, no extra build complexity)

**Settings Path:**
```cpp
// OLD (Windows)
SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wpath);

// NEW (Cross-platform)
// macOS: ~/Library/Application Support/Augmentinel/
// Linux: ~/.config/Augmentinel/
// Windows: %APPDATA%/Augmentinel/
std::filesystem::path GetSettingsPath() {
#ifdef __APPLE__
    const char* home = getenv("HOME");
    return fs::path(home) / "Library/Application Support/Augmentinel";
#elif _WIN32
    // Use SHGetFolderPath
#else
    const char* config = getenv("XDG_CONFIG_HOME");
    if (!config) {
        const char* home = getenv("HOME");
        return fs::path(home) / ".config/Augmentinel";
    }
    return fs::path(config) / "Augmentinel";
#endif
}
```

**Estimated Lines:** ~100-150 lines

---

### 🗑️ REMOVE ENTIRELY (VR Components)

**Files to Delete or Ignore:**
- `src/VRView.cpp/h` (~600 lines)
- `src/OpenVR.cpp/h` (~300 lines)
- `openvr/` directory (headers, libs, bindings)
- `shaders/OpenVR_VS/PS.hlsl`
- `shaders/Mirror_VS/PS.hlsl`

**Code to Remove:**
- VR-specific paths in `Augmentinel.cpp` (check for `IsVR()` calls)
- VR input bindings
- HMD reset functionality
- Sky view VR distance calculations (keep flat version)

**References to Clean:**
- `Application.cpp:60-64` - VR view creation
- `Augmentinel.cpp:8` - VRView.h include
- Input bindings for VR controllers

---

## Interface Analysis

### IGame / IScene Pattern

**Excellent Design:** Clean separation between game logic and rendering

```cpp
// Game logic implements IGame
struct IGame {
    virtual void Render(IScene* pScene) = 0;
};

// Renderer implements IScene
struct IScene {
    virtual void DrawModel(Model& model, const Model& linkedModel = {}) = 0;
    virtual bool IsPointerVisible() const = 0;
};
```

**Flow:**
1. `Application::Run()` calls `Game::Frame()` for logic update
2. `Application::Run()` calls `View::Render(game)` for rendering
3. `View::Render()` calls `game->Render(this)` passing itself as IScene
4. Game calls `pScene->DrawModel()` for each visible model

**This pattern stays intact!** Just implement `IScene` in `OpenGLRenderer`

### ISentinelEvents Pattern

**Spectrum emulator callbacks:**
```cpp
struct ISentinelEvents {
    virtual void OnTitleScreen() = 0;
    virtual void OnLandscapeGenerated() = 0;
    virtual void OnNewPlayerView() = 0;
    virtual void OnPlayerDead() = 0;
    virtual void OnInputAction(uint8_t& action) = 0;
    virtual void OnGameModelChanged(int id, bool player_initiated) = 0;
    // ... etc
};
```

**Used by:** `Spectrum::RunFrame()` calls these at specific points in game code execution

**Implementation:** `Augmentinel` implements this interface

**No changes needed** - this abstraction is perfect

---

## Math Library Decision

**DirectXMath Types Used:**
- `XMFLOAT3`, `XMFLOAT4` - storage types
- `XMVECTOR`, `XMMATRIX` - SIMD computation types
- `XMLoadFloat3`, `XMStoreFloat3` - conversion functions
- `XMMatrixRotationRollPitchYaw`, `XMMatrixLookToLH`, etc.
- `BoundingBox` from DirectXCollision

**Option A: Keep DirectXMath**
- ✅ No code changes needed
- ✅ Microsoft open-sourced it, works on all platforms
- ✅ Excellent SIMD performance
- ✅ Available via GitHub: https://github.com/microsoft/DirectXMath
- ❌ Name might be confusing in OpenGL context
- ❌ One extra dependency

**Option B: Convert to GLM**
- ✅ More "native" to OpenGL ecosystem
- ✅ Similar API, easier for OpenGL developers to understand
- ✅ Header-only library
- ❌ Need to convert ~1000 lines of math code
- ❌ Some API differences (e.g., no SIMD wrapper types)
- ❌ Time consuming

**Recommendation:** **Keep DirectXMath**
- Saves significant porting time
- Performance is excellent
- Well-tested and maintained
- Can always convert to GLM later if desired

**Mapping (if converting):**
| DirectXMath | GLM |
|-------------|-----|
| `XMFLOAT3` | `glm::vec3` |
| `XMFLOAT4` | `glm::vec4` |
| `XMMATRIX` | `glm::mat4` |
| `XMLoadFloat3(&f)` | Just use `f` directly |
| `XMStoreFloat3(&f, v)` | `f = v` |
| `XMMatrixRotationRollPitchYaw(p,y,r)` | `glm::eulerAngleYXZ(y,p,r)` |
| `XMMatrixLookToLH(pos,dir,up)` | `glm::lookAt(pos, pos+dir, up)` |

---

## Dependencies Summary

### Remove:
- Win32 SDK
- DirectX SDK
- Direct3D 11
- XAudio2
- OpenVR SDK

### Add:
- **SDL2** (windowing, input, OpenGL context)
- **OpenGL 3.3+** (rendering)
- **SDL2_mixer** (audio) OR **OpenAL** (3D audio)
- **SimpleIni** or **inih** (settings) - header-only or single file
- **DirectXMath** (keep) OR **GLM** (convert)

### Keep:
- Standard C++17
- Standard library (filesystem, chrono, etc.)

---

## Build System Changes

**Current:** Visual Studio project/solution (`.vcxproj`, `.sln`)

**New:** CMake for cross-platform builds

**CMakeLists.txt structure:**
```cmake
cmake_minimum_required(VERSION 3.15)
project(Augmentinel)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
# SDL2_mixer via find_package or add_subdirectory

add_executable(Augmentinel
    src/main.cpp
    src/Application.cpp
    src/OpenGLRenderer.cpp
    src/Augmentinel.cpp
    src/Spectrum.cpp
    src/Model.cpp
    src/Camera.cpp
    src/Audio.cpp
    src/Settings.cpp
    src/Utils.cpp
    # ... etc
    z80/Z80.c
)

target_include_directories(Augmentinel PRIVATE
    src
    z80
    # DirectXMath if keeping
)

target_link_libraries(Augmentinel
    SDL2::SDL2
    SDL2_mixer::SDL2_mixer
    OpenGL::GL
)
```

---

## Risk Assessment

### Low Risk:
- ✅ Z80 emulator is portable
- ✅ Game logic is well abstracted
- ✅ Clean interface boundaries
- ✅ SDL2 and OpenGL are well-documented

### Medium Risk:
- ⚠️ Shader conversion (syntax differences, matrix ordering)
- ⚠️ First-time OpenGL renderer implementation
- ⚠️ Testing without VR to ensure game is still playable

### High Risk Areas:
- ⚠️ Subtle rendering bugs (winding order, depth testing, blending)
- ⚠️ Performance compared to D3D11
- ⚠️ Missing visual effects during conversion

---

## Testing Strategy

1. **Phase-by-phase validation:**
   - Window + OpenGL context → render clear color
   - Single triangle → verify rendering pipeline
   - Single cube → verify transforms
   - Landscape mesh → verify complex geometry
   - Game integration → verify Spectrum extraction

2. **Visual regression testing:**
   - Take screenshots of Windows version
   - Compare with SDL2/OpenGL version
   - Check: colors, geometry, effects, animations

3. **Functional testing:**
   - Can complete landscape 0000 (first level)
   - All input actions work
   - Audio plays correctly
   - Settings persist

---

## Future Enhancements (Post-Port)

1. Linux support (should work with minimal changes)
2. Vulkan renderer (for better performance)
3. OpenAL for spatial audio
4. Gamepad support via SDL2
5. Window scaling/retina support
6. Metal renderer for native macOS

---

## Key File Reference

**Portable (reuse):**
- `z80/Z80.c` - Z80 emulator
- `src/Spectrum.cpp/h` - Game state extraction
- `src/Augmentinel.cpp/h` - Game logic (minor changes)
- `src/Sentinel.h` - Constants and interfaces
- `src/Camera.cpp/h` - Camera logic
- `src/Animate.cpp/h` - Animations

**Adapt:**
- `src/Model.cpp/h` - Remove D3D11 members, add GL buffers

**Rewrite:**
- `src/Application.cpp/h` → SDL2 version
- `src/View.cpp/h` → OpenGL renderer base
- `src/FlatView.cpp/h` → OpenGL renderer implementation
- `src/Audio.cpp/h` → SDL2_mixer version
- `src/Settings.cpp/h` → Cross-platform INI
- `shaders/*.hlsl` → GLSL versions

**Remove:**
- `src/VRView.cpp/h`
- `src/OpenVR.cpp/h`
- `openvr/` directory

---

## Code Snippets for Future Reference

### Vertex Structure (Keep As-Is)
```cpp
// src/Vertex.h - This is perfect for OpenGL
struct Vertex {
    XMFLOAT3 pos;        // 12 bytes, offset 0
    XMFLOAT3 normal;     // 12 bytes, offset 12
    UINT32 colour;       // 4 bytes, offset 24
    XMFLOAT2 texcoord;   // 8 bytes, offset 28
};
// Total: 36 bytes per vertex
```

### OpenGL Vertex Attribute Setup
```cpp
// Corresponds to Vertex structure above
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (void*)offsetof(Vertex, pos));
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (void*)offsetof(Vertex, normal));
glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex),
                      (void*)offsetof(Vertex, colour));
glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                      (void*)offsetof(Vertex, texcoord));
```

### Main Loop Pattern
```cpp
// OLD: Application.cpp:88-109 (Win32)
while (msg.message != WM_QUIT) {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    auto elapsed = CalculateElapsed();
    m_pGame->Frame(elapsed);
    m_pView->BeginScene();
    m_pView->Render(m_pGame.get());
    m_pView->EndScene();
}

// NEW: SDL2 version
bool quit = false;
while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) quit = true;
        ProcessEvent(event);
    }
    auto elapsed = CalculateElapsed();
    m_pGame->Frame(elapsed);
    m_pRenderer->BeginScene();
    m_pRenderer->Render(m_pGame.get());
    m_pRenderer->EndScene();
    SDL_GL_SwapWindow(m_window);
}
```

---

## Questions for Future Implementation

1. **Math Library:** Keep DirectXMath or convert to GLM?
   - Recommendation: Keep DirectXMath initially

2. **Audio:** SDL2_mixer (simple) or OpenAL (3D)?
   - Recommendation: SDL2_mixer initially, OpenAL if spatial audio needed

3. **Settings:** Which INI library?
   - Recommendation: SimpleIni (header-only)

4. **OpenGL Version:** 3.3 core (compatible) or 4.x (modern)?
   - Recommendation: 3.3 core for maximum compatibility

5. **Shader embedding:** Load from files or embed in binary?
   - Recommendation: Load from files initially for easier debugging

---

## Conclusion

This port is **highly feasible** with the proposed SDL2+OpenGL approach. The clean architecture with well-defined interfaces (`IGame`, `IScene`, `ISentinelEvents`) makes it straightforward to replace the platform-specific layers while keeping the core game logic intact.

The biggest chunks of work are:
1. OpenGL renderer implementation (~600-800 lines)
2. Shader conversion (~350 lines HLSL → GLSL)
3. SDL2 application layer (~300 lines)

The payoff is significant: a cross-platform game that runs on macOS, Linux, and Windows with a single codebase.
