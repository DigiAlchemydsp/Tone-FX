# Tone+FX

**A master effect suite for the Elektron Digitone mk1 / Digitone Keys (OS 1.43).**

Tone+FX is a set of [elekloader](https://github.com/irpina/elekloader) mods,
**packaged and released together as one suite**. It adds master ring modulation,
a low/high-shelf EQ, a Crunch/Redux lo-fi stage, a wavefolder with four fold
modes (plus an internal overdrive), a five-mode modulation insert (wobble /
pitch shift / flanger / autopan / stereo width), output metering, a five-page UI
on the master page tree and an animated TONE+FX boot splash.

This repository is **release-only**: it ships the built `.elemod` packages and
the docs to install them. It contains **no source and no firmware**. The source
lives in the development repo, so this repo stays a safe place to cut releases
from while the older `RingTone` repo is still around.

> Unofficial and unsupported. Not affiliated with, endorsed by or supported by
> Elektron. Flashing modified firmware is at your own risk.

## Screenshots

![DIGI RING page](Screenshots/RING.png)
![DIGI FOLD page](Screenshots/FOLD.png)
![DIGI TILT page](Screenshots/TILT.png)

## The suite

**One file is the whole suite:** [`elemods/tonefx-3.1c.elemod`](elemods/) bundles
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
- **digiwave** — one modulation insert, five modes: **WOB** (pitch wobble /
  vibrato), **SHFT** (pitch shift), **FLAN** (flanger), **PAN** (autopan),
  **WIDE** (stereo width). One page, shared SPEED/DEPTH controls.
- **digictl** — the DIGI pages and all the controls; keeps the settings per
  pattern and adds MIDI CC control.
- **tonesplash** — an animated TONE+FX boot splash (one of three animations per
  boot, drawn at the intro).

Everything is `GPL-2.0-or-later` (see [`LICENSE`](LICENSE)).

## Install

1. Get **elekloader** and your own stock
   `Digitone_and_Digitone_Keys_OS1.43.syx` (Elektron's download). elekloader
   builds a custom OS on your machine; it never distributes firmware.
2. In elekloader's window, add the stock `.syx`, then **+ Install from file…**
   and pick `core-dn1-2.0a.elemod` (the required core; elekloader ships it) and
   `tonefx-3.1c.elemod` (the whole suite). Add any of your other mods too.
3. Tick the core and `tonefx`, set the OS version tag, and **BUILD FIRMWARE**.
4. Flash the resulting `.syx` with Elektron Transfer, as for any OS update;
   press **YES** on the unit and do not power off until it finishes.

Or from the command line (elekloader from source):

```powershell
# Windows PowerShell
./tools/build.ps1 -Stock "Digitone_and_Digitone_Keys_OS1.43.syx" -Out ToneFX.syx -Version 3.1c
```

```sh
# POSIX
sh tools/build.sh Digitone_and_Digitone_Keys_OS1.43.syx ToneFX.syx 3.1c
```

**Recovery** (the bootloader is never changed): hold **FUNC** while powering on
for the startup menu, press **TRIG 4 (OS UPGRADE)**, then send the stock `.syx`
with Transfer's legacy OS upgrade mode.

## Use

The pages live on the **master page tree**: **FUNC + LFO** cycles the master
pages. Tone+FX appends five of its own after the stock ones. **LEFT / RIGHT**
(or the on-screen `<` `>` arrows) rotate the whole master page list. The stock
**top status bar** (pattern name, tempo, …) stays visible above every page.

| page | encoders | shows |
|---|---|---|
| **RING** | **A** on/off · **E** depth · **F** frequency | a centred ring animation, the depth/frequency bars on the right and the values bottom-left |
| **FOLD** | **A** on/off · **D** fold type · **E/F/G/H** amount | the spiral (left), the big mode title, the amount as a vertical slider next to the L/R meter |
| **TILT** | **A** on/off · **E** low shelf · **H** high shelf | a bent low/high response curve |
| **CRUSH** | **A** on/off · **D** mode · **E** amount · **F** tune | the big mode title (BITS / SR / CRUNCH) and the amount/tune bars |
| **WAVE** | **A** on/off · **D** mode · **E** speed · **F** depth · **G** mix · **H** feedback | the big mode title (WOB / SHFT / FLAN / PAN / WIDE) and the speed/depth/mix bars |

Every parameter is **0–127** and moves **one step per notch** — the stock
convention (EQ LOW/HIGH are 0–127 with **64 = flat**). PAGE and every stock key
keep their stock meaning.

**The settings are stored per pattern** (inside the saved pattern data,
`"DGX1"` v6): they follow a pattern switch / reload and travel with a project
save, like stock parameters.

### Fold type (FOLD, encoder D)

The wavefolder's *fold type* is a continuous **0–127** knob spread over four
modes; turning it sweeps smoothly between them (interpolated) and caps at CLEAN
(min) and TRSH (max):

| mode | fold range | overdrive |
|---|---|---|
| **CLEAN** | 36% | off |
| **MUD** | full (the original folder) | off |
| **DIST** | 36% | hard-clip overdrive |
| **TRSH** | full | hard-clip overdrive |

The overdrive is a cheap stateless saturation (a gain into a clamp), applied
only in DIST/TRSH, so CLEAN/MUD cost nothing extra per sample.

### WAVE modes (WAVE, encoder D)

One modulation insert over a single delay line. **SPEED** is the LFO rate,
**DEPTH** the amount; the big title changes with the mode:

| mode | engine | depth means |
|---|---|---|
| **WOB** | modulated, interpolated short delay (vibrato) | detune depth |
| **SHFT** | two crossfaded grains read at a ratio | semitones (±12) |
| **FLAN** | modulated delay + feedback + MIX | sweep depth |
| **PAN** | sine pan-law, no delay | pan depth |
| **WIDE** | mid/side, no LFO | width (64 = bypass) |

### CRUSH modes (CRUSH, encoder D)

| mode | stage |
|---|---|
| **BITS** | bit-reduce to 16…4 bits, optional TPDF dither (tune) |
| **SR** | sample-and-hold decimator (amount = divide, tune = extra) |
| **CRUNCH** | Q8 gain into a hard clip (drive) |

### MIDI CC control

All the FX parameters respond to **incoming MIDI CC** on a set of numbers the
Digitone does not use, so they can be played and sequenced from an external
controller/DAW. A **CC move is saved with the pattern**, like a page edit. Only
CCs the stock DN does not use are taken; every other CC falls through to the
stock handler untouched.

| CC | parameter | CC | parameter |
|---|---|---|---|
| `8` | RING depth | `41` | FOLD type |
| `11` | RING on/off | `67` | EQ on/off |
| `36` | RING rate | `68` | meter on/off |
| `37` | FOLD on/off | `69` | EQ low shelf |
| `40` | FOLD amount | `96` | EQ high shelf |
| `20` | CRUSH on/off | `25` | WAVE mode |
| `21` | CRUSH mode | `26` | WAVE speed |
| `22` | CRUSH amount | `27` | WAVE depth |
| `23` | CRUSH tune | `28` | WAVE mix |
| `24` | WAVE on/off | `29` | WAVE feedback |

On/off switches at value **64**; EQ LOW/HIGH are 0–127 with **64 = flat**; the
FOLD type is 0–127. Incoming CCs on **any** MIDI channel drive the master FX.
The method (the hooked stock CC router `0x400ED94E`, the `jmp` requirement) is in
the source repo's `docs/MIDI-CC.md`.

Signal order on the master mix is **ring → EQ → crush → fold → wave → meter**.
Ring, EQ and fold are on by default.

### Load — yours to manage

The Digitone's **CPU and DSP are shared** between the internal FM voices and
these added effects. Running everything at once **can stress the CPU/DSP** and
cause dropouts on busy patterns. Manage it yourself: turn off what you are not
using and back off ring depth / fold amount / crush / wave on busy patterns.

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

**Stress-tested on real hardware over 3 days: PASS** (a Digitone mk1) for the
3.0b ring/EQ/fold/meter suite. **3.1c** (crush + wave) compiles clean, links
byte-verified, boots and settles in the **digiemu** emulator (`dsp_running=2`),
and the first hardware pass is **working**; stress testing continues. Current
package versions are listed in [`CHANGELOG.md`](CHANGELOG.md). No firmware is
included or distributed here; the `.elemod`s carry only our own code and patch
sites (byte-checked against your stock file at build time).

## Known bugs / future fixes

- **Slider behaviour:** the page sliders/bars should track the stock parameter
  widgets more closely (shared smoothing, value readouts).
- **Encoder acceleration:** the base rate now matches stock (one step per
  detent); the fast-turn curve is whatever the OS driver delivers.
- **LFO destinations:** driving the DIGI parameters from a track LFO is
  **parked** — the bridge is in the code but off, with no page control.
- **SHFT quality:** the pitch shifter is deliberately lightweight (linear
  interpolation, ~10 ms grains) — lo-fi, with some wow.
- **MIX** is used by FLAN only; WOB and SHFT are wet-only.
- **CC 20–29** are not yet checked against the stock CC table.
- **CPU/DSP load** is the user's to manage (see *Load* above). CPU/DSP/dB
  headroom has not yet been formally measured.
