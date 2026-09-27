# Windows code signing

Signs `Augmentinel.exe` so Windows SmartScreen names a verified publisher instead
of "unknown publisher". Signing runs **in CI** (GitHub Actions) through **SSL.com
eSigner** cloud signing, exactly as in blast_radius and Lineage, and with **the
same certificate and eSigner subscription**: nothing new needs to be bought or
validated. The only one-time step is adding the four secrets to this repository.

The CI plumbing is in `.github/workflows/ci.yml`, gated on `ES_USERNAME`: until
the secrets exist, every signing step is skipped and the build ships unsigned.

## Secrets

| Value | Where | GitHub secret |
| --- | --- | --- |
| SSL.com account username (login email) | your account | `ES_USERNAME` |
| SSL.com account password | your account | `ES_PASSWORD` |
| eSigner credential ID | eSigner dashboard | `ES_CREDENTIAL_ID` |
| eSigner TOTP/automation secret | eSigner dashboard | `ES_TOTP_SECRET` |

Add them under kranzky/augmentinel > Settings > Secrets and variables > Actions
> Secrets > New repository secret, with the same values as in blast_radius and
Lineage. GitHub never shows a secret again after it is saved, so take the values
from your password manager or the SSL.com dashboard. Or, from a shell:

```bash
gh secret set ES_USERNAME -R kranzky/augmentinel       # prompts for the value
gh secret set ES_PASSWORD -R kranzky/augmentinel
gh secret set ES_CREDENTIAL_ID -R kranzky/augmentinel
gh secret set ES_TOTP_SECRET -R kranzky/augmentinel
gh secret set BUTLER_API_KEY -R kranzky/augmentinel    # for the itch.io publish job
```

If the certificate ever has to be set up from scratch, follow Lineage's SIGNING.md
"One-time setup" (SSL.com IV/OV certificate plus eSigner Tier 1, validation,
eSigner enrolment and the TOTP automation secret).

## When CI signs

Signing uses eSigner quota (Tier 1 allows 240 signings a year, shared with the
other projects), so it does **not** run on pull requests. It runs:

- on every release tag push (`v*`), and
- on Actions > CI > Run workflow with **sign** ticked (and the platform set to
  `windows` or `all`).

If signing was requested but `ES_USERNAME` is empty, the run posts a warning
annotation and ships unsigned rather than failing.

## How CI uses it

In the Windows leg of `.github/workflows/ci.yml`:

1. Build `Augmentinel.exe` (Release, x64, everything static including the CRT)
   with its embedded icon and version resource (`packaging/windows/Augmentinel.rc.in`).
   CI checks that the version resource is present.
2. Smoke-test it on Mesa's software OpenGL (the runners have no GPU driver), from
   a scratch copy so the Mesa DLLs never ship.
3. SSL.com's CodeSignTool v1.3.2, run on the runner's JDK 17, signs the exe into
   `build\signed` (`malware_block=false` skips eSigner's slow scan gate).
   CodeSignTool refuses to write into the input's own directory, so the signed
   copy is then copied back over `build\Release\Augmentinel.exe`.
4. `Get-AuthenticodeSignature` must report `Valid`, or the build fails.
5. The signed exe, game data, `README.txt` and `COPYING` go into the
   `Augmentinel-windows-x64` artifact. On tags, the itch.io `windows` channel is
   pushed from that artifact.

CI runs CodeSignTool's jar directly on JDK 17 rather than through
`CodeSignTool.bat` or the `sslcom/esigner-codesign` action: their bundled Java
11.0.2 doesn't trust the "SSL.com TLS RSA Root CA 2022" chain that `cs.ssl.com`
moved to on 2026-09-22 (`PKIX path building failed`).

## Verifying a build

Download the `Augmentinel-windows-x64` artifact and either right-click
`Augmentinel.exe` > Properties > **Digital Signatures**, or:

```powershell
Get-AuthenticodeSignature .\Augmentinel.exe
# Status should be 'Valid'; SignerCertificate.Subject = your validated name
(Get-Item .\Augmentinel.exe).VersionInfo | Format-List ProductName,ProductVersion,FileDescription
```

## SmartScreen note

A standard IV/OV certificate (like eSigner here) does **not** give instant
SmartScreen trust the way an EV certificate does. The "unknown publisher" wording
goes away immediately, because the publisher is named and verified. The
reputation prompt fades as downloads accumulate.
