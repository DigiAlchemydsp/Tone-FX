# Tone+FX 2.3c — per-voice filter removed

Tone+FX is now the master **ring → EQ → fold** chain plus the output meter, with
**two** master pages. The experimental per-voice filter has been removed from the
suite (it is parked in the source repo's `filter-and-digimachine` branch and is
not part of Tone+FX).

## Install

**One file is the whole suite:** `tonefx-2.3c.elemod` bundles `digictl` and the
four FX mods into a single **format-2** mod, so it combines with your other mods.
It does **not** include the core: install `core-dn1-2.0a.elemod` (required;
elekloader provides it) plus anything else you use.

## Changes

- **The `digifilter` mod is gone.** Tone+FX ships `digimeter`, `digieq`,
  `digiring`, `digifold` and `digictl` only. The master pages are now **DIGI FX**
  (ring + meter) and **DIGI FOLD / EQ** — two pages, rotated with **LEFT / RIGHT**.
- **digictl 1.9** drops the DIGI FILTER page and the filter MIDI CCs; `CC97`,
  `CC100`, `CC101` and `CC103` are free again. The per-pattern store is now 12
  params (version 3).
- The **MIDI CC** map is RING `8` / `11` / `36`, FOLD `37` / `40`, EQ LOW `67` /
  `69` and EQ HIGH `68` / `96`. A CC move is saved with the pattern.
- **Encoder feel** (from 2.3b) is retained: one step per detent, stock-like.
- Everything else is unchanged.

## Tested

- Emulator (`dn1-2.3c-c763a782`): boots, settles and `dsp_running=2`. The page
  tree is `[16, 15, 17, 0, 1]` — the two Tone+FX pages, no filter. The
  screenshots, per-pattern store and MIDI-CC tests pass
  (`digiemu_fx_screenshots.py`, `digiemu_pattern_store.py`,
  `digiemu_midi_cc.py`).
- **Hardware pass pending.**
