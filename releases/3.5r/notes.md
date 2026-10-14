# Tone+FX 3.5r — OS 1.44 support, two page trees, UI pass

An **eleven-page** master FX suite for the Digitone mk1 — **RING · FOLD · TILT ·
CRUSH · PITCH · STEREO · PERFORM · WASH · RADIO · DESTROY · SPACE** — plus the
output meter and the animated TONE+FX boot splash. Everything ships as one
elemod, and **one build now runs on OS 1.43 and OS 1.44**.

## Install

**One file is the whole suite:** `tonefx-3.5r.elemod` (OS 1.43) or
`tonefx-3.5r-os1.44.elemod` (OS 1.44) bundles the boot splash plus `digictl` and
the FX mods into a single **format-2** mod, so it combines with your other mods.
It does **not** include the core: install `core-dn1-4.0.elemod` (required;
elekloader provides it) plus anything else you use.

## New in 3.5r

- **OS 1.44 support.** The firmware addresses are `#ifndef *_ADDR` defaults (1.43)
  that the `ports.1.44` entry overrides through `cflags`/`defsym`; it carries the
  eleven 1.44 patch sites. Both builds patch-check clean against their stock
  image, and **1.44 is hardware-verified** (digiemu cannot settle OS 1.44, so it
  is not emulator-tested).
- **Moved to elekloader 0.5.0 + core-dn1 4.0** (the mods' RAM base is now
  `0x43EC0000`, 1.25 MB). Build `.run 28889`, `.bss 19454`, `imports nothing`.
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
- **Per-pattern store:** `"DGX1"` **v10** (28 params) + `"DGX2"` **v2** (macros).

## Tried and reverted (no shipped change)

Fusing the master passes into one `CORE_AUDIO_INSERT` callback was bit-correct
once the core's Q1.31 in/out was converted, but on hardware it **raised CPU/DSP
load ~15 %**, so it was dropped. Write-up in the source repo's
`docs/OPTIMIZATION.md` §12.3.

## Tested

- Compiles clean (`.run 28889`, `.bss 19454`, imports nothing); links as
  `core 4.0 > tonefx 3.5r`; both the 1.43 and 1.44 builds patch-check with every
  site byte-verified.
- Emulator (`dn1`): boots, settles, `dsp_running=2`; all eleven pages and both
  page trees present; the flanger/pitch lines verified independent; the
  per-pattern store round-trips. The **OS 1.44** build is not emulator-tested
  (digiemu cannot settle 1.44) — it is hardware-verified instead.
- Hardware: the 3.2 line was stress-tested on two Digitone mk1 units; **3.5r is
  a new build — please report anything you find.**

## Known issues

- **PAGE can still jump some FX levels** (the stock-event store guard does not
  cover every path).
- **PITCH can fire on a note-off** from a PHASER trig.
- **SHFT quality:** the pitch shifter is deliberately lightweight.
- **PERFORM trig LEDs** do not light yet.

The full, current list is in the README's "Known bugs / future fixes".
