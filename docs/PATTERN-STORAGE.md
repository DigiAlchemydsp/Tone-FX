# Per-pattern storage for the DIGI FX params (Digitone mk1, OS 1.43)

Status: **implemented (digictl 2.0, shipped in the merged `tonefx` mod),
verified in digiemu.** The FX parameters save and reload with the pattern, like
stock parameters, instead of living only in the mod's `.bss`.

## The problem

The DIGI FX parameters (`digiring_*`, `digieq_*`, `digifold_*`, `digimeter_on`,
`digimod_*`, `digicrush_*`, `digiwave_*`) are C globals in the mods' `.bss` (RAM
`0x47BE0000-0x47C00000`). That region is cleared at boot, so the settings would
be lost at power-off and would not change with the pattern.

## The pattern store (measured)

The project's patterns are an array of `0x1611D`-byte storage structs:

```
pattern(slot) = 0x407FC414 + slot * 0x1611D        (slot 0..127)
current pattern pointer: *(0x4138E214)
```

Confirmed from the pattern dispatcher `0x40094EF4` (it stores its argument at
`0x4138E214`) and the manager code at `0x400FE03A`, which computes
`0x407FC414 + index*0x1611D`. `*0x4138E214` moved by exactly `0x1611D` on a
pattern switch (measured in digiemu). The struct is `patternStorage_v11_t`
(RTTI); the render's algorithm getter `0x4009444A` reads its last long at
`+0x16119`. The OS copies the struct wholesale (several
`movel #90397,%sp@-` memcpy/memset sites, e.g. `0x400157F0`).

## The reserved block

Each `track_t` record is `0x3D0` bytes (8 tracks: 4 synth at `t=0..3`, 4 MIDI
at `t=4..7`). Every track has a 64-byte area at `track_t + 0x100`:

- **synth tracks** use it (voice/sound data — nonzero);
- **MIDI tracks** were measured **zero in all 128 slots**, and the OS's
  parameter mirror never wrote it, so `pattern + 0x1040` (MIDI track 0's block)
  is a safe, saved, 64-byte scratch per pattern.

## The layout

`digictl` keeps **24 parameters** there (8 + 2*24 = 56 of the 64 bytes):

```
+0..3   "DGX1" magic
+4..5   version (7)
+6..7   count (24)
+8..    24 x int16, big-endian, in DN_STORE order
```

The 24 params are ring (on, depth, rate), EQ (on, lo, hi), fold (on, amount,
mode), meter (on), the parked LFO bridge (dest, voice), **crush** (on, mode,
amount, tune) and **wave** (on, pmode, smode, page, speed, depth, mix, fb).
Older versions: 3 dropped the per-voice-filter params, 6 added CRUSH/WAVE,
7 added the split WAVE page/mode params. An older block fails the magic check
and is treated as a fresh pattern, so it starts from the live values.

## The sync and the adopt guard

`ev_tick` (one UI frame) runs `dn_store_sync()`:

- **new pattern pointer** (switch / first sync): if the magic is present, load
  the block into the globals; otherwise write the globals (a fresh pattern
  adopts the live values);
- **a page edit** (`dn_store_dirty`, set by every encoder/key/CC branch): write
  the globals into the block;
- **the block changed behind us**: adopt it **only if it still carries the
  `"DGX1"` magic** (a genuine pattern reload the OS rewrote). If the magic is
  gone, a **stock write** landed in that memory — reclaim our block instead of
  reading it. This guard is what stops a stock OS event (a PAGE press, a MIDI
  action, …) from jumping our parameters.

Because the block is part of the saved pattern struct, the values follow a
pattern switch / reload and travel with a project save, exactly like stock
parameters. New patterns (no magic) start from whatever the mods currently hold,
so the sound never jumps.

## Verified in digiemu

`dn1-3.1i` (core-dn1-2.0a + the merged suite, digictl 2.0): at boot the current
pattern's block holds `"DGX1"` v7 + the defaults; editing a global writes it on
the tick; poking the block with a valid magic reloads it; a non-DGX1 write is
ignored/reclaimed (the stock-event guard).

## Risks / open items

- The MIDI-track block was measured on **unconfigured** tracks. A *configured*
  MIDI track may write there. The 3.1i magic guard prevents a jumped param, but
  if a clash is found, move the store to another zero run (the other three MIDI
  blocks `+0x1410`, `+0x17E0`, `+0x1BB0`, or the 81-byte runs at
  `+0x0E7BA`/`+0x11009`/`+0x13858`/`+0x160A7`).
- Power-off persistence relies on the OS saving the pattern struct (auto-save of
  the temp project); confirmed per-pattern in digiemu, not yet on hardware.
- The store is per pattern, not the "reload pattern" backup the OS may keep in a
  separate copy; the adopt guard covers a reload that rewrites the struct.
