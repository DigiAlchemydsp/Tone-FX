# Tone+FX 2.2a — per-pattern settings, stock-scaled controls

The DIGI FX / FILTER / FOLD-EQ settings are saved **per pattern**, like stock
parameters (they follow a pattern switch / reload and travel with a project
save), and the RING/FOLD/EQ controls now use the stock **0..127** scale.

## Packages

`core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`, `digifold-1.1`,
`digifilter-1.0`, **`digictl-1.6`**.

## Changes

- **digictl 1.6** keeps the 17 FX parameters in a reserved block inside the
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

- **Real hardware** (a Digitone mk1): stress-tested with the full suite — the
  build boots and holds up under long, busy patterns, and the per-pattern
  settings survive pattern switches, reloads and power cycles.
- Emulator (`dn1-2.2d`, test tags 2.2a…2.2d): boots and settles,
  `dsp_running=2`; the master pages and live render run with 0 faults; the
  fold/EQ layout is clean; the settings persist per pattern
  (`digiemu_pattern_store.py`).

## Known bugs / future fixes

- Slider behaviour (match the stock parameter widgets), encoder acceleration,
  per-parameter value readouts and consistent page titles/units.
- LFO destinations for the DIGI parameters remain **parked** (off, no page
  control).
- CPU/DSP load is the user's to manage on busy patterns.
