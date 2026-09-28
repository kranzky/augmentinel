# macOS packaging

`build_dmg.sh` produces a code-signed, notarised, stapled
`dist/Augmentinel-<version>-macOS.dmg` that opens cleanly through Gatekeeper, plus
`dist/Augmentinel-<version>-macos.zip` (the same stapled app, `README.txt` and
`COPYING`) for the itch.io `mac` channel. It is ported from blast_radius and uses
the same Developer ID certificate and notarisation profile.

Signing runs locally on the Mac, not in CI. CI builds the same universal2 bundle,
ad-hoc signed, for testing only.

## One-time setup

You need an **Apple Developer Program** membership.

1. **Developer ID Application certificate** in your login keychain (the one
   blast_radius and Lineage use):

   ```bash
   security find-identity -v -p codesigning | grep "Developer ID Application"
   ```

2. **A notarytool credential profile** in the keychain, never in the repo. The
   script defaults to the existing `lineage-notary` profile; to use another, create
   it like this and pass `NOTARY_PROFILE=augmentinel-notary`:

   ```bash
   xcrun notarytool store-credentials augmentinel-notary \
     --apple-id "you@example.com" \
     --team-id 869X25LQ6F \
     --password "<app-specific-password>"
   ```

   App-specific passwords are made at <https://appleid.apple.com> under Sign-In
   and Security > App-Specific Passwords.

## Build a release DMG

From the repository root, on the tagged commit:

```bash
./packaging/macos/build_dmg.sh     # or: ./build.sh package
```

The script:

1. builds a Release `Augmentinel.app`, universal2 (arm64 + x86_64), checks both
   slices with `lipo`, and checks that only system libraries are linked (SDL2 and
   SDL2_mixer are static);
2. codesigns it with `Developer ID Application: Jason Hutchens (869X25LQ6F)`,
   hardened runtime, timestamped, and verifies with `codesign --verify --deep --strict`;
3. smoke-tests the signed app: it boots the emulator, taps through the title
   screen into landscape 0000, and saves `build-macos-release/smoke.png`
   (this needs a logged-in desktop session; `SKIP_SMOKE=1` skips it);
4. packages a drag-to-Applications DMG (`create-dmg`, falling back to `hdiutil`)
   and signs the DMG;
5. submits the DMG to Apple with `notarytool --wait`, then staples and validates
   the DMG *and* the app inside it;
6. writes the itch.io zip (see [docs/releasing.md](../../docs/releasing.md)).

Useful overrides:

- `SKIP_NOTARIZE=1`: signed but not notarised (fast local check; it will still
  warn on other Macs).
- `UNSIGNED=1`: no identity at all. It ad-hoc signs, skips DMG signing and
  notarisation, and names the outputs `*-unsigned.*`. The script also falls back to
  this, with a warning, when `SIGN_IDENTITY` is not in the keychain. Unsigned
  builds are for testing only; never upload them.
- `SIGN_IDENTITY="Developer ID Application: …"` / `NOTARY_PROFILE=…` if yours
  differ from the defaults.
- `MACOS_ARCHS=arm64` for a faster single-arch test build.
- `DMG_SKIP_LAYOUT=1` skips create-dmg's Finder AppleScript, which can raise an
  Automation prompt when run non-interactively.

## Bundle identity

`packaging/macos/Info.plist.in` is configured by CMake when
`AUGMENTINEL_MACOS_BUNDLE=ON`:

| Key | Value |
| --- | --- |
| `CFBundleName` / `CFBundleDisplayName` | Augmentinel |
| `CFBundleIdentifier` | `com.kranzky.Augmentinel` (unchanged from 1.6.x) |
| `CFBundleShortVersionString` / `CFBundleVersion` | `project(VERSION)` in CMakeLists.txt |
| `CFBundleIconFile` | `icon` (`resources/icon.icns`) |
| `LSMinimumSystemVersion` | `CMAKE_OSX_DEPLOYMENT_TARGET` (10.15) |

`resources/icon.icns` is generated from `resources/icon.png` (256×256):

```bash
mkdir icon.iconset
for s in 16 32 128 256; do
  sips -z $s $s resources/icon.png --out icon.iconset/icon_${s}x${s}.png
  [ $((s*2)) -le 256 ] && sips -z $((s*2)) $((s*2)) resources/icon.png --out icon.iconset/icon_${s}x${s}@2x.png
done
iconutil -c icns icon.iconset -o resources/icon.icns && rm -r icon.iconset
```

The game needs no entitlements: it links only system frameworks and static
libraries, and uses no JIT, DYLD overrides or library-validation exceptions. The
hardened runtime alone satisfies notarisation. It writes only to
`~/Library/Application Support/Augmentinel/`, never inside the (sealed) bundle.

## Verifying

```bash
codesign --verify --deep --strict -v build-macos-release/stage/Augmentinel.app
codesign -dv --verbose=4 build-macos-release/stage/Augmentinel.app 2>&1 | grep -E "Authority|flags|TeamIdentifier"
spctl -a -vvv -t install dist/Augmentinel-<version>-macOS.dmg   # -> source=Notarized Developer ID
xcrun stapler validate dist/Augmentinel-<version>-macOS.dmg
```
