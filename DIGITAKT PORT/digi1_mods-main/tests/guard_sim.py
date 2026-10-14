#!/usr/bin/env python3
"""Digi Mono's overload guard (mods/digimono/digimono.c), its own code on the PC, under made-up loads.

    python3 tests/guard_sim.py            (needs gcc)

The guard's section of digimono.c (from "the overload guard" to the render's stack) is compiled with the
voices' struct and a DTIM0 counter the test sets: each simulated render takes the stock part plus every
playing voice's block (a voice resting, or with the whole set resting, costs nothing; under eco a voice
costs its eco cost). Checks:
  1. a load that fits: no eco, no rest;
  2. over 7/8 of the period but under 94 % once eco is on: eco, and no voice rests;
  3. over 94 % even with eco: the heaviest voice rests, the others play; eco stays on; the voice comes
     back only when there is room for it (no blips), and then stays;
  4. the load gone: eco ends after 2 s of calm, and the rests' strikes are forgotten;
  5. a load that is not Digi Mono's (the stock part alone over 94 %): every voice rests (16 ms), never a
     render over the period for more than 12 in a row.
"""
import ctypes, os, re, subprocess, sys, tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "mods", "digimono", "digimono.c")
FAIL = []
PERIOD = 88000


def check(ok, what):
    print(("  ok    " if ok else "  FAIL  ") + what)
    if not ok:
        FAIL.append(what)


def build():
    s = open(SRC).read()
    struct = re.search(r"struct digimono_voice \{.*?\};", s, re.S).group(0)
    guard = s[s.index("/* ---- the overload guard"):s.index("/* The render's own stack")]
    guard = guard.replace("#define DTCN0        (*(volatile const uint32_t *)0xFC07000C)", "uint32_t DTCN0;")
    c = ("#include <stdint.h>\nstruct mono_voice { int x; };\nuint8_t digimono_eco;\n" + struct +
         "\nstruct digimono_voice voices[8];\nstatic int shed;\nint get_shed(void) { return shed; }\n" + guard)
    work = tempfile.mkdtemp(prefix="guard_")
    open(os.path.join(work, "g.c"), "w").write(c)
    so = os.path.join(work, "g.so")
    subprocess.check_call(["gcc", "-O1", "-Wall", "-shared", "-fPIC", "-o", so, os.path.join(work, "g.c")])
    return ctypes.CDLL(so), struct


class Voice(ctypes.Structure):
    _fields_ = [("mv", ctypes.c_int), ("live", ctypes.c_uint8), ("quiet", ctypes.c_uint8),
                ("mach", ctypes.c_uint8), ("strikes", ctypes.c_uint8), ("rest", ctypes.c_uint16),
                ("cost", ctypes.c_uint16), ("want", ctypes.c_uint16)]


def main():
    lib, struct = build()
    check("uint8_t strikes;" in struct and "uint16_t rest;" in struct and "uint16_t cost;" in struct and "uint16_t want;" in struct,
          "the voice's struct as this test has it")
    voices = (Voice * 8).in_dll(lib, "voices")
    dtcn = ctypes.c_uint32.in_dll(lib, "DTCN0")
    eco = ctypes.c_uint8.in_dll(lib, "digimono_eco")
    t = [0]

    def run(blocks, stock, full, cheap):
        """blocks renders; voice v costs full[v] (cheap[v] under eco); -> per-render records"""
        rec = []
        for _ in range(blocks):
            dtcn.value = t[0]
            lib.digimono_rin()
            shed = lib.get_shed()
            dur = stock
            for v in range(8):
                c = 0 if (shed or voices[v].rest) else (cheap[v] if eco.value else full[v])
                c = c or 60 if not (shed or voices[v].rest) else 20     # a voice asleep: its few checks
                voices[v].cost = c
                dur += c
            t[0] = (t[0] + dur) & 0xffffffff
            dtcn.value = t[0]
            lib.digimono_rout()
            rec.append((dur, eco.value, [voices[v].rest for v in range(8)], shed))
            t[0] = (t[0] + max(0, PERIOD - dur)) & 0xffffffff
        return rec

    stock = int(0.72 * PERIOD)                   # FAST AUDIO's stock render
    # three voices: an ENS (8 %), a CHORDS (8 %) and a MACRO 2OP FM with feedback (20 %); eco: 6, 6, 11
    full = [int(x * PERIOD) for x in (0.08, 0.08, 0.20, 0, 0, 0, 0, 0)]
    cheap = [int(x * PERIOD) for x in (0.06, 0.06, 0.11, 0, 0, 0, 0, 0)]

    print("1. a load that fits")
    r = run(3000, stock, [int(0.04 * PERIOD)] * 3 + [0] * 5, [int(0.03 * PERIOD)] * 3 + [0] * 5)
    check(not any(e for _, e, _, _ in r) and not any(any(x) for _, _, x, _ in r), "no eco, no voice rests")

    print("2. over 7/8, under 94 % with eco")
    light = [int(x * PERIOD) for x in (0.06, 0.06, 0.08, 0, 0, 0, 0, 0)]   # 92 % in all; eco: 85 %
    r = run(3000, stock, light, [int(x * PERIOD) for x in (0.04, 0.04, 0.05, 0, 0, 0, 0, 0)])
    check(r[0][1] == 1 and all(e for _, e, _, _ in r), "eco from the first render on, and it stays")
    check(not any(any(x) for _, _, x, _ in r) and not any(s for *_, s in r), "no voice rests")

    print("3. over 94 % even with eco")
    r = run(1500, stock, full, cheap)            # 108 % / 95 % with eco
    rested = [v for v in range(8) if any(x[v] for _, _, x, _ in r)]
    check(rested == [2], "only the heaviest voice (MACRO) rests (rested: %s)" % rested)
    check(not any(s for *_, s in r), "never every voice")
    late = r[-500:]
    check(max(d for d, *_ in late) <= PERIOD, "the render back within its period (%d %%)"
          % (100 * max(d for d, *_ in late) // PERIOD))
    check(all(e for _, e, _, _ in r), "eco on throughout")
    back = sum(1 for a, b in zip(r, r[1:]) if a[2][2] and not b[2][2])
    check(back == 0, "MACRO, which does not fit, does not come back for blips (%d times back)" % back)

    print("3b. room again")
    r = run(400, int(0.60 * PERIOD), full, cheap)
    check(not r[-1][2][2] and all(not x[2] for _, _, x, _ in r[-300:]), "MACRO plays again, and stays")

    print("4. the load gone")
    r = run(3200, int(0.5 * PERIOD), [0] * 8, [0] * 8)
    off = next((i for i, (_, e, _, _) in enumerate(r) if not e), None)
    check(off is not None and 2950 <= off <= 3050, "eco ends after 2 s of calm (render %s)" % off)
    check(all(v.strikes == 0 for v in voices), "the strikes forgotten")
    check(not any(x for x in r[-1][2]), "no voice resting")

    print("5. a load that is not Digi Mono's")
    r = run(200, int(0.97 * PERIOD), full, cheap)
    sheds = sum(1 for *_, s in r if s)
    check(sheds > 0, "every voice rests while the render stays over 94 %% (%d renders)" % sheds)
    run_over = best = 0
    for d, *_ in r:
        run_over = run_over + 1 if d > PERIOD else 0
        best = max(best, run_over)
    check(best <= 12, "at most 12 renders in a row over the period (%d)" % best)
    print()
    if FAIL:
        print("%d FAILED" % len(FAIL))
        sys.exit(1)
    print("ALL GUARD CHECKS PASSED")


if __name__ == "__main__":
    main()
