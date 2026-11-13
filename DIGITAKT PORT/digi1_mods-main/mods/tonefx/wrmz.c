/* SPDX-License-Identifier: GPL-2.0-or-later
 * digiwrmz: the buffer-FX DELAY engine. A Core A master insert appended after the
 * Tone+FX chain (order 62). Digitone mk1 / Keys, OS 1.43.
 *
 * ONE dedicated delay line (mono, 24-bit), WRMZ: a modulated delay (time,
 * feedback, tone, wow/flutter). The wet is mono and mixed into both output
 * channels (dry stays stereo). True bypass when off. Integer, no libc, no
 * 64-bit mul.
 */
#include "wrmz.h"
#include "buffx.h"
#include "emac.h"

int digiwrmz_on = 0;
int digiwrmz_time = 40;         /* 0..127 delay time */
int digiwrmz_fb = 32;           /* 0..127 feedback */
int digiwrmz_tone = 64;         /* 0..127 damping */
int digiwrmz_mod = 0;           /* 0..127 modulation depth */
int digiwrmz_rate = 40;         /* 0..127 modulation rate */
int digiwrmz_mix = 64;          /* 0..127 dry/wet */

#define WZ_N     (1u << 14)             /* 0.34 s @48k, mono + a guard */
#define WZ_MASK  (WZ_N - 1u)
#define WZ_MAXD  (WZ_N - 8u)

static volatile unsigned wz_off;

#if defined(BX_S16_LINE)
typedef short wz_samp;
static inline int wz_rd(const wz_samp *b, unsigned idx) { return (int)b[idx] << 8; }
static inline int wz_rdi(const wz_samp *b, unsigned idx, int frac)
{
    int a = b[idx];
    return (a + bx_mul15(b[idx + 1] - a, frac)) << 8;
}
static inline wz_samp wz_mk(int v) { return (wz_samp)bx_sat16(v >> 8); }
#else
typedef int wz_samp;
static inline int wz_rd(const wz_samp *b, unsigned idx) { return b[idx]; }
static inline int wz_rdi(const wz_samp *b, unsigned idx, int frac) { return bx_read_at(b, idx, frac); }
static inline wz_samp wz_mk(int v) { return bx_soft(v, 0x7fffff); }
#endif

static wz_samp wz_buf[WZ_N + 1] __attribute__((aligned(16)));
static unsigned wz_w;
static unsigned wz_phase;
static int wz_lp;
static int wz_dc[2];
static int wz_mix;

void digiwrmz_render_in(void)
{
    wz_off = dn_out_off();
}

void digiwrmz_render_out(void)
{
    int *p;
    int i, on, mix_t, mstep, fb, lpc;
    unsigned inc, ph, mhz;
    int l0, ls0;

    on = digiwrmz_on;
    mix_t = on ? bx_wet(digiwrmz_mix * 258) : 0;
    p = (int *)(DN_OUT_BASE + wz_off);

    if (!on && wz_mix == 0)
        return;                                 /* true bypass: no work off */

    mhz = 100u + (unsigned)(digiwrmz_rate * 2900) / 127u;
    inc = bx_inc_mhz(mhz, DN_OUT_FRAMES);
    ph = wz_phase;
    fb = (digiwrmz_fb * 26214) / 127;           /* cap 0.80 */
    lpc = 21000 + (digiwrmz_tone - 64) * 180;
    if (lpc < 800)
        lpc = 800;
    else if (lpc > 32700)
        lpc = 32700;

    mstep = bx_step(wz_mix, mix_t);
    l0 = bx_sin(ph);
    ls0 = (bx_sin(ph + inc) - l0) >> 5;

    bx_emac_enter();

    {
        int base = 48 + (digiwrmz_time * ((int)WZ_MAXD - 900 - 48)) / 127;
        int span = (digiwrmz_mod * 900) / 127;
        for (i = 0; i < DN_OUT_FRAMES; i++) {
            int l = p[0], r = p[1];
            int mono = (l + r) >> 1;
            int dq = base * 4096 + ((span * l0) >> 3);
            int fr, wet;
            unsigned idx;
            if (dq < (2 * 4096))
                dq = 2 * 4096;
            idx = (wz_w - (unsigned)(dq >> 12)) & WZ_MASK;
            fr = dq & 0xfff;
            wet = span ? wz_rdi(wz_buf, idx, fr) : wz_rd(wz_buf, idx);

            {
                int q[2];
                bx_mul15x2(wet - l, wet - r, wz_mix, q);
                p[0] = l + q[0];
                p[1] = r + q[1];
            }
            wz_lp = bx_lp_e(wz_lp, wet, lpc);
            {
                int f = mono + bx_mul15(wz_lp, fb);
                f = bx_dc_e(wz_dc, f);
                wz_buf[wz_w] = wz_mk(f);
            }
            if (wz_w == 0)
                wz_buf[WZ_N] = wz_buf[0];
            wz_w = (wz_w + 1) & WZ_MASK;
            l0 += ls0;
            wz_mix += mstep;
            p += 2;
        }
    }

    bx_emac_leave();
    wz_phase = ph + inc;
}