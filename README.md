# Tone+FX

**A master effect suite for the Elektron Digitone mk1 / Digitone Keys (OS 1.43 and 1.44) and the Digitakt mk1 (OS 1.53 and 1.54), with a new DELAYS module (WRMZ / GRAINZ).**

Tone+FX 4.0b is a set of [elekloader](https://github.com/irpina/elekloader) mods,
**packaged and released together as one suite**. It adds master ring modulation,
a low/high-shelf EQ, a Crunch/Redux lo-fi stage, a wavefolder with four fold
modes (plus an internal overdrive), a modulation insert with five modes (wobble,
pitch shift, flanger, autopan, stereo width) on two pages, **a buffer-FX DELAYS
module — WRMZ (modulated delay) / GRAINZ (granular cloud)**, output metering, a
**twelve-page UI across two master page trees** (the six FX pages, the DELAYS
page, a static **PERFORM** page whose 16 trigs are momentary performance pads,
plus four one-slider **macro** pages — **WASH / RADIO / DESTROY / SPACE**) and an
animated TONE+FX boot splash.

**Four OS files, two hosts.** On the **Digitone mk1 / Digitone Keys** take
`tonefx-4.0b.elemod` (OS 1.43) or `tonefx-4.0b-os1.44.elemod` (OS 1.44). On the
**Digitakt mk1** — **gdeo607**'s port, source mirrored under
[`DIGITAKT PORT/`](DIGITAKT%20PORT/) — take `tonefx-4.0b-dt.elemod` (OS 1.53) or
`tonefx-4.0b-dt-os1.54.elemod` (OS 1.54); both now carry the DELAYS module and the
CPU pass too. Each links as `core 4.0 > tonefx 4.0b` and patches
**byte-for-byte** against its own stock file. See
[its Tone+FX page](https://github.com/gdeo607/digi1_mods/blob/main/mods/tonefx/README.md).

**Special thanks to [gdeo607](https://github.com/gdeo607), who did most of the
work on the Digitakt port** — the DT1 render path, page model, keys, pattern
store and its cross-platform release tooling are his.

This repository is **release-only**: it ships the built `.elemod` packages and
the docs to install them, plus a copy of the **Digitakt port's source** under
[`DIGITAKT PORT/`](DIGITAKT%20PORT/). It contains **no firmware**. The Digitone
source lives in the development repo, so this repo stays a safe place to cut
releases from while the older `RingTone` repo is still around. The **Digitakt
mk1** port (OS 1.53 / 1.54) is by **gdeo607** (kept in the
[digi1_mods](https://github.com/gdeo607/digi1_mods) project, vendored here). See
[`HANDOFF.md`](HANDOFF.md) for the release-side state and the cross-platform
release checklist.

> Unofficial and unsupported. Not affiliated with, endorsed by or supported by
> Elektron. Flashing modified firmware is at your own risk.

## Pages

**Twelve** master pages on **two trees** (**FUNC + LFO** opens the master pages;
**LEFT / RIGHT** rotates, **UP** or **DOWN** switches trees):

- **Main scroll** — the stock master pages, then our FX pages in play order
  **FOLD · CRUSH · RING · STEREO · PITCH · TILT · DELAYS**.
- **Macro tree** (**UP**) — **PERFORM · WASH · RADIO · DESTROY · SPACE**
  (LEFT/RIGHT cycle within it; **UP** returns to the FX page you left). **PERFORM
  is not in the main scroll.**

Each new FX page has a frameless animated graphic in the leftmost third that
collapses to a line when the effect is at neutral; the **PERFORM** page is static
(no animation); the four **macro** pages each show one vertical slider plus a
per-page graphic. Page layout: the mode name is centred over the title, and the
depth/rate sliders with their labelled readouts sit in the rightmost third;
**FUNC + an encoder turn snaps that parameter to 0 % / 100 %**.

Main scroll:

| FOLD | CRUSH | RING |
|---|---|---|
| ![DIGI FOLD page](Screenshots/FOLD.png) | ![DIGI CRUSH page](Screenshots/CRUSH.png) | ![DIGI RING page](Screenshots/RING.png) |

| STEREO | PITCH | TILT |
|---|---|---|
| ![DIGI STEREO page](Screenshots/STEREO.png) | ![DIGI PITCH page](Screenshots/PITCH.png) | ![DIGI TILT page](Screenshots/TILT.png) |

**DELAYS** (WRMZ / GRAINZ — the new buffer-FX page):

| DELAYS |
|---|
| ![DELAYS page](Screenshots/DELAYS.png) |

Macro tree (the static PERFORM page and the four one-slider macros):

| PERFORM | WASH | RADIO | DESTROY | SPACE |
|---|---|---|---|---|
| ![PERFORM page](Screenshots/PERFORM.png) | ![WASH macro page](Screenshots/WASH.png) | ![RADIO macro page](Screenshots/RADIO.png) | ![DESTROY macro page](Screenshots/DESTROY.png) | ![SPACE macro page](Screenshots/SPACE.png) |

Each macro page is one vertical slider (**A** on/off, **B–H** amount) plus a
per-page graphic: **WASH** (wobble + WRMZ wash), **RADIO** (EQ tilt + TRSH
fold + SRR), **DESTROY** (fold + bits + ring), **SPACE** (WAVE stereo width).

Any page whose section is switched off draws a small framed **OFF** box; a page
driven by a macro page draws a **MACRO** box in the same slot.

## The suite

**One file is the whole suite**, per device and OS:

| file | device | OS | required core |
|---|---|---|---|
| [`tonefx-4.0b.elemod`](elemods/) | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| [`tonefx-4.0b-os1.44.elemod`](elemods/) | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| [`tonefx-4.0b-dt.elemod`](elemods/) | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| [`tonefx-4.0b-dt-os1.54.elemod`](elemods/) | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

Each bundles every mod below into a single elekloader mod, so it **combines with
your other mods**. None includes the required core — install the matching core
(elekloader provides it) plus anything else you use.

- **digimeter** — output level metering (drawn on the CRUSH and FOLD pages).
- **digieq** — master low-shelf / high-shelf EQ (LO + HI knobs).
- **digiring** — master ring modulator / tremolo, with an animated page.
- **digicrush** — Crunch / Redux: bit-reduce (BITS), sample-rate hold (REDUX) or
  a hard-clip drive (CRUNCH), one stateless stage at a time.
- **digifold** — master wavefolder with CLEAN / MUD / DIST / TRSH fold modes and
  an internal overdrive; a spiral display.
- **digiwave** — one modulation engine, **two pages**: **PITCH** (WOB pitch
  wobble, SHFT pitch shift, each with a dry/wet MIX) and **STEREO** (FLAN
  flanger, PAN autopan, WIDE stereo width).
- **digiwrmz** — the **DELAYS** buffer-FX module: one mono delay line with two
  modes — **WRMZ** (modulated delay) and **GRAINZ** (three drifting granular
  taps). True-bypass when off.
- **digictl** — the pages and all the controls; keeps the settings per pattern
  and adds MIDI CC control, plus the **PERFORM** page and its momentary trig
  pads.
- **tonesplash** — an animated TONE+FX boot splash (one of three animations per
  boot, drawn at the intro).

Everything is `GPL-2.0-or-later` (see [`LICENSE`](LICENSE)).

## Install

1. Get **elekloader** and your own stock OS file (Elektron's download) — the
   Digitone `Digitone_and_Digitone_Keys_OS1.43.syx` (or your OS 1.44 `.syx`), or
   the Digitakt `Digitakt_OS1.53.syx` (or the OS 1.54 official `.zip`).
   elekloader builds a custom OS on your machine; it never distributes firmware.
2. In elekloader's window, add the stock file, then **+ Install from file…** and
   pick the matching **core** (elekloader ships it) and **suite**:
   - **Digitone 1.43** — `core-dn1-4.0.elemod` + `tonefx-4.0b.elemod`
   - **Digitone 1.44** — `core-dn1-4.0-os1.44.elemod` + `tonefx-4.0b-os1.44.elemod`
   - **Digitakt 1.53** — `core-4.0.elemod` + `tonefx-4.0b-dt.elemod`
   - **Digitakt 1.54** — `core-4.0-os1.54.elemod` + `tonefx-4.0b-dt-os1.54.elemod`
   Add any of your other mods too.
3. Tick the core and `tonefx`, set the OS version tag, and **BUILD FIRMWARE**.
4. Flash the resulting `.syx` with Elektron Transfer, as for any OS update;
   press **YES** on the unit and do not power off until it finishes.

Or from the command line (elekloader from source):

```powershell
# Windows PowerShell (OS 1.43)
./tools/build.ps1 -Stock "Digitone_and_Digitone_Keys_OS1.43.syx" -Out ToneFX.syx -Version 4.0b
```

```sh
# POSIX
sh tools/build.sh Digitone_and_Digitone_Keys_OS1.43.syx ToneFX.syx 4.0b
```

**Recovery** (the bootloader is never changed): hold **FUNC** while powering on
for the startup menu, press **TRIG 4 (OS UPGRADE)**, then send the stock `.syx`
with Transfer's legacy OS upgrade mode.

## Use

The pages live on the **master page tree**: **FUNC + LFO** opens it. Tone+FX
appends eleven of its own after the stock ones and splits them across **two
trees** — the **main scroll** (stock pages + the six FX pages, in play order
**FOLD · CRUSH · RING · STEREO · PITCH · TILT**) and the **macro tree**
(**UP** or **DOWN**: **PERFORM · WASH · RADIO · DESTROY · SPACE**). **LEFT /
RIGHT** (or the on-screen `<` `>` arrows) rotate within the current tree, and
**UP** returns from the macro tree to the FX page you left. **FUNC + an encoder
turn snaps that parameter to 0 % / 100 %.** The stock **top status bar** (pattern
name, tempo, …) stays visible above every page. PAGE keeps its stock meaning; no
stock key or encoder is taken.

| page | encoders | shows |
|---|---|---|
| **RING** | **A** on/off · **E** depth · **F** frequency | a centred ring animation |
| **FOLD** | **A** on/off · **D** fold type · **E/F/G/H** amount | the spiral, the big mode title, the amount slider + L/R meter |
| **TILT** | **A** on/off · **E** low shelf · **H** high shelf | a bent low/high response curve |
| **CRUSH** | **A** on/off · **D** mode · **E** amount · **F** tune | a waveform (left) that is quantised / held / clipped by the mode + L/R meter |
| **PITCH** | **A** on/off · **D** mode · **E** speed · **F** depth · **G** mix | two vertical lines that wobble and intertwine |
| **STEREO** | **A** on/off · **D** mode · **E** speed · **F** depth · **G** mix · **H** feedback | a horizontal line that ripples / splits / spreads (FLAN shows all four sliders; PAN/WIDE show two) |
| **DELAYS** | **A** on/off · **D** mode (WRMZ/GRAINZ) · **B/C/E/F/G/H** = TIM, FB, TON, MOD, RAT, MIX (WRMZ) or DEN, SIZ, PIT, SPR, FB, MIX (GRAINZ) | a moving line whose wobble follows MIX; a fixed MIX slider + a secondary slider for the parameter you last moved |
| **PERFORM** | **A** ring depth · **B** fold amount · **C** crush amount · **D** wave pitch depth · **E** wave stereo depth · **F** EQ low · **G** EQ high · **H** wave speed | a static depth readout per FX block; the trigs are momentary pads (see **Performance controls**) |

Every parameter is **0–127** and moves **one step per notch** — the stock
convention (EQ LOW/HIGH are 0–127 with **64 = flat**).

**Per-pattern settings** (Digitone: one reserved MIDI track, channel 16, packed;
Digitakt: a mod-owned RAM table): the settings — including the macro pages —
save and reload with the pattern. On the Digitone only that one MIDI track is
touched, so channels 1-15 and the synth tracks are unaffected; a fresh pattern
adopts the live values. Each page
**true-bypasses** when its **A** is off (fades out, then stops using CPU). An FX
or macro page that is off draws a framed **OFF** box; a page driven by a macro
page draws a **MACRO** box.

### Fold type (FOLD, encoder D)

Continuous **0–127** spread over four modes (interpolated, capped at CLEAN/TRSH):
**CLEAN** (36 % fold) · **MUD** (full fold) · **DIST** (36 % fold + overdrive) ·
**TRSH** (full + overdrive). The overdrive is a stateless saturation, applied
only in DIST/TRSH.

### PITCH modes (PITCH, encoder D)

**WOB** — vibrato: a modulated, interpolated short delay. **SHFT** — pitch shift
(±12 st) from two crossfaded grains. **E** speed, **F** depth (detune / semitones),
**G** mix (dry/wet).

### STEREO modes (STEREO, encoder D)

**FLAN** — flanger (modulated delay + feedback; feedback capped at 75 % to avoid
self-oscillation). **PAN** — autopan. **WIDE** — stereo width (64 = bypass).
**E** speed, **F** depth, **G** mix (flanger), **H** feedback (flanger).

### CRUSH modes (CRUSH, encoder D)

**BITS** — bit-reduce to 24…4 bits, optional TPDF dither (**F** tune). **REDUX**
— sample-and-hold decimator. **CRUNCH** — Q8 gain into a hard clip.

### DELAYS modes (DELAYS, encoder D)

One mono delay line, two modes (the wet is mono, mixed into both channels — the
dry stays stereo). **D** switches the mode; **A** is on/off; **B, C, E, F, G, H**
edit the active mode's six parameters; the page shows a fixed **MIX** slider and
a secondary slider that appears while you move any of the other five.

- **WRMZ** — a modulated delay: **TIM** (time), **FB** (feedback, capped at
  0.80), **TON** (damping), **MOD** (depth), **RAT** (0.1–3.0 Hz wow/flutter),
  **MIX**.
- **GRAINZ** — a granular **cloud**: three drifting, interpolated taps off the
  same line — **DEN** (drift), **SIZ** (tap spacing), **PIT** (parked), **SPR**
  (spread), **FB**, **MIX**.

### MIDI CC control

All FX parameters respond to **incoming MIDI CC** on numbers the Digitone does
not use; a CC move is saved with the pattern. On/off switch at value **64**;
EQ LOW/HIGH are 0–127 with **64 = flat**. CCs on **any** MIDI channel drive the
master FX.

| CC | parameter | CC | parameter |
|---|---|---|---|
| `8` | RING depth | `40` | FOLD amount |
| `11` | RING on/off | `41` | FOLD type |
| `36` | RING rate | `67` | EQ on/off |
| `37` | FOLD on/off | `68` | meter on/off |
| `69` | EQ low shelf | `96` | EQ high shelf |
| `20` | CRUSH on/off | `21` | CRUSH mode |
| `22` | CRUSH amount | `23` | CRUSH tune |
| `24` | WAVE on/off | `25` | PITCH mode |
| `26` | WAVE speed | `27` | WAVE depth |
| `28` | WAVE mix | `29` | WAVE feedback |
| `30` | STEREO mode | `31` | WAVE group |
| `80` | DELAYS on/off | `81` | DELAYS mode (WRMZ/GRAINZ) |
| `82` | DELAYS 1 (TIM/DEN) | `83` | DELAYS 2 (FB/SIZ) |
| `84` | DELAYS 3 (TON/PIT) | `85` | DELAYS 4 (MOD/SPR) |
| `86` | DELAYS 5 (RAT/FB) | `87` | DELAYS 6 (MIX) |

Signal order on the master mix is **ring → EQ → crush → fold → wave → WRMZ →
ceiling → meter**. Ring, EQ and fold are on by default.

## Performance controls

A **static** touch page — **PERFORM** — in the **macro tree**. Press **UP** (or
**DOWN**) from any FX page to open the macro tree (it lands on **PERFORM**), then
**LEFT / RIGHT** to reach the macros; **UP** returns to the FX page you left. It
draws no animation, so it adds no CPU while it is open.

The **eight encoders set one depth per FX engine** (0–127, one step per notch,
saved with the pattern like every other DIGI control). Every engine has its own
depth — fold and crush are separate, and the WAVE engine has an independent
pitch depth and stereo depth:

| encoder | engine depth | encoder | engine depth |
|---|---|---|---|
| **A** | RING depth | **E** | WAVE stereo depth |
| **B** | FOLD amount | **F** | EQ low shelf |
| **C** | CRUSH amount | **G** | EQ high shelf |
| **D** | WAVE pitch depth | **H** | WAVE speed |

**The 16 trig keys are momentary performance pads**, one block per engine. A
block of four pads is four increasing intensities (1–4); the two-pad fold and
crush blocks use intensities **2 and 4**:

| trigs | block | what the pad drives |
|---|---|---|
| **1 2 9 10** | **RING** | ring depth |
| **3 4** | **FOLD** | fold amount |
| **11 12** | **CRUSH** | crush amount |
| **5 6 13 14** | **PITCH** | the WAVE pitch depth |
| **7 8 15 16** | **STEREO** | the WAVE stereo depth |

**Hold a pad:** it saves the engine's resting value (and its on/off), turns the
engine on, and **ramps its depth up** to `resting + (127 - resting) * intensity
/ 4`, where it holds. **Release:** the depth ramps back to the pattern value and
the on/off is restored. A pad **never writes the per-pattern store**, so the
saved value is always the resting position — releasing, leaving the page, or
even saving the project mid-pad leaves your values intact. The ramp is one cheap
step per audio block, so it is smooth and click-free and costs almost nothing
while idle. Because each engine has its own depth, the pads never fight: a pitch
pad cannot move the stereo depth, and a fold pad cannot move crush.

## Macro pages

Four **one-knob macro** pages bundle several engines behind a single amount
control. Each shows one vertical slider on the right (**A** = on/off, **B–H** =
amount) and a per-page animated graphic on the left; the macro drives the
engines while it is on and restores them when it turns off.

| page | graphic | what it drives |
|---|---|---|
| **WASH** | drifting dots (Haze) | WAVE wobble (75% depth / 30% rate at full) + WRMZ delay mix/feedback |
| **RADIO** | big ring + cone | EQ tilt (low to 0, high to 127 at full) + TRSH fold drive + SRR |
| **DESTROY** | stair-stepped sine | fold + bit-reduction + ring modulator |
| **SPACE** | splitting centre line | WAVE stereo width |

## CPU footprint

Everything runs on the Digitone's **main CPU (Core A)**; the FM DSP (Core B) is
untouched. Approximate instructions per active stereo frame (32 frames/block,
1500 blocks/s); a bypassed section costs ~0. Cheapest → heaviest:

| # | block / mode | ≈ insn/frame |
|---|---|---|
| 1 | CRUSH — BITS | ~10 |
| 2 | CRUSH — REDUX | ~12 |
| 3 | meter | ~15 |
| 4 | WAVE — WIDE | ~18 |
| 5 | FOLD — CLEAN/MUD | ~20 |
| 6 | WAVE — PAN | ~24 |
| 7 | CRUSH — CRUNCH | ~25 |
| 8 | RING | ~28 |
| 9 | EQ | ~30 |
| 10 | FOLD — DIST/TRSH | ~35 |
| 11 | WAVE — WOB | ~45 |
| 12 | WAVE — FLAN | ~55 |
| 13 | WAVE — SHFT | ~75 |

Turn off the pages you are not using; each true-bypasses. Full ledger in
[`docs/OPTIMIZATION.md`](docs/OPTIMIZATION.md).

**4.0a CPU pass** (measured; nothing audible changes): the eMAC mode switch is
hoisted to **once per block** instead of once per handler; the wave delay lines
are **s16** (half their DDR traffic and RAM); the wave **mode-change flush spike
is removed** (each mode's worst case now equals its p99); and animated pages
redraw at **~5 Hz**. The build is size-optimised (`-Os`): `.run 34,398 → 26,018`
bytes, and it still `imports nothing`. The new **DELAYS** module adds one mono
delay line (the package's RAM is 47,246 bytes, mostly that line). On hardware an
**eMAC-off** A/B idled lower but ran **worse while processing**, so the eMAC is
kept. Ledger: `docs/OPTIMIZATION-ROUND.md` §10–§12 (source repo).

**Output ceiling:** a final soft limiter keeps the master mix below the 24-bit
full scale (transparent below ~-2.5 dBFS, asymptotic max ~-0.7 dBFS), so a hot
flanger-feedback / pitch-shift / drive setting cannot hard-clip.

**3.5p performance pass** (measured; nothing audible changes): page graphics draw
straight to the framebuffer instead of one firmware call per pixel (36 call sites
→ 0); the output meter only scans while its page is up; block-constant branches in
CRUSH/FOLD are hoisted; the per-pattern store is gated off the UI tick; and the
WAVE delay line is shorter. Net: the suite's RAM state dropped to
**19,446 bytes** (−6.1 KB) and it still `imports nothing`. Full ledger in the
source repo's `docs/OPTIMIZATION.md`.

### Load — yours to manage

The Digitone's **CPU and DSP are shared** between the internal FM voices and
these added effects, so Tone+FX is a budget you spend: it is happiest when you
leave headroom for the synth itself. A few habits keep it clean on a busy unit:

- **Turn off the filter on tracks you are not using** — set the track's
  **filter type to 0 (off)**. An idle track's filter still costs CPU for its
  voices every block; freeing it buys that time back for the master FX.
- **A busy pattern has less CPU headroom.** More voices, notes and parameter
  locks on the FM engine leave less for the master FX. On busy patterns, back
  off RING depth / FOLD amount / CRUSH / WAVE, and turn off the pages you are
  not using (each true-bypasses to ~0 cost).
- **The animated panels are for setting up, not for staying on.** The RING /
  FOLD / TILT / CRUSH / PITCH / STEREO pages redraw every UI frame, and that
  redraw is CPU while the page is on screen. Dial the effect in, then **move on**
  — press **UP** to the static **PERFORM** page, or go to any internal page or
  panel. Once you leave a DIGI page it stops redrawing, and that CPU comes back.

## Releases

- `elemods/` at the repository root is the **current release** (this is what
  [`SHA256SUMS`](SHA256SUMS) covers).
- [`releases/`](releases/) holds a **packaged copy of each cut release**
  (`README`, `CHANGELOG`, `LICENSE`, `SHA256SUMS`, `elemods/`, `tools/`,
  `Screenshots/` and the `.zip`), newest first.
- [`archive/`](archive/) holds **superseded releases** (e.g. `2.1h`).

## Verify a download

```sh
sha256sum -c SHA256SUMS        # or: Get-FileHash elemods/* -Algorithm SHA256
```

## Status

Tested in the **digiemu** emulator (boot/settle, `dsp_running=2`, all twelve
pages incl. **DELAYS**; a live diag of WRMZ + GRAINZ with real SSI renders —
**0 faults**; `_probe_mix` 0 clips; the PERFORM page's live-audio check). The
**OS 1.44** port builds and patch-checks clean (digiemu cannot settle 1.44). The
4.0a CPU pass was A/B'd on hardware — the eMAC was kept (it is slower idle but
faster while processing) and the wave mode-change spike is gone. Current package
version is in [`CHANGELOG.md`](CHANGELOG.md). No firmware is included or
distributed here; the `.elemod`s carry only our own code and patch sites
(byte-checked against your stock file at build time).

## Known bugs / future fixes

Tracked as they come up; nothing here blocks normal use.

**Bugs / rough edges we want fixed**

- ~~**PAGE still jumps some FX levels:** pressing the **PAGE** key could push
  some DIGI parameters to a high value — the stock-event store guard did not
  cover every path.~~ **Fixed** — the per-pattern store is now a mod-owned RAM
  table; no stock event can reach a DIGI parameter.
- **PITCH fires on a note-off:** a **PHASER** trig can activate the PITCH (WAVE)
  effect on the **note-off** message instead of on the note.
- ~~**Microtiming / locked trigs on MIDI tracks:** LEFT / RIGHT would not nudge a
  MIDI-track trig and the volume could jump.~~ **Fixed in 3.5p** — the
  key/encoder handlers now disarm the moment the master view closes.
- **Slider feel:** the page sliders and value bars should track the stock
  parameter widgets more closely.
- **Encoder acceleration:** the base rate is stock (one step per detent); the
  fast-turn curve is whatever the OS driver delivers, so a fast spin can feel
  different from a stock parameter.
- **SHFT quality:** the pitch shifter is deliberately lightweight (linear
  interpolation, ~10 ms grains) and a little warbly — a quality pass, or a
  hi-fi option, is wanted.
- **DELAYS GRAINZ PIT is parked:** the granular taps read at rate 1.0, so the
  **PIT** knob does nothing yet (a planned tap-rate / transpose control).
- ~~**WOB / SHFT have no dry-wet:** MIX is used by FLAN only.~~ **Fixed in
  3.5r** — PITCH gains a **G mix** (dry/wet) so WOB/SHFT dial in like the
  flanger.
- **PERFORM trig LEDs:** the pads do not yet light the trig keys (yellow) — a
  planned follow-up, once the Digitone LED driver is reverse-engineered.
- ~~**Configured-MIDI-track store risk:** the per-pattern block lived in an unused
  MIDI-track area that was zero on *unconfigured* tracks; a configured MIDI track
  wrote its channel/page there, so the stock data and our values corrupted each
  other.~~ **Fixed** — the store moved to a mod-owned RAM table (digictl 2.8);
  the MIDI-track pattern bytes are no longer read or written.
- **Pattern persistence is RAM-only:** the FX settings follow a pattern switch in
  a session but are not carried over a power cycle or a project load — the price
  of never touching stock pattern data.

**Wanted next (features / quality)**

- **LFO destinations:** drive the DIGI parameters from a track LFO (a bridge is
  parked in the source; it needs the stock destination table).
- **Pre-FX routing:** an option to place the chain before the sends/return
  effects instead of after them.
- **Fuse the six master passes** into one loop — order-of-magnitude fewer
  loads/stores per frame, real CPU back. (Tried in 3.5r as a
  `CORE_AUDIO_INSERT`; reverted — it raised CPU/DSP ~15 % on hardware.)
- **Redraw gating:** only redraw an animated page while it is visible, and fold
  the meter into the fused loop.
- **More PERFORM pads:** pads for EQ, WAVE speed and CRUSH tune, and optionally
  tempo-synced ramp shapes.
- **Measured headroom:** render-worst / deadline margin and peak/RMS dB with the
  FX off vs on, so the load advice is measured rather than estimated.
- **CC 20–31:** verify against the stock CC table (`0x4018D104`) before
  sequencing WAVE/CRUSH from a DAW.
- **Output ceiling:** confirm it is inaudible at normal levels and matched to the
  24-bit full scale.
