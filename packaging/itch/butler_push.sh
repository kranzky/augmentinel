#!/usr/bin/env bash
# Push a signed Augmentinel build to an itch.io channel with butler.
#
#   ./packaging/itch/butler_push.sh mac     dist/Augmentinel-<version>-macos.zip
#   ./packaging/itch/butler_push.sh windows <folder with signed Augmentinel.exe>
#
# CI pushes the windows channel itself on v* tags (.github/workflows/ci.yml).
# The mac channel is pushed from the Mac with this script after
# packaging/macos/build_dmg.sh has signed, notarised and stapled the app, since
# macOS signing does not run in CI. See docs/releasing.md.
#
# Environment:
#   ITCH_TARGET=kranzky/augmentinel  itch.io user/game
#   DRY_RUN=1                        run the checks without pushing
set -euo pipefail

CHANNEL="${1:?usage: butler_push.sh <mac|windows> <zip-or-folder>}"
SRC="${2:?usage: butler_push.sh <mac|windows> <zip-or-folder>}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
ITCH_TARGET="${ITCH_TARGET:-kranzky/augmentinel}"
VERSION="$(sed -nE 's/^project\(Augmentinel VERSION ([0-9.]+).*/\1/p' "$ROOT/CMakeLists.txt")"

case "$CHANNEL" in
    mac|windows) ;;
    *) echo "error: channel must be 'mac' or 'windows'" >&2; exit 1 ;;
esac
[ -e "$SRC" ] || { echo "error: $SRC not found" >&2; exit 1; }
case "$SRC" in
    *unsigned*) echo "error: refusing to publish an unsigned build: $SRC" >&2; exit 1 ;;
esac

# Only macOS can check the notarisation of a macOS build; do it before upload.
if [ "$CHANNEL" = "mac" ]; then
    CHECK="$(mktemp -d)"
    trap 'rm -rf "$CHECK"' EXIT
    if [ -d "$SRC" ]; then cp -R "$SRC/." "$CHECK/"; else ditto -x -k "$SRC" "$CHECK"; fi
    APP="$CHECK/Augmentinel.app"
    [ -d "$APP" ] || { echo "error: no Augmentinel.app at the top of $SRC" >&2; exit 1; }
    [ -f "$CHECK/README.txt" ] || { echo "error: README.txt missing from $SRC" >&2; exit 1; }
    APP_VERSION="$(/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "$APP/Contents/Info.plist")"
    [ "$APP_VERSION" = "$VERSION" ] \
        || { echo "error: app is $APP_VERSION but CMakeLists.txt is $VERSION" >&2; exit 1; }
    codesign --verify --deep --strict "$APP"
    xcrun stapler validate "$APP"
    spctl -a -vvv -t exec "$APP" 2>&1 | tee /dev/stderr | grep -q "Notarized Developer ID" \
        || { echo "error: Augmentinel.app is not notarised (Developer ID)" >&2; exit 1; }
fi

if [ -z "${BUTLER:-}" ]; then
    if command -v butler >/dev/null 2>&1; then BUTLER="$(command -v butler)"
    else BUTLER="$HOME/Library/Application Support/itch/apps/butler/butler"
    fi
fi
[ -x "$BUTLER" ] || { echo "error: butler not found; set BUTLER=<path>" >&2; exit 1; }

cmd=("$BUTLER" push "$SRC" "$ITCH_TARGET:$CHANNEL" --userversion "$VERSION")
echo "==> ${cmd[*]}"
if [ "${DRY_RUN:-0}" = "1" ]; then
    echo "==> DRY_RUN=1: not pushing"
    exit 0
fi
"${cmd[@]}"
"$BUTLER" status "$ITCH_TARGET:$CHANNEL"
