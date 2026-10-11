# Tone+FX — New FX implementation plan

Status: **implemented in the source repo** (`digitone_corea`, `tonefx` 3.1c,
unreleased hardware-test build). Two new modules: **digicrush** and
**digiwave**. This file is the plan, the design envelope, and the integration
checklist; the source and its `HANDOFF.md` are the truth.

## 0. Implemented (3.1c)

- `mods/digicrush/crush.c` — BITS / SR / CRUNCH, stateless, no RAM.
- `mods/digiwave/wave.c` — WOB / SHFT / FLAN / PAN / WIDE over one 16 KB shared
  delay line and one block-rate `sin256` LFO.
- `digictl` — DIGI CRUSH (kind 3) and DIGI WAVE (kind 4) pages, encoders,
  per-pattern store **"DGX1" v6 / 22 params**, MIDI CC 20–29.
- `tonefx/mod.json` — `digicrush_render_out` order 57 (EQ→fold),
  `digiwave_render_out` order 62 (fold→meter).

Build: `.run 18880`, `.bss 25222`; link RAM `0x47be0000-0x47beae3c`
(**86,700 B spare**). digiemu boot/settle PASS (`dsp_running=2`).
Artifacts: `digitone_corea/testout/tonefx-3.1c.elemod`,
`test-3.1c.syx`. Click fix: swept delays are fractional, clamped and
interpolated. Open: CPU/DSP/dB headroom not yet measured; CC 20–29 unverified
against the stock table.

## 1. Scope

| module | what | tier |
|---|---|---|
| `digicrush` | Crunch / Redux: bit-reduce + sample-rate hold + drive | 0 (stateless) |
| `digiwave` | one modulation mod, 5 modes: **WOB / SHFT / FLAN / PAN / WIDE** | 0–3 (mode-dependent) |

Everything else in the suite (ring, EQ, fold, meter, digictl, splash) is
unchanged. Both new modules follow the existing module pattern: each is a
format-2 elemod, exports its parameters and `_render_*`, contributes to the
render event with an `order`, and lets `digictl` own the page, the encoders,
the per-pattern store and the MIDI CC map.

## 2. Design envelope

- **CPU**: main ColdFire (m68k ISA). The second ColdFire ("DSP") renders the
  eight FM voices; the FX hook the **main-CPU audio render (vector 191)**.
- **Audio**: **48 kHz**, **32-frame stereo blocks**, samples **int32 Q1.31**
  (the codec is 24-bit).
- **Block budget**: 166,667 core cycles per 667 µs block @250 MHz. Stock
  render ≈ 56.6% of a block; the master stage alone ≈ 31.9%. An effect should
  stay in the ~1–2 % of a block range so several can stack.
- **Handler ABI**: `ev_render_in` / `ev_render_out` handlers are
  **`void f(void)`**, run at **interrupt level, 1500×/s**. The core saves
  d0–d7/a0–a5 (d1/a0/a1 kept around handlers). The EMAC is in use: **leave
  MACSR as found**.
- **RAM**: `.run` + `.bss` = **128 KB shared by every mod**; `.fast` 2304 B.
  Long buffers go to the **`bulk` region** (0x44000000–0x47BE0000, ~60 MB)
  claimed as `resources.regions`, which needs **core-dn1 3.2**.
- **Reusable assets already in the suite**: `sin256` sine table, the ring
  LFO, the fold's stateless saturator, the `dg_vbar`/`dg_title`/`dg_arrows`
  widgets, and the `"DGX1"` per-pattern store.

## 3. Signal order

```
ring → EQ → digicrush → fold → digiwave → meter
```

`digicrush` sits before the folder so Redux/drive feed the fold; `digiwave`
sits after the folder but before the meter so its stereo moves and pitch are
measured. Existing master-FX orders (out 50/55/60/65) are **not touched**;
add `digicrush` at an order between EQ and fold, and `digiwave` between fold
and meter.

> Verify which event the existing master FX actually subscribe to (the elemod
> exports both `_render_in` and `_render_out`); mirror ring/eq/fold. The
> expected case is `ev_render_out` (post master stage). If they use
> `ev_render_in`, use that.

## 4. digicrush — Crunch / Redux

Three independent, stateless per-sample stages combined under one page. All
masks, ratios and gains are computed **once per block**; the sample loop is
straight-line.

### 4.1 Bit-reduce (Redux "bits")
- `mask = (int32)(-1 << (32 - bits))`; per sample `x &= mask`.
- Optional TPDF dither: fast xorshift noise added before the mask (removes the
  staircase at low depths). Dither on/off is the mode knob's high half or a
  separate switch.

### 4.2 Sample-rate hold (Redux "SR")
- Counter `n` reloaded to `ratio-1`; at 0 latch the input into `held`, else
  emit `held`. Knob 0–127 → divide 1..64 (table).
- Branch-free: `subq.l #1,d6 / bge.s keep / move / reload`.

### 4.3 Crunch drive
- `y = saturate(x * gain)`, Q15 gain into a clamp; a 2-mul cubic `x - x^3/3`
  for the soft variant. **Reuse the fold module's saturator** so `amount 0` is
  an exact bypass.

### 4.4 Page
- **A** on/off · **D** mode (BITS / SR / CRUNCH / all) · **E** amount/depth ·
  **F** tune (dither / drive / ratio). Amount 0 → exact bypass, handler
  early-outs.

### 4.5 Cost
≈ 2–5 cyc/sample/channel, **no RAM**. Inner-loop sketch:

```asm
| bit-reduce + dither: d0=x, d6=LFSR, d7=mask
    move.l  d6,d1
    lsl.l   #1,d1
    eor.l   d6,d1
    move.l  d1,d6
    add.l   d1,d0
    and.l   d7,d0

| SR-hold: d0=in, d1=held, d6=counter, d5=ratio-1
    subq.l  #1,d6
    bge.s   1f
    move.l  d0,d1
    move.l  d5,d6
1:  move.l  d1,(a0)+
```

## 5. digiwave — one mod, five modes, one page

`digiwave` replaces the separate Wobble and Shift ideas and folds in Autopan,
Stereo Width and Flanger. One page, one store block, one CC set.

### 5.1 Modes and control map

| mode | engine | SPEED | DEPTH | extra |
|---|---|---|---|---|
| **WOB** wobble | modulated delay, no dry | LFO rate | detune depth | MIX |
| **SHFT** pitch | 2-tap resample + crossfade | optional LFO drift (0 = static) | semitones | MIX |
| **FLAN** flanger | modulated delay + feedback | LFO rate | sweep depth | FB, MIX |
| **PAN** autopan | pan-law LFO, no delay | LFO rate | pan depth | — |
| **WIDE** width | mid/side, no LFO | ignored | width | — |

- Mode is **stepped, not interpolated** (algorithms differ): map 0–127 to the
  five buckets on detent.
- The page shows a big **mode title** (FOLD precedent) and the E/F/G/H labels
  and units change with the mode, e.g. F = `DTUNE`, `SEMI+12`, `SWEEP`,
  `AMT`, `WIDTH`; E grays out in WIDE.

### 5.2 Shared engine

One delay-line primitive serves WOB, FLAN and SHFT:

```
void dl_read_frac(dl, posQ16_16, *o);   // integer index + linear interpolation
```

- Delay buffer length is a power of two → wrap with `AND`, never modulo.
- One block-rate LFO from `sin256`, **linearly ramped across the 32 samples**
  so there is no per-sample sine evaluation.
- WOB / FLAN: `pos = write - (baseDelay + depth·LFO)`; FLAN adds feedback and a
  dry/wet mix, WOB has no dry.
- SHFT: `pos += r` per sample, `r = 2^(semitones/12)` in Q16.16 from a
  128-entry ratio table; two taps at `pos` and `pos + L/2`, complementary
  triangle windows (256-entry Q15 table indexed by `pos & (L-1)`), jump a tap
  by `L` when its window hits zero.
- Cheap paths: PAN is two per-sample pan-law gains from the LFO (no delay);
  WIDE is M/S computed once per block (no LFO).
- Mode dispatch happens **once per block**, not per sample. `off`, or WIDE at
  width 64, is an exact bypass with an early-out.

### 5.3 Cost estimate

| mode | ~cyc/sample/ch | RAM |
|---|---|---|
| PAN | 4 | 0 |
| WIDE | 4 | 0 |
| WOB | 12–20 | delay line |
| FLAN | 15–22 | delay line |
| SHFT | 30–50 | delay line |

One delay line (~15–30 KB) is shared by WOB/FLAN/SHFT: keep it in `.bss` if it
fits, else claim it in `bulk` (needs core-dn1 3.2). PAN/WIDE need no RAM.

### 5.4 Pitch-shifter reference

```asm
| a0=line, d1=rp Q16.16, d2=0xffff, d4=r Q16.16
    add.l   d4,d1
    move.l  d1,d3
    lsr.l   #8,d3            | integer index
    | tap1: interp (a0,d3.l),(a0,d3.l+4) -> d5
    | tap2: +L/2 offset, same
    | window1, window2 (phase = rp & mask), muls, add -> out
```

## 6. Cross-cutting optimization rules

1. **Q1.31 end-to-end**; use the EMAC for 32×32→64 and **restore MACSR**.
2. **All coefficient/table math once per block**; no `DIV` in inner loops —
   precompute reciprocals.
3. **LFO at block rate + linear ramp** across the 32 samples.
4. **Branch-free** inner loops: `Scc`/masked moves for clip and select, a
   counter for the decimator, straight-line sample loops.
5. **Register state** in d2–d7, pointers in a0/a1 with `(a0)+`.
6. **Exact bypass** at off / amount 0 / ratio 1 → handler early-out.
7. **Memory**: power-of-two, 4-byte-aligned lines; short lines in fast SRAM,
   long lines in `bulk`.
8. **Block-rate smoothing** of every control to avoid clicks.
9. **One delay engine** shared by WOB/FLAN/SHFT.

## 7. Integration

- **Events**: subscribe `digicrush_render_out` and `digiwave_render_out` (see
  §3 note) with orders that place them as in §3, leaving 50/55/60/65 alone.
- **digictl**: add one page each, with mode titles and mode-dependent labels;
  reuse `dg_vbar`/`dg_title`/`dg_arrows`.
- **Store**: extend the `"DGX1"` per-pattern block and **bump its version and
  length** to hold the new parameters.
- **MIDI CC**: assign new CCs from the pool the DN does not use, one per
  parameter (on/off, mode, speed, depth, mix, fb, bits, SR, crunch).
- **Chain**: `ring → EQ → digicrush → fold → digiwave → meter`.
- **Load**: keep a per-module bypass so users can manage the shared CPU/DSP
  (the known `README.md` caveat).

## 8. Memory budget

| item | where | size |
|---|---|---|
| digicrush | — | ~0 |
| digiwave state | `.bss` | < 256 B |
| digiwave delay line | `.bss` or `bulk` | ~15–30 KB |
| UI (page widgets, tables) | `.run` | small |

The 128 KB `.run`+`.bss` pool is shared with ring/EQ/fold/meter/digictl/
splash, so prefer `bulk` for the delay line and keep `.bss` small.

## 9. Implementation order

1. **digicrush** — no RAM, no new infrastructure. Add the digictl page/store/
   CC. Lowest risk; ships first.
2. **digiwave**: PAN + WIDE first (cheap, no delay engine), then WOB (introduces
   the delay line + `sin256` LFO), then FLAN (feedback + dry/wet), then SHFT
   (constant-rate read + crossfade).
3. Bump the per-pattern store; add the MIDI CCs.
4. Verify in digiemu (boot, settle, `dsp_running=2`, live render, 0 faults),
   then stress-test on hardware as for 3.0a/3.0b.

## 10. Testing

- Emulator: boot + settle; each mode renders with no faults; per-pattern store
  survives pattern switch/reload; CC applies and marks the store dirty.
- Hardware: long busy patterns with each mode; check for dropouts at high
  depth; confirm bypass is exact (bit-identical passthrough).
- Mono compatibility check for WIDE; dry-tail movement check for PAN/FLAN/WOB.

## 11. Open questions / risks

- **Shift quality**: linear interpolation + triangle window is deliberate
  "lightweight"; expose it as-is (optionally a Hann window later).
- **Where the master FX hook** (`ev_render_in` vs `ev_render_out`) must be
  confirmed from the source before wiring orders.
- **WIDE semantics**: M/S width is a master stereo move; document the mono
  trade-off on the page/README.
- **RAM**: if the shared delay line does not fit `.bss`, `digiwave` requires
  core-dn1 3.2 for the `bulk` region.
