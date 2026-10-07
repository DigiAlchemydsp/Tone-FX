# Tone+FX 3.0a — three pages, fold modes, shelf EQ

Tone+FX is now a **three-page** master FX suite: **RING** (ring modulator),
**FOLD** (wavefolder with four fold modes plus an overdrive) and **TILT**
(low/high shelf EQ), plus the output meter. The stock top status bar stays
above every page.

## Install

**One file is the whole suite:** `tonefx-3.0a.elemod` bundles `digictl` and the
four FX mods into a single **format-2** mod, so it combines with your other mods.
It does **not** include the core: install `core-dn1-2.0a.elemod` (required;
elekloader provides it) plus anything else you use.

## Changes

- **Three pages** (`FUNC+LFO`; `LEFT`/`RIGHT` or the on-screen `< >` rotate the
  whole master list): **RING** (A on/off, E depth, F frequency), **FOLD**
  (A on/off, D fold type, E/F/G/H amount) and **TILT** (A on/off, E low shelf,
  H high shelf).
- **Fold type** is a continuous **0–127** knob spread over four modes — CLEAN
  (36% fold), MUD (full fold), DIST (36% fold + hard-clip overdrive) and TRSH
  (full fold + overdrive) — interpolated between modes and capped at CLEAN/TRSH.
- **Meter** moved to the FOLD page, fixed the stuck-at-0, and it is now on MIDI
  CC `68`.
- **MIDI CC** now covers every FX parameter: RING `8`/`11`/`36`, FOLD
  `37`/`40`/`41`, EQ `67`/`69`/`96`, meter `68`.
- The per-pattern store is v5 (12 params).

## Tested

- Emulator (`dn1-3.0a-*`): boots, settles, `dsp_running=2`; the master page tree
  is `[16, 15, 17, 0, 1, 2]`. The screenshots, navigation, per-pattern store and
  MIDI-CC tests all pass.
- **Hardware pass pending.**
