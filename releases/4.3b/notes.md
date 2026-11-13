# Tone+FX 4.3b — lite + TOEJAM

A footprint-reduced release for the **Digitone mk1 / Keys (OS 1.44)** and the
**Digitakt mk1 (OS 1.54)**. It adds the **TOEJAM STUTTER** page and removes the
large macro modules to free code and RAM.

## Files

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-4.3b-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.1-os1.44.elemod` |
| `tonefx-4.3b-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0.1-os1.54.elemod` |
| `core-4.0.1-os1.54.elemod` | Digitakt mk1 | 1.54 | — |

The Digitakt core is included because Tone+FX DT needs `fw_tempo`. Use the
provided 4.0.1 core; core 4.0 lacks the export, and core 4.3/5.0 claim
Tone+FX's master sites. The Digitone core is provided by elekloader.

## Install

1. Open elekloader with your own stock OS file.
2. Add the matching core, then the matching Tone+FX elemod.
3. Build the firmware and flash it with Transfer.

No firmware is included.

## TOEJAM

Hold a trig on the STUTTER page to freeze and loop the newest slice of the
finished master mix; release to return live. The 16 trigs are momentary pads:
straight, dotted, triplet and long zones, four pads each. Lengths follow the
machine tempo. Digitone ring: stereo, ~2.7 s. Digitakt ring: mono, ~0.68 s.

`FUNC + LFO` opens the master pages; **UP** opens the performance tree
(`PERFORM` → `STUTTER`).

## Removed in 4.3b

- Macro pages: `WASH`, `RADIO`, `DESTROY`, `SPACE`.
- Crush mode: `BITS` (REDUX and CRUNCH remain).
- Delay mode: `GRAINZ` (WRMZ remains).

The related parameters, CCs and UI are gone. The per-pattern store is `DGX4` v2
(32 params Digitakt, 35 Digitone); old v1 blocks are ignored and reclaimed.

## Verified

- Digitone 1.44: `.run` 22,514 / `.bss` 571,342; `core 4.1 > tonefx 4.3b`.
- Digitakt 1.54: `.run` 20,352 / `.bss` 104,300; `core 4.0.1 > tonefx 4.3b`.
- Patch is byte-exact against each stock file.
- Emulator/hardware testing pending.

The Digitakt port is based on **gdeo607**'s work.
