# Tone+FX 3.1i — output ceiling, store guard, smoother ratios

A six-page master FX suite for the Digitone mk1 — **RING · FOLD · TILT · CRUSH ·
PITCH · STEREO** — plus the output meter and the animated TONE+FX boot splash.
Everything ships as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.1i.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.1i

- **Output ceiling**: a final soft limiter keeps the master mix below the 24-bit
  full scale (transparent below ~-2.5 dBFS, asymptotic max ~-0.7 dBFS), so a hot
  flanger-feedback / pitch-shift / drive setting cannot hard-clip. The flanger
  feedback write is clamped too.
- **Stock OS events can no longer move a DIGI parameter.** The per-pattern sync
  now adopts its block only if it still carries the `"DGX1"` magic — this fixes
  the reported **TILT/EQ jumping when PAGE (or other keys) are pressed**.
- Bounds fix (encoder accumulator), and the pitch-shift ratio now glides.

## Tested

- Compiles clean (`.run 20904`, `.bss 25270`, imports nothing); link
  byte-verified (sections 2/4/5/6/7/8; ~86 KB RAM spare, ~1.69 MB flash spare).
- Emulator (`dn1-3.1i`): boots, settles, `dsp_running=2`; all six pages present.
- Hardware: confirm the EQ-jump fix and that the ceiling is inaudible at normal
  levels.
