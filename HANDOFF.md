# Tone+FX — handoff — 4.0a (Digitone 1.43/1.44 + DELAYS; Digitakt stays 3.5r)

Workspace: `C:\Users\benan\Music\ELEKTRON`. **Current release: 4.0a**, shipped as
two Digitone `.elemod`s built from the **`ToneFX-BufferFX-exp`** source tree (the
suite + the new **DELAYS** module + a CPU/headroom pass). The community Digitakt
port stays at its last cross-platform release **3.5r**. This file is the
release-side handoff.

## 0. The repos

| | source (Digitone) | release | Digitakt port |
|---|---|---|---|
| path | `digitone_corea/` | `Tone+FX/` (this repo) | `Tone+FX/DIGITAKT PORT/digi1_mods-main/` |
| remote | `DigiAlchemydsp/RingTone` (`main`) | `DigiAlchemydsp/Tone-FX` (`master`) | `gdeo607/digi1_mods` (`main`) |
| authors | Digital Alchemy DSP | Digital Alchemy DSP | **gdeo607** (mostly), with Digital Alchemy DSP |

The **release repo is release-only**: it ships the built `.elemod`s plus the
docs. The only source it carries is the **vendored Digitakt port** under
`DIGITAKT PORT/` (so the cross-platform release is self-contained here).

## 1. What ships

One elemod is the whole suite for one device + OS; none includes a core.

| file (in `elemods/`) | device | OS | required core |
|---|---|---|---|
| `tonefx-4.0a.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-4.0a-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-4.0a-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-4.0a-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

`SHA256SUMS` covers all four. The GitHub release **v4.0a** carries the four
elemods + `Tone+FX-4.0a.zip`. Old releases are in `releases/`; superseded ones in
`archive/`.

## 2. The suite at a glance (4.0a)

- **Twelve pages, two trees.** Main scroll (LEFT/RIGHT): stock pages then
  **FOLD · CRUSH · RING · STEREO · PITCH · TILT · DELAYS**. Macro tree (**UP** or
  **DOWN**): **PERFORM · WASH · RADIO · DESTROY · SPACE**. PERFORM is not in the
  main scroll. **FUNC + encoder turn** snaps 0 %/100 %.
- **DELAYS** (kind 11) is the new buffer-FX module: one mono delay line, two
  modes — **WRMZ** (modulated delay) and **GRAINZ** (three granular taps); **D**
  switches mode; store `"DGX3"` v1; **MIDI CC 80–87**.
- **Chain** (`ev_render_out`): ring → EQ → crush → fold → wave → WRMZ → ceiling
  → meter.
- **UI pass:** full-height sliders; one-line bottom caption (label idle, live
  value for 20 frames on move, centred on the middle digit); meter at x108/113 on
  **CRUSH** and **FOLD**; a macro-driven page draws **MACRO**, a bypassed one
  **OFF**.
- **PITCH** has a dry/wet **MIX** (WOB/SHFT); **CRUSH** mode `SR` is **REDUX**;
  **STEREO FLAN** shows four sliders.
- **Stores:** Digitone `"DGX1"` v10 (28) + `"DGX2"` v2 (macros); Digitakt
  `"DX"` v8 (35, packed, RAM-only).

## 3. Cross-platform release checklist

The full checklist lives in the port:
`DIGITAKT PORT/digi1_mods-main/mods/tonefx/README.md#releasing-tonefx--cross-platform-multi-os`.
In short: build all four, `lint` + `patch` each, run the emulator tests, then
name/package. See §5 for the exact commands.

**Shared effect source:** `crush.c eq.c fold.c meter.c ring.c wave.c` are the
same code in both trees; the port differs only by `#include "corea.h"` and the
file header comment. Keep them in step (3-way: ours = the port, base = the
Digitone 3.2a it forked from, theirs = the new Digitone).

## 4. Verified (this 4.0a cut)

| build | `.run` | `.bss` | sites | relocs | lint | patch |
|---|---|---|---|---|---|---|
| Digitone 1.43 | 26018 | 47246 | 11 | 967 | links | byte-exact |
| Digitone 1.44 | 26018 | 47246 | 11 | 967 | links | byte-exact |
| Digitakt 1.53 | 23184 | 38840 | 4 | 909 | links | byte-exact |
| Digitakt 1.54 | 23184 | 38840 | 4 | 909 | links | byte-exact |

All four `imports nothing` and link as `core 4.0 > tonefx 4.0a`, built with
`-DBX_EMAC_HOISTED -DBX_S16_LINE -Os`. Emulator: Digitone `dn1` boots/settles,
all twelve pages incl. DELAYS, live diag (WRMZ + GRAINZ, real renders) **0
faults**, `_probe_mix` 0 clips; the 1.44 build builds and patch-checks clean
(digiemu cannot settle 1.44). CPU ledger: `ToneFX-BufferFX-exp/docs/OPTIMIZATION-ROUND.md`
§10–§12.

## 5. Build / verify commands

```powershell
$env:PATH="C:\SysGCC\m68k-elf\bin;"+$env:PATH; $env:ELEKLOADER_CROSS="m68k-elf-"
$env:PYTHONPATH="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW\elekloader-main"
$E="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW"     # core elemods
```

Digitone 4.0a (`workdir "ToneFX-BufferFX-exp"`):

```powershell
python -m elekloader.sdk.build mods\tonefx-buffx --stock C:\...\Digitone_and_Digitone_Keys_OS1.43.syx
python -m elekloader.sdk.build mods\tonefx-buffx --stock C:\...\Digitone_and_Digitone_Keys_OS1.44.syx
```

Digitakt (`workdir "Tone+FX\DIGITAKT PORT\digi1_mods-main"`):

```powershell
python -m elekloader.sdk.build mods\tonefx --stock C:\...\Digitakt_OS1.53.syx
python -m elekloader.sdk.build mods\tonefx --stock C:\...\Digitakt_OS1.54_dist.zip
```

Verify each pair with `python -m elekloader.lint <mod> --stock <stock> --with
<core>` and `python -m elekloader.patch --stock <stock> --mod <core> --mod <mod>
--out <tmp.syx> --version 4.0a`. `--version` must be **exactly 4 ASCII chars**.

## 6. Cutting a release here

1. Drop the new elemod(s) into `elemods/`, refresh `SHA256SUMS`.
2. Update `README.md` (downloads table, install) and `CHANGELOG.md`.
3. Copy the package into `releases/<ver>/` (README, CHANGELOG, LICENSE,
   SHA256SUMS, notes.md, Screenshots/, tools/, elemods/) and rebuild
   `releases/Tone+FX-<ver>.zip` from its contents.
4. `git tag -a v<ver>`, push, `gh release create v<ver>` with `notes.md` as the
   body and the elemods + zip as assets.

## 7. Open items

- Digitone: **PAGE can still jump some FX levels**; **PITCH can fire on a
  note-off**; **PERFORM trig LEDs** not lit; **SHFT** quality; **DELAYS GRAINZ
  PIT parked** (taps read at rate 1.0). See the release README's "Known bugs /
  future fixes".
- **Digitakt stays at 3.5r** — the DELAYS module and the 4.0a CPU pass are not
  ported. Porting them needs the DT1 render-path/site work by gdeo607.
- Digitakt port: the 3.5r **UI pass** (full-height sliders, live-value caption,
  meter position) is **not** applied to the DT1; its emulator test was not run to
  completion; no DT1 unit test for 3.5r.
- `digihealth-1.1-os1.44.elemod` sits untracked in the repo root (not part of
  Tone+FX).

## 8. Version trail

4.0a (DELAYS/WRMZ+GRAINZ + CPU pass, Digitone-only) · 3.5r (OS 1.44 + UI pass + cross-platform release) · 3.5q · 3.5p (perf pass +
flanger/microtiming fixes) · 3.5a–d (macro pages, split WAVE on/off) · 3.3a/b ·
3.2a/b (PERFORM page, split WAVE depths) · 3.1a–i · 3.0a/b. Full history:
[`CHANGELOG.md`](CHANGELOG.md).
