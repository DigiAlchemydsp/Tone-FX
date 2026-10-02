# Releasing Tone+FX

Maintainer notes. This repo is **release-only**: it ships built `.elemod`
packages and docs, never source and never firmware. The source lives in the
development repo (`digitone_corea` / RingTone).

## What a release contains

- `elemods/` — all seven packages, always the full set:
  `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.0`, `digiring-1.0`,
  `digifold-1.0`, `digifilter-1.0`, `digictl-1.3`.
- `SHA256SUMS` — hashes of every `.elemod`, kept byte-exact by
  `.gitattributes` (`*.elemod -text`).
- `README.md`, `CHANGELOG.md`, `LICENSE`, `tools/build.ps1`, `tools/build.sh`.
- A GitHub **Release** tagged `v<os-version>` (e.g. `v2.1h`) with a zip of the
  above plus the individual `.elemod`s attached.

The OS version tag is what the unit shows (4 characters, e.g. `2.1h`); the git
tag is `v` + that.

## Cutting a release

### 1. Build in the dev repo
Environment (this machine):

```powershell
$env:PATH = "C:\SysGCC\m68k-elf\bin;" + $env:PATH
$env:ELEKLOADER_CROSS = "m68k-elf-"
$env:PYTHONPATH = "C:\Users\benan\Music\ELEKTRON\elekloader"
$stock = "C:\Users\benan\Music\ELEKTRON\Digitone_and_Digitone_Keys_OS1.43.syx"
$m = "C:\Users\benan\Music\ELEKTRON\digitone_corea\mods"
$core = "C:\Users\benan\Music\ELEKTRON\elekloader\mods\core-dn1\out\core-2.0a.elemod"
```

Bump `version` in the `mod.json` of every changed mod, then build each:

```powershell
foreach ($x in 'digimeter','digieq','digiring','digifold','digifilter','digictl') {
  python -m elekloader.sdk.build "$m\$x" --stock $stock
}
```

### 2. Test
```powershell
python "C:\Users\benan\Music\ELEKTRON\digitone_corea\tests\filter_model.py"
# link + patch a syx, then in digiemu (patched-Unicorn venv):
python -m elekloader.patch --stock $stock --mod $core `
  --mod "$m\digimeter\out\digimeter-1.1.elemod" --mod "$m\digieq\out\digieq-1.0.elemod" `
  --mod "$m\digiring\out\digiring-1.0.elemod" --mod "$m\digifold\out\digifold-1.0.elemod" `
  --mod "$m\digifilter\out\digifilter-1.0.elemod" --mod "$m\digictl\out\digictl-1.3.elemod" `
  --out "$env:TEMP\opencode\ToneFX.syx" --version <osver>
python -m emu.portable --add "$env:TEMP\opencode\ToneFX.syx" --yes   # boots + settles, dsp_running=2
python "C:\Users\benan\Music\ELEKTRON\digitone_corea\tests\digiemu_digifilter.py" --fw <dn1-...>
```
Record in the CHANGELOG what actually ran (emulator, and whether a hardware pass
happened).

### 3. Update this repo
- Copy the rebuilt `.elemod`s into `elemods/` (same filenames; if a mod's
  version bumped, the filename changes — update the lists in this doc, README
  and `tools/build.*`).
- Regenerate `SHA256SUMS`:
  ```powershell
  $root = "C:\Users\benan\Music\ELEKTRON\Tone+FX"
  (Get-ChildItem "$root\elemods\*.elemod" | Sort-Object Name |
     ForEach-Object { "{0}  elemods/{1}" -f (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower(), $_.Name }) |
   Set-Content -LiteralPath "$root\SHA256SUMS" -Encoding ascii
  ```
- Add a `## <osver>` section to `CHANGELOG.md`; update `README.md` if the
  controls or load notes changed.

### 4. Commit and release
```powershell
$root = "C:\Users\benan\Music\ELEKTRON\Tone+FX"          # always pass --repo, or run from here
git -C $root add -A; git -C $root commit -m "Tone+FX <osver>: <summary>"
git -C $root push origin master

# package + release
$tmp = "C:\Users\benan\AppData\Local\Temp\opencode\ToneFX-pkg"
# build Tone+FX-<osver>.zip from README/CHANGELOG/LICENSE/SHA256SUMS/elemods/tools
gh release create v<osver> --repo DigiAlchemydsp/Tone-FX --target master `
  --title "Tone+FX <osver>" --notes-file "$tmp\notes.md" `
  "$tmp\Tone+FX-<osver>.zip" (Get-ChildItem "$root\elemods\*.elemod" | ForEach-Object FullName)
```

**Gotcha:** `gh release create` infers the repo from the current directory.
Always pass `--repo DigiAlchemydsp/Tone-FX` (a wrong-cwd run once published a
release onto the old `DigiFilter` repo).

## Versioning

- Mod `version` fields and the OS tag move independently: e.g. suite `2.1h` has
  `digifilter-1.0` + `digictl-1.3`.
- Keep the OS tag unique per build (the unit shows it); it is how a tester tells
  a stale flash from a new one.

## Safety

- **Never** commit `*.syx`/`*.bin` (`.gitignore` blocks them). Releases carry
  no firmware; the user builds the OS from their own stock file.
- Keep `*.elemod -text` in `.gitattributes`: without it git rewrites the JSON
  line endings and breaks `SHA256SUMS`.
- Recovery for users: hold **FUNC** at power-on → **TRIG 4 (OS UPGRADE)** →
  stock `.syx`.
