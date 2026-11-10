# Tone+FX 4.1a — core-dn1 4.1 compatibility

A compatibility release: the Digitone files move to elekloader
**core-dn1 4.1**, and the Digitakt files stay on the Digitakt core 4.0. There is
**no audio or UI change** versus 4.0c. **Four OS files, two hosts:** Digitone
mk1 / Keys (OS 1.43, 1.44) and, through **gdeo607**'s port, the Digitakt mk1
(OS 1.53, 1.54).

## Install

Each file is the whole suite for one OS; none includes the core.

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-4.1a.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.1.elemod` |
| `tonefx-4.1a-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.1-os1.44.elemod` |
| `tonefx-4.1a-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-4.1a-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

Install the matching core (elekloader provides it), tick the core and `tonefx`,
set the OS version tag and **BUILD FIRMWARE**; flash with Transfer. Or use
`tools/build.ps1` / `tools/build.sh`. No firmware is included.

## New in 4.1a

- **Digitone: moved to `core-dn1` 4.1.** 4.1 adds the `ev_note_queue` event
  (each queued note event with the time it plays). Tone+FX does not subscribe to
  it, so behaviour is unchanged; the files now link as `core 4.1 > tonefx 4.1a`.
  The mod still asks for `"core": "4.0"`, so it also runs with core-dn1 4.0.
- **Digitakt: no change.** The latest Digitakt core line is still 4.0, so the DT
  files link as `core 4.0 > tonefx 4.1a`.
- The permanent PITCH -> DELAYS -> STEREO chain, the WASH CPU pass, the
  per-pattern store (`"DGX4"`, CH 16) and the UI are identical to 4.0c.

## Upgrading

- **Digitone:** replace `core-dn1-4.0*` with `core-dn1-4.1*` and the 4.0c suite
  with the matching 4.1a file. The per-pattern store is unchanged (MIDI track 4
  at `pattern+0x1BB0`, `"DGX4"`, CH 16).
- **Digitakt:** the mod-owned RAM table is unchanged.

## Verified

- Build/lint/patch: all four `imports nothing`; Digitone links as
  `core 4.1 > tonefx 4.1a`, Digitakt as `core 4.0 > tonefx 4.1a`; patch is
  byte-exact against each stock file.
- Footprints are identical to 4.0c: Digitone `.run` 25,594 / `.bss` 47,182;
  Digitakt `.run` 23,096 / `.bss` 51,232.

See `docs/PATTERN-STORAGE.md` and `CHANGELOG.md` in the repository.

Shop summary: A master effect suite for the Digitone mk1/Keys and Digitakt mk1: ring, fold, tilt EQ, crush, pitch/stereo, delays, macros and performance pads.
Tested on a unit: As of 4.1a: emulator (digiemu) verified; the audio path is unchanged from 4.0c, which ran on hardware.
