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

### macOS, Windows and Linux (SDL2 + OpenGL)

This fork ports Augmentinel to SDL2 and OpenGL 3.3 for macOS and Windows. You need
CMake 3.21+ and a C++17 compiler (Xcode command line tools, or Visual Studio 2019+).
SDL2, SDL2_mixer and DirectXMath are downloaded and built by CMake, and linked
statically.

```bash
./build.sh            # macOS/Linux: build/Augmentinel
build.bat             # Windows: build\Release\Augmentinel.exe
```

Run `./build.sh debug` for a debug build, and `ctest --test-dir build` for the
smoke test (it boots into landscape 0000 and saves a screenshot).

Signed release builds come from `./build.sh package` on macOS (Developer ID,
notarised DMG) and from CI on Windows (SSL.com eSigner). See
[docs/releasing.md](docs/releasing.md).

Player controls and settings are described in [packaging/README.txt](packaging/README.txt).

### Windows (original D3D11 + VR)

Simon Owen's original Win32/Direct3D 11 version, with OpenVR support, is kept in
the tree (`Augmentinel.sln`) but no longer builds, because the SDL port replaced
several files it shares. Use the [upstream repository](https://github.com/simonowen/augmentinel)
for the VR version.

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
