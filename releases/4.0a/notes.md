# Tone+FX 4.0a — DELAYS (WRMZ / GRAINZ) + CPU pass

The **eleven-page** Tone+FX suite **plus a new buffer-FX module**: a dedicated
mono delay line that serves two modes — **WRMZ** (a modulated delay) and
**GRAINZ** (a cloud of three drifting taps on the same line) — edited on one
**DELAYS** page. Twelve pages now, Digitone mk1 / Digitone Keys, **OS 1.43 and
1.44** from one source.

This is the first release built on the **ToneFX-BufferFX** line, so it also
carries a **CPU/headroom pass** over the whole suite (see *New in 4.0a*).

> **Digitakt mk1:** this release is **Digitone-only**. The Digitakt port's last
> release is **3.5r** (`tonefx-3.5r-dt.elemod` / `-dt-os1.54.elemod`, OS 1.53 /
> 1.54), still on the [v3.5r release](https://github.com/DigiAlchemydsp/Tone-FX/releases/tag/v3.5r)
> — the DELAYS module and this CPU pass are not ported there.

## Install

Each file is the whole suite + DELAYS for one OS; none includes the core.

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-4.0a.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-4.0a-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |

Install the matching core (elekloader provides it), tick the core and `tonefx`,
set the OS version tag and **BUILD FIRMWARE**; flash the `.syx` with Transfer.
Or use `tools/build.ps1` / `tools/build.sh`. No firmware is included.

## New in 4.0a

### DELAYS — WRMZ / GRAINZ (one delay line, two modes)

A **DELAYS** page (**kind 11**) after the FX chain. **D** switches the mode; the
rest of the encoders edit the active mode's six parameters. True-bypasses when
off (the line is untouched). **Mono** wet, mixed into both channels (the dry
stays stereo — widen externally).

- **WRMZ** — a **modulated delay**: TIME, FB (feedback, capped at 0.80), TONE
  (one-pole damping), MOD (depth), RATE (0.1–3.0 Hz wow/flutter), MIX.
- **GRAINZ** — a **grain-ish cloud**: three drifting, interpolated taps off the
  same line — DENS (drift), SIZE (tap spacing), PIT (parked), SPR (spread), FB,
  MIX.

Per-pattern store **`DGX3` v1** (15 params); **MIDI CC 80–87**. The suite's
output ceiling and meter still run last, so the chain is
**ring → EQ → crush → fold → wave → WRMZ → ceiling → meter**.

### CPU / headroom pass

- **eMAC mode switch hoisted** to once per block (`digictl_macsr_begin/_end`,
  `ev_render_out` order 49/74) instead of once per handler. On hardware an
  **eMAC-off** A/B idled lower but ran **worse while processing**, so the eMAC
  is kept.
- **Wave lines s16** (were 24-bit) with the interpolation done in the line's
  scale — halves the wave delay lines' DDR traffic and RAM.
- **Wave `wv_flush` spike fixed** — the one-time whole-line clear (2564 stores)
  on a mode change is gone (only FLAN's own feedback line is cleared); the
  per-mode **worst case now equals its p99** (e.g. SHFT worst +9,984 → +5,491
  cycles/block).
- **UI redraw cap** — animated pages redraw at ~5 Hz (was ~10), and a page whose
  section is off is static.
- **Size-optimised** build (`-Os`): `.run 34,398 → 26,018` (−24 %), for i-cache.

## Tested

- Both builds compile clean and **`imports nothing`** — Digitone 1.43 / 1.44
  (`.run 26018`, `.bss 47246`, 11 sites) — each links as `core 4.0 > tonefx 4.0a`
  and patches **byte-for-byte** against its own stock file.
- Emulator (Digitone `dn1`): boots, settles, `dsp_running=2`; all twelve pages
  incl. DELAYS; live diag (WRMZ + GRAINZ, real SSI renders) **0 faults**;
  `_probe_mix` 0 clips. The **OS 1.44** build is patch-checked; digiemu cannot
  settle 1.44.
- CPU ledger and the A/B write-ups: source repo
  `docs/OPTIMIZATION-ROUND.md` §10–§12.

## Known issues

Same as 3.5r — **PAGE can still jump some FX levels**, **PITCH can fire on a
note-off**, **SHFT is deliberately lightweight**, **PERFORM trig LEDs** do not
light. The DELAYS page's GRAINZ **PIT** control is parked (the taps read at rate
1.0). Full, current list in the README's "Known bugs / future fixes".
