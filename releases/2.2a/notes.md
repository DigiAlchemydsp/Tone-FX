# Tone+FX 2.2a — per-pattern settings, stock-scaled controls

The DIGI FX / FILTER / FOLD-EQ settings are saved **per pattern**, like stock
parameters (they follow a pattern switch / reload and travel with a project
save), and the RING/FOLD/EQ controls now use the stock **0..127** scale.

## Packages

`core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`, `digifold-1.1`,
`digifilter-1.0`, **`digictl-1.5`**.

## Changes

- **digictl 1.5** keeps the 17 FX parameters in a reserved block inside the
  saved pattern data. A fresh pattern adopts the current values; a pattern
  reload that rewrites the block is detected and adopted.
- **Stock-scaled parameters:** RING depth/rate, FOLD amount and EQ LOW/HIGH are
  now 0..127 (one step per notch), scaled inside the DSP, instead of the old
  large ranges. EQ centre 64 = flat; defaults: ring depth 51 / rate 6, EQ low 33
  / high 111.
- **Page fix:** the DIGI FOLD / EQ page title no longer clips the top row, and
  the leftover `MOD` readout was removed (the LFO destination bridge stays
  parked, off by default).
- No new key or encoder is taken.

## Tested

- Emulator (`dn1-2.2c`, OS tag 2.2c): boots and settles, `dsp_running=2`; the
  master pages and live render run with 0 faults; the fold/EQ layout is clean;
  the settings persist per pattern (`digiemu_pattern_store.py`).
- **Not yet a hardware audio pass.**
