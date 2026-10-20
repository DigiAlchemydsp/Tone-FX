# Tone+FX 4.0c — permanent PITCH → DELAYS → STEREO chain + WASH CPU

A performance release on the 4.0 line. The master chain order is now
**permanent**, and the **WASH** macro is cheaper. **Four OS files, two hosts:**
Digitone mk1 / Keys (OS 1.43, 1.44) and, through **gdeo607**'s port, the
Digitakt mk1 (OS 1.53, 1.54).

## Install

Each file is the whole suite for one OS; none includes the core.

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-4.0c.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-4.0c-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-4.0c-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-4.0c-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

Install the matching core (elekloader provides it), tick the core and `tonefx`,
set the OS version tag and **BUILD FIRMWARE**; flash with Transfer. Or use
`tools/build.ps1` / `tools/build.sh`. No firmware is included.

## New in 4.0c

### Permanent chain order

```
ring → EQ → crush → fold → PITCH → DELAYS (WRMZ/GRAINZ) → STEREO → ceiling → meter
  50    55     57      60      61            62               63       64       65
```

The WAVE insert is split into two permanent passes
(`digiwave_pitch_render_out` / `digiwave_stereo_render_out`) with the delay
between them, so the DELAYS module is always **after PITCH** and **before
STEREO** — not conditional on the active page. Only the group selected by the
current page runs; the other pass costs a single branch.

### WASH CPU

While WASH is on it now:
- forces the WRMZ to a **static read** (`MOD = 0`), so no interpolation;
- **skips the DC blocker** (the feedback saturator still bounds the loop), with
  the DC state reset on enter/leave to avoid a transient;
- the L/R wet mix uses **`bx_mul15x2`** (two eMAC MACs back to back) in WOB and
  WRMZ/GRAINZ.

The DC-skip and `MOD = 0` are scoped by the per-engine macro capture/restore,
so turning WASH off restores the delay exactly as it was.

## Upgrading

- **Digitone:** the 4.0b per-pattern store (MIDI track 4 at `pattern+0x1BB0`,
  `"DGX4"`, CH 16) is unchanged. Reserve MIDI track 4 on channel 16.
- **Digitakt:** the mod-owned RAM table is unchanged.

## Verified

- Build/lint/patch: all four link as `core 4.0 > tonefx 4.0c`, `imports nothing`,
  patch byte-exact against each stock file.
- Emulator: the DN chain resolves 61/62/63 (pitch, delay, stereo); the macro
  values and the per-engine capture/restore were checked in 4.0b, and the
  DC-skip/`MOD=0` path compiles and links in the same handlers.

See `docs/PATTERN-STORAGE.md` and `CHANGELOG.md` in the repository.
