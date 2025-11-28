#!/bin/bash
set -e

# Augmentinel Build Script
# Usage: ./build.sh [debug|release|clean|package]

BUILD_TYPE="${1:-release}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
RELEASE_DIR="$PROJECT_DIR/release"
APP_NAME="Augmentinel"
VERSION="1.6.0"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info() { echo -e "${GREEN}[INFO]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

build_debug() {
    info "Building Debug configuration..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . --parallel
    info "Debug build complete: $BUILD_DIR/$APP_NAME"
}

build_release() {
    info "Building Release configuration..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --parallel
    info "Release build complete: $BUILD_DIR/$APP_NAME"
}

clean() {
    info "Cleaning build directories..."
    rm -rf "$BUILD_DIR"
    rm -rf "$RELEASE_DIR"
    info "Clean complete"
}

package_macos() {
    info "Creating macOS app bundle..."

    # Build release first
    build_release

    # Create app bundle structure
    APP_BUNDLE="$RELEASE_DIR/$APP_NAME.app"
    CONTENTS="$APP_BUNDLE/Contents"
    MACOS="$CONTENTS/MacOS"
    RESOURCES="$CONTENTS/Resources"

    rm -rf "$APP_BUNDLE"
    mkdir -p "$MACOS"
    mkdir -p "$RESOURCES"

    # Copy executable
    cp "$BUILD_DIR/$APP_NAME" "$MACOS/"

    # Copy resources
    cp "$BUILD_DIR/48.rom" "$RESOURCES/"
    cp "$BUILD_DIR/sentinel.sna" "$RESOURCES/"
    cp -r "$BUILD_DIR/shaders" "$RESOURCES/"
    cp -r "$BUILD_DIR/sounds" "$RESOURCES/"

    # Create app icon from PNG
    ICON_SOURCE="$PROJECT_DIR/resources/icon.png"
    if [[ -f "$ICON_SOURCE" ]]; then
        info "Creating app icon..."
        ICONSET_DIR="$RELEASE_DIR/AppIcon.iconset"
        rm -rf "$ICONSET_DIR"
        mkdir -p "$ICONSET_DIR"

        # Generate all required icon sizes
        sips -z 16 16     "$ICON_SOURCE" --out "$ICONSET_DIR/icon_16x16.png" > /dev/null
        sips -z 32 32     "$ICON_SOURCE" --out "$ICONSET_DIR/icon_16x16@2x.png" > /dev/null
        sips -z 32 32     "$ICON_SOURCE" --out "$ICONSET_DIR/icon_32x32.png" > /dev/null
        sips -z 64 64     "$ICON_SOURCE" --out "$ICONSET_DIR/icon_32x32@2x.png" > /dev/null
        sips -z 128 128   "$ICON_SOURCE" --out "$ICONSET_DIR/icon_128x128.png" > /dev/null
        sips -z 256 256   "$ICON_SOURCE" --out "$ICONSET_DIR/icon_128x128@2x.png" > /dev/null
        sips -z 256 256   "$ICON_SOURCE" --out "$ICONSET_DIR/icon_256x256.png" > /dev/null
        sips -z 512 512   "$ICON_SOURCE" --out "$ICONSET_DIR/icon_256x256@2x.png" > /dev/null
        sips -z 512 512   "$ICON_SOURCE" --out "$ICONSET_DIR/icon_512x512.png" > /dev/null
        sips -z 1024 1024 "$ICON_SOURCE" --out "$ICONSET_DIR/icon_512x512@2x.png" > /dev/null 2>&1 || \
            cp "$ICON_SOURCE" "$ICONSET_DIR/icon_512x512@2x.png"  # Use original if can't upscale

        # Convert iconset to icns
        iconutil -c icns "$ICONSET_DIR" -o "$RESOURCES/AppIcon.icns"
        rm -rf "$ICONSET_DIR"
        info "App icon created"
    else
        warn "Icon not found at $ICON_SOURCE, skipping icon creation"
    fi

    # Create Info.plist
    cat > "$CONTENTS/Info.plist" << EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>
    <string>$APP_NAME</string>
    <key>CFBundleDisplayName</key>
    <string>$APP_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.kranzky.$APP_NAME</string>
    <key>CFBundleVersion</key>
    <string>$VERSION</string>
    <key>CFBundleShortVersionString</key>
    <string>$VERSION</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>
    <key>CFBundleIconFile</key>
    <string>AppIcon</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSSupportsAutomaticGraphicsSwitching</key>
    <true/>
</dict>
</plist>
EOF

    # Create PkgInfo
    echo -n "APPL????" > "$CONTENTS/PkgInfo"

    # Ad-hoc code sign the app bundle
    # This prevents the "app is damaged" error for users who download directly
    info "Code signing app bundle (ad-hoc)..."
    codesign --force --deep --sign - "$APP_BUNDLE"
    info "Code signing complete"

    info "App bundle created: $APP_BUNDLE"

    # Create DMG for distribution (optional)
    if command -v hdiutil &> /dev/null; then
        info "Creating DMG..."
        DMG_NAME="$APP_NAME-$VERSION-macOS.dmg"
        rm -f "$RELEASE_DIR/$DMG_NAME"
        hdiutil create -volname "$APP_NAME" -srcfolder "$APP_BUNDLE" -ov -format UDZO "$RELEASE_DIR/$DMG_NAME"
        info "DMG created: $RELEASE_DIR/$DMG_NAME"
    fi
}

package_linux() {
    info "Creating Linux package..."

    # Build release first
    build_release

    # Create package directory
    PACKAGE_DIR="$RELEASE_DIR/$APP_NAME-$VERSION-linux"
    rm -rf "$PACKAGE_DIR"
    mkdir -p "$PACKAGE_DIR"

    # Copy executable and resources
    cp "$BUILD_DIR/$APP_NAME" "$PACKAGE_DIR/"
    cp "$BUILD_DIR/48.rom" "$PACKAGE_DIR/"
    cp "$BUILD_DIR/sentinel.sna" "$PACKAGE_DIR/"
    cp -r "$BUILD_DIR/shaders" "$PACKAGE_DIR/"
    cp -r "$BUILD_DIR/sounds" "$PACKAGE_DIR/"

    # Create run script
    cat > "$PACKAGE_DIR/run.sh" << 'EOF'
#!/bin/bash
cd "$(dirname "$0")"
./$APP_NAME "$@"
EOF
    sed -i "s/\$APP_NAME/$APP_NAME/g" "$PACKAGE_DIR/run.sh" 2>/dev/null || \
        sed -i '' "s/\$APP_NAME/$APP_NAME/g" "$PACKAGE_DIR/run.sh"
    chmod +x "$PACKAGE_DIR/run.sh"
    chmod +x "$PACKAGE_DIR/$APP_NAME"

    # Create tarball
    cd "$RELEASE_DIR"
    tar -czvf "$APP_NAME-$VERSION-linux.tar.gz" "$(basename "$PACKAGE_DIR")"

    info "Linux package created: $RELEASE_DIR/$APP_NAME-$VERSION-linux.tar.gz"
}

case "$BUILD_TYPE" in
    debug)
        build_debug
        ;;
    release)
        build_release
        ;;
    clean)
        clean
        ;;
    package)
        if [[ "$OSTYPE" == "darwin"* ]]; then
            package_macos
        elif [[ "$OSTYPE" == "linux"* ]]; then
            package_linux
        else
            error "Unsupported platform: $OSTYPE"
        fi
        ;;
    *)
        echo "Usage: $0 [debug|release|clean|package]"
        echo ""
        echo "Commands:"
        echo "  debug   - Build debug configuration"
        echo "  release - Build release configuration (default)"
        echo "  clean   - Remove build directories"
        echo "  package - Build release and create distributable package"
        exit 1
        ;;
esac
