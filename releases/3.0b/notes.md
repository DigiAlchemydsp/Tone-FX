# Tone+FX 3.0b — TONE+FX boot splash

Tone+FX is now a **three-page** master FX suite — **RING**, **FOLD**, **TILT** —
plus the output meter and an animated **TONE+FX boot splash**. The splash and
the FX ship as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.0b.elemod` bundles the boot splash
plus `digictl` and the four FX mods into a single **format-2** mod, so it
combines with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## Changes

- **Boot splash (`tonesplash`)**: an animated TONE+FX wordmark drawn at the
  intro, picking one of three animations at random each boot (accelerated
  particles, a laser trace, or a laser draw + spark bloom). Inert once the UI
  takes over.
- Everything else is unchanged from 3.0a (three pages, fold modes, shelf EQ,
  meter, MIDI CC).

## Tested

- Emulator (`dn1-3.0b-*`): boots, settles, `dsp_running=2`; the intro (splash)
  runs clean. The 3.0a tests (screenshots, navigation, per-pattern store,
  MIDI-CC) still pass.
- Hardware stress-tested over 3 days: **PASS** (a Digitone mk1).
