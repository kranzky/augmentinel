# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Augmentinel is a re-skinned version of Geoff Crammond's classic game "The Sentinel" (aka The Sentry). It emulates the ZX Spectrum version for authentic gameplay while adding modern enhancements including accelerated 3D rendering, VR support, and features from other platform ports.

**IMPORTANT:** This codebase has TWO implementations:
1. **SDL2 + OpenGL (Port)** - macOS/Windows - ✅ **COMPLETE - Ready for Release**
2. **Windows (Legacy)** - Win32 + Direct3D 11 + VR - For VR headset support only

## Build Commands

### SDL2 + OpenGL (macOS) - **RECOMMENDED**

**Status**: ✅ COMPLETE - Ready for Release

**Build (recommended):**
```bash
./build.sh debug      # Debug build with symbols
./build.sh release    # Optimized release build (-O3)
./build.sh clean      # Remove build directories
./build.sh package    # Create app bundle + DMG for distribution
```

**Run:**
```bash
./build/Augmentinel                # Normal mode (ESC to exit)
./build/Augmentinel --screenshot   # Capture screenshot.png and exit
```

**Release Packaging:**
```bash
./build.sh package
# Creates:
#   release/Augmentinel.app              - macOS app bundle
#   release/Augmentinel-1.6.0-macOS.dmg  - Distribution DMG
```

### SDL2 + OpenGL (Windows) - **RECOMMENDED**

**Prerequisites:**
1. Visual Studio 2019+ with C++ tools
2. CMake 3.15+
3. vcpkg package manager

**Install dependencies via vcpkg:**
```cmd
vcpkg install sdl2:x64-windows sdl2-mixer:x64-windows sdl2-ttf:x64-windows glew:x64-windows
```

**Build:**
```cmd
build.bat debug       # Debug build
build.bat release     # Optimized release build
build.bat clean       # Remove build directories
build.bat package     # Build and create distributable package
```

**Release Output:**
```
release\Augmentinel-1.6.0-windows\
├── Augmentinel.exe
├── 48.rom
├── sentinel.sna
├── shaders\
├── sounds\
└── *.dll (SDL2, GLEW, etc.)
```

### Implementation Complete (Both Platforms):
- ✅ SDL2 windowing and OpenGL 3.3 Core context
- ✅ GLSL shaders (Sentinel.vert/frag, Effect.vert/frag)
- ✅ Full game loop with input system (keyboard + mouse)
- ✅ Complete gameplay - playable through all levels
- ✅ Audio system with sound pack switching (keys 1-4)
- ✅ Settings persistence (SimpleIni) - all settings saved/restored
- ✅ Fullscreen toggle (F11 or ALT+Enter)
- ✅ Energy UI display with orthographic projection
- ✅ Screen effects (fade, dissolve, desaturate)

**Key Files:**
- `build.sh` - macOS build and packaging script
- `build.bat` - Windows build and packaging script
- `CMakeLists.txt` - Cross-platform CMake configuration
- `src/OpenGLRenderer.cpp/h` - OpenGL renderer implementation
- `src/Application.cpp/h` - SDL2 application/window management
- `src/Settings.cpp/h` - Settings persistence (SimpleIni)
- `shaders/*.vert, *.frag` - GLSL shaders (OpenGL 3.3)

### Windows (Legacy - DirectX 11 + VR)

For VR headset support only. Uses original Win32/D3D11 codebase.

**Build:**
```cmd
:: Open Augmentinel.sln in Visual Studio 2019+
:: Or from command line:
msbuild Augmentinel.sln /p:Configuration=Release /p:Platform=x64
```

**Requirements:**
- Visual Studio 2019 or later (v142 toolset)
- Windows 10 SDK
- DirectX SDK (June 2010)
- OpenVR SDK (included)

## Architecture

### Core Components

**Application Layer** (`Application.cpp/h`)
- Main application entry point and window management
- Owns and coordinates View, Audio, and Game subsystems
- Handles Win32 message loop and user input

**Emulation Core** (`Spectrum.cpp/h`)
- Emulates ZX Spectrum 48K to run original Sentinel game ROM
- Uses Z80 CPU emulator from Manuel Sainz de Baranda y Goñi
- Hooks into game code at specific addresses to intercept and augment behavior
- Extracts game state (landscape, models, player position) from Spectrum memory
- ROM file: `48.rom`, Snapshot: `sentinel.sna`

**Game Logic** (`Augmentinel.cpp/h`)
- Implements game state machine: TitleScreen → LandscapePreview → Game → SkyView → Complete
- Bridges between emulated Spectrum logic and modern 3D rendering
- Manages animations, ray testing for object selection, and landscape codes
- Interfaces with Spectrum emulator via `ISentinelEvents` callbacks

**View System** (`View.h`, `FlatView.cpp/h`, `VRView.cpp/h`)
- Abstract base class `View` with two implementations:
  - `FlatView`: Traditional monitor rendering with mouse free look
  - `VRView`: OpenVR-based VR headset rendering
- Direct3D 11 rendering pipeline with MSAA support
- Manages cameras, shaders, and render targets
- Effect system for transitions (dissolve, desaturate, fade, fog)

**Audio System** (`Audio.cpp/h`)
- Plays tunes from different platform versions (BBC/C64/Spectrum/Amiga)
- Background music from Amiga version
- HRTF spatial sound effects via X3DAudio

**Model & Rendering** (`Model.cpp/h`)
- Model extraction from Spectrum memory
- Vertex data representation and transformation
- Coordinate conversion between Spectrum's polar system and Cartesian 3D space

### Data Flow

1. `main.cpp` creates `Application`
2. `Application::Init()` creates `View` (Flat or VR based on command line), `Audio`, and `Augmentinel` game
3. `Augmentinel` creates `Spectrum` emulator and loads snapshot
4. Game loop: `Application::Run()` → `Game::Frame()` → `Spectrum::RunFrame()` → `Augmentinel::Render()` → `View::Render()`
5. Spectrum hooks fire callbacks to `ISentinelEvents` (implemented by `Augmentinel`)
6. `Augmentinel` extracts models from Spectrum memory and renders via `IScene` interface

### Key Directories

- `src/` - All C++ source and header files
- `shaders/` - HLSL shader files (Sentinel, Effect, Mirror, OpenVR, Pointer)
- `z80/` - Z80 CPU emulator library
- `openvr/` - OpenVR SDK headers, libraries, and VR controller bindings
- `resources/` - Application resources (icon, manifest, RC file)

### Shader Pipeline

HLSL shaders are compiled as part of the build:
- `Sentinel_VS/PS.hlsl` - Main game object rendering
- `Effect_VS/PS.hlsl` - Post-processing effects (dissolve, desaturate, fade)
- `Mirror_VS/PS.hlsl` - Mirror/reflection rendering
- `OpenVR_VS/PS.hlsl` - VR-specific rendering
- `Pointer_VS/PS.hlsl` - Object selection pointer

Shaders are compiled to shader model 4.0 and embedded as header files in intermediate directory.

### External Dependencies

- **Z80 Emulator**: `z80/Z80.c` - Compiled without precompiled headers, GPL v3 licensed
- **OpenVR**: `openvr/` - Delay-loaded DLL for VR support
- **Direct3D 11**: Windows SDK, used for all rendering
- **X3DAudio**: HRTF spatial audio (x3daudio1_7_hrtf library)

### Platform Notes

- Currently Windows-only (Win32 API + D3D11)
- Future portability planned (see `TODO.txt` for conversion to OpenGL/Vulkan, wxWidgets/Qt, OpenXR)
- Uses C++17 standard
- Static runtime library linkage (MultiThreaded/MultiThreadedDebug)

### Important Constants

- Spectrum runs at 3.5 MHz, 50 FPS (50 frames/second)
- Spectrum memory: 16KB ROM + 48KB RAM
- Game logic executes in Z80 emulator, rendering/audio handled natively
- Unlocked 57,344 hex landscapes (vs original 10,000)

---

## SDL2 + OpenGL Port Architecture (In Development)

### Platform Abstraction

**Platform.h** - Central platform header that handles cross-platform differences:
- Detects platform (PLATFORM_MACOS, PLATFORM_WINDOWS, PLATFORM_LINUX)
- Includes SDL2 and OpenGL headers
- Includes DirectXMath (cross-platform math library)
- Maps VK_* virtual key codes to SDL keycodes
- Wraps Windows-specific code in `#ifdef PLATFORM_WINDOWS`

### OpenGL Renderer (`OpenGLRenderer.cpp/h`)

Implements the `View` interface using OpenGL 3.3 Core Profile:

**Initialization:**
- Creates and compiles GLSL shader programs (Sentinel, Effect)
- Creates uniform buffer objects (UBOs) for shader constants
- Binds uniform blocks to binding points (VertexConstants→0, PixelConstants→1)

**Shader Pipeline:**
- `Sentinel.vert/frag` - Main game rendering with lighting, fog, z-fade
- `Effect.vert/frag` - Post-processing (dissolve, desaturate, fade)
- Shaders loaded at runtime from `shaders/` directory
- std140 layout for uniform blocks matches C++ struct layout

**Uniform Buffers:**
- VertexConstants: 480 bytes (WVP, W, Palette[20], EyePos, lighting flags)
- PixelConstants: 32 bytes (dissolved, noise, view effects)
- Both structs have padding for 16-byte alignment (std140 requirement)

**Key Differences from Windows Version:**
- Runtime shader loading vs compile-time embedding
- Uniform buffers (UBOs) vs constant buffers
- GLSL syntax (mix, clamp) vs HLSL (lerp, saturate)
- Column-major matrices (requires transpose from DirectXMath row-major)
- No binding qualifiers in shaders (OpenGL 3.3 limitation)

### SDL2 Application (`Application.cpp/h`)

Replaces Win32 API with SDL2:
- Window creation and OpenGL context management
- Event loop (SDL_PollEvent vs GetMessage)
- Input handling (keyboard, mouse)
- Screenshot capability (`--screenshot` flag)
  - Uses stb_image_write.h to save PNG
  - Renders one frame and exits for automated testing

### Shared Components (Cross-Platform)

These components work on both platforms with minimal changes:
- **Spectrum.cpp/h** - Z80 emulator (platform-independent)
- **Model.cpp/h** - Model extraction and representation
- **Camera.cpp/h** - Camera mathematics
- **Augmentinel.cpp/h** - Game logic (with platform-specific sections wrapped)

**Portability Strategy:**
- Model class has platform-specific members wrapped in `#ifdef PLATFORM_WINDOWS`
- OpenGL renderer stores VBO/IBO handles separately (not in Model)
- DirectXMath used for math (works on all platforms with proper defines)

### Build System

**CMake** (replaces Visual Studio solution):
- FetchContent for DirectXMath dependency
- SDL2 and OpenGL found via find_package
- Shaders copied to build directory (not compiled)
- Resources (48.rom, sentinel.sna, sounds/) auto-copied

### Current Port Status

See `PORTING_TODO.md` for detailed checklist. Summary:

**Phase 1 (Complete ✅):** Build system, foundation, stubs
**Phase 2 (Complete ✅):** Shader Pipeline - Full 3D Rendering Operational!
- ✅ Shaders converted (HLSL → GLSL)
- ✅ Shader programs compiled and linked
- ✅ Uniform buffers created and updating
- ✅ Test triangle renders with correct colors
- ✅ Camera and projection matrices working
- ✅ Perspective transformation functional
- ✅ Matrix transposition (row-major → column-major)

**Phase 3 (Complete ✅):** Model Rendering & Gameplay
- ✅ GPU model upload with VBO/IBO caching (vertex buffer pointer as cache key)
- ✅ DrawModel() renders from Spectrum memory
- ✅ Full game integration (game updates enabled)
- ✅ Mouse and keyboard input system (SDL2)
- ✅ Object creation and absorption working
- ✅ Complete gameplay - playable through full levels
- ✅ Performance: ~60 FPS, efficient geometry sharing

**Phase 4 (Complete ✅):** Game Integration - Audio, UI & Effects Complete
- ✅ Phase 4.1: Professional audio system (extended implementation)
  - Multi-channel management: Looping effects, tunes, sound effects isolated
  - Music continuous playback (never interrupted by tunes/effects)
  - Sound pack switching: Keys 1-4 for Amiga/C64/BBC/Spectrum with hot-reload
  - MP3 music format (93% disk space savings)
  - Music starts at landscape select (not on startup)
- ✅ Phase 4.2 (partial): Audio & Input fixes
  - Music toggle uses pause/resume (not halt) for proper re-enable
  - Volume up key fixed (= key instead of + which requires shift)
  - Modifier keys excluded from VK_ANY (allows ALT-TAB, screenshots)
  - State change sound stopping refined (only landscape↔game transitions)
- ✅ Phase 4.4: Energy UI display (orthographic projection, dynamic positioning)
- ✅ Phase 4.5: Screen effects & transitions (framebuffer post-processing)
- ✅ Phase 4.6: Bug fixes (landscape navigation, icon positioning, key repeat)
- ⏳ Phase 4.2 (remaining): Settings persistence, fullscreen toggle
- ⏳ Phase 4.3: Game state testing (deferred to Phase 5)

**Important Notes for Phase 4.4 (Energy Icons):**
- Energy icons are **3D models** extracted from Spectrum memory via `Spectrum::IconToModel()`, NOT bitmaps
- Icons use orthographic projection (set `model.orthographic = true` in `OnAddEnergySymbol()`)
- Orthographic coordinate system for 1600x900 resolution: `x=[-800,800], y=[-450,450], z=[NEAR_CLIP, FAR_CLIP]`
- Icon positioning: Dynamic based on window size, `scale=27, spacing=35`
- `OpenGLRenderer::GetOrthographicMatrix()` uses `NEAR_CLIP` to `FAR_CLIP` depth range (NOT 0.0 to 1.0)
- `DrawModel()` checks `model.orthographic` flag and uses orthographic projection instead of perspective
- Icon rendering differs by mode:
  - **Flat mode**: Static positions set in `OnAddEnergySymbol()` (top-left corner)
  - **VR mode**: Dynamic positions relative to camera (repositioned each frame)
- Icons only render during `GameState::Game`, not on title screen
- symbol_idx values: 0=empty, 1=robot(blue), 2=tree(green), 4=boulder(cyan), 6=gold robot(yellow)

**Future Phases:**
- Phase 5: Polish & testing
  - Fix deprecation warnings (codecvt_utf8 in Utils.h)
  - Settings persistence (SimpleIni integration)
  - Fullscreen toggle (F11)
  - Comprehensive gameplay testing
- Phase 6: Final verification & documentation

**Known Issues / Suggested Improvements:**
- Deprecation warnings from `std::codecvt_utf8` (C++17 deprecated, still functional)
- Settings don't persist between sessions (stub implementation)
- Consider gamepad support for Steam Deck compatibility

**Recent Improvements (2025-11-25):**
- ✅ Spatial audio implemented (stereo panning + distance attenuation via Mix_SetPosition)
- ✅ PlaySound() memory leak fixed (one-off chunks now tracked and cleaned up)
