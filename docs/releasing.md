# Releasing Augmentinel

Augmentinel ships on itch.io (<https://kranzky.itch.io/augmentinel>) as two
butler channels, `windows` and `mac`. Windows is signed and pushed by CI. macOS is
signed, notarised and pushed from the Mac.

## One-time setup

- [ ] **Windows signing secrets** `ES_USERNAME`, `ES_PASSWORD`, `ES_CREDENTIAL_ID`
      and `ES_TOTP_SECRET`, the same values as blast_radius
      ([packaging/windows/SIGNING.md](../packaging/windows/SIGNING.md)). CI refuses
      to publish an unsigned exe.
- [ ] **`BUTLER_API_KEY` secret.** Generate a key at
      <https://itch.io/user/settings/api-keys> and add it to this repo under
      Settings > Secrets and variables > Actions. While it is unset, the CI
      publish job only warns.
- [ ] **macOS**: the Developer ID certificate and the `lineage-notary` notarytool
      profile in the Mac's keychain ([packaging/macos/README.md](../packaging/macos/README.md)).
- [ ] **butler** on the Mac, logged in once with `butler login`. The itch app's
      copy lives in `~/Library/Application Support/itch/apps/butler/`.

## Release checklist

1. **Bump the version** in `CMakeLists.txt` (`project(Augmentinel VERSION x.y.z)`),
   in a PR, and merge it. This one number becomes the window title, the bundle
   version, the exe version resource and the itch.io user version.
2. **Tag** the merge commit on `main` and push the tag:

   ```bash
   git switch main && git pull
   git tag vX.Y.Z && git push origin vX.Y.Z
   ```

3. **CI (tag run)**: Windows and macOS build and smoke-test. Windows then signs
   `Augmentinel.exe` with eSigner and verifies it. The `publish` job checks that
   the tag matches the CMake version, refuses an unsigned exe, and pushes
   `Augmentinel-windows-x64` to `kranzky/augmentinel:windows` with
   `--userversion X.Y.Z`. Watch it with `gh run watch`.
4. **macOS: sign and notarise locally**, from the tagged commit:

   ```bash
   git switch --detach vX.Y.Z
   ./packaging/macos/build_dmg.sh
   ```

   This produces `dist/Augmentinel-X.Y.Z-macOS.dmg` and
   `dist/Augmentinel-X.Y.Z-macos.zip`, both notarised and stapled. Check that the
   script ends with `source=Notarized Developer ID`.
5. **Push the mac channel with butler**:

   ```bash
   DRY_RUN=1 ./packaging/itch/butler_push.sh mac dist/Augmentinel-X.Y.Z-macos.zip
   ./packaging/itch/butler_push.sh mac dist/Augmentinel-X.Y.Z-macos.zip
   ```

   Before it uploads, the script refuses unsigned files and checks the app's
   version, signature, stapled ticket and notarisation.
6. **Verify on itch.io.** `butler status kranzky/augmentinel` should show X.Y.Z on
   both channels. Install through the itch app on each OS: Windows should show a
   verified publisher, and macOS should open without a Gatekeeper warning. Attach
   the DMG to a GitHub release too if you like (`gh release create vX.Y.Z dist/*.dmg`).

## If something goes wrong

- **Tag does not match the version**: the publish job fails before pushing.
  Delete the tag, fix the version in a PR, and tag again.
- **Windows unsigned**: the ES_* secrets are missing or eSigner failed. Nothing
  is published. Fix it and re-run the tag's workflow.
- **Notarisation rejected**: `xcrun notarytool log <submission-id> --keychain-profile lineage-notary`
  shows why.
- **Bad build pushed**: push a fixed build to the same channel. itch.io keeps
  channel history, and players get the newest build.
