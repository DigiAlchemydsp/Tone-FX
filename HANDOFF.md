# Tone+FX — handoff — 3.5r (four OS files: Digitone 1.43/1.44 + Digitakt 1.53/1.54)

Workspace: `C:\Users\benan\Music\ELEKTRON`. **Current release: 3.5r**, shipped as
four `.elemod`s from two source trees. This file is the release-side handoff; the
Digitone source has its own (gitignored) `HANDOFF.md` in `digitone_corea/`.

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
| `tonefx-3.5r.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-3.5r-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-3.5r-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-3.5r-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

`SHA256SUMS` covers all four. The GitHub release **v3.5r** carries the four
elemods + `Tone+FX-3.5r.zip`; tag `v3.5r` → `7149075`. Old releases are in
`releases/`; superseded ones in `archive/`.

## 2. The suite at a glance (3.5r)

- **Eleven pages, two trees.** Main scroll (LEFT/RIGHT): stock pages then
  **FOLD · CRUSH · RING · STEREO · PITCH · TILT**. Macro tree (**UP** or
  **DOWN**): **PERFORM · WASH · RADIO · DESTROY · SPACE**. PERFORM is not in the
  main scroll. **FUNC + encoder turn** snaps 0 %/100 %.
- **Chain** (`ev_render_out`): ring → EQ → crush → fold → wave → ceiling → meter.
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

## 4. Verified (this 3.5r cut)

| build | `.run` | `.bss` | sites | relocs | lint | patch |
|---|---|---|---|---|---|---|
| Digitone 1.43 | 28889 | 19454 | 11 | 997 | links | byte-exact |
| Digitone 1.44 | 28889 | 19454 | 11 | 997 | links | byte-exact |
| Digitakt 1.53 | 25795 | 11100 | 4 | 974 | links | byte-exact |
| Digitakt 1.54 | 25795 | 11100 | 4 | 974 | links | byte-exact |

All `imports nothing`, link as `core 4.0 > tonefx 3.5r`. The built elemods are
**byte-identical to the committed ones** (reproducible). The 1.43 patched main OS
is byte-identical to 3.5p. Emulator: Digitone `dn1` boots/settles; the port's
unicorn test still names the pre-3.2b `digiwave_depth` (its documented 3.5r gap).

## 5. Build / verify commands

```powershell
$env:PATH="C:\SysGCC\m68k-elf\bin;"+$env:PATH; $env:ELEKLOADER_CROSS="m68k-elf-"
$env:PYTHONPATH="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW\elekloader-main"
$E="C:\Users\benan\Music\ELEKTRON\00 ELEKLOADER NEW"     # core elemods
```

Digitone (`workdir digitone_corea`):

```powershell
python -m elekloader.sdk.build mods\tonefx --stock C:\...\Digitone_and_Digitone_Keys_OS1.43.syx
python -m elekloader.sdk.build mods\tonefx --stock C:\...\dn1-1.44-d4f200d0\Digitone_OS1.44.syx
```

Digitakt (`workdir "Tone+FX\DIGITAKT PORT\digi1_mods-main"`):

```powershell
python -m elekloader.sdk.build mods\tonefx --stock C:\...\Digitakt_OS1.53.syx
python -m elekloader.sdk.build mods\tonefx --stock C:\...\Digitakt_OS1.54_dist.zip
```

Verify each pair with `python -m elekloader.lint <mod> --stock <stock> --with
<core>` and `python -m elekloader.patch --stock <stock> --mod <core> --mod <mod>
--out <tmp.syx> --version 3.5r`. `--version` must be **exactly 4 ASCII chars**.

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
  note-off**; **PERFORM trig LEDs** not lit; **SHFT** quality; headroom not
  formally measured. See the release README's "Known bugs / future fixes".
- Digitakt port: the 3.5r **UI pass** (full-height sliders, live-value caption,
  meter position) is **not** applied to the DT1; its emulator test was not run to
  completion; no DT1 unit test for 3.5r.
- `digihealth-1.1-os1.44.elemod` sits untracked in the repo root (not part of
  Tone+FX).

## 8. Version trail

3.5r (OS 1.44 + UI pass + cross-platform release) · 3.5q · 3.5p (perf pass +
flanger/microtiming fixes) · 3.5a–d (macro pages, split WAVE on/off) · 3.3a/b ·
3.2a/b (PERFORM page, split WAVE depths) · 3.1a–i · 3.0a/b. Full history:
[`CHANGELOG.md`](CHANGELOG.md).
