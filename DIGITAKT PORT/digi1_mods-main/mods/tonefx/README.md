# Tone+FX for the DT1 — 4.3b lite

**Tone+FX 4.3b** by Digital Alchemy DSP, ported from its original FM-groovebox
version to the DT1 (OS 1.54). The source is theirs
([DigiAlchemydsp/RingTone](https://github.com/DigiAlchemydsp/RingTone); the
release repo is [DigiAlchemydsp/Tone-FX](https://github.com/DigiAlchemydsp/Tone-FX)),
under GPL-2.0-or-later, so this port is GPL-2.0-or-later too (`LICENSE`).

Unofficial. Flash at your own risk and keep the official OS file for recovery.

## What it does

Master effects on every output (main, headphones and USB audio), in this
permanent order: **ring → TILT (low/high shelf) → CRUSH → FOLD → PITCH → WRMZ
(DELAYS) → STEREO → output ceiling → meter**. The WAVE insert is split into its
PITCH and STEREO passes so the delay always sits between them.

**Main scroll** (`FUNC + LFO`, then `LEFT/RIGHT`): stock master pages, then
`FOLD → CRUSH → RING → STEREO → PITCH → TILT → DELAYS`.

**PERFORM tree** (`UP`, then `LEFT/RIGHT`): `PERFORM → STUTTER`; `UP` returns
to the FX page you left.

| page | knobs | |
|---|---|---|
| **DIGI RING** | A on/off · E depth · F frequency | ring modulator / tremolo |
| **DIGI FOLD** | A on/off · D fold type (CLEAN/MUD/DIST/TRSH) · E-H amount | wavefolder, with the L/R meter |
| **DIGI TILT** | A on/off · E low shelf · H high shelf (64 = flat) | |
| **DIGI CRUSH** | A on/off · D mode (REDUX/CRUNCH) · E amount · F tune | |
| **DIGI PITCH** | A on/off · D mode (WOB/SHFT) · E speed · F depth · G mix | |
| **DIGI STEREO** | A on/off · D mode (FLAN/PAN/WIDE) · E speed · F depth · G mix · H feedback | |
| **DELAYS** | A on/off · B/C/E/F/G/H = TIM, FB, TON, MOD, RAT, MIX | WRMZ only |
| **PERFORM** | A-H = one depth per engine | static; 16 momentary trig pads |
| **STUTTER** | 16 trig pads | TOEJAM: freeze/loop the finished master mix |

### TOEJAM / STUTTER

Hold a trig to freeze and loop the newest slice of the finished master mix;
release to return live. The 16 trigs are momentary pads in four zones:

| trigs | zone | pads |
|---|---|---|
| 1–4 | straight | 1/32, 1/16, 1/8, 1/4 |
| 5–8 | dotted | 1/16., 1/8., 1/4., 1/2. |
| 9–12 | triplet | 1/16T, 1/8T, 1/4T, 1/2T |
| 13–16 | long | 1/2, 1/1, 2/1, 4/1 |

The DT ring is mono, ~0.68 s. Lengths follow the Digitakt tempo (`fw_tempo`).

### Removed in 4.3b

- macro pages `WASH`, `RADIO`, `DESTROY`, `SPACE`;
- `BITS` crush mode;
- `GRAINZ` delay mode.

Their parameters, CCs, UI and store fields are gone. The per-pattern store is
`DGX4` **v2** with 32 params; old v1 blocks are ignored and reclaimed.

## What the port changed

- **Sound path.** The effects process the master mix at the render stage that
  spreads it over the outputs (0x400721e6, Digi EQ's site), so the main outputs,
  headphones and USB audio all carry them. `master.c` copies the block
  (0x8000ea70, 8 bits hotter than the outputs) as 24-bit samples and puts back
  only what changed; with every effect off the master is bit-exact.
- **The pages.** The DT1 master view shares one view class with track pages.
  The suite appends nine **kind-0** entries in a row after the stock master
  pages and tells them apart by place. Navigation is index-based. The title
  getter names each page.
- **Keys.** DT1 ids: UP 14, DOWN 15, LEFT 16, RIGHT 17, trigs 24..39, FUNC held
  = flags bit 1. The PERFORM/STUTTER pads ignore a trig while a modifier
  (FUNC, TRK, PTN, BANK, SONG, GLOBAL, SAMPLE, TEMPO) is held.
- **Knobs.** `DC_PER_STEP = 1`: one step per knob notch.
- **MIDI CC.** Hooked at the DT1 CC entry (0x400d4a90; 0x400d4cb8 on 1.54).
  CRUSH and WAVE use 42..53 because 20..31 are the DT1's own SRC/auto-channel
  CCs.

| CC | | CC | |
|---|---|---|---|
| 8 | ring depth | 42 | CRUSH on/off |
| 11 | ring on/off | 43 | CRUSH mode |
| 36 | ring rate | 44 | CRUSH amount |
| 37 | FOLD on/off | 45 | CRUSH tune |
| 40 | FOLD amount | 46 | WAVE on/off |
| 41 | FOLD type | 47 | PITCH mode |
| 67 | EQ on/off | 48 | WAVE speed |
| 68 | meter on/off | 49 | WAVE depth |
| 69 | EQ low shelf | 50 | WAVE mix |
| 96 | EQ high shelf | 51 | WAVE feedback |
| | | 52 | STEREO mode |
| | | 53 | WAVE group |
| 80 | DELAYS on/off | 82–87 | WRMZ 1–6 |

- **Per-pattern settings.** 32 params ride **MIDI track 8's saved record**
  (`kit + 0x8ba`, `"DGX4"` v2, packed 1-7 bits each). Reserve MIDI track 8 on
  channel 16 and leave it alone. The eight DT1 MIDI records sit at
  `kit + 0x59a + t*0x70`; the store uses record 7, starting 0x10 bytes in and
  spanning +0x10..+0x51, inside the 41 saved words. A stock write is detected
  by the magic and reclaimed.
- **RAM.** Core 4.0.1 gives the mods 256 KB. 4.3b takes `.run` 20,352 and `.bss`
  104,300 (the mono ToeJam ring and the WRMZ line dominate).

## Using it with other mods

- **Digi EQ: not together.** Both take the master view's draw (vtable slot 4)
  and the render site 0x400721e6, and both use kind-0 pages.
- **Everything else: combines**, subject to the 256 KB mod RAM limit.
- **CPU.** The cost is per audio block of 0.67 ms. A very busy project with
  heavy settings could click; digihealth's FAST AUDIO gives the render room.

## Build and test

Use the **adapted Digitakt core** `core-4.0.1-os1.54.elemod` (the 4.0 core with
`fw_tempo`). Core 4.0 lacks the symbol; core 4.3/5.0 claim Tone+FX's master
sites. One source builds only for 1.54 in this release:

```sh
python -m elekloader.sdk.build mods/tonefx --stock Digitakt_OS1.54.syx
python -m elekloader.patch --stock Digitakt_OS1.54.syx \
  --mod core-4.0.1-os1.54.elemod --mod out/tonefx-4.3b-os1.54.elemod \
  --out ToneFX-DT.syx --version 4.3b
```

`--version` must be exactly 4 ASCII chars.

- `tests/emu_tonefx.py` covers the CC entry, the chain and the registers kept.
- `tests/digiemu_tonefx.py` covers pages, knobs, PERFORM, output level, USB and
  the per-pattern store.
- 4.3b build/lint/patch pass. Hardware/emulator testing is pending.

## Files

| | |
|---|---|
| `meter.c` `eq.c` `ring.c` `fold.c` `crush.c` `wave.c` | the effects (Digital Alchemy DSP's) |
| `master.c` `master_glue.s` | the chain on the master mix |
| `digictl.c` | pages, knobs, keys, PERFORM/STUTTER, store and CC map |
| `cc_glue.s` | the DT1 CC entry |
| `page_glue.s` | the title getter and stock page change |
| `../digitojam/toejam.c` | TOEJAM ring and STUTTER page |
| `render.c` `splash.s` | boot splash, not built |
| `corea.h` `sin256.h` | render helpers and sine table |

---

# Releasing Tone+FX — cross-platform

Tone+FX 4.3b ships for the latest OS of each host:

| device | OS | source tree | core |
|---|---|---|---|
| Digitone mk1 / Keys | 1.44 | `digitone_corea` | `core-dn1-4.1-os1.44.elemod` |
| Digitakt mk1 | 1.54 | this repo (`digi1_mods`) | `core-4.0.1-os1.54.elemod` |

The effect sources are shared where possible; each tree carries its device's
render path, UI model, keys, CC map and per-pattern store.

## Build

```sh
export ELEKLOADER_CROSS=m68k-elf-
export PYTHONPATH=<elekloader checkout>

python -m elekloader.sdk.build mods/tonefx --stock Digitakt_OS1.54.syx
```

Digitone: see `digitone_corea/mods/tonefx/README.md`.

## Verify

For each build: `elekloader.lint` with the matching core, then `elekloader.patch`
byte-exact. Update `elemods/`, `SHA256SUMS`, `README.md`, `CHANGELOG.md`,
`HANDOFF.md`, `releases/<ver>/` and the release zip.

Keep the shared effect code in step between trees (3-way merge: ours = this
port, base = the Digitone 3.2a fork point, theirs = the new Digitone). The
render plumbing, page model and store are the DT1's own.
