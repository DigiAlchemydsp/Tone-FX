# Tone+FX 3.1h — WAVE two pages, true bypass, page art

Tone+FX is now a **six-page** master FX suite — **RING · FOLD · TILT · CRUSH ·
PITCH · STEREO** — plus the output meter and the animated TONE+FX boot splash.
Everything ships as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.1h.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.1h

- **WAVE split into two pages**: **PITCH** (WOB / SHFT) and **STEREO** (FLAN /
  PAN / WIDE), each with its own mode over one shared delay engine. Flanger
  feedback capped at 75%.
- **Click-free true bypass per section** — each page's **A** off fades out and
  then stops using CPU (exact passthrough).
- **Page art** in the leftmost third, frameless, collapsing to a line at
  neutral. PITCH shows two vertical lines that wobble and intertwine.
- Per-pattern store **`"DGX1"` v7 (24 params)**, MIDI CC **20–31**.
- Fixes: Wobble/Flanger click (fractional, clamped, interpolated delay), Crush
  bit mapping (amount 0 = identity).

## Tested

- Compiles clean (`.run 20528`, `.bss 25262`, imports nothing); link
  byte-verified (sections 2/4/5/6/7/8; ~86 KB RAM spare, ~1.69 MB flash spare).
- Emulator (`dn1-3.1h`): boots, settles, `dsp_running=2`; all six pages present.
- Hardware: first passes working (a Digitone mk1). CPU/DSP/dB headroom is not yet
  formally measured.
