# Releasing Tone+FX

Maintainer notes. This repo is **release-only**: it ships built `.elemod`
packages and docs, never source and never firmware. The source lives in the
development repo (`digitone_corea` / RingTone).

## What a release contains

- `elemods/` — all seven packages, always the full set:
  `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`,
  `digifold-1.1`, `digifilter-1.0`, `digictl-1.6`. This is the **current**
  release (what `SHA256SUMS` covers).
- `SHA256SUMS` — hashes of every `.elemod`, kept byte-exact by
  `.gitattributes` (`*.elemod -text`).
- `README.md`, `CHANGELOG.md`, `LICENSE`, `tools/build.ps1`, `tools/build.sh`.
- `releases/<os-version>/` — a **frozen package** of a cut release: its own
  `README`, `CHANGELOG`, `LICENSE`, `SHA256SUMS`, `elemods/`, `tools/`,
  `Screenshots/` and `Tone+FX-<os-version>.zip`.
- `archive/<os-version>/` — the same package for a **superseded** release.
- A GitHub **Release** tagged `v<os-version>` (e.g. `v2.2a`) with a zip of the
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
  --mod "$m\digimeter\out\digimeter-1.1.elemod" --mod "$m\digieq\out\digieq-1.1.elemod" `
  --mod "$m\digiring\out\digiring-1.1.elemod" --mod "$m\digifold\out\digifold-1.1.elemod" `
  --mod "$m\digifilter\out\digifilter-1.0.elemod" --mod "$m\digictl\out\digictl-1.6.elemod" `
  --out "$env:TEMP\opencode\ToneFX.syx" --version <osver>
python -m emu.portable --add "$env:TEMP\opencode\ToneFX.syx" --yes   # boots + settles, dsp_running=2
python "C:\Users\benan\Music\ELEKTRON\digitone_corea\tests\digiemu_pattern_store.py" --fw <dn1-...>
```

(For the DIGI pages, `tests/digiemu_fx_screenshots.py` captures them.)
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
- **Freeze the package** into `releases/<osver>/` (copy the updated `README.md`,
  `CHANGELOG.md`, `LICENSE`, `SHA256SUMS`, `elemods/`, `tools/`, `Screenshots/`)
  and build `releases/<osver>/Tone+FX-<osver>.zip` from those items; put the
  release notes next to it (`notes.md`).
- **Archive the previous release:** if `releases/<old>/` does not exist yet,
  build it from the previous root state (see `archive/2.1h/` for an example) and
  move it to `archive/<old>/`.

### 4. Commit and release
```powershell
$root = "C:\Users\benan\Music\ELEKTRON\Tone+FX"          # always pass --repo, or run from here
git -C $root add -A; git -C $root commit -m "Tone+FX <osver>: <summary>"
git -C $root push origin master

# package already frozen under releases/<osver>/
$zip = "$root\releases\<osver>\Tone+FX-<osver>.zip"
gh release create v<osver> --repo DigiAlchemydsp/Tone-FX --target master `
  --title "Tone+FX <osver>" --notes-file "$root\releases\<osver>\notes.md" `
  $zip (Get-ChildItem "$root\elemods\*.elemod" | ForEach-Object FullName)
```

**Gotcha:** `gh release create` infers the repo from the current directory.
Always pass `--repo DigiAlchemydsp/Tone-FX` (a wrong-cwd run once published a
release onto the old `DigiFilter` repo).

## Versioning

- Mod `version` fields and the OS tag move independently: e.g. suite `2.2a` has
  `digifilter-1.0` + `digictl-1.6`.
- Keep the OS tag unique per build (the unit shows it); it is how a tester tells
  a stale flash from a new one.

## Safety

- **Never** commit `*.syx`/`*.bin` (`.gitignore` blocks them). Releases carry
  no firmware; the user builds the OS from their own stock file.
- Keep `*.elemod -text` in `.gitattributes`: without it git rewrites the JSON
  line endings and breaks `SHA256SUMS`.
- Recovery for users: hold **FUNC** at power-on → **TRIG 4 (OS UPGRADE)** →
  stock `.syx`.
