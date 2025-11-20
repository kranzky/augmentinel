# Augmentinel

By Simon Owen (simon@simonowen.com)

---

## Introduction

Augmentinel is re-skinned version of the Geoff Crammond classic: [The
Sentinel](https://en.wikipedia.org/wiki/The_Sentinel_(video_game)) (aka The
Sentry).

It emulates the Spectrum version of the game for the original gameplay, then
adds the best features from other ports, plus a few modern extras.

For more details see: https://simonowen.com/spectrum/augmentinel/

## Features

- Accelerated 3D rendering with mouse free look.
- VR support for OpenVR-compatible headsets.
- Palette and landscape colours from PC version.
- BBC/C64/Spectrum/Amiga tunes and HRTF spatial sound effects.
- Background music from Amiga version.
- Sky view from PC/ST/Amiga versions.
- Unlocked the hex landscapes for a total of 57344.
- Pixel-perfect object selection.
- All remaining game logic runs as normal.

## Building

### Windows (Original)

Building the Windows version requires Visual Studio 2019 or later.

The Windows code uses the Win32 and D3D11 APIs.

### macOS/Linux (SDL2 + OpenGL Port) 🚧 In Progress

A cross-platform port using SDL2 and OpenGL is currently in development.

**Status**: Phase 1 Complete ✅, Phase 2 Complete ✅ 🎉

**Current Phase**: Phase 3 - Model Rendering

**Recent Milestones**:
- ✅ SDL2 + OpenGL 3.3 build system working
- ✅ HLSL shaders converted to GLSL (Sentinel & Effect)
- ✅ Shader programs compiled and linked successfully
- ✅ Uniform buffers (UBOs) created and updating correctly
- ✅ Test triangle renders with perspective projection
- ✅ Camera system operational (view + projection matrices)
- ✅ Full 3D rendering pipeline functional
- ✅ Screenshot tool for automated testing (`--screenshot`)

**Next**: Phase 3 - Upload and render game models from Spectrum emulator

**Known Issues**:
- Game updates temporarily disabled during Phase 2 testing (will re-enable in Phase 3)

#### Prerequisites

**macOS:**
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake sdl2 sdl2_mixer
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-mixer-dev
```

#### Build Instructions

```bash
# Clone repository (if not already done)
cd augmentinel

# Create build directory and configure
mkdir build
cd build
cmake ..

# Build
cmake --build .

# Run (from build directory)
./Augmentinel
```

#### Clean Build

To perform a complete clean build:

```bash
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
```

#### Build Details

- **C++ Standard**: C++17
- **Graphics API**: OpenGL 3.3 Core Profile
- **Math Library**: DirectXMath (cross-platform)
- **Windowing**: SDL2
- **Audio**: SDL2_mixer (planned for Phase 4)
- **Build System**: CMake 3.15+

#### Testing

```bash
# Run normally (ESC to exit)
./Augmentinel

# Capture screenshot and exit (for automated testing)
./Augmentinel --screenshot

# Show help
./Augmentinel --help
```

The screenshot tool renders one frame, saves `screenshot.png` (1600x900), and exits automatically.

#### Controls

R	        Create Robot
B	        Create Boulder
T	        Create Tree
A	        Absorb Object
Q	        Transfer to Robot
H	        Hyperspace
U	        U-turn
Mouse Move	Pan View
L Button	Absorb
R Button	Transfer to Robot
P / Pause	Pause Game
Esc	Quit


#### Current Limitations

- Model rendering not yet implemented (Phase 2.8-2.11, then Phase 3)
- Audio system stubbed out (Phase 4)
- Settings system stubbed out (Phase 4)
- VR support not yet ported (future enhancement)

For detailed porting progress, see:
- `PORTING_TODO.md` - Detailed task checklist
- `PORTING_PLAN.md` - Implementation plan
- `PORTING_ANALYSIS.md` - Technical analysis

## License

The Augmentinel source code is licensed under the [GNU GPL v3.0
license](https://www.gnu.org/licenses/gpl-3.0.html).

Z80 CPU emulation by [Manuel Sainz de Baranda y Goñi](https://github.com/redcode/Z80),
licensed under GNU GPL v3.

X3DAudio HRTF support by [Roman Kosmos](https://github.com/kosumosu/x3daudio1_7_hrtf),
licensed under GNU GPL v3.



## Disclaimer

This is an unofficial fan creation, distributed without charge. I have no
affiliation with the original developer, publisher, or other rights holders.

## Contact

Simon Owen  
[https://simonowen.com](https://simonowen.com)
