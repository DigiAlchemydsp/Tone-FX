# Tone+FX 3.2b — separate WAVE pitch/stereo depths, split fold from crush

A seven-page master FX suite for the Digitone mk1 — **RING · FOLD · TILT ·
CRUSH · PITCH · STEREO · PERFORM** — plus the output meter and the animated
TONE+FX boot splash. Everything ships as one elemod.

## Install

**One file is the whole suite:** `tonefx-3.2b.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.2b

- **Each PERFORM pad now drives one engine, and the WAVE pitch and stereo
  depths are independent** (previously they shared one depth and the fold/crush
  pads moved both amounts).
- **WAVE has two depths** (`digiwave_pdepth` / `digiwave_sdepth`): the PITCH and
  STEREO pages, the PERFORM **D/E** encoders and **CC 27** each edit only their
  own group.
- **Trigs:** `1 2 9 10` ring, `3 4` **fold only**, `11 12` **crush only**,
  `5 6 13 14` pitch, `7 8 15 16` stereo. Fold and crush have two pads each
  (intensities 2 and 4); the four-pad blocks keep 1–4.
- Per-pattern store is now **`"DGX1"` v8 / 25 params**; an older v7 block adopts
  the live values. `digictl` 2.2.

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

- Compiles clean (`.run 22722`, `.bss 25390`, imports nothing); link
  byte-verified (sections 2/4/5/6/7/8; ~82 KB RAM spare, ~1.69 MB flash spare).
- Emulator (`dn1-3.2b`): boots, settles, `dsp_running=2`; all seven pages
  present (`kinds=[16,15,17,0,1,2,3,4,5,6]`).
- Live-audio check of the pads: fold `0 -> 63` and crush `0 -> 63` each move
  alone; pitch `64 -> 79` and stereo `64 -> 111` are independent; ring
  `51 -> 70 -> 51`. Store v8 and the MIDI-CC regressions pass.
- Hardware: **stress-tested on two Digitone mk1 units — in parallel and in
  series — over synths, drums, vocals and master tracks (including vinyl)**, on
  the 3.2 line. See the README's "Known bugs / future fixes" for tracked
  follow-ups.
