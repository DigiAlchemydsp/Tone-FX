# Tone+FX — DSP, CPU and footprint notes (Digitone mk1, OS 1.43)

What the suite's code does, how it is built, the fixed-point conventions, the
optimization methods used (and why), what is repurposed from the stock OS and
from RingTone, and the RAM / flash / headroom ledger. This is the technical
companion to `HANDOFF.md`; `DSP.md` is the address/architecture reference.

Current package: `tonefx 3.2b` (`.run 22722`, `.bss 25390`).

## 1. Where the code runs

Two ColdFire MCF5441x CPUs (`DSP.md`):

- **Core A (main CPU)** — UI, sequencer, **effects and the mix**. Every master
  FX in this suite is here.
- **Core B ("DSP")** — the eight FM voices. Not touched by the suite.

The suite is **subscription-only**: it never patches a stock instruction. Each
FX is a handler on the core's render events:

- `ev_render_in` (`void f(void)`, order 40/50) — remembers which half of the
  output buffer this block writes: `dn_out_off()` reads eDMA ch54's SADDR
  `0xFC0456C0` and returns `0x100` or `0`.
- `ev_render_out` (order 50..65) — processes `0x80001A00 + off` (256 B,
  **32 stereo frames, int32, 24-bit left-justified**) in place.

Interrupt level, 1500 blocks/s, 48 kHz, 667 us per block. Handlers are
`void f(void)`; d1/a0/a1 are free; the EMAC is saved/restored by the core, but a
handler that uses it must leave `MACSR` as found.

**Chain order** is the `ev_render_out` order:
`ring(50) → EQ(55) → crush(57) → fold(60) → wave(62) → meter(65)`.

## 2. Module inventory

| mod | source | lines | stage / role |
|---|---|---|---|
| `digimeter` | `meter.c` | 62 | L/R peak (log bars), scanned from the output half; read by the page |
| `digieq` | `eq.c` | 63 | one-pole low/high split; low/high shelf, Q14 gains |
| `digiring` | `ring.c` | 59 | 256-entry Q15 sine carrier; ring/tremolo |
| `digicrush` | `crush.c` | 106 | BITS mask / SR hold / CRUNCH clip — one stateless stage |
| `digifold` | `fold.c` | 100 | triangle wavefolder + hard-clip overdrive |
| `digiwave` | `wave.c` | 275 | WOB/SHFT/FLAN/PAN/WIDE, one 16 KB delay line + one LFO |
| `digictl` | `digictl.c` + `cc_glue.s` | ~730 + 28 | the 6 master pages (RING/FOLD/TILT/CRUSH/PITCH/STEREO) + the PERFORM page, encoders, keys, per-pattern store, MIDI CC, momentary trig pads |
| `tonesplash` | `render.c` + `splash.s` | 212 + 18 | boot wordmark (intro only) |
| shared | `src/corea.h`, `src/sin256.h` | 56 + 39 | Core A helpers + sine table |

Total ≈ 1,633 lines (C+asm+headers); 1,833 with `mod.json`s.

## 2b. Per-block CPU footprint (Core A)

All of it is **Core A**; the suite's **DSP (Core B) footprint is 0**. Costs are
per **active** stereo frame (32 frames/block, 1500 blocks/s), approximate, from
the compiled inner loops (`m68k-elf-objdump`) plus the source. A **bypassed**
section costs ~0 (see §4.3).

| # | block / mode | ≈ insn/frame | driver |
|---|---|---|---|
| 1 | `digicrush` — BITS | ~10 | AND mask (dither adds ~12: LFSR muls) |
| 2 | `digicrush` — SR | ~12 | hold counter, 2 loads / 2 stores |
| 3 | `digimeter` | ~15 | 2 loads + abs + compare (no mul) |
| 4 | `digiwave` — WIDE | ~18 | 2× `dn_mul14` + mid/side |
| 5 | `digifold` — CLEAN / MUD | ~20 | 2× Q8 gain + reflect |
| 6 | `digiwave` — PAN | ~24 | 3× `dn_mul15` + pan law |
| 7 | `digicrush` — CRUNCH | ~25 | 2× Q8 gain + clamp |
| 8 | `digiring` | ~28 | `sin256` interp + 2× `dn_mul15` |
| 9 | `digieq` | ~30 | one-pole + 2× `dn_mul14` |
| 10 | `digifold` — DIST / TRSH | ~35 | + 2× drive (gain → clamp) |
| 11 | `digiwave` — WOB | ~45 | 2 interpolated delay reads + blend |
| 12 | `digiwave` — FLAN | ~55 | WOB + feedback write |
| 13 | `digiwave` — SHFT | ~75 | 8 loads, 4 interp muls, window + crossfade |

Compiled `_render_out` sizes corroborate (whole function incl. setup / all
modes): `meter 162 · eq 322 · ring 326 · fold 356 · crush 476 · wave 2178`.

Notes: the delay modes (WOB/FLAN/SHFT) are ALU-light but touch the 16 KB **delay
line in DDR**, whose accesses cost more than the instruction count implies. The
whole suite runs on Core A; reduce DSP load only via fewer/blanked FM voices
(`ev_voice_on` / section 7).

## 3. Fixed-point rules (why the code looks the way it does)

- **No FPU** on the MCF5441x, **no libc, no `memcpy`/`memset`, no 64-bit
  multiply helper**. All arithmetic is integer, and `build.py` reports
  `imports nothing` — that is a hard constraint, not a nicety.
- The master bus is **int32, 24-bit left-justified**; the factory pattern peaks
  near `2^21`, so headroom for gain is ~13 bits before clipping at 24 bits.
- Helpers (`src/corea.h`):
  - `dn_q15(x,c)   = (x>>15)*c` — cheap, quantizes in steps of `c`.
  - `dn_q14d(x,d)  = (x>>14)*d` — cheap gain near 1.
  - **`dn_mul15(x,c) = (x>>15)*c + (((x & 0x7fff)*c)>>15)`** — exact
    `(x*c)>>15` with 32-bit ops only. The cheap form quantized the ring/EQ wet
    signal in steps of the carrier (~-36 dB noise); the two-term form removed
    that, at the cost of one extra multiply.
  - `dn_mul14` is the same for a Q14 coefficient.
- **No `-ffast-math`, no floats anywhere.**

## 4. Optimization methods used

Ordered roughly by how much they bought.

1. **Master-insert subscription, not a patch.** Because every FX is just an
   `ev_render_in`/`out` handler, all modules combine, there is no byte conflict,
   and the chain order is data (`order`), not code. Zero stock-byte footprint
   for the FX themselves.
2. **Block-rate parameter + per-sample ramp.** Targets are read from globals
   once per block and stepped across the 32 frames
   (`step = (target - cur) >> 5`), so knob moves and bypass are click-free with
   no per-sample condition tests. Used by ring, EQ, fold, wave.
3. **Click-free ramped true bypass (per section).** Each section's amount ramps
   to its **neutral value** over ~1 block (ring depth 0, EQ flat, fold amount 0,
   crush amount 0, WAVE per-mode neutral: WOB wet 0, FLAN mix 0, SHFT dry,
   PAN centre, WIDE 64), and once settled the handler `return`s **before any
   per-sample loop** — a true bypass with zero per-sample cost and an exact
   passthrough. This replaced the hard on/off for fold/crush/WAVE (which
   clicked) and fixed crush's bit mapping so amount 0 is identity (24 bits, not
   16). WAVE's SHFT mode gained a dry/wet fade so it can bypass cleanly; PAN and
   WIDE now ramp too.
4. **Block-rate LFO (one sine per block).** `digiwave` and `digiring` evaluate
   `sin256` at the block start and a linear per-sample step
   (`lstep = (sin(next) - sin(now)) >> 5`), replacing a per-sample sine
   (≈8 instructions/frame) with one multiply-free add.
5. **Fractional, clamped, interpolated swept delay** (`digiwave` WOB/FLAN).
   The first cut used an integer delay and a modulation span larger than the
   base delay, so the read index both stepped by whole samples and could go
   **negative** (`(wv_w + WV_N - (unsigned)d)` wrapped to garbage) — audible
   clicks. The fix: delay in **Q16.16**, read through `wv_read` (linear
   interpolation), and **clamp to >= 2 samples** so it can never wrap.
6. **Guard sample.** `wv_buf[ch][WV_N]` mirrors index 0, so a fractional read
   at `idx+1` needs no `& WV_MASK`.
7. **Derived + collapsed crossfade window (SHFT).** The two grains' windows are
   complementary, so the mix is
   `g0*w0 + g1*(1-w0) = g1 + (g0-g1)*w0` — **one multiply per channel instead of
   two**, and no second table lookup.
8. **Shared read position (SHFT).** `idx`/`frac` are computed once per grain and
   used for both L and R, instead of twice per channel.
9. **Precomputed tables.** `sin256` (Q15) is shared by ring, the wave LFO and
   the UI; `wv_ratio[128]` (Q16.16 `2^(semi/12)`) and `wv_win[256]` (Q15
   triangle) are `const` in `.run`, built at compile time.
10. **Stateless stage selection** (`crush`): BITS/SR/CRUNCH are chosen once per
    block; only one runs, so a bypassed branch costs nothing.
11. **Cheapest viable saturator.** fold's overdrive and crush's CRUNCH are a
    single Q8 multiply + two compares (stateless), not a `tanh`.
12. **Shared infrastructure / one engine, two WAVE pages.** One per-pattern
    store, one CC router `jmp`, one page factory and one invalidate path serve
    all modules. `digiwave`'s five modes are split over **two pages** — kind 4
    PITCH (WOB / SHFT) and kind 5 STEREO (FLAN / PAN / WIDE) — over one shared
    delay engine. Each page has its **own mode** (`digiwave_pmode`,
    `digiwave_smode`) so a page only ever shows and selects its own modes, plus
    an active-group flag (`digiwave_page`) set when a page is edited, which
    decides which mode the engine runs. Speed/depth are shared; mix/feedback are
    STEREO (FLAN). Feedback is capped at **0.75** (`fb = fb_knob*24575/127`) to
    keep the flanger from self-oscillating into distortion.
13. **Per-mod compiler flags (A/B, measured).** `mod.json` accepts `cflags`,
    appended after the device `-O2` (`elekloader/sdk/build.py:125`):

    | flags | `.run` | `.bss` | note |
    |---|---|---|---|
    | `-O2` (default) | 20528 | 25262 | imports nothing |
    | `-O3 -funroll-loops` | 35112 | 25230 | **imports `memset`** (loop distribution) — unusable |
    | `-O3 -fno-tree-loop-distribute-patterns` | 24152 | 25230 | +28 % code, imports nothing |

    Kept at `-O2`: `-O3` nearly doubles the code (or +28 % without loop
    distribution) for an unmeasured cycle gain, and unrolling reintroduces a
    `memset` dependency the no-libc build cannot satisfy.
14. **Output ceiling (`digictl_limit_out`, order 64).** A cheap soft limiter on
    the final mix: below a knee (~-2.5 dBFS) it is transparent; above it the
    excess is halved and hard-capped at ~-0.7 dBFS, so a hot flanger-feedback /
    pitch-shift / drive setting never reaches 24-bit clipping (0x7fffff).
    `digicrush`'s drive and `digifold` already clamp their own output; the
    flanger feedback write is clamped at the top of the delay loop too.
15. **Six passes over the buffer (identified, not yet fused).** Today ring, EQ,
    crush, fold, wave and meter each loop the 32 frames, so each frame is
    loaded/stored ~6 times. A single fused `tonefx_master()` loop (load L/R
    once, run the stages inline, store once) is the next big win — see §8.
16. **PERFORM page: static draw + cheap momentary pads.** The page (kind 6,
    reached with UP) has no animated graphics — the same pixels every frame.
    Its 16 trigs are momentary pads, one block per engine that ramp the engine's
    depth in `ev_render_in` (`pf_apply`), one step per block, and restore the
    pattern value on release. It costs a single flag test when idle, writes no
    store, and the ramp is a handful of integer ops only while a pad (or its
    release) is live. Each engine has its own depth: fold/crush are separate and
    the WAVE engine has independent PITCH / STEREO depths.

## 5. Repurposed / reused

- **`sin256.h`** — carried over from RingTone; now shared by ring, the wave LFO
  and the RING page animation.
- **The RingTone code base** — `meter.c`, `eq.c`, `ring.c`, `fold.c`,
  `digictl.c` are the original suite modules, extended here.
- **The master view's page vector.** The stock master view keeps its page kinds
  in a vector at `view+124`; `digictl` **appends** kinds 0–6 (RING, FOLD, TILT,
  CRUSH, PITCH, STEREO, PERFORM) after the stock pages and patches only the
  view's draw slot (`0x40199BBC`). No stock page is taken.
- **The unused MIDI-track 64-byte block.** Each pattern's four MIDI tracks
  leave `track+0x100` zero in all 128 slots and the OS never writes it;
  `digictl` uses `pattern+0x1040` for the **"DGX1"** per-pattern store
  (`v8`, 25 int16, 58 of 64 bytes).
- **The parked LFO bridge** (`digimod_dest`/`digimod_voice`, per-voice FLTR
  FREQ read at `0x80003544`) is kept in the store but has no page yet.
- **The stock CC router** (`0x400ED94E`) is hooked once (`cc_glue.s`, `op:"jmp"`)
  and dispatches to `digictl_cc_apply`; only CCs the stock table does not use
  are taken.
- **The intro draw/flush sites** (9 call sites) are reused by `tonesplash`.
- **The EMAC** is left to the core's save/restore; the FX avoid it today (see
  §8) but the render already pays its save/restore.
- **`build.py` size reporting / the linker** reports `imports nothing`, which is
  what keeps the no-libc build honest.

## 6. Footprint / headroom ledger (3.2b)

| item | value |
|---|---|
| `.run` (code + rodata) | 22,722 B |
| `.bss` (RAM state) | 25,390 B |
| — of which `digiwave` delay line | 2048 × 2 × 4 = 16,384 B (+8 guard) |
| — of which tables (`sin256`, `wv_ratio`, `wv_win`) | ~1,024 + 512 + 512 B (`.run`) |
| **RAM spare** | **~86 KB of 128 KB** |
| flash spare (3.2b patch) | ≈1.69 MB |
| sites / relocations | 11 / 646; imports nothing |

Budgets: `.run`+`.bss` share 128 KB across every linked mod; the delay line is
the single largest allocation. If it ever needs to grow, the `bulk` region
(0x44000000–0x47BE0000) needs core-dn1 3.2 as a claimed region; today the suite
runs on core-dn1 2.0a.

## 7. Verified vs pending

Verified: compiles clean, links/patch byte-verified (sections 2/4/5/6/7/8),
digiemu boot/settle (`dsp_running=2`), all six FX pages present
(`kinds=[16,15,17,0,1,2,3,4,5]`) and the PERFORM page (kind 6,
`kinds=[16,15,17,0,1,2,3,4,5,6]`), hardware boot + first manual pass working.
The PERFORM page is verified live (real SSI renders): UP enters it, an encoder
raises the pattern value, and each engine's pads move only their own depth and
restore on release (ring 51→70→51; fold 0→63; crush 0→63; WAVE pitch 64→79;
WAVE stereo 64→111, with the other depth unchanged). Store v8 round-trips.

Pending:
- **CPU headroom** — render-vector worst/p99 cycles and deadline margin
  (`emu.cftiming.deadline_report`) not yet captured; the full
  `emu.portable --check --timing` run was aborted.
- **DSP (Core B)** — untouched by the suite; its render is not part of the FX
  budget.
- **dB** — peak/RMS with FX off vs on not yet measured.
- **CC 20–31** — not checked against the stock CC table (`0x4018D104`).

## 8. Future footprint work

1. **Fuse the six master passes** into one loop (§4.15): order-of-magnitude
   fewer loads/stores per frame.
2. **Use the EMAC** for the Q15 multiplies (`dn_mul15`/`dn_mul14` are two muls +
   shifts; the EMAC does an exact 32×32→64 MAC in one op). The core already
   saves/restores EMAC; leave `MACSR` as found.
3. **Trim SHFT further**: nearest-neighbour on the second grain (drop 2–4 interp
   muls), optional by knob/flag; the crossfade is already one mul per channel.
4. **Meter**: gate on page visibility (`dc_vis`) and fold peak detection into
   the fused loop.
5. **Loop-level**: unroll 2–4 frames; align the delay line and tables
   (`__attribute__((aligned(16)))`).
6. **`.fast` SRAM**: blocked — the DN device profile claims no `.fast` area and
   the render block is not self-contained (`docs/FAST-AUDIO-DN.md`).
7. **Reconsider `-O3`** only if a measured render-margin win appears.
