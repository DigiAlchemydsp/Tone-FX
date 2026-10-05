# Tone+FX 2.3a — MIDI CC control

The DIGI FX now respond to **incoming MIDI CC**, like the stock parameters, so
they can be played and sequenced from an external controller or DAW. A CC move
is stored with the pattern, like a page edit.

## Install

**One file is the whole suite:** `tonefx-2.3a.elemod` bundles `digictl` and every
FX mod into a single **format-2** mod, so it combines with your other mods. It
does **not** include the core: install `core-dn1-2.0a.elemod` (required;
elekloader provides it) plus anything else you use.

## Changes

- **digictl 1.7** hooks the stock central CC router (`0x400ED94E`). Only CC
  numbers the Digitone does not already use are taken:

  | CC | parameter | CC | parameter |
  |---|---|---|---|
  | 8 | RING depth | 69 | EQ LOW gain |
  | 11 | RING on/off | 96 | EQ HIGH gain |
  | 36 | RING rate | 97 | FILTER on/off |
  | 37 | FOLD on/off | 100 | FILTER mode |
  | 40 | FOLD amount | 101 | FILTER freq |
  | 67 | EQ LOW on/off | 103 | FILTER reso |
  | 68 | EQ HIGH on/off | | |

  On/off switches at value 64; EQ gains are 0..127 with 64 = flat; FILTER mode
  maps the value to BP / BP2 / COMB / TRASH.
- Everything else is unchanged from 2.2a.

## Tested

- Emulator (`dn1-2.2f`): the patched router path applies every CC and sets the
  pattern-store dirty flag (`digiemu_midi_cc.py`). The raw MIDI-receive path is
  hardware-validated — digiemu has no MIDI input model.
- Real hardware: base suite stress-tested on a Digitone mk1 (2.2a); the CC
  feature itself should get a hardware pass.
