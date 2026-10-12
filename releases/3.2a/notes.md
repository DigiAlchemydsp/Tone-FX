# Tone+FX 3.2a — PERFORM page (performance controls)

A seven-page master FX suite for the Digitone mk1 — **RING · FOLD · TILT ·
CRUSH · PITCH · STEREO · PERFORM** — plus the output meter and the animated
TONE+FX boot splash. Everything ships as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.2a.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.2a

- **A new static page, PERFORM.** Press **UP** from any of the six DIGI pages to
  reach it (**UP** again returns). It draws no animation, so it adds no CPU.
- **One depth per FX block on the encoders:** A RING depth, B FOLD amount,
  C CRUSH amount, D WAVE pitch depth, E WAVE stereo depth, F EQ low, G EQ high,
  H WAVE speed. Saved with the pattern like every DIGI control.
- **The 16 trig keys are momentary performance pads**, four blocks of four
  increasing intensities: **1 2 9 10** ring, **3 4 11 12** fold+crush,
  **5 6 13 14** pitch, **7 8 15 16** pan. Hold a pad and it turns the block on
  and ramps its depth up (to `base + (127 - base) * intensity / 4`); release and
  it ramps back to the saved pattern value. A pad never writes the per-pattern
  store, so your saved values are always the resting position.
- The per-pattern store is **unchanged** (`"DGX1"` v7 / 24 params); no new MIDI
  CC.
- Not yet done: the trig keys do not light yellow (planned follow-up).

## CPU headroom

The Digitone shares its CPU/DSP between the FM voices and these effects, so
leave headroom for the synth:

- **Turn off the filter on tracks you are not using** (set the track's filter
  **type to 0**) — an idle track's filter still costs CPU every block.
- **A busy pattern has less CPU headroom:** back off RING depth / FOLD amount /
  CRUSH / WAVE, and turn off the pages you are not using (each true-bypasses).
- **The animated pages are for setting up.** Their redraw costs CPU while the
  page is on screen, so dial the effect in and **move on** (press **UP** to the
  static PERFORM page, or go to any internal page). Leaving a DIGI page stops
  its redraw.

## Tested

- Compiles clean (`.run 22596`, `.bss 25366`, imports nothing); link
  byte-verified (sections 2/4/5/6/7/8; ~82 KB RAM spare, ~1.69 MB flash spare).
- Emulator (`dn1-3.2a`): boots, settles, `dsp_running=2`; all seven pages
  present (`kinds=[16,15,17,0,1,2,3,4,5,6]`).
- Live-audio check of the pads: ring `51 -> 70 -> 51`, fold/crush `0 -> 31 -> 0`;
  the per-pattern store and the MIDI-CC hook regressions pass.
- Hardware: **stress-tested on two Digitone mk1 units — in parallel and in
  series — over synths, drums, vocals and master tracks (including vinyl)**, plus
  a single unit. See the README's "Known bugs / future fixes" for the tracked
  follow-ups.
