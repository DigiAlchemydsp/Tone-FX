/* SPDX-License-Identifier: GPL-2.0-or-later
 * TOEJAM: a momentary master-buffer stutter/loop.
 *
 * One stereo s16 ring keeps the newest finished mix: ~2.7 s on the Digitone, ~0.68 s on the Digitakt.  While the
 * STUTTER page is up, the 16 trigs are pads in four zones of four:
 *   zone 1  straight   1/32  1/16  1/8   1/4
 *   zone 2  dotted     1/16. 1/8.  1/4.  1/2.
 *   zone 3  triplet    1/16T 1/8T  1/4T  1/2T
 *   zone 4  long       1/2   1/1   2/1   4/1
 * Holding a pad freezes the ring and loops the newest slice; releasing it
 * resumes live audio.  Lengths follow fw_tempo on both machines; the Digitakt
 * core must carry the backported tempo export (this tree's core 4.0.1).
 */
#include "toejam.h"

#ifdef TOEJAM_DT
#include "digitakt-mk1/core3.h"
extern volatile int32_t fw_tempo;
extern int tfx_blk[64];
#define TJ_OUT_PTR ((int *)tfx_blk)
static unsigned tj_tempo_x120(void)
{
    return fw_tempo > 0 ? (unsigned)fw_tempo : 14400u;
}
#else
#include "digitone-mk1/core3.h"
#define TJ_OUT_BASE 0x80001A00u
#define TJ_OUT_HALF 0x100u
#define TJ_DMA_SRC  (*(volatile uint32_t *)0xFC0456C0u)
static volatile uint32_t tj_off;
static unsigned tj_tempo_x120(void)
{
    int32_t t = fw_tempo;
    return t > 0 ? (unsigned)t : 14400u;
}
#endif

#ifdef TOEJAM_DT
#define TJ_N        32768u              /* frames, ~0.68 s at 48 kHz */
#define TJ_CH       1                   /* mono ring: half the RAM */
#else
#define TJ_N        131072u             /* frames, ~2.73 s at 48 kHz */
#define TJ_CH       2                   /* stereo ring */
#endif
#define TJ_MASK     (TJ_N - 1u)
#define TJ_MAXLEN   (TJ_N - 8u)
#define TJ_MINLEN   32u

static int16_t tj_ring[TJ_CH * TJ_N] __attribute__((aligned(16)));
static volatile uint32_t tj_w;          /* next live frame to write */
static volatile uint32_t tj_anchor;     /* newest live frame when the hold began */
static volatile uint32_t tj_len;        /* frames in the loop, 0 = live */
static volatile uint32_t tj_pos;        /* frame in the loop */
static volatile int32_t  tj_pad_v = -1; /* active pad, -1 = live */
static volatile uint32_t tj_held_v;     /* pads currently down */

static inline int16_t tj_sat16(int32_t v)
{
    if (v > 32767)
        return 32767;
    if (v < -32768)
        return -32768;
    return (int16_t)v;
}

/* Frame count of pad t.  A quarter note is 345600000 / (BPM x 120) frames at
 * 48 kHz.  The pairs are (num, den) quarter notes. */
static uint32_t tj_len_for(int32_t t)
{
    static const uint8_t num[16] = {
        1, 1, 1, 1,    3, 3, 3, 3,
        1, 1, 2, 4,    2, 4, 8, 16
    };
    static const uint8_t den[16] = {
        8, 4, 2, 1,    8, 4, 2, 1,
        6, 3, 3, 3,    1, 1, 1, 1
    };
    uint32_t qfr, len;
    unsigned tempo = tj_tempo_x120();

    if (!tempo)
        tempo = 14400u;
    qfr = 345600000u / tempo;
    len = qfr * (uint32_t)num[t] / (uint32_t)den[t];
    if (len < TJ_MINLEN)
        len = TJ_MINLEN;
    if (len > TJ_MAXLEN)
        len = TJ_MAXLEN;
    return len;
}

void toejam_all_off(void)
{
    tj_held_v = 0;
    tj_pad_v = -1;
    tj_len = 0;
    tj_pos = 0;
}

void toejam_down(int t)
{
    if (!tj_held_v)                     /* first pad: the slice ends here */
        tj_anchor = tj_w;
    tj_held_v |= 1u << t;
    tj_len = tj_len_for(t);
    tj_pos = 0;
    tj_pad_v = t;
}

void toejam_up(int t)
{
    int32_t n;
    tj_held_v &= ~(1u << t);
    if (!tj_held_v) {
        toejam_all_off();
        return;
    }
    if (tj_pad_v != t)
        return;
    for (n = 15; n >= 0; n--)
        if (tj_held_v & (1u << n))
            break;
    tj_pad_v = n;
    tj_len = tj_len_for(n);
    tj_pos = 0;
}

int toejam_held(int t)
{
    if (t < 0 || t > 15)
        return 0;
    return (tj_held_v & (1u << t)) != 0;
}

int toejam_pad(void)
{
    return tj_pad_v;
}

int toejam_bpm(void)
{
    int32_t t = fw_tempo / 120;
    return t > 0 ? t : 120;
}

void toejam_enc(int id, int d)
{
    (void)id;
    (void)d;
}

void toejam_render_in(void)
{
#ifndef TOEJAM_DT
    tj_off = (TJ_DMA_SRC < (TJ_OUT_BASE + TJ_OUT_HALF)) ? TJ_OUT_HALF : 0u;
#endif
}

void toejam_render_out(void)
{
#ifdef TOEJAM_DT
    int32_t *p = TJ_OUT_PTR;
#else
    int32_t *p = (int32_t *)(TJ_OUT_BASE + tj_off);
#endif
    uint32_t len = tj_len;
    uint32_t pos = tj_pos;
    int32_t pad = tj_pad_v;
    int i;

    if (pad >= 0 && len) {              /* frozen loop */
        uint32_t base = (tj_anchor - len) & TJ_MASK;
        for (i = 0; i < 32; i++) {
            uint32_t idx;
            if (pos >= len)
                pos = 0;
            idx = (base + pos) & TJ_MASK;
#ifdef TOEJAM_DT
            {
                int32_t m = (int32_t)tj_ring[idx] << 8;
                p[0] = m;
                p[1] = m;
            }
#else
            p[0] = (int32_t)tj_ring[2 * idx] << 8;
            p[1] = (int32_t)tj_ring[2 * idx + 1] << 8;
#endif
            p += 2;
            pos++;
        }
        tj_pos = pos;
    } else {                            /* live: keep the history current */
        uint32_t w = tj_w;
        for (i = 0; i < 32; i++) {
#ifdef TOEJAM_DT
            tj_ring[w] = tj_sat16((p[0] + p[1]) >> 9);
#else
            tj_ring[2 * w] = tj_sat16(p[0] >> 8);
            tj_ring[2 * w + 1] = tj_sat16(p[1] >> 8);
#endif
            p += 2;
            w = (w + 1) & TJ_MASK;
        }
        tj_w = w;
    }
}

static const char *const tj_lbl[16] = {
    "1/32", "1/16", "1/8", "1/4",
    "1/16.", "1/8.", "1/4.", "1/2.",
    "1/16T", "1/8T", "1/4T", "1/2T",
    "1/2", "1/1", "2/1", "4/1"
};
static const char *const tj_zone[4] = { "STR", "DOT", "TRP", "LNG" };

void toejam_draw(void *bmp)
{
    int32_t row, col, i, bpm, ms, pad;
    uint32_t len;

    fw_fillrect(bmp, 0, 0, 127, 52, 0);

    fw_textf(bmp, (const void *)fw_font5, 4, 45, -1, "STUTTER");
    bpm = toejam_bpm();
    pad = tj_pad_v;
    len = tj_len;
    ms = (int32_t)(len / 48u);
    fw_textf(bmp, (const void *)fw_font5, 48, 45, -1, "%d BPM", (int)bpm);
    if (pad >= 0)
        fw_textf(bmp, (const void *)fw_font5, 92, 45, -1, "%dms", (int)ms);

    for (col = 0; col < 4; col++)
        fw_textf(bmp, (const void *)fw_font5, col * 32 + 2, 35, -1,
                 "%s", tj_zone[col]);

    for (col = 0; col < 4; col++) {
        for (row = 0; row < 4; row++) {
            int32_t x = col * 32 + 1;
            int32_t y = 26 - row * 8;
            i = col * 4 + row;
            if (tj_held_v & (1u << i))
                fw_framerect(bmp, x - 1, y - 1, x + 29, y + 5, 1);
            fw_textf(bmp, (const void *)fw_font5, x + 2, y, -1,
                     "%s", tj_lbl[i]);
        }
    }
}
