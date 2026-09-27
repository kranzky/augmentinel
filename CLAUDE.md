# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project Overview

Augmentinel is a re-skinned version of Geoff Crammond's "The Sentinel" (aka The
Sentry), originally by Simon Owen for Windows/D3D11/VR. It emulates the ZX Spectrum
version for authentic gameplay, hooking the game code to drive modern 3D rendering,
sound and music.

This fork ports it to **SDL2 + OpenGL 3.3** for macOS and Windows; that is the only
build that works. Simon's Win32/D3D11/OpenVR code (`Augmentinel.sln`, `FlatView`,
`VRView`, `OpenVR`, `BufferHeap`, `StateTracker`, `stdafx`, the HLSL shaders, and
`#ifdef PLATFORM_WINDOWS` blocks) is kept in the tree but no longer builds. The SDL
build never defines `PLATFORM_WINDOWS`, including on Windows, so those blocks are inert.

## Build

CMake 3.21+ and a C++17 compiler. SDL2 2.32, SDL2_mixer 2.8 (WAV + minimp3 only)
and DirectXMath are fetched as pinned tarballs and linked statically; there are no
system dependencies and no DLLs/dylibs to ship.

```bash
./build.sh [debug|release|clean|package]   # macOS/Linux; build/Augmentinel
build.bat  [debug|release|clean|package]   # Windows; build\Release\Augmentinel.exe
ctest --test-dir build                     # smoke test (needs a desktop session)
```

`./build.sh package` runs `packaging/macos/build_dmg.sh`: universal2 app bundle,
Developer ID signing, notarisation, DMG and itch.io zip in `dist/`. Windows release
builds are signed in CI (`.github/workflows/ci.yml`, SSL.com eSigner). See
`docs/releasing.md`, `packaging/macos/README.md` and `packaging/windows/SIGNING.md`.

The version lives only in `project(Augmentinel VERSION x.y.z)` in `CMakeLists.txt`;
it is generated into `Version.h`, `Info.plist` and the Windows `.rc`.

If an old `build/` fails to configure (for example "Threads are needed"), it holds a
stale cache from the pre-1.7 Homebrew build: run `./build.sh clean`.

## Command line (testing)

```bash
./build/Augmentinel --screenshot out.png --frames 300 --press 30:Space --press 120:Return
```

`--press FRAME:KEY` taps an SDL-named key (press and release in the same frame),
`--frames` sets when the screenshot is taken, then the app exits. The example boots
into landscape 0000; CI and `build_dmg.sh` use it as a smoke test. Always check
visual changes this way and look at the PNG.

## Architecture

- `main.cpp` parses arguments and shows fatal errors in a message box.
- `Application` owns the SDL window, GL context, `OpenGLRenderer`, `Audio`, the game
  and `DebugOverlay`; runs the loop; handles TAB (stats), F11/Alt+Enter
  (fullscreen) and 1-4 (sound packs). It finds resources beside the executable or in
  a bundle's `Contents/Resources`, and keeps settings in `SDL_GetPrefPath`
  (`~/Library/Application Support/Augmentinel/settings.ini`,
  `%APPDATA%\Augmentinel\settings.ini`), importing older settings files once.
- `Spectrum` wraps the Z80 emulator (`z80/`), loads `48.rom` + `sentinel.sna`,
  patches the game and installs hooks that call `ISentinelEvents`.
- `Augmentinel` (upstream game logic, tab-indented) is the state machine:
  Reset → TitleScreen → LandscapePreview → Game → SkyView/PlayerDead/ShowKiller/Complete.
  It extracts models from Spectrum memory and draws them through `IScene`.
- `View` holds cameras, effects and input; `OpenGLRenderer` implements it.
- `Audio` uses SDL_mixer: channel 0 for the looping "seen" drone, a tune group
  and an effect group, positional panning, streamed MP3 music.
- `Settings` wraps SimpleIni; every change is saved immediately.
- `GL.h`/`GL.cpp` load OpenGL 3.3 entry points through SDL (replacing GLEW); on
  macOS the system `gl3.h` is used directly.

### Input model (`View`)

A key press is latched until an action consumes it, so each press triggers one
action even if the emulated game only polls input later. `InputAction()` consumes
presses, except the continuous turn/look actions, which are active while held. A
press and release in the same frame (a tap or trackpad click) still triggers once.
`EndInputFrame()` drops unconsumed taps; focus loss releases all keys.

### Effects

`View::TransitionEffect(effect, target, elapsed, time)` moves an effect towards a
target at a rate covering the full 0-1 range in `time` seconds, and returns true
once it is already at the target. It is stateless, so it can be called every frame.

### Rendering

- The scene renders into a multisampled FBO (`MsaaSamples` setting, default 4),
  then is blitted to the window, or resolved into a texture for the effect shader
  (dissolve, desaturate, fade) when any view effect is active.
- Sizes are drawable pixels (HiDPI aware). The UI's orthographic space is 1000
  units high from the bottom-left; energy icons are placed there.
- GPU meshes are cached per shared vertex/index vector, and `weak_ptr`s detect
  freed geometry, so the cache never needs manual clearing.
- DirectXMath matrices are row-major for row vectors; GLSL reads them transposed,
  so `M * v` in the shaders equals `v * M` in C++. No explicit transpose is needed.
- The landscape is drawn without back-face culling; everything else culls
  clockwise-wound back faces.
- GLSL shaders live in `shaders/*.vert|frag` and are loaded at runtime. The UBO
  structs in `View.h` must match the std140 blocks.

## Conventions

- Port files use 4-space indents and Allman braces (`.editorconfig`); upstream
  files (`Augmentinel.cpp`, `Spectrum.cpp`, `Model.cpp`, ...) use tabs. Match the file.
- Warnings are on (`-Wall -Wextra`, `/W3`); keep the build warning-free.
- `g_resourcePath` is an `fs::path`; build paths with `/` and use `u8path`/`u8string`
  for anything that crosses into SDL or C APIs, so non-ASCII paths work on Windows.
