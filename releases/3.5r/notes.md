# Tone+FX 3.5r — OS 1.44 support, two page trees, UI pass

An **eleven-page** master FX suite — **RING · FOLD · TILT · CRUSH · PITCH ·
STEREO · PERFORM · WASH · RADIO · DESTROY · SPACE** — plus the output meter and
the animated TONE+FX boot splash. **One release now covers four OS files on two
hosts:** the **Digitone mk1 / Digitone Keys** (OS 1.43, 1.44) and, through a
community port, the **Digitakt mk1** (OS 1.53, 1.54).

## Install

Each file is the whole suite for one device + OS; none includes the core.

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-3.5r.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-3.5r-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-3.5r-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-3.5r-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

The Digitakt variants are **gdeo607's port** (source and details in
[digi1_mods](https://github.com/gdeo607/digi1_mods), mirrored here under
[`DIGITAKT PORT/`](../DIGITAKT%20PORT/)); **special thanks to gdeo607, who did
most of the work on the Digitakt port.**

## New in 3.5r

- **OS 1.44 support (Digitone).** The firmware addresses are `#ifndef *_ADDR`
  defaults (1.43) that the `ports.1.44` entry overrides through `cflags`/`defsym`;
  it carries the eleven 1.44 patch sites. **1.44 is hardware-verified** (digiemu
  cannot settle OS 1.44, so it is not emulator-tested).
- **Digitakt mk1 port (community, by gdeo607):** the same 3.5r suite for
  **OS 1.53 and 1.54** (`ports.1.54`), eleven pages incl. the macros; only the
  render path, page model, keys, CC map and store differ per device.
- **Moved to elekloader 0.5.0 + core 4.0** (the mods' RAM base is now
  `0x43EC0000`, 1.25 MB). `imports nothing` on all four builds.
- **Two page trees.** The **main scroll** (LEFT/RIGHT) is the stock master pages
  then our FX pages in play order **FOLD · CRUSH · RING · STEREO · PITCH ·
  TILT**. **UP** (or **DOWN**) opens the **macro tree**: **PERFORM · WASH ·
  RADIO · DESTROY · SPACE**. PERFORM is no longer in the main scroll.
- **FUNC + an encoder turn snaps that parameter to 0 % / 100 %.**
- **UI pass:** full-height sliders, a one-line caption at the bottom row that
  shows the **live value** while that encoder is moved (20 frames) with the value
  centred on its middle digit; the mode name centred over the title; sliders +
  labelled readouts in the rightmost third. The output meter moved to **x 108 /
  113** and is drawn on the **CRUSH** and **FOLD** pages.
- **CRUSH:** the sample-rate-hold mode **SR** is renamed **REDUX**.
- **PITCH** gains a **G mix** (dry/wet) — WOB/SHFT now have a blend like the
  flanger. **STEREO:** FLAN shows four sliders (SPD, DEP, MIX, FB).
- **Macro cue:** a page driven by a macro page draws a framed **MACRO** box where
  an off page draws **OFF**.
- **Per-pattern store:** Digitone `"DGX1"` **v10** (28 params) + `"DGX2"` **v2**;
  Digitakt `"DX"` **v8** (35 params, packed).

## Tried and reverted (no shipped change)

Fusing the master passes into one `CORE_AUDIO_INSERT` callback was bit-correct
once the core's Q1.31 in/out was converted, but on hardware it **raised CPU/DSP
load ~15 %**, so it was dropped. Write-up in the source repo's
`docs/OPTIMIZATION.md` §12.3.

## Tested

- All four builds compile clean and `imports nothing` — Digitone 1.43 / 1.44
  (`.run 28889`, `.bss 19454`, 11 sites) and Digitakt 1.53 / 1.54 (`.run 25795`,
  `.bss 11100`, 4 sites) — each links as `core 4.0 > tonefx 3.5r` and patches
  **byte-for-byte** against its own stock file (the 1.43 main OS is byte-identical
  to 3.5p).
- Emulator (Digitone `dn1`): boots, settles, `dsp_running=2`; all eleven pages
  and both page trees present; the flanger/pitch lines verified independent; the
  per-pattern store round-trips. The **OS 1.44** build is hardware-verified
  instead (digiemu cannot settle 1.44). The Digitakt port's emulator suite keeps
  its documented 3.5r gap (its unicorn test still names the pre-3.2b
  `digiwave_depth`).
- Hardware: the 3.2 line was stress-tested on two Digitone mk1 units; **3.5r is
  a new build — please report anything you find.**

## Known issues

- **PAGE can still jump some FX levels** (the stock-event store guard does not
  cover every path).
- **PITCH can fire on a note-off** from a PHASER trig.
- **SHFT quality:** the pitch shifter is deliberately lightweight.
- **PERFORM trig LEDs** do not light yet.

The full, current list is in the README's "Known bugs / future fixes".
