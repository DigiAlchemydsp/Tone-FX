# Changelog

## 2.2a — per-pattern settings, stock-scaled controls

Packages: `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`,
`digifold-1.1`, `digifilter-1.0`, `digictl-1.6`.

### Pattern persistence (digictl 1.6)

- The 17 FX parameters are now stored **per pattern** inside the saved pattern
  data, so they follow a **pattern switch / reload** and travel with a
  **project save / power-off**, like stock parameters.
- Each pattern's storage struct (0x1611D bytes, `patternStorage_v11_t`) leaves an
  unused 64-byte block on its four MIDI tracks (`track_t+0x100`). `digictl` keeps
  a **`"DGX1"` v2** block there (17 int16 values). It loads the block into the
  live parameters at boot and on a pattern change, writes them back on a page
  edit, and adopts a block the OS rewrote (a pattern reload). A fresh pattern
  adopts the current values, so the sound never jumps.
- See `docs/PATTERN-STORAGE.md` in the source repo for the measured layout.

### Encoder behaviour

- Every master-page parameter is now **0..127** and moves **one step per
  notch** — the stock convention — instead of *1/127 of an ad-hoc range*.
  The DSP scales 0..127 internally: ring depth/rate, fold amount, EQ gains
  (EQ centre **64 = flat**). `digictl`'s OS-style accumulation (one step per 16
  counts, direction reset) is unchanged, so the OS's fast-turn acceleration is
  still honoured. The **DIGI FILTER** page was already 0..127.
- Defaults kept: ring depth 51 / rate 6, EQ low 33 / high 111, fold 0.
- `digiring 1.1`, `digieq 1.1`, `digifold 1.1` carry the new 0..127 scale;
  `digimeter` and `digifilter` are unchanged from 2.1h.

### UI

- **DIGI FOLD / EQ:** the title no longer clips the top row, and the leftover
  `MOD …` readout was removed (the LFO-destination bridge stays **parked** in
  the code, off by default). No new key or encoder is taken.

### Tested

- **Real hardware** (a Digitone mk1): stress-tested with the full suite — the
  build boots and holds up under long, busy patterns, and the per-pattern
  settings survive pattern switches, reloads and power cycles.
- Emulator (`dn1-2.2d`, test tags 2.2a…2.2d): boots and settles,
  `dsp_running=2`; the master pages and the live render run with 0 faults; the
  fold/EQ page draws cleanly; the settings persist per pattern
  (`digiemu_pattern_store.py`).

### Known bugs / future fixes

- **Slider behaviour:** make the faders/bars match the stock parameter widgets
  more closely (EQ fill from the centre, a shared smoothing, value readouts).
- **Encoder acceleration:** match the stock fast-turn curve per parameter
  (currently one step per notch plus whatever acceleration the OS delivers).
- **LFO destinations:** driving the DIGI parameters from a track LFO is
  **parked** — the bridge is in the code but off, with no page control.
- **UI improvements:** consistent titles/units across the three pages and
  per-parameter value readouts.

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
