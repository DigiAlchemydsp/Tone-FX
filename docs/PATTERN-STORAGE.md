# Per-pattern storage for the DIGI FX params (digictl 2.9)

Status: **Digitone: one reserved MIDI track (channel 16), packed. Digitakt: a
mod-owned RAM table.** MIDI tracks 1-3 (channels 1-15) and the synth tracks are
never touched.

## Why not the old blocks (channels 1-3)

Earlier builds spread the params over the pattern struct's MIDI-track blocks at
`pattern + 0x1040 / +0x1410 / +0x17E0` — `track_t + 0x100` of MIDI tracks 1-3.
They measured zero on *unconfigured* tracks, but a **configured** MIDI track
writes its channel/page there, so the stock data and our values corrupted each
other (jumping FX values, moved MIDI channels).

## Digitone: one reserved MIDI track (channel 16)

All params live in **one** MIDI track's 64-byte block:

```
track_t = 0x3D0; MIDI track 4 (t = 7) -> pattern + 7*0x3D0 + 0x100 = pattern + 0x1BB0
```

Reserve that track on **MIDI channel 16** and leave it alone. The store writes
only `pattern + 0x1BB0`; the other three MIDI-track blocks and every synth track
are untouched, so channels 1-15 are unaffected.

The 51 params are **packed** (1-7 bits each, `dn_stbits`) into the block:

```
+0..3   "DGX4" magic
+4      version (1)
+5..6   count (51)
+7..    bitstream, little-endian, 1-7 bits per param
```

260 bits -> 33 bytes + 7-byte header = 40 of the 64 bytes; the tail is cleared.

`ev_tick` runs `dn_store_sync()`: on a pattern switch it loads a block carrying
`"DGX4"` (or adopts the live values for a fresh pattern); a page/CC edit writes
the globals into the block; a block whose magic is gone (a stock write) is
reclaimed instead of read. The old blocks at `+0x1040/+0x1410/+0x17E0` are never
read or written again.

## Digitakt port

The DT1 has no equivalent MIDI-track block; its store stays a **mod-owned RAM
table** (`dn_tab[128][48]` in `.bss`, keyed by `*0x4199dc44`). RAM-only, no stock
memory read or written, and no conflict with the kit spare that other mods use.

## The PERFORM page momentary pads

Unchanged. The 16 trig pads (`pf_apply` in `digictl.c`) override a block's depth
only while the pad is held (and the short release ramp), restoring the saved
value afterwards. They **never set `dn_store_dirty`** and never write the store,
so the pattern value is always the resting position. Pad state lives in plain
`.bss` (`pf_*`).

## Verified in digiemu

- **Digitone `dn1-4.0a`** (core-dn1 4.0 + the suite): the blocks at
  `+0x1040/+0x1410/+0x17E0` carry no magic and stay clean; `+0x1BB0` carries
  `"DGX4"`; an edit is saved; a value set on one pattern does not leak to
  another; a return reloads it.
- **Digitakt `dt1-4.0a`**: a param edit lands in `dn_tab[slot]`; the kit spare at
  `+0x530` is untouched.
