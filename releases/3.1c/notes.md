# Tone+FX 3.1c — Crunch/Redux + Wave modulation

Tone+FX is now a **five-page** master FX suite — **RING**, **FOLD**, **TILT**,
**CRUSH**, **WAVE** — plus the output meter and the animated TONE+FX boot
splash. Everything ships as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.1c.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.1c

- **digicrush — Crunch / Redux**: BITS (bit-reduce + TPDF dither), SR
  (sample-rate hold) and CRUNCH (hard-clip drive), one stateless stage at a
  time. New **DIGI CRUSH** page.
- **digiwave — one modulation insert, five modes**: WOB (pitch wobble), SHFT
  (pitch shift), FLAN (flanger), PAN (autopan), WIDE (stereo width). One page,
  shared SPEED/DEPTH, one delay line + one LFO. New **DIGI WAVE** page.
- **Click fix**: WOB/FLAN swept delays are fractional, interpolated and
  clamped (they previously stepped by whole samples and could wrap negative).
- **SHFT CPU**: shared read position, derived crossfade window, guard sample,
  block-rate LFO.
- `digictl`: per-pattern store bumped to **"DGX1" v6 (22 params)**, MIDI CC
  **20–29**. Signal order **ring → EQ → crush → fold → wave → meter**.

## Tested

- Compiles clean (`.run 18880`, `.bss 25222`, imports nothing), links
  byte-verified (sections 2/4/5/6/7/8; 86,396 B RAM spare, 1,693,728 B flash
  spare).
- Emulator (`dn1-3.1c`): boots, settles, `dsp_running=2`.
- Hardware: first pass working (a Digitone mk1); stress testing continues.
- CPU/DSP/dB headroom not yet formally measured.
