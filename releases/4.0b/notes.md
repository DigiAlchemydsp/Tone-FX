# Tone+FX 4.0b — per-pattern store on one reserved MIDI track + macro rework

A maintenance release on the 4.0 line. It fixes the reported **jumping FX
values / moved MIDI channels / PAGES** by moving the per-pattern store out of
the stock MIDI-track blocks, and reworks the **WASH** and **RADIO** macros.
**Four OS files, two hosts:** Digitone mk1 / Keys (OS 1.43, 1.44) and, through
**gdeo607**'s port, the Digitakt mk1 (OS 1.53, 1.54).

## Install

Each file is the whole suite for one OS; none includes the core.

| file | device | OS | required core |
|---|---|---|---|
| `tonefx-4.0b.elemod` | Digitone mk1 / Keys | 1.43 | `core-dn1-4.0.elemod` |
| `tonefx-4.0b-os1.44.elemod` | Digitone mk1 / Keys | 1.44 | `core-dn1-4.0-os1.44.elemod` |
| `tonefx-4.0b-dt.elemod` | Digitakt mk1 | 1.53 | `core-4.0.elemod` |
| `tonefx-4.0b-dt-os1.54.elemod` | Digitakt mk1 | 1.54 | `core-4.0-os1.54.elemod` |

Install the matching core (elekloader provides it), tick the core and `tonefx`,
set the OS version tag and **BUILD FIRMWARE**; flash with Transfer. Or use
`tools/build.ps1` / `tools/build.sh`. No firmware is included.

## Per-pattern store (Digitone)

The FX params no longer live in the pattern struct's MIDI-track blocks for
channels 1-3 (`pattern+0x1040/+0x1410/+0x17E0`). A *configured* MIDI track
writes its channel/page there, so the stock data and our values corrupted each
other — the **jumping FX values and moved MIDI channels / pages**.

All 51 params are now **packed** (1-7 bits each) into **one reserved MIDI
track**: MIDI track 4 (`pattern+0x1BB0`, magic `"DGX4"`), on **MIDI channel 16**.
Only that one track is touched, so **MIDI tracks 1-3 (channels 1-15) and the
synth tracks are never read or written**. The params save and reload with the
pattern (switch, reload, project save).

> **Reserve MIDI track 4 on channel 16 and leave it alone** — that track is the
> store. Channels 1-15 are unaffected.

## Per-pattern store (Digitakt)

Stays a mod-owned RAM table (`dn_tab[128][48]` in `.bss`, keyed by the current
kit). No stock memory is read or written, and it no longer competes for the kit
spare that other mods use. RAM-only (as before).

## Macros

- **WASH**: WAVE **WOB only** (75% depth / 30% rate at full) plus the **WRMZ**
  delay's **MIX and FB**; the graphic is a full-screen Haze dot cloud.
- **RADIO**: **EQ tilt** (low → 0, high → 127 at full) + TRSH fold drive +
  **SRR-only** crush at a lower amount.
- Capture/restore is now **per engine**, so an engine a macro does not drive is
  left live: **CRUSH / REDUX is no longer tied to WASH**.

## Upgrading from 4.0a

- Digitone: 4.0a patterns carry the old `"DGX1/2/3"` magic in the MIDI-track
  blocks. 4.0b never reads them; a fresh pattern starts from the live values.
  Re-save the pattern/project after setting the FX you want as the base.
- Digitakt: 4.0a's kit-spare blocks are ignored; the new RAM table starts from
  the live values.

## Verified

- Build/lint/patch: all four link as `core 4.0 > tonefx 4.0b`, `imports nothing`,
  patch byte-exact against each stock file.
- digiemu (`dn1-4.0a`, `dt1-4.0a`): the old store addresses carry no magic; the
  CH-16 block carries `"DGX4"`; edits are saved; values are isolated per pattern
  and reload on return. Macro values checked live (WASH 95/38 + WRMZ 96/64;
  RADIO 0/127 + SRR 40; CRUSH stays live under WASH).

See `docs/PATTERN-STORAGE.md` in the repository.
