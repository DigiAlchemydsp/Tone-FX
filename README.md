# Tone+FX

**A master effect suite for the Elektron Digitone mk1 / Digitone Keys (OS 1.43).**

Tone+FX is a set of [elekloader](https://github.com/irpina/elekloader) mods,
**packaged and released together as one suite**. It adds master ring modulation,
a low/high-shelf EQ, a Crunch/Redux lo-fi stage, a wavefolder with four fold
modes (plus an internal overdrive), a modulation insert with five modes (wobble,
pitch shift, flanger, autopan, stereo width) on two pages, output metering, a
**six-page UI** on the master page tree and an animated TONE+FX boot splash.

This repository is **release-only**: it ships the built `.elemod` packages and
the docs to install them. It contains **no source and no firmware**. The source
lives in the development repo, so this repo stays a safe place to cut releases
from while the older `RingTone` repo is still around.

> Unofficial and unsupported. Not affiliated with, endorsed by or supported by
> Elektron. Flashing modified firmware is at your own risk.

## Pages

Six master pages (**FUNC + LFO**; **LEFT / RIGHT** rotates). Each new page has a
frameless animated graphic in the leftmost third that collapses to a line when
the effect is at neutral.

| RING | FOLD | TILT |
|---|---|---|
| ![DIGI RING page](Screenshots/RING.png) | ![DIGI FOLD page](Screenshots/FOLD.png) | ![DIGI TILT page](Screenshots/TILT.png) |

| CRUSH | PITCH | STEREO |
|---|---|---|
| ![DIGI CRUSH page](Screenshots/CRUSH.png) | ![DIGI PITCH page](Screenshots/PITCH.png) | ![DIGI STEREO page](Screenshots/STEREO.png) |

## The suite

**One file is the whole suite:** [`elemods/tonefx-3.1h.elemod`](elemods/) bundles
every mod below into a single elekloader mod, so it **combines with your other
mods**. It does **not** include the required core — install `core-dn1-2.0a.elemod`
too (elekloader provides it), plus anything else you use.

- **digimeter** — output level metering (drawn on the DIGI FOLD page).
- **digieq** — master low-shelf / high-shelf EQ (LO + HI knobs).
- **digiring** — master ring modulator / tremolo, with an animated page.
- **digicrush** — Crunch / Redux: bit-reduce (BITS), sample-rate hold (SR) or a
  hard-clip drive (CRUNCH), one stateless stage at a time.
- **digifold** — master wavefolder with CLEAN / MUD / DIST / TRSH fold modes and
  an internal overdrive; a spiral display.
- **digiwave** — one modulation engine, **two pages**: **PITCH** (WOB pitch
  wobble, SHFT pitch shift) and **STEREO** (FLAN flanger, PAN autopan, WIDE
  stereo width).
- **digictl** — the pages and all the controls; keeps the settings per pattern
  and adds MIDI CC control.
- **tonesplash** — an animated TONE+FX boot splash (one of three animations per
  boot, drawn at the intro).

Everything is `GPL-2.0-or-later` (see [`LICENSE`](LICENSE)).

## Install

1. Get **elekloader** and your own stock
   `Digitone_and_Digitone_Keys_OS1.43.syx` (Elektron's download). elekloader
   builds a custom OS on your machine; it never distributes firmware.
2. In elekloader's window, add the stock `.syx`, then **+ Install from file…**
   and pick `core-dn1-2.0a.elemod` (the required core; elekloader ships it) and
   `tonefx-3.1h.elemod` (the whole suite). Add any of your other mods too.
3. Tick the core and `tonefx`, set the OS version tag, and **BUILD FIRMWARE**.
4. Flash the resulting `.syx` with Elektron Transfer, as for any OS update;
   press **YES** on the unit and do not power off until it finishes.

Or from the command line (elekloader from source):

```powershell
# Windows PowerShell
./tools/build.ps1 -Stock "Digitone_and_Digitone_Keys_OS1.43.syx" -Out ToneFX.syx -Version 3.1h
```

```sh
# POSIX
sh tools/build.sh Digitone_and_Digitone_Keys_OS1.43.syx ToneFX.syx 3.1h
```

**Recovery** (the bootloader is never changed): hold **FUNC** while powering on
for the startup menu, press **TRIG 4 (OS UPGRADE)**, then send the stock `.syx`
with Transfer's legacy OS upgrade mode.

## Use

The pages live on the **master page tree**: **FUNC + LFO** cycles the master
pages. Tone+FX appends six of its own after the stock ones. **LEFT / RIGHT** (or
the on-screen `<` `>` arrows) rotate the whole master page list. The stock **top
status bar** (pattern name, tempo, …) stays visible above every page. PAGE keeps
its stock meaning; no stock key or encoder is taken.

| page | encoders | shows |
|---|---|---|
| **RING** | **A** on/off · **E** depth · **F** frequency | a centred ring animation |
| **FOLD** | **A** on/off · **D** fold type · **E/F/G/H** amount | the spiral, the big mode title, the amount slider + L/R meter |
| **TILT** | **A** on/off · **E** low shelf · **H** high shelf | a bent low/high response curve |
| **CRUSH** | **A** on/off · **D** mode · **E** amount · **F** tune | a waveform (left) that is quantised / held / clipped by the mode |
| **PITCH** | **A** on/off · **D** mode · **E** speed · **F** depth | two vertical lines that wobble and intertwine |
| **STEREO** | **A** on/off · **D** mode · **E** speed · **F** depth · **G** mix · **H** feedback | a horizontal line that ripples / splits / spreads |

Every parameter is **0–127** and moves **one step per notch** — the stock
convention (EQ LOW/HIGH are 0–127 with **64 = flat**).

**Per-pattern settings** (store `"DGX1"` v7): the settings follow a pattern
switch / reload and travel with a project save, like stock parameters. Each page
**true-bypasses** when its **A** is off (fades out, then stops using CPU).

### Fold type (FOLD, encoder D)

Continuous **0–127** spread over four modes (interpolated, capped at CLEAN/TRSH):
**CLEAN** (36 % fold) · **MUD** (full fold) · **DIST** (36 % fold + overdrive) ·
**TRSH** (full + overdrive). The overdrive is a stateless saturation, applied
only in DIST/TRSH.

### PITCH modes (PITCH, encoder D)

**WOB** — vibrato: a modulated, interpolated short delay. **SHFT** — pitch shift
(±12 st) from two crossfaded grains. **E** speed, **F** depth (detune / semitones).

### STEREO modes (STEREO, encoder D)

**FLAN** — flanger (modulated delay + feedback; feedback capped at 75 % to avoid
self-oscillation). **PAN** — autopan. **WIDE** — stereo width (64 = bypass).
**E** speed, **F** depth, **G** mix (flanger), **H** feedback (flanger).

### CRUSH modes (CRUSH, encoder D)

**BITS** — bit-reduce to 24…4 bits, optional TPDF dither (**F** tune). **SR** —
sample-and-hold decimator. **CRUNCH** — Q8 gain into a hard clip.

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

Signal order on the master mix is **ring → EQ → crush → fold → wave → meter**.
Ring, EQ and fold are on by default.

## CPU footprint

Everything runs on the Digitone's **main CPU (Core A)**; the FM DSP (Core B) is
untouched. Approximate instructions per active stereo frame (32 frames/block,
1500 blocks/s); a bypassed section costs ~0. Cheapest → heaviest:

| # | block / mode | ≈ insn/frame |
|---|---|---|
| 1 | CRUSH — BITS | ~10 |
| 2 | CRUSH — SR | ~12 |
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

### Load — yours to manage

The Digitone's **CPU and DSP are shared** between the internal FM voices and
these added effects. Running everything at once **can stress the CPU/DSP** and
cause dropouts on busy patterns. Manage it: turn off what you are not using and
back off RING depth / FOLD amount / CRUSH / WAVE on busy patterns.

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

Tested in the **digiemu** emulator (boot/settle, `dsp_running=2`, all six pages)
and on hardware (a Digitone mk1). Current package version is in
[`CHANGELOG.md`](CHANGELOG.md). No firmware is included or distributed here; the
`.elemod`s carry only our own code and patch sites (byte-checked against your
stock file at build time).

## Known bugs / future fixes

- **Slider behaviour:** the page sliders/bars should track the stock parameter
  widgets more closely.
- **Encoder acceleration:** the base rate matches stock; the fast-turn curve is
  whatever the OS driver delivers.
- **LFO destinations:** driving the DIGI parameters from a track LFO is parked.
- **SHFT quality:** the pitch shifter is deliberately lightweight (linear
  interpolation, ~10 ms grains) — lo-fi.
- **MIX** is used by FLAN only; WOB and SHFT are wet-only.
- **CC 20–31** are not yet checked against the stock CC table.
- **CPU/DSP load** is the user's to manage; a formal headroom measurement is
  pending.
