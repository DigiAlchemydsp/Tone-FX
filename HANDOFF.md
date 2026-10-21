# Tone+FX — handoff — 4.0c (four OS files: Digitone 1.43/1.44 + Digitakt 1.53/1.54)

Workspace: `C:\Users\benan\Music\ELEKTRON`. **Current release: 4.0c**, four
`.elemod`s: two **Digitone** built from `ToneFX-BufferFX-exp` (the suite + the
**DELAYS** module + a CPU/headroom pass) and two **Digitakt** from the vendored
port under `DIGITAKT PORT/`. This file is the release-side handoff.

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
| `tonefx-4.0c.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-4.0c-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-4.0c-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-4.0c-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

`SHA256SUMS` covers all four. The GitHub release **v4.0c** carries the four
elemods + `Tone+FX-4.0c.zip`. Old releases are in `releases/`; superseded ones in
`archive/`.

## 2. The suite at a glance (4.0c)

- **Twelve pages, two trees.** Main scroll (LEFT/RIGHT): stock pages then
  **FOLD · CRUSH · RING · STEREO · PITCH · TILT · DELAYS**. Macro tree (**UP** or
  **DOWN**): **PERFORM · WASH · RADIO · DESTROY · SPACE**. PERFORM is not in the
  main scroll. **FUNC + encoder turn** snaps 0 %/100 %.
- **DELAYS** (kind 11) is the buffer-FX module: one mono delay line, two
  modes — **WRMZ** (modulated delay) and **GRAINZ** (three granular taps); **D**
  switches mode; per-pattern store (packed, `"DGX4"`); **MIDI CC 80–87**.
- **Macros:** WASH = WOB only (75% depth / 30% rate) + WRMZ mix/fb; RADIO = EQ
  tilt (0/127) + TRSH fold + SRR. Capture/restore is per engine (CRUSH is not
  tied to WASH).
- **WASH CPU:** WRMZ `MOD=0` (static read) + the DC blocker skipped while WASH
  is on (saturator still bounds FB; state reset on enter/leave), and the L/R wet
  mix uses `bx_mul15x2` in WOB and WRMZ/GRAINZ.
- **Chain** (permanent): ring → EQ → crush → fold → **PITCH → WRMZ/GRAINZ →
  STEREO** → ceiling → meter. The WAVE insert is split into its PITCH (order 61)
  and STEREO (63) passes with the delay at 62, so the order is fixed.
- **UI pass:** full-height sliders; one-line bottom caption (label idle, live
  value for 20 frames on move, centred on the middle digit); meter at x108/113 on
  **CRUSH** and **FOLD**; a macro-driven page draws **MACRO**, a bypassed one
  **OFF**.
- **PITCH** has a dry/wet **MIX** (WOB/SHFT); **CRUSH** mode `SR` is **REDUX**;
  **STEREO FLAN** shows four sliders.
- **Stores:** Digitone — all 51 params packed into **MIDI track 4
  (`pattern+0x1BB0`, `"DGX4"`, CH 16)**; Digitakt — a mod-owned RAM table
  (`dn_tab`, 48 params).

## 3. Cross-platform release checklist

The full checklist lives in the port:
`DIGITAKT PORT/digi1_mods-main/mods/tonefx/README.md#releasing-tonefx--cross-platform-multi-os`.
In short: build all four, `lint` + `patch` each, run the emulator tests, then
name/package. See §5 for the exact commands.

**Shared effect source:** `crush.c eq.c fold.c meter.c ring.c wave.c` are the
same code in both trees; the port differs only by `#include "corea.h"` and the
file header comment. Keep them in step (3-way: ours = the port, base = the
Digitone 3.2a it forked from, theirs = the new Digitone).

## 4. Verified (this 4.0c cut)

| build | `.run` | `.bss` | sites | relocs | lint | patch |
|---|---|---|---|---|---|---|
| Digitone 1.43 | 25594 | 47182 | 11 | 970 | links | byte-exact |
| Digitone 1.44 | 25594 | 47182 | 11 | 970 | links | byte-exact |
| Digitakt 1.53 | 23096 | 51232 | 4 | 931 | links | byte-exact |
| Digitakt 1.54 | 23096 | 51232 | 4 | 931 | links | byte-exact |

All four `imports nothing` and link as `core 4.0 > tonefx 4.0c`, built with
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

Digitone 4.0c (`workdir "ToneFX-BufferFX-exp"`):

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
--out <tmp.syx> --version 4.0c`. `--version` must be **exactly 4 ASCII chars**.

## 6. Cutting a release here

1. Drop the new elemod(s) into `elemods/`, refresh `SHA256SUMS`.
2. Update `README.md` (downloads table, install) and `CHANGELOG.md`.
3. Copy the package into `releases/<ver>/` (README, CHANGELOG, LICENSE,
   SHA256SUMS, notes.md, Screenshots/, tools/, elemods/) and rebuild
   `releases/Tone+FX-<ver>.zip` from its contents.
4. `git tag -a v<ver>`, push, `gh release create v<ver>` with `notes.md` as the
   body and the elemods + zip as assets.

## 7. Open items

- Digitone: **PITCH can fire on a note-off**; **PERFORM trig LEDs** not lit;
  **SHFT** quality; **DELAYS GRAINZ PIT parked** (taps read at rate 1.0). See the
  release README's "Known bugs / future fixes".
- **Per-pattern store reworked (digictl 2.9).** Digitone: the FX params no
  longer live in the MIDI-track blocks for channels 1-3
  (`+0x1040/+0x1410/+0x17E0`, which a configured MIDI track writes); all of them
  are packed into **one reserved MIDI track, t=7 at `pattern+0x1BB0`**
  (`"DGX4"`), on **MIDI channel 16**. Only that track is touched; channels
  1-15 and the synth tracks are untouched, and the params save/reload with the
  pattern. Digitakt: stays a mod-owned RAM table (`dn_tab`, `.bss`) — no
  equivalent block. Rebuilt + linted for all four OS files; verified in digiemu
  (`dn1-4.0a`, `dt1-4.0a`). Packaged in 4.0c. See `docs/PATTERN-STORAGE.md`.
- **Digitakt mk1** is at 4.0c too (DELAYS + CPU pass). Its `DC_PER_STEP`
  (=1), master-page-cycle fix and the per-pattern RAM table are hardware-validated
  but not covered by the port's emulator test (which still assumes ±4 encoder
  deltas / 11 pages). A PR syncing these upstream is open:
  [gdeo607/digi1_mods#6](https://github.com/gdeo607/digi1_mods/pull/6).
- Digitakt port: the 3.5r **UI pass** (full-height sliders, live-value caption,
  meter position) is **not** applied to the DT1.
- **Next round (in-depth review + optimization):** the WASH delay line still
  dominates — half-rate WRMZ wet (~−3.5 k cyc/block, darker), mono WOB
  (~−1.5 k), or a delay-time cap; then 4-accumulator MAC interleaving, eMAC on
  `fold`/`crush` Q8 and WAVE WIDE, and a fresh `cftiming` pass on 4.0c. Review
  `dn_store_sync`, the macro capture/restore and the split WAVE passes too.
- `Screenshots/WASH.png` still shows the old shore band (the page is now the
  dot cloud).
- `digihealth-1.1-os1.44.elemod` sits untracked in the repo root (not part of
  Tone+FX).

## 8. Version trail

4.0c (permanent PITCH → DELAYS → STEREO chain + WASH CPU; four OS files) · 4.0b (per-pattern store on one reserved MIDI track CH 16 + macro rework) · 4.0a (DELAYS/WRMZ+GRAINZ + CPU pass; four OS files, the DT1 port included) · 3.5r (OS 1.44 + UI pass + cross-platform release) · 3.5q · 3.5p (perf pass +
flanger/microtiming fixes) · 3.5a–d (macro pages, split WAVE on/off) · 3.3a/b ·
3.2a/b (PERFORM page, split WAVE depths) · 3.1a–i · 3.0a/b. Full history:
[`CHANGELOG.md`](CHANGELOG.md).
