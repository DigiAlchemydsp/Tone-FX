# DIGI MIDI CC control (Digitone mk1, OS 1.43)

Status: **implemented (digictl 2.0), verified in digiemu (hook path).** The
RING / FOLD / TILT / CRUSH / PITCH / STEREO parameters respond to **incoming
MIDI Control Change** messages, like the stock internal parameters, so they can
be played and sequenced from an external controller/DAW.

## How it is hooked

Every incoming CC reaches the stock central CC router at **`0x400ED94E`**
(`Brain`/MIDI layer; per-track and global). Its first two instructions are
`lea sp@(-24),sp` / `moveq #8,d0` (6 bytes); `digictl` patches them to
**`jmp digictl_cc_disp`** (`cc_glue.s`). The glue reads the CC number and value
from the caller's stack (`sp@(8)`=CC, `sp@(12)`=value), calls
`digictl_cc_apply()` (in `digictl.c`), then executes the displaced
instructions and jumps into the stock router at `0x400ED954`. CCs we do not own
are left to the stock handler untouched.

> **The site must be `op: "jmp"`, not `jsr`.** A `jsr` pushes a return address,
> so the glue would run with `sp` four bytes lower and read the wrong slots
> (track/CC instead of CC/value), and the stock frame would be four bytes off —
> the stock `rts` then lands in garbage (e.g. `P4199E2F4` on CC 8). With `jmp`
> the glue sees the original entry layout and never returns (`jmp` into stock).

The apply sets the same globals the page does and marks the pattern store dirty,
so a CC move is saved with the pattern (see `docs/PATTERN-STORAGE.md`). Our
parameters change only from our own page encoders, keys and these CCs — the
per-pattern sync only adopts a block that still carries the `"DGX1"` magic, so a
stock OS event cannot reach any DIGI parameter.

## CC map

Only CC numbers the **stock DN CC table does not use** are taken (the table
lives at `0x4018D104`, 182 entries; the ones we use are listed below). The risky
`CC11` (Expression) is only a Toggle.

| CC | parameter | notes |
|---|---|---|
| `8`   | RING depth | 0..127 (`0` = no ring) |
| `11`  | RING on/off | >=64 = ON |
| `36`  | RING rate | 0..127 (maps internally to ~1..1906 Hz) |
| `37`  | FOLD on/off | >=64 = ON |
| `40`  | FOLD amount | 0..127 (`0` = bypass) |
| `41`  | FOLD type | 0..127 (CLEAN/MUD/DIST/TRSH, interpolated) |
| `67`  | EQ on/off | >=64 = ON |
| `68`  | meter on/off | >=64 = ON |
| `69`  | EQ low shelf | 0..127 (64 = flat) |
| `96`  | EQ high shelf | 0..127 (64 = flat) |
| `20`  | CRUSH on/off | >=64 = ON |
| `21`  | CRUSH mode | 0..127 (BITS / SR / CRUNCH) |
| `22`  | CRUSH amount | 0..127 (`0` = bypass) |
| `23`  | CRUSH tune | 0..127 (dither / extra SR ratio) |
| `24`  | WAVE on/off | >=64 = ON |
| `25`  | PITCH mode | 0..127 (WOB / SHFT) — also activates the PITCH page |
| `26`  | WAVE speed | 0..127 (LFO rate) |
| `27`  | WAVE depth | 0..127 (depth / semitones / width) |
| `28`  | WAVE mix | 0..127 (flanger dry/wet) |
| `29`  | WAVE fb | 0..127 (flanger feedback; scaled to 0..0.75 max) |
| `30`  | STEREO mode | 0..127 (FLAN / PAN / WIDE) — also activates STEREO |
| `31`  | WAVE group | >=64 = STEREO active, else PITCH |

**CC 20–31 are not yet checked against the stock CC table** (`0x4018D104`,
182 entries) — confirm they are free before sequencing WAVE/CRUSH from a DAW.

Not CC-controlled: the parked `digimod_*` LFO bridge.

Incoming CCs on **any** MIDI channel drive the (global) master FX; the values
follow the active pattern and are saved with it.

## Verified

`tests/digiemu_midi_cc.py` enters the **real patched site `0x400ED94E`** with a
crafted caller frame (track=9 makes the stock router return early) and checks
the CC list plus the dirty flag (extend `CASES` with 20–31). Entering the real site (not the glue directly)
is what catches a `jsr`-vs-`jmp` stack mistake. digiemu has no MIDI input model,
so **the raw MIDI receive path itself is validated on hardware**, not in the
emulator.

## Changing the map

The numbers are `#define DN_CC_*` in `mods/digictl/digictl.c`. Keep them to
free CCs (see above) and update this table and the test `CASES`.

The map is edited **in two places**, because the suite ships as the merged
`mods/tonefx/` mod: change the `#define`s in `digictl.c`, and keep the
`0x400ED94E` site as `op: "jmp"` in **both** `mods/digictl/mod.json` and
`mods/tonefx/mod.json`. Rebuild `tonefx` for the release.
