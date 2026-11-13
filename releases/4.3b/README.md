# Tone+FX 4.3b — lite

**A master effect suite for the Elektron Digitone mk1 / Digitone Keys (OS 1.44) and the Digitakt mk1 (OS 1.54), now with the TOEJAM stutter page.**

Tone+FX 4.3b is a set of [elekloader](https://github.com/irpina/elekloader) mods
packaged as one suite. It adds master ring modulation, a low/high-shelf EQ, a
Redux/Crunch lo-fi stage, a wavefolder, a five-mode modulation insert (PITCH and
STEREO), one WRMZ delay line, output metering, and a performance tree with the
PERFORM pads and the new TOEJAM STUTTER page.

> Unofficial and unsupported. Not affiliated with, endorsed by or supported by
> Elektron. Flashing modified firmware is at your own risk. No stock firmware is
> included.

## Download

| file | device | OS | required core |
|---|---|---|---|
| [`tonefx-4.3b-os1.44.elemod`](elemods/tonefx-4.3b-os1.44.elemod) | Digitone mk1 / Keys | 1.44 | `core-dn1-4.1-os1.44.elemod` |
| [`tonefx-4.3b-os1.54.elemod`](elemods/tonefx-4.3b-os1.54.elemod) | Digitakt mk1 | 1.54 | `core-4.0.1-os1.54.elemod` |
| `core-4.0.1-os1.54.elemod` | Digitakt mk1 | 1.54 | — (adapted core) |

The Digitakt core is included because Tone+FX DT needs the `fw_tempo` export.
The Digitone core is provided by elekloader (`core-dn1` 4.1 or newer).

## Install

1. Open elekloader and load your own stock OS file.
2. Add the matching **core** and then the matching **Tone+FX** elemod.
3. Build the firmware and send it with Transfer.

For the Digitakt, use **`core-4.0.1-os1.54.elemod`**, not core 4.0, 4.3 or 5.0.
The provided core links with Tone+FX DT and adds the tempo export; the other DT
cores either lack `fw_tempo` or claim Tone+FX's master sites.

## TOEJAM — STUTTER page

TOEJAM freezes and loops the finished master mix. Hold a trig on the STUTTER
page to loop the last slice of output; release it to return to live audio. The
16 trigs are momentary pads in four zones:

| trigs | zone | pads |
|---|---|---|
| 1–4 | straight | 1/32, 1/16, 1/8, 1/4 |
| 5–8 | dotted | 1/16., 1/8., 1/4., 1/2. |
| 9–12 | triplet | 1/16T, 1/8T, 1/4T, 1/2T |
| 13–16 | long | 1/2, 1/1, 2/1, 4/1 |

Loop lengths follow the machine tempo. The Digitone ring is stereo (~2.7 s);
the Digitakt ring is mono (~0.68 s, same length, half the RAM).

## Pages

**FUNC + LFO** opens the master pages. **LEFT/RIGHT** rotates them.

- **Main scroll:** stock master pages, then `FOLD`, `CRUSH`, `RING`, `STEREO`,
  `PITCH`, `TILT`, `DELAYS`.
- **UP** opens the performance tree: `PERFORM`, then `LEFT/RIGHT` to `STUTTER`.
  **UP** returns to the FX page you left.

CRUSH has two modes: `REDUX` and `CRUNCH`. DELAYS is the single WRMZ delay.
PERFORM is the eight-knob momentary trig-pad page.

## Removed in 4.3b

To reduce the footprint, Tone+FX 4.3b removes:

- the macro pages `WASH`, `RADIO`, `DESTROY` and `SPACE`;
- the `BITS` crush mode (REDUX and CRUNCH remain);
- the `GRAINZ` delay mode (WRMZ remains).

The related parameters and MIDI CCs are gone. The per-pattern store is now
`DGX4` v2 (32 params on the Digitakt, 35 on the Digitone); blocks written by
older versions are ignored and reclaimed, so settings return to defaults once.

## Verified

- Digitone 1.44: `.run` 22,514 / `.bss` 571,342; links as `core 4.1 > tonefx 4.3b`.
- Digitakt 1.54: `.run` 20,352 / `.bss` 104,300; links as `core 4.0.1 > tonefx 4.3b`.
- Patch is byte-exact against the stock files. Emulator/hardware testing is
  pending for this revision.

The Digitakt port is based on **gdeo607**'s work on the DT1 render path, page
model, keys and pattern store.

See [`CHANGELOG.md`](CHANGELOG.md) and [`notes.md`](notes.md).
