# Per-pattern storage for the DIGI FX params (4.3b)

Status: **Digitone: one reserved MIDI track (channel 16), packed. Digitakt: one
reserved MIDI track (channel 16), packed, in the kit record the pattern
serializer saves.** MIDI tracks 1-3 on the Digitone (channels 1-15) and the
synth tracks are never touched.

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

The 35 params are **packed** (1-7 bits each, `dn_stbits`) into the block:

```
+0..3   "DGX4" magic
+4      version (2)
+5..6   count (35)
+7..    bitstream, little-endian, 1-7 bits per param
```

204 bits -> 26 bytes + 7-byte header = 33 of the 64 bytes; the tail is cleared.

`ev_tick` runs `dn_store_sync()`: on a pattern switch it loads a block carrying
`"DGX4"` (or adopts the live values for a fresh pattern); a page/CC edit writes
the globals into the block; a block whose magic is gone (a stock write) is
reclaimed instead of read. The old blocks at `+0x1040/+0x1410/+0x17E0` are never
read or written again. 4.3b bumps the store to **v2**; v1 blocks are ignored and
reclaimed, so settings return to defaults once after upgrading.

## Digitakt port

The DT1 has the same idea in its kit: eight MIDI-track records at
`kit + 0x59a + t*0x70` (0x6c bytes each), which the stock pattern serializer
copies to the +Drive and back (`0x4007a300` -> `0x40079d22` on save,
`0x40079f86` -> `0x40079cc2` on load). Reserve **MIDI track 8 on channel 16**
and use its record:

```
record 7 -> kit + 0x59a + 7*0x70 = kit + 0x8aa
store at record + 0x10 .. + 0x51 (the first 41 words the serializer carries)
```

The store starts 0x10 bytes in, past the byte the loader remaps at +8, and is
packed `"DGX4"` v2 (32 params, 170 bits -> 22 bytes + header). It therefore survives a
pattern switch, a project load and a power cycle, and it does not touch the
sound records that Digi Matrix / Digi Poly / Digi EQ use (`kitstore.h`). A
stock MIDI-track write removes the magic and is reclaimed from the live values;
a genuine pattern record is adopted. The old RAM table (`dn_tab`) is gone.

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
- **Digitakt `dt1-4.0a`** (before this change): a param edit landed in
  `dn_tab[slot]`; the kit spare at `+0x530` was untouched. The record-based store
  is built and lints/patches for 1.53 and 1.54; re-run `tests/digiemu_tonefx.py`
  (the `STORE_ONLY=1` plan) on a patched-Unicorn digiemu checkout to verify it.
