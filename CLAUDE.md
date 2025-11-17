# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Augmentinel is a re-skinned version of Geoff Crammond's classic game "The Sentinel" (aka The Sentry). It emulates the ZX Spectrum version for authentic gameplay while adding modern enhancements including accelerated 3D rendering, VR support, and features from other platform ports.

## Build Commands

**Build the project:**
```bash
# Open solution in Visual Studio 2019 or later
# Build using Visual Studio GUI or:
msbuild Augmentinel.sln /p:Configuration=Release /p:Platform=x64
```

**Supported configurations:**
- Debug|Win32
- Debug|x64
- Release|Win32
- Release|x64

**Requirements:**
- Visual Studio 2019 or later (v142 toolset)
- Windows 10 SDK
- DirectX SDK (June 2010) for D3D11 libraries

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
