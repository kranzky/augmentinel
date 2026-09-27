#!/usr/bin/env bash
# Build, codesign, notarise and staple a distributable macOS DMG (and an itch.io
# zip) for Augmentinel. Ported from blast_radius's packaging/macos/build_dmg.sh.
#
#   ./packaging/macos/build_dmg.sh
#
# Prerequisites (one-time; see packaging/macos/README.md):
#   - A "Developer ID Application" certificate in your login keychain.
#   - A notarytool credential profile, e.g. the existing Lineage one:
#       xcrun notarytool store-credentials lineage-notary \
#         --apple-id <you@example.com> --team-id 869X25LQ6F --password <app-specific-pw>
#
# Outputs, in dist/:
#   Augmentinel-<version>-macOS.dmg   drag-to-Applications disk image
#   Augmentinel-<version>-macos.zip   Augmentinel.app + README.txt, for butler/itch.io
# Unsigned builds carry an "-unsigned" suffix so they cannot be mistaken for a release.
#
# Environment overrides:
#   SIGN_IDENTITY="Developer ID Application: ..."  signing identity
#   NOTARY_PROFILE=lineage-notary                   notarytool keychain profile
#   SKIP_NOTARIZE=1   signed but not notarised (fast local check)
#   UNSIGNED=1        no identity at all: ad-hoc sign, skip DMG signing and
#                     notarisation. Used automatically when SIGN_IDENTITY is not
#                     in the keychain. Such a build will not open on other Macs
#                     without a Gatekeeper override; use it only for testing.
#   MACOS_ARCHS=arm64 single-arch build for a faster local test
#   SKIP_SMOKE=1      skip running the signed app (it needs a logged-in GUI session)
#   DMG_SKIP_LAYOUT=1 skip create-dmg's Finder/AppleScript window layout (for
#                     headless or non-interactive sessions)
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD="${BUILD:-$ROOT/build-macos-release}"
DIST="$ROOT/dist"
STAGE="$BUILD/stage"
ITCH="$BUILD/itch"
APP_NAME="Augmentinel.app"
EXE="Augmentinel"
VERSION="$(sed -nE 's/^project\(Augmentinel VERSION ([0-9.]+).*/\1/p' "$ROOT/CMakeLists.txt")"
VOL="Augmentinel"

SIGN_IDENTITY="${SIGN_IDENTITY:-Developer ID Application: Jason Hutchens (869X25LQ6F)}"
NOTARY_PROFILE="${NOTARY_PROFILE:-lineage-notary}"

# Decide signed vs unsigned before spending time on the build.
UNSIGNED="${UNSIGNED:-0}"
if [ "$UNSIGNED" != "1" ]; then
    if ! security find-identity -v -p codesigning 2>/dev/null | grep -qF "$SIGN_IDENTITY"; then
        echo "WARNING: signing identity not found in the keychain:"
        echo "           $SIGN_IDENTITY"
        echo "         Building UNSIGNED (ad-hoc). Set SIGN_IDENTITY or install the"
        echo "         certificate for a release build (packaging/macos/README.md)."
        UNSIGNED=1
    fi
fi
SUFFIX=""
[ "$UNSIGNED" = "1" ] && SUFFIX="-unsigned"
DMG="$DIST/Augmentinel-${VERSION}-macOS${SUFFIX}.dmg"
ZIP="$DIST/Augmentinel-${VERSION}-macos${SUFFIX}.zip"

# Ship a universal2 (arm64 + x86_64) binary so the app runs natively on both
# Apple Silicon and Intel Macs. CMAKE_OSX_ARCHITECTURES is a global cache
# variable, so SDL2 and SDL2_mixer build both slices too.
MACOS_ARCHS="${MACOS_ARCHS:-arm64;x86_64}"

echo "==> Configure + build Augmentinel $VERSION (Release, bundle, archs: $MACOS_ARCHS)"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release -DAUGMENTINEL_MACOS_BUNDLE=ON \
    -DCMAKE_OSX_ARCHITECTURES="$MACOS_ARCHS"
cmake --build "$BUILD" --target "$EXE" -j

echo "==> Verify binary slices"
APP_BIN="$BUILD/$APP_NAME/Contents/MacOS/$EXE"
lipo -archs "$APP_BIN"
for arch in ${MACOS_ARCHS//;/ }; do
    lipo -archs "$APP_BIN" | tr ' ' '\n' | grep -qx "$arch" \
        || { echo "ERROR: $APP_BIN is missing the '$arch' slice"; exit 1; }
done

echo "==> Verify only system libraries are linked"
# Dependency lines are indented; the others name the file and architecture.
if otool -L "$APP_BIN" | awk '/^[[:space:]]/ {print $1}' | grep -vE '^(/System/Library/|/usr/lib/)'; then
    echo "ERROR: $APP_BIN links non-system libraries (listed above)"; exit 1
fi

echo "==> Stage $APP_NAME"
rm -rf "$STAGE" "$ITCH"; mkdir -p "$STAGE"
cp -R "$BUILD/$APP_NAME" "$STAGE/$APP_NAME"
/usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" -c "Print :CFBundleShortVersionString" \
    -c "Print :LSMinimumSystemVersion" "$STAGE/$APP_NAME/Contents/Info.plist"

if [ "$UNSIGNED" = "1" ]; then
    echo "==> Codesign (ad-hoc, UNSIGNED build: no identity, no notarisation)"
    codesign --force --deep --sign - "$STAGE/$APP_NAME"
else
    echo "==> Codesign (Developer ID, hardened runtime, timestamped)"
    codesign --force --deep --options runtime --timestamp \
        --sign "$SIGN_IDENTITY" "$STAGE/$APP_NAME"
fi
codesign --verify --deep --strict --verbose=2 "$STAGE/$APP_NAME"

# The signed bundle must still start and reach the game: boot the emulator, tap
# through the title screen, start landscape 0000 and capture a frame. This needs
# a GUI session (OpenGL), so it can be skipped for headless runs.
if [ "${SKIP_SMOKE:-0}" != "1" ]; then
    echo "==> Smoke-test the staged app"
    "$STAGE/$APP_NAME/Contents/MacOS/$EXE" --screenshot "$BUILD/smoke.png" \
        --frames 300 --press 30:Space --press 120:Return
fi

echo "==> Build DMG"
mkdir -p "$DIST"; rm -f "$DMG"
if command -v create-dmg >/dev/null 2>&1; then
    layout=()
    [ "${DMG_SKIP_LAYOUT:-0}" = "1" ] && layout=(--skip-jenkins)
    create-dmg \
        --volname "$VOL" \
        --volicon "$ROOT/resources/icon.icns" \
        --window-size 640 360 \
        --icon-size 110 \
        --icon "$APP_NAME" 165 180 \
        --app-drop-link 470 180 \
        ${layout[@]+"${layout[@]}"} \
        "$DMG" "$STAGE" \
    || hdiutil create -volname "$VOL" -srcfolder "$STAGE" -ov -format UDZO "$DMG"
else
    hdiutil create -volname "$VOL" -srcfolder "$STAGE" -ov -format UDZO "$DMG"
fi

make_zip() {
    # itch.io/butler build: the app plus the player readme and licence. ditto keeps the
    # bundle's executable bit and the stapled ticket; no AppleDouble files.
    mkdir -p "$ITCH"
    cp -R "$STAGE/$APP_NAME" "$ITCH/$APP_NAME"
    cp "$ROOT/packaging/README.txt" "$ROOT/COPYING" "$ITCH/"
    rm -f "$ZIP"
    ditto -c -k --norsrc --noextattr --noacl "$ITCH" "$ZIP"
    echo "==> itch.io zip: $ZIP"
}

if [ "$UNSIGNED" = "1" ]; then
    make_zip
    echo "==> UNSIGNED build (not for release): $DMG"
    exit 0
fi

echo "==> Sign the DMG"
codesign --force --sign "$SIGN_IDENTITY" --timestamp "$DMG"

if [ "${SKIP_NOTARIZE:-0}" = "1" ]; then
    make_zip
    echo "==> SKIP_NOTARIZE=1: signed but not notarised: $DMG"
    exit 0
fi

echo "==> Notarise (submits to Apple and waits)"
xcrun notarytool submit "$DMG" --keychain-profile "$NOTARY_PROFILE" --wait

echo "==> Staple"
xcrun stapler staple "$DMG"
xcrun stapler validate "$DMG"
# The DMG's ticket also covers the app inside it, so the app can be stapled
# too; the itch.io zip then opens cleanly offline.
xcrun stapler staple "$STAGE/$APP_NAME"
xcrun stapler validate "$STAGE/$APP_NAME"
make_zip

echo "==> Done: $DMG"
spctl -a -vvv -t install "$DMG" || true
spctl -a -vvv -t exec "$STAGE/$APP_NAME" || true
