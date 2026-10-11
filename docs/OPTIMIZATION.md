# Tone+FX CPU / footprint optimization (worktree `fx-opt`)

This worktree (`digitone_corea_fxopt`, branch `fx-opt`) is a clean copy of the
FX baseline (`fx-additions`, commit with digicrush + digiwave) for optimizing
CPU instructions and code size, A/B against `digitone_corea` (`fx-additions`).
`HANDOFF.md` has the session context.

Baseline build (branch `fx-additions`): **`.run 18880`, `.bss 25222`**, 11
sites, 497 relocations, imports nothing.

## How to A/B footprint

Build each tree and compare the SDK's size line. Both trees need the same env
(`$env:ELEKLOADER_CROSS`, `$env:PYTHONPATH`); see `HANDOFF.md` §4.

```powershell
# baseline
python -m elekloader.sdk.build "mods\tonefx" --stock <stock.syx>   # in digitone_corea
# optimized
python -m elekloader.sdk.build "mods\tonefx" --stock <stock.syx>   # in digitone_corea_fxopt
```

Footprint = the printed `.run` (code + rodata) and `.bss` (RAM). **Cycles** need
the emulator: `emu.portable --check --timing` (slow) reports the render
vector's deadline margin and CPU busy %; `emu.cftiming.deadline_report(clock,
vector, period)` gives worst/p99 cycles against the 667 us block. A/B the same
page/scenario in both builds.

## Measured so far (compiler flags only, no code change)

`mod.json` supports per-mod `cflags` (appended after the device's `-O2`; see
`elekloader/sdk/build.py:125`).

| flags | `.run` | `.bss` | notes |
|---|---|---|---|
| (`-O2` baseline) | 18880 | 25222 | imports nothing |
| `-O3 -funroll-loops` | 35112 | 25230 | **imports `memset`** (loop distribution) — unusable |
| `-O3 -fno-tree-loop-distribute-patterns` | 24152 | 25230 | +28% code, imports nothing |

`-O3` grows code ~28% for an unmeasured cycle gain; `-funroll-loops` needs
`-fno-tree-loop-distribute-patterns` to avoid a `memset` import (no libc). Keep
`-O2` unless the emulator shows a real render-margin win.

## Optimization menu (best impact first)

1. **Fuse the six master passes into one loop.** ring(50) -> eq(55) -> crush(57)
   -> fold(60) -> wave(62) -> meter(65) each loop the 32-frame buffer today
   (`ring.c:45`, `eq.c:49`, `fold.c:102`, `crush.c`, `wave.c`, `meter.c:43`):
   ~6 loads + 6 stores per frame. One fused loop loads L/R once and stores once.
   Keep the modular handlers for standalone builds; add a `tonefx_master()` fast
   path for the merged suite.
2. **EMAC multiplies.** `dn_mul15`/`dn_mul14` (`src/corea.h:55`) are 2 muls +
   2 shifts. The MCF5441x EMAC does an exact 32x32->64 MAC in one op; the core
   already saves/restores EMAC around the render, so a handler may use it but
   must leave `MACSR` as found (`DSP.md` §2.4/§4).
3. **SHFT trim.** Nearest-neighbour on the second grain (or both) drops 2-4
   interp muls; only RAM changes with grain length.
4. **Meter.** Only scan when the page is shown (gate on `dc_vis`); fold peak
   detection into the fused loop.
5. **Loop-level.** Unroll 2-4 frames; hoist table/state pointers; mark the
   delay line + tables `__attribute__((aligned(16)))`.
6. **Blocked:** `.fast` on-chip SRAM — the DN device profile claims no `.fast`
   area and the render block is not self-contained (`docs/FAST-AUDIO-DN.md`).
