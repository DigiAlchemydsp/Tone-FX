/* SPDX-License-Identifier: GPL-2.0-or-later
 * digicrush: master Redux / Crunch insert (sample-rate hold, drive).
 * Digitone mk1 / Keys, OS 1.43.
 *
 * Two stateless/near-stateless lo-fi stages on the output half:
 *   SR      sample-and-hold every `ratio` samples (a decimator, "REDUX")
 *   CRUNCH  a Q8 gain into a hard clip (a cheap saturation)
 *
 * Everything is computed once per block; the inner loop is straight-line
 * integer code (no 64-bit multiply, no library call). ev_render_in remembers
 * the output half, ev_render_out processes it in place. Subscriptions only:
 * it combines with every mod.
 */
#include "corea.h"
#include "emac.h"

/* runtime parameters (exported; digictl edits them). 0..127, one step/notch. */
int digicrush_on = 0;
int digicrush_mode = 0;         /* 0..127 -> SR / CRUNCH */
int digicrush_amount = 0;       /* depth of the selected stage */
int digicrush_tune = 0;         /* extra ratio (SR) */

#define DC_MODE_SR   0
#define DC_MODE_DRV  1

static volatile unsigned dc_off;

static int dc_mode;
static int dc_ratio;            /* SR: hold length 1..64 */
static unsigned dc_gain;        /* DRV: Q8 gain, 256 = 1.0x */

/* state */
static int dc_hold_l, dc_hold_r;
static int dc_cnt;
static int dc_amt_cur;          /* ramped amount: OFF fades to 0, then skips */

void digicrush_render_in(void)
{
    dc_off = dn_out_off();
}

static inline int dc_q8(int x, unsigned g)
{
    return bx_q8(x, (int)g);
}

static inline int dc_clip(int v)
{
    if (v > 0x7fffff) return 0x7fffff;
    if (v < -0x800000) return -0x800000;
    return v;
}

void digicrush_render_out(void)
{
    int *p, i, amt, amt_t;

    amt_t = digicrush_on ? digicrush_amount : 0;
    if (amt_t == 0 && dc_amt_cur == 0)
        return;
    dc_amt_cur += (amt_t - dc_amt_cur) >> 5;
    if (dc_amt_cur == 0)
        return;
    amt = dc_amt_cur;

    dc_mode = (digicrush_mode * 2) >> 7;
    if (dc_mode > DC_MODE_DRV)
        dc_mode = DC_MODE_DRV;

    if (dc_mode == DC_MODE_SR) {
        dc_ratio = 1 + (amt * 63 >> 7) + (digicrush_tune * 63 >> 7);
        if (dc_ratio > 64) dc_ratio = 64;
    } else {
        dc_gain = 256u + (unsigned)(amt * 6);
    }

    p = (int *)(DN_OUT_BASE + dc_off);

    if (dc_mode == DC_MODE_SR) {
        int cnt = dc_cnt, hl = dc_hold_l, hr = dc_hold_r, ratio = dc_ratio;
        for (i = 0; i < DN_OUT_FRAMES; i++) {
            if (cnt <= 0) { hl = p[0]; hr = p[1]; cnt = ratio; }
            cnt--;
            p[0] = hl;
            p[1] = hr;
            p += 2;
        }
        dc_cnt = cnt;
        dc_hold_l = hl;
        dc_hold_r = hr;
    } else {
        unsigned g = dc_gain;
        bx_emac_enter();
        for (i = 0; i < DN_OUT_FRAMES; i++) {
            p[0] = dc_clip(dc_q8(p[0], g));
            p[1] = dc_clip(dc_q8(p[1], g));
            p += 2;
        }
        bx_emac_leave();
    }
}