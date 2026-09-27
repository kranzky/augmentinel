#!/usr/bin/env bash
# Augmentinel build script for macOS and Linux.
#
#   ./build.sh [debug|release|clean|package]
#
#   debug    Debug build in build/ (./build/Augmentinel)
#   release  Release build in build/ (default)
#   clean    Remove build output
#   package  macOS: signed, notarised DMG and itch.io zip in dist/
#            (packaging/macos/build_dmg.sh; see packaging/macos/README.md)
#
# SDL2 and SDL2_mixer are downloaded and built by CMake, so only CMake and a
# C++17 compiler are needed.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="$ROOT/build"

build() {
    cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE="$1"
    cmake --build "$BUILD" --parallel
    echo "Built $BUILD/Augmentinel ($1)"
}

case "${1:-release}" in
    debug)   build Debug ;;
    release) build Release ;;
    clean)   rm -rf "$BUILD" "$ROOT/build-macos-release" "$ROOT/dist" ;;
    package)
        [[ "$OSTYPE" == darwin* ]] || { echo "package is macOS only; Windows packages are built by CI" >&2; exit 1; }
        exec "$ROOT/packaging/macos/build_dmg.sh"
        ;;
    *)
        sed -n '2,13p' "$0" | sed 's/^# \{0,1\}//'
        exit 1
        ;;
esac
