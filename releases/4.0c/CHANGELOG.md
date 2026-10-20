# Changelog

## 4.0c — permanent chain order + WASH CPU

Packages: `core-dn1-4.0`, `tonefx-4.0c` (Digitone 1.43) and `tonefx-4.0c-os1.44`
(Digitone 1.44); `core-4.0`, `tonefx-4.0c-dt` (Digitakt 1.53) and
`tonefx-4.0c-dt-os1.54` (Digitakt 1.54).

- **Permanent chain order:** ring → EQ → crush → fold → **PITCH → DELAYS →
  STEREO** → ceiling → meter. The WAVE insert is split into two permanent
  passes (`digiwave_pitch_render_out` / `digiwave_stereo_render_out`) with the
  delay between them, so the order no longer depends on the active page. The
  inactive wave pass is a single branch.
- **WASH CPU:** while WASH is on it forces the WRMZ to a **static read**
  (`MOD = 0`) and **skips the DC blocker** (the feedback saturator still bounds
  it), and the DC state is reset on enter/leave. This removes the two hottest
  per-frame MACs behind the delay; WASH scoping is handled by the per-engine
  macro capture/restore.
- **eMAC:** the L/R wet-mix pairs in WOB and WRMZ/GRAINZ now use `bx_mul15x2`
  (two MACs back to back on separate accumulators) instead of a serial
  `mac → movclr → mac → movclr` chain.

## 4.0b — per-pattern store + macro rework (MIDI-channel / PAGE-jump fix)

Packages: `core-dn1-4.0`, `tonefx-4.0b` (Digitone 1.43) and `tonefx-4.0b-os1.44`
(Digitone 1.44); `core-4.0`, `tonefx-4.0b-dt` (Digitakt 1.53) and
`tonefx-4.0b-dt-os1.54` (Digitakt 1.54).

- **Digitone: all FX params now ride the pattern in ONE reserved MIDI track.**
  The old scheme spread them over the MIDI-track blocks at
  `pattern+0x1040/+0x1410/+0x17E0` (`"DGX1/2/3"`, MIDI tracks 1-3); a
  *configured* MIDI track writes its channel/page there, so the stock data and
  our values corrupted each other — the reported **jumping FX values and moved
  MIDI channels / pages**. The 51 params are now packed (1-7 bits each) into
  **MIDI track 4's block at `pattern+0x1BB0`** (`"DGX4"`, v1), reserved on
  **MIDI channel 16**. Only that one track is touched; MIDI tracks 1-3
  (channels 1-15) and the synth tracks are never read or written. The params
  save and reload with the pattern (pattern switch, reload, project save).
- **Digitakt: the store stays a mod-owned RAM table** (`dn_tab[128][48]` in
  `.bss`, keyed by `*0x4199dc44`) — no equivalent MIDI-track block exists, and
  this avoids the kit spare contested by other mods. RAM-only.
- Digitone `.bss` stays ~19.4 KB (3.5r) / ~47.2 KB (4.0a); the packed block
  adds no table. Digitakt `.bss` ~51.2 KB. Both still `imports nothing`.
- Verified in digiemu (`dn1-4.0a`): the old blocks carry no magic and stay clean;
  `+0x1BB0` carries `"DGX4"`; an edit is saved; a value set on one pattern does
  not leak to another; a return reloads it. See `docs/PATTERN-STORAGE.md`.
- **Macros:** **WASH** now drives **WAVE WOB only** (75% depth / 30% rate at
  full) plus the **WRMZ delay's MIX and FB**, and its graphic is a full-screen
  Haze dot cloud (56 dots, cheap). **RADIO** now uses an **EQ tilt** (low → 0,
  high → 127 at full), TRSH fold drive and **SRR-only** crush at a lower amount.
  Macro capture/restore is now **per engine**: an engine a macro does not drive
  is left live, so **CRUSH/REDUX is no longer tied to WASH**; the WRMZ
  on/mode/mix/fb are captured when WASH drives them.

## 4.0a — DELAYS (WRMZ / GRAINZ) + CPU pass

Packages: `core-dn1-4.0`, `tonefx-4.0a` (Digitone 1.43) and `tonefx-4.0a-os1.44`
(Digitone 1.44); `core-4.0`, `tonefx-4.0a-dt` (Digitakt 1.53) and
`tonefx-4.0a-dt-os1.54` (Digitakt 1.54). **Four OS files, two hosts.**

- **New DELAYS module (kind 11).** One dedicated, mono delay line serves two
  modes, appended after the FX chain: **WRMZ** — a modulated delay
  (TIME / FB / TONE / MOD / RATE / MIX) — and **GRAINZ** — a cloud of three
  drifting, interpolated taps on the same line (DENS / SIZE / PIT / SPR / FB /
  MIX). **D** switches the mode; **A** is on/off; **B, C, E, F, G, H** edit the
  active mode's six parameters. True-bypass when off. Per-pattern store
  **`"DGX3"` v1** (15 params); **MIDI CC 80–87**. The chain is now
  **ring → EQ → crush → fold → wave → WRMZ → ceiling → meter**.
- **CPU / headroom pass.**
  - The eMAC mode switch is **hoisted to once per block** (`digictl_macsr_begin`
    / `_end`, `ev_render_out` order 49/74) instead of once per handler. An
    **eMAC-off** hardware A/B idled lower but ran **worse while processing**, so
    the eMAC is kept.
  - **Wave lines are s16** (were 24-bit), interpolated in the line's scale —
    halves the wave lines' DDR traffic and RAM.
  - The **wave `wv_flush` mode-change spike is removed** — only FLAN's own
    feedback line is cleared, so each mode's **worst case now equals its p99**
    (SHFT worst +9,984 → +5,491 cycles/block).
  - Animated UI pages redraw at **~5 Hz** (was ~10); a page whose section is off
    is static.
  - Built **`-Os`**: `.run 34,398 → 26,018` (−24 %), for i-cache. `.bss
    85,114 → 47,246`. `imports nothing` on both builds.
- Two page trees (main scroll + macro tree) are unchanged; the DELAYS page is
  appended to the master page tree.
- **Digitakt mk1 port brought to 4.0a** (community port by **gdeo607**, source
  mirrored under `DIGITAKT PORT/`): the same DELAYS module and CPU pass, twelve
  pages. DT1-specific: `DC_PER_STEP` **4 → 1** (one step per knob notch on the
  hardware; 4 was only ever right in the emulator), the master-page cycle now
  admits **only the stock master-FX kinds (COMP / internal / external mixer)
  plus ours** (a non-master page sharing the vector no longer rides along), and
  the per-pattern store is repacked to `"DX"` **v9** (48 params, 1-bit
  on/off/switch params, 34 bytes) so the DELAYS settings persist.

## 3.5r — OS 1.44 support, two page trees, UI pass

Packages: `core-dn1-4.0`, `tonefx-3.5r` (Digitone 1.43) and
`tonefx-3.5r-os1.44` (Digitone 1.44). The community **Digitakt mk1** port
(`core-4.0`, `tonefx-3.5r` / `tonefx-3.5r-os1.54`, OS 1.53 / 1.54) ships in the
[`gdeo607/digi1_mods`](https://github.com/gdeo607/digi1_mods) project; a copy of
its source is under [`DIGITAKT PORT/`](DIGITAKT%20PORT/).

- **OS 1.44 support.** One source now builds both OS versions: the firmware
  addresses are `#ifndef *_ADDR` defaults (1.43) that the `ports.1.44` entry
  overrides through `cflags`/`defsym`, and it carries the eleven 1.44 sites.
  Both builds lint/patch clean with every site byte-checked against its stock
  image; **1.44 is hardware-verified**.
- **Moved to elekloader 0.5.0 + core-dn1 4.0a.** The suite links as
  `core 4.0 > tonefx 3.5r` and the mods' RAM base is now `0x43EC0000` (1.25 MB,
  was 128 KB). Build `.run 28889`, `.bss 19454`, `imports nothing`.
- **Two page trees.** The **main scroll** (**LEFT / RIGHT**, unlimited) is the
  stock master pages then our FX pages in play order **FOLD · CRUSH · RING ·
  STEREO · PITCH · TILT**. **UP** (or **DOWN**) opens the **macro tree**:
  **PERFORM · WASH · RADIO · DESTROY · SPACE** (LEFT/RIGHT cycle within it; UP
  returns to the FX page you left). **PERFORM is no longer in the main scroll.**
- **FUNC + an encoder turn snaps that parameter to 0 % / 100 %.**
- **UI pass:** the vertical sliders are full height again; a one-line caption at
  the bottom row shows the **live value** while that encoder is moved (for 20
  frames), with the value centred on its **middle digit**; the mode name is
  centred over the page title, and the sliders + labelled readouts sit in the
  rightmost third. The output meter moved to **x 108/113** (clear of the `>`
  arrow and the CRUSH TUN slider frame) and is drawn on the **CRUSH** and
  **FOLD** pages.
- **CRUSH:** the sample-rate-hold mode **SR** is renamed **REDUX**.
- **PITCH** gains a **G mix** (dry/wet) control — three sliders (SPD, DEP,
  MIX); **WOB / SHFT** now have a dry/wet blend.
- **STEREO:** **FLAN** shows **four** sliders (SPD, DEP, MIX, FB); **PAN /
  WIDE** show two.
- **Macro cue:** a page driven by a macro page draws a framed **MACRO** box in
  the same slot where an off page draws **OFF**.
- **Per-pattern store:** `"DGX1"` **v10** (28 params) + `"DGX2"` **v2** (the
  macros). Both follow a pattern switch and a project save.
- **Cross-platform release (four OS files, two source trees).** The same 3.5r
  effect code ships from the Digitone source (`digitone_corea`, this release)
  and the community Digitakt mk1 port by **gdeo607** (`digi1_mods`, source
  mirrored under `DIGITAKT PORT/`); only the render path, page model, keys, CC
  map and store differ per device. All four builds — Digitone 1.43 / 1.44 and
  Digitakt 1.53 / 1.54 — link as `core 4.0 > tonefx 3.5r` and patch
  byte-for-byte against their own stock file. See the port's
  [`mods/tonefx/README.md`](DIGITAKT%20PORT/digi1_mods-main/mods/tonefx/README.md)
  for the full cross-platform release checklist.
- **Tried and reverted (no shipped change):** fusing the master passes into one
  `CORE_AUDIO_INSERT` callback was bit-correct but raised CPU/DSP load ~15 % on
  hardware, so it was dropped (write-up in the source repo's
  `docs/OPTIMIZATION.md` §12.3).
- `digictl` 3.5r. Verified: all four builds are format-2, subscription-only and
  `imports nothing` — Digitone 1.43 / 1.44 both `.run 28889`, `.bss 19454`,
  11 sites; Digitakt 1.53 / 1.54 both `.run 25795`, `.bss 11100`, 4 sites — and
  each patches byte-for-byte against its stock file (the 1.43 main OS is
  byte-identical to 3.5p), linking `core 4.0 > tonefx 3.5r`. The emulator suite
  keeps the port's documented 3.5r gap (its unicorn test still names the
  pre-3.2b `digiwave_depth`).

## 3.5p — macro pages, performance pass, flanger & microtiming fixes

Packages: `core-dn1-2.0a`, `tonefx-3.5p`.

An eleven-page suite now: **RING · FOLD · TILT · CRUSH · PITCH · STEREO ·
PERFORM · WASH · RADIO · DESTROY · SPACE**.

- **New macro pages** — one vertical slider (A on/off, B–H amount) + a per-page
  graphic: **WASH** (WOB + SRR + EQ tilt), **RADIO** (band cut + TRSH fold +
  crush), **DESTROY** (fold + bits + ring), **SPACE** (WAVE stereo width).
- **WAVE on/off split:** `digiwave_on` (PITCH) and `digiwave_son` (STEREO)
  toggle independently.
- **Per-pattern store v10 + `"DGX2"`:** block 1 is `"DGX1"` v10 / 28 params;
  block 2 is `"DGX2"` v1 / 6 params (the macros). Both follow a pattern switch
  and a project save.
- **New: OFF box** — an FX/macro page whose section is off draws a framed
  **OFF** message box.
- **Fix — flanger/pitch link:** FLAN gained its **own** delay line, so its
  feedback can no longer distort the PITCH (WOB/SHFT) block.
- **Fix — FLAN bypass:** the FLAN mix ramp stored its target instead of the
  ramped value, so switching FLAN off buzzed every block; it now bypasses
  cleanly.
- **Fix — MIDI-track microtiming / volume:** the key/encoder handlers now disarm
  as soon as the master view closes (they no longer eat LEFT/RIGHT or leak
  encoder turns in the sequencer).
- **Performance pass (measured):** direct framebuffer page drawing, the meter
  scans only while its page is up, block-constant branches hoisted in CRUSH/FOLD,
  and the per-pattern store compares every few UI ticks. The output ceiling runs
  on every block.
- `digictl` 3.5p. Verified: `.run 27475`, `.bss 19446`, imports nothing; eleven
  pages present; independent WAVE lines; store round-trips.

## 3.2b — separate WAVE pitch/stereo depths, split fold from crush

Packages: `core-dn1-2.0a`, `tonefx-3.2b`.

- **Fix:** on the PERFORM page the pitch and stereo pads (and encoders D/E)
  shared one WAVE depth, and the fold/crush pads drove both amounts together.
- **WAVE now has two depths:** `digiwave_pdepth` (PITCH) and `digiwave_sdepth`
  (STEREO), replacing the single `digiwave_depth`. The PITCH/STEREO pages'
  encoder **F**, the PERFORM **D/E** encoders and **CC 27** each edit only their
  own group's depth, so the two effects are independent.
- **Trigs are one block per engine:** `1 2 9 10` ring, `3 4` **fold only**,
  `11 12` **crush only**, `5 6 13 14` pitch, `7 8 15 16` stereo. Fold and crush
  have two pads each (intensities 2 and 4); the four-pad blocks keep 1–4.
- **Per-pattern store v8 / 25 params** (adds `digiwave_pdepth`,
  `digiwave_sdepth`; an older v7 block adopts the live values). `digictl` 2.2.
- Verified: compiles clean (`.run 22722`, `.bss 25390`, imports nothing), link
  byte-verified, digiemu boot/settle with all seven pages, and a live-audio
  check — fold `0→63` and crush `0→63` each move alone, pitch `64→79` and
  stereo `64→111` are independent, ring `51→70→51`; store v8 and the MIDI-CC
  regressions pass.

## 3.2a — PERFORM page (performance controls)

Packages: `core-dn1-2.0a`, `tonefx-3.2a`.

- **New master page `PERFORM` (kind 6)** — a **static** page (no animation, no
  added CPU) reached with **UP** from any of the six DIGI pages (**UP** again
  returns it to the page you came from); **LEFT / RIGHT** still cycle onto it.
- **One depth per FX block on the encoders:** A RING depth, B FOLD amount,
  C CRUSH amount, D WAVE pitch depth, E WAVE stereo depth, F EQ low,
  G EQ high, H WAVE speed. Settable with the pattern like every DIGI control.
- **The 16 trig keys are momentary performance pads** in four blocks of four
  increasing intensities: **1 2 9 10** ring, **3 4 11 12** fold+crush,
  **5 6 13 14** pitch, **7 8 15 16** pan. A held pad saves the block's depth /
  on-state, turns it on and ramps the depth up to `base + (127-base)*intensity/4`
  (`pf_apply`, one step per audio block in `ev_render_in`); release ramps back to
  the saved pattern value and restores on/off. The pad **never writes the
  per-pattern store**, so the saved value is always the resting position.
- **`digictl` 2.1.** Per-pattern store **unchanged**: `"DGX1"` **v7 / 24
  params**; no new MIDI CC.
- **Not done:** the trig keys do not yet light yellow (needs the Digitone LED
  driver; a planned follow-up).
- Verified: compiles clean (`.run 22596`, `.bss 25366`, imports nothing), link
  byte-verified (sections 2/4/5/6/7/8; ~82 KB RAM spare, ~1.69 MB flash spare),
  digiemu boot/settle (`dsp_running=2`) with all seven pages
  (`kinds=[16,15,17,0,1,2,3,4,5,6]`), plus a **live-audio** check (ring 51→70→51,
  fold/crush 0→31→0) and the store / MIDI-CC regressions.

## 3.1i — output ceiling, store guard, smoother ratios

Packages: `core-dn1-2.0a`, `tonefx-3.1i`.

- **Output ceiling** (`digictl_limit_out`, last render stage): a cheap soft
  limiter that keeps the master mix below the 24-bit full scale — transparent
  below ~-2.5 dBFS, asymptotic max ~-0.7 dBFS. A hot flanger-feedback / pitch
  / drive setting can no longer reach hard clipping. The flanger feedback write
  is clamped too; crush/fold already clamp their own output.
- **Stock events can no longer move our params.** The per-pattern sync now
  adopts the block **only if it still carries the `"DGX1"` magic**; a stock
  write into that memory is reclaimed instead of read. This fixes the reported
  **TILT/EQ jumping when PAGE / other keys are pressed**.
- **Bounds fix:** the encoder accumulator array is now 10 entries (was 9) with
  an encoder-id bounds check — index 9 used to write out of bounds.
- **Smoother ratios:** the pitch-shift depth→ratio index now glides
  (`wv_shf_dep`) instead of stepping.
- Verified: compiles clean (`.run 20904`, `.bss 25270`, imports nothing), link
  byte-verified, digiemu boot/settle (`dsp_running=2`) with all six pages.

## 3.1h — WAVE two pages, true bypass, page art

Packages: `core-dn1-2.0a`, `tonefx-3.1h`.

- **WAVE split into two pages**: **PITCH** (WOB / SHFT) and **STEREO** (FLAN /
  PAN / WIDE), each with its **own mode** over one shared delay engine. Speed /
  depth are shared; mix / feedback are STEREO (FLAN). Flanger feedback is capped
  at **75 %** to avoid self-oscillation.
- **Click-free true bypass per section**: each effect ramps its amount to the
  neutral value, then stops processing entirely — exact passthrough at zero
  per-sample cost. Turning a page off is clean and frees its CPU.
- **Page art**: frameless animated graphics in the leftmost third that **collapse
  to a line at neutral** — CRUSH a scrolling waveform quantised / held / clipped
  by the mode; PITCH **two vertical lines that wobble and intertwine at more
  nodes with depth/speed**; STEREO a ripple (FLAN), a split (PAN) or a spread
  (WIDE).
- **Pitch shifter**: crossfade collapsed to one multiply per channel.
- **Fixes**: Wobble/Flanger clicks (the swept delay is now fractional,
  interpolated and clamped — it could step by whole samples and wrap negative);
  Crush bit mapping (amount 0 is now identity, 24 bits).
- **Store**: per-pattern block now **`"DGX1"` v7 (24 params)**. New MIDI CC
  **20–31**.
- Verified: compiles clean (`.run 20528`, `.bss 25262`, imports nothing), link
  byte-verified, digiemu boot/settle (`dsp_running=2`) with all six pages.

## 3.1c — Crunch/Redux + Wave modulation (click fix, CPU opt)

Packages: `core-dn1-2.0a`, `tonefx-3.1c`.

- **New master insert `digicrush` (Crunch / Redux)** — one stateless stage at a
  time: **BITS** (bit-reduce to 16…4 bits, optional TPDF dither), **SR**
  (sample-and-hold decimator) and **CRUNCH** (Q8 gain into a hard clip). New
  **DIGI CRUSH** page (kind 3): A on/off, D mode, E amount, F tune.
- **New master insert `digiwave`** — one modulation insert, one page, five
  modes over a single shared delay line and one LFO: **WOB** (pitch
  wobble/vibrato), **SHFT** (pitch shift, ±12 st), **FLAN** (flanger),
  **PAN** (autopan), **WIDE** (stereo width). New **DIGI WAVE** page
  (kind 4): A on/off, D mode, E speed, F depth, G mix, H feedback.
- **Click fix**: the swept delay in WOB/FLAN is now **fractional,
  interpolated and clamped**; before, it stepped by whole samples and could go
  negative (wrapping the read index to garbage) — the audible clicks.
- **SHFT CPU**: read position computed once per grain and shared by L/R, the
  crossfade window derived (`w1 = 2^15-1-w0`), a guard sample dropping index
  masks; the LFO is block-rate. Exact-bypass guards on every mode.
- **`digictl`**: pages 3/4 wired, encoders, per-pattern store bumped to
  **"DGX1" v6 (22 params)**, new MIDI CC **20–29**. Signal order is
  **ring(50) → EQ(55) → crush(57) → fold(60) → wave(62) → meter(65)**.
- Verified: compiles clean (`.run 18880`, `.bss 25222`, imports nothing), link
  byte-verified (sections 2/4/5/6/7/8), digiemu boot/settle
  (`dsp_running=2`), first hardware pass working. CPU/DSP/dB headroom not yet
  formally measured.

## 3.0b — TONE+FX boot splash

Packages: `core-dn1-2.0a`, `tonefx-3.0b`.

- **The TONE+FX boot splash (`tonesplash`) is bundled into the suite**, so the
  splash and the FX ship as **one elemod**. It draws an animated TONE+FX wordmark
  at the intro, picking one of three animations at random each boot (accelerated
  particles, a laser trace, or a laser draw + spark bloom). Inert once the UI
  takes over.
- Everything else is unchanged from 3.0a.

## 3.0a — three pages, fold modes, shelf EQ

Packages: `core-dn1-2.0a`, `tonefx-3.0a` (digimeter, digieq, digiring,
digifold, digictl).

- **Three master pages** (`FUNC+LFO`; `LEFT`/`RIGHT` or the on-screen `< >`
  rotate the whole list): **RING**, **FOLD**, **TILT**. The stock top status
  bar (pattern name, tempo, …) stays above every page.
  - **RING** — **A** on/off, **E** depth, **F** frequency; a centred ring
    animation, the depth/frequency bars on the right and the values bottom-left.
  - **FOLD** — **A** on/off, **D** fold type, **E/F/G/H** amount; the spiral
    (left), the big mode title, and the amount as a vertical slider next to the
    L/R meter.
  - **TILT** — **A** on/off, **E** low shelf, **H** high shelf; a bent low/high
    response curve.
- **Fold type** is a continuous **0–127** knob spread over four modes — **CLEAN**
  (36% fold), **MUD** (full fold), **DIST** (36% fold + hard-clip overdrive) and
  **TRSH** (full fold + overdrive) — interpolated between modes and capped at
  CLEAN/TRSH. The overdrive is a stateless saturation, so CLEAN/MUD cost nothing
  extra per sample.
- **digieq** is back to independent low/high shelves (`digieq_lo` / `digieq_hi`).
- **Meter** moved to the FOLD page; fixed the occasional stuck-at-0 (the master
  view now redraws every tick), and it is now on **MIDI CC `68`**.
- **Navigation** cleanup: `LEFT`/`RIGHT` rotates the whole master page vector and
  no longer stalls.
- **MIDI CC** now covers every FX parameter: RING `8`/`11`/`36`, FOLD
  `37`/`40`/`41`, EQ `67`/`69`/`96`, meter `68`. The per-pattern store is v5
  (12 params).

## 2.3c — per-voice filter removed

Packages: `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`,
`digifold-1.1`, `digictl-1.9`.

- **The per-voice filter (`digifilter`) is removed from the suite.** Tone+FX is
  now the master **ring → EQ → fold** chain plus the output meter, with **two**
  master pages: **DIGI FX** and **DIGI FOLD / EQ**. The filter was
  experimental/future work; it is parked in the source repo's
  `filter-and-digimachine` branch and is not part of Tone+FX.
- **digictl 1.9**: drops the DIGI FILTER page and the filter MIDI CCs
  (`CC97` / `CC100` / `CC101` / `CC103` are free again); the per-pattern store is
  now 12 params (version 3). The ring / EQ / fold / meter controls are unchanged.
- Everything else is unchanged from 2.3b.

## 2.3b — stock encoder feel

Packages: `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`,
`digifold-1.1`, `digifilter-1.0`, `digictl-1.8`.

- **digictl 1.8**: the DIGI FX pages now step **one value per encoder detent**,
  like the stock parameters. The Digitone panel sends **4 wire counts per
  detent** and the stock 0..127 params move **one step per detent**; the old code
  required 16 counts per step — that is the encoder driver's dead-zone **startup**
  (counts before the first move), not the per-step size — so a full 0..127 sweep
  took ~4x too many turns. The driver's own fast-turn acceleration is unchanged
  (the flushed delta is clamped to +/-30), so a fast turn still moves several
  steps. Emulator-verified: `+4` -> `+1`, `+30` -> `+7`, four `+1` -> `+1`.
- Everything else is unchanged from 2.3a.

## 2.3a — MIDI CC control

Packages: `core-dn1-2.0a`, `digimeter-1.1`, `digieq-1.1`, `digiring-1.1`,
`digifold-1.1`, `digifilter-1.0`, `digictl-1.7`.

- **digictl 1.7**: the DIGI FX respond to **incoming MIDI CC**, like the stock
  parameters, so they can be played and sequenced from an external
  controller/DAW. It hooks the stock central CC router (`0x400ED94E`); only CC
  numbers the Digitone does not already use are taken.
  - RING — depth `CC8`, on/off `CC11`, rate `CC36`
  - FOLD — on/off `CC37`, amount `CC40`
  - EQ — LOW on/off `CC67`, HIGH on/off `CC68`, LOW `CC69`, HIGH `CC96`
  - FILTER — on/off `CC97`, mode `CC100`, freq `CC101`, reso `CC103`
  - A CC move is saved with the pattern (it marks the per-pattern store dirty).
- **Packaging:** the suite ships as a **single format-2 mod `tonefx-2.3a.elemod`**
  — `digictl` plus every FX mod, combined, and **without the core**, so it
  combines with `core-dn1-2.0a.elemod` (required) and any of your other mods.
  `tools/build.*` patch the core and `tonefx` together.
- Everything else is unchanged from 2.2a.
- Emulator-tested (`dn1-2.2f`): the patched router path applies all CCs and sets
  the dirty flag (`tests/digiemu_midi_cc.py`). The raw MIDI-receive path is
  hardware-validated — digiemu has no MIDI input model.

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
