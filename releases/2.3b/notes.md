# Tone+FX 2.3b — stock encoder feel

The DIGI FX page encoders now step **one value per detent**, matching the stock
parameter controls.

## Install

**One file is the whole suite:** `tonefx-2.3b.elemod` bundles `digictl` and every
FX mod into a single **format-2** mod, so it combines with your other mods. It
does **not** include the core: install `core-dn1-2.0a.elemod` (required;
elekloader provides it) plus anything else you use.

## Changes

- **digictl 1.8** — the DIGI FX / DIGI FILTER / DIGI FOLD-EQ encoders move **one
  step per encoder detent** (stock convention) instead of needing ~4 detents per
  step. The Digitone panel sends **4 wire counts per detent** and the stock
  0..127 params move one step per detent; the previous build divided by 16 —
  which is the encoder driver's dead-zone **startup**, not the step size — so a
  full 0..127 sweep took ~4x too many turns.
- The encoder driver's own **fast-turn acceleration** is unchanged (the flushed
  delta is clamped to ±30), so a fast turn still moves several steps.
- **The MIDI CC control from 2.3a is included.** The full CC map is in the
  README; a CC move is saved with the pattern.
- Everything else is unchanged from 2.3a.

## Tested

- Emulator (`dn1-2.3b-abd64ee5`): boots, settles and `dsp_running=2`. Encoder
  step measured live against the patched FREQ value: `+4` counts (= one detent)
  -> `+1`, `+8` -> `+2`, `+30` (fast) -> `+7`, four `+1` -> `+1`. The page and
  persistence tests pass (`digiemu_digifilter.py`, `digiemu_pattern_store.py`,
  `digiemu_midi_cc.py`, `digiemu_fx_screenshots.py`).
- **Hardware pass pending** — the encoder feel should be confirmed on a unit.
