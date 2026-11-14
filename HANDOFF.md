# Tone+FX — handoff — 4.3b lite (Digitone 1.44 + Digitakt 1.54)

Workspace: `C:\Users\benan\Music\ELEKTRON`. **Current release: 4.3b**, two
latest-OS `.elemod`s plus the adapted Digitakt core. This file is the
release-side handoff.

## 0. The repos

| | source (Digitone) | release | Digitakt port |
|---|---|---|---|
| path | `digitone_corea/` | `Tone+FX/` (this repo) | `Tone+FX/DIGITAKT PORT/digi1_mods-main/` |
| remote | `DigiAlchemydsp/RingTone` (`main`) | `DigiAlchemydsp/Tone-FX` (`master`) | `gdeo607/digi1_mods` (`main`) |

The release repo is release-only: it ships the built elemods and docs. The
vendored Digitakt port under `DIGITAKT PORT/` is the only source it carries.

## 1. What ships (v4.3b)

| file | device | OS | required core |
|---|---|---|---|
| `elemods/tonefx-4.3b-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.1-os1.44.elemod` |
| `elemods/tonefx-4.3b-os1.54.elemod` | Digitakt mk1 | 1.54 | `elemods/core-4.0.1-os1.54.elemod` |
| `elemods/core-4.0.1-os1.54.elemod` | Digitakt mk1 | 1.54 | — |

`SHA256SUMS` covers all three. The GitHub release **v4.3b** carries the zip
`Tone+FX-4.3b.zip`, the two suites, the adapted core and the sums. Older
releases remain under `releases/`.

The Digitakt core is **4.0.1**, a 4.0 core with the `fw_tempo` export
backported from the 5.0 core line. Do not substitute core 4.0 (no tempo), or
core 4.3/5.0 (they claim Tone+FX's master output/page sites).

## 2. The suite at a glance (4.3b)

- **Main scroll** (`FUNC + LFO`, then `LEFT/RIGHT`): stock master pages, then
  **FOLD → CRUSH → RING → STEREO → PITCH → TILT → DELAYS**.
- **PERFORM tree** (`UP`, then `LEFT/RIGHT`): **PERFORM → STUTTER**. `UP`
  returns to the FX page you left. STUTTER is not in the main scroll.
- **TOEJAM / STUTTER**: hold a trig to freeze and loop the newest slice of the
  finished master mix; release to return live. Four zones of four pads:
  straight, dotted, triplet and long. Lengths follow `fw_tempo`. Digitone ring
  is stereo (~2.7 s); Digitakt ring is mono (~0.68 s).
- **CRUSH**: `REDUX` and `CRUNCH` only.
- **DELAYS**: one WRMZ delay line only.
- **PERFORM**: the eight-knob momentary trig-pad page, unchanged.
- **Chain** (permanent): ring → EQ → crush → fold → **PITCH → WRMZ → STEREO** →
  ceiling → meter.

### Removed in 4.3b

- macro pages `WASH`, `RADIO`, `DESTROY`, `SPACE`;
- `BITS` crush mode;
- `GRAINZ` delay mode.

Their parameters, CCs, UI and store fields are gone.

## 3. Pattern store

- Digitone: `DGX4` **v2**, **35 params**, MIDI track 4 (`pattern + 0x1BB0`,
  reserved CH 16).
- Digitakt: `DGX4` **v2**, **32 params**, MIDI track 8's saved kit record
  (`kit + 0x8ba`, reserved CH 16).
- Blocks written by the old v1 layout are ignored and reclaimed; settings return
  to defaults once after upgrading.
- The PERFORM pads never dirty the store. STUTTER needs no store.

## 4. Footprints / verification

| build | `.run` | `.bss` | core | lint | patch |
|---|---:|---:|---|---|---|
| Digitone 1.44 | 22,514 | 571,342 | `core 4.1` | links | byte-exact |
| Digitakt 1.54 | 20,352 | 104,300 | `core 4.0.1` | links | byte-exact |

- Digitone imports `fw_fillrect, fw_font5, fw_framerect, fw_tempo, fw_textf`.
- Digitakt imports the same set.
- Emulator/hardware testing for this revision is **pending**. Build/lint/patch
  are verified.
- Fixed during the release: the Digitone STUTTER page initially fell through to
  the stock AMP page because the `toejam_draw()` branch was missing after the
  macro removal. It is restored in the shipped 4.3b build.

## 5. Build / verify

```powershell
$env:PATH="C:\SysGCC\m68k-elf\bin;"+$env:PATH
$env:ELEKLOADER_CROSS="m68k-elf-"
$env:PYTHONPATH="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW\elekloader-main"
$dn144="C:\Users\benan\Music\ELEKTRON\Digitone_and_Digitone_Keys_OS1.44.syx"
$dt154="C:\Users\benan\Music\ELEKTRON\Digitakt_OS1.54_dist.zip"
$E="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW"
```

Digitone (`workdir "digitone_corea"`):

```powershell
python -m elekloader.sdk.build mods\tonefx --stock $dn144
python -m elekloader.lint mods\tonefx\out\tonefx-4.3b-os1.44.elemod `
  --stock $dn144 --with "$E\core-dn1-4.1-os1.44.elemod"
```

Digitakt (`workdir "Tone+FX\DIGITAKT PORT\digi1_mods-main"`):

```powershell
python -m elekloader.sdk.build mods\tonefx --stock $dt154
python -m elekloader.lint mods\tonefx\out\tonefx-4.3b-os1.54.elemod `
  --stock $dt154 --with "$E\elekloader-main\out\core-4.0.1-os1.54.elemod"
```

The adapted Digitakt core is built from `elekloader-main/mods/core` after adding
`FW fw_tempo, FW_TEMPO` to `fw.s` and `FW_TEMPO` to both OS defsyms
(1.53 `0x4020db60`, 1.54 `0x4020df10`), version `4.0.1`.

## 6. Cutting the next release

1. Bump the suites in both source trees, build, lint, patch.
2. Drop the elemods into `elemods/`, refresh `SHA256SUMS`.
3. Update `README.md`, `CHANGELOG.md`, `HANDOFF.md`.
4. Copy the package into `releases/<ver>/` (README, notes, CHANGELOG, LICENSE,
   SHA256SUMS, elemods, tools) and rebuild `releases/Tone+FX-<ver>.zip`.
5. Commit, tag `v<ver>`, push, `gh release create v<ver>` with `notes.md` and
   the elemods + zip as assets.

## 7. Open items

- **Hardware/emulator pass for 4.3b**: both STUTTER pages, page tree
  navigation, REDUX/CRUNCH, WRMZ, and the `DGX4` v2 store reload.
- Decide whether to keep the old 4.1a-era screenshots; the release package
  currently ships none.
- Digitakt port docs/tests still contain older 4.0c/4.1a wording in a few
  places; the suite README and release notes are authoritative for 4.3b.
- The Digitakt `core-4.0.1` backport is local to this workspace. If upstream
  ever exports `fw_tempo` without claiming the master sites, switch to it.
- Possible further RAM reduction: remove WRMZ (≈33 KB) or shrink the DT mono
  ring further. Not in 4.3b.

## 8. Version trail

4.3b (lite: macros/BITS/GRAINZ removed, TOEJAM STUTTER, DT tempo core) ← 4.1a
(core-dn1 4.1) ← 4.0c (permanent chain + WASH CPU) ← 4.0b (per-pattern store) ←
4.0a (WRMZ/GRAINZ) ← 3.5r (UI pass) ← 3.5p ← 3.5a–d ← 3.3a/b ← 3.2a/b
(PERFORM) ← 3.1a–i ← 3.0a/b. Full history: [`CHANGELOG.md`](CHANGELOG.md).
