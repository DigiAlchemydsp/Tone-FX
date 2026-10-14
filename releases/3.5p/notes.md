# Tone+FX 3.5p — macro pages, a performance pass, flanger & microtiming fixes

An **eleven-page** master FX suite for the Digitone mk1 — **RING · FOLD · TILT ·
CRUSH · PITCH · STEREO · PERFORM · WASH · RADIO · DESTROY · SPACE** — plus the
output meter and the animated TONE+FX boot splash. Everything ships as one
elemod.

## Install

**One file is the whole suite:** `tonefx-3.5p.elemod` bundles the boot splash
plus `digictl` and the FX mods into a single **format-2** mod, so it combines
with your other mods. It does **not** include the core: install
`core-dn1-2.0a.elemod` (required; elekloader provides it) plus anything else you
use.

## New in 3.5p

- **Four macro pages** — each is one vertical slider on the right (A = on/off,
  B–H = amount) with a per-page animated graphic on the left:
  - **WASH** — wobble (WAVE WOB) + sample-rate reduction (CRUSH SR) + an EQ tilt.
  - **RADIO** — a deep band cut (EQ low/high) + TRSH fold drive + CRUSH.
  - **DESTROY** — fold + bit-reduction + ring modulator.
  - **SPACE** — WAVE stereo width.
- **WAVE on/off is split** (`digiwave_on` for PITCH, `digiwave_son` for STEREO),
  so the two groups toggle independently.
- **Per-pattern store** is now block 1 `"DGX1"` **v10** (28 parameters) plus a
  second block `"DGX2"` **v1** (the macros): every page and every macro follows a
  pattern switch and travels with a project save.
- **OFF message box:** any FX or macro page whose section is switched off draws
  a small framed **OFF** box, so a bypassed page is unmistakable.
- **Redraw gating:** animated pages redraw at ~10 Hz and static ones at ~4 Hz;
  leaving a page stops its redraw.

## Fixes

- **Flanger/pitch link:** the FLAN, WOB and SHFT modes shared one delay line and
  the flanger wrote its feedback into it, so switching the flanger off left
  residue that distorted the PITCH block. FLAN now has its **own** delay line;
  the two can no longer interfere.
- **Flanger never truly bypassed:** the FLAN mix ramp stored the target instead
  of the ramped value, so switching it off restarted at full mix every block — a
  per-block flanger buzz. Fixed; FLAN now bypasses cleanly.
- **MIDI-track microtiming / volume:** the key and encoder handlers could stay
  armed after the master view was closed, so in the sequencer "hold a trig +
  LEFT/RIGHT" did not nudge the microtiming and encoder turns could leak. They
  now disarm the moment the master view closes.

## Performance

A measured performance pass (friendly to busy patterns): the page graphics draw
straight to the framebuffer instead of one firmware call per pixel; the output
meter only scans while its page is up; block-constant branches in CRUSH/FOLD are
hoisted; and the per-pattern store only fully compares every few UI ticks. The
output ceiling (soft limiter) still runs on every block.

## Tested

- Compiles clean (`.run 27475`, `.bss 19446`, imports nothing); links with the
  core (RAM ~47.5 KB, flash spare ~1.68 MB).
- Emulator (`dn1-3.5p`): boots, settles, `dsp_running=2`; all **eleven** pages
  present (`kinds=[16,15,17,0,1,2,3,4,5,6,7,8,9,10]`); the flanger/pitch lines
  verified independent; the per-pattern store round-trips.
- Hardware: the 3.2 line was stress-tested on two Digitone mk1 units; **3.5p is
  a new build — please report anything you find.**

## Known issues

- **PAGE can still jump some FX levels** (the stock-event store guard does not
  cover every path).
- **PITCH can fire on a note-off** from a PHASER trig.
- **SHFT quality:** the pitch shifter is deliberately lightweight; its grain
  history is shorter in 3.5p.
- **PERFORM trig LEDs** do not light yet.

The full, current list is in the README's "Known bugs / future fixes".
