# Changelog

## 2.1h — first Tone+FX suite release

Packages: `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.0`, `digiring-1.0`,
`digifold-1.0`, `digifilter-1.0`, `digictl-1.3`.

- **Three master pages** (`FUNC+LFO`; `LEFT`/`RIGHT` rotate): **DIGI FX**
  (ring), **DIGI FILTER**, **DIGI FOLD / EQ**.
- **digifilter**: per-voice `BP / BP2 / COMB / TRASH`, routed per voice with a
  **4-voice cap**; simpler integer-delay comb; coefficients once per block,
  silent voices skipped.
- **digieq**: independent low/high band enables; full-precision gain (noise fix).
- **digiring**: interpolated carrier, full-precision multiply (noise fix).
- **digifold**: exact bypass at amount 0; spiral display for the fold amount.
- **digictl**: every encoder/slider moves **1/127 of its range per step**
  (uniform speed); the FIRQ/RES curve and stock keys (PAGE etc.) untouched;
  meter runs unconditionally.
- Emulator-tested (boot, settle, `dsp_running=2`; UI + live-audio render, 0
  faults). Not yet a hardware audio pass.
