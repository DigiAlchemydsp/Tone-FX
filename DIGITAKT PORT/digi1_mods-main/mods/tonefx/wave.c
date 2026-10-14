/* SPDX-License-Identifier: GPL-2.0-or-later
 * digiwave: one modulation insert, five modes, one set of controls.
 * Digital Alchemy DSP's, for the original FM groovebox (and its Keys model), OS 1.43; ported to
 * the DT1 (OS 1.53 / 1.54) through corea.h.
 *
 *   WOB   vibrato: a sine-modulated, interpolated short delay (pitch wobble)
 *   SHFT  pitch shifter: two crossfaded grains read at a ratio (transpose)
 *   FLAN  flanger: a modulated, interpolated short delay + feedback + mix
 *   PAN   autopan: a sine pan-law (no delay)
 *   WIDE  stereo width: mid/side (no LFO)
 *
 * One sine LFO (sin256), ramped at block rate (one sine per block, then a
 * linear step -- no per-sample sine), and one shared delay line serve
 * WOB/SHFT/FLAN. The delay is fractional (linear interpolation) and clamped
 * positive, so a swept delay is click-free. All integer, no 64-bit multiply.
 * ev_render_in remembers the output half; ev_render_out processes in place.
 * Subscriptions only: it combines with every mod.
 */
#include "corea.h"
#include "sin256.h"

/* runtime parameters (exported; digictl edits them). 0..127, one step/notch. */
int digiwave_on = 0;            /* PITCH group (WOB / SHFT) on/off */
int digiwave_son = 0;           /* STEREO group (FLAN / PAN / WIDE) on/off */
int digiwave_pmode = 0;         /* PITCH page: 0..127 -> WOB / SHFT */
int digiwave_smode = 0;         /* STEREO page: 0..127 -> FLAN / PAN / WIDE */
int digiwave_page = 0;          /* active group: 0 PITCH, 1 STEREO */
int digiwave_speed = 40;        /* LFO rate */
int digiwave_pdepth = 64;       /* PITCH page depth (wobble / semitones) */
int digiwave_sdepth = 64;       /* STEREO page depth (sweep / pan / width) */
int digiwave_mix = 64;          /* wet mix (FLAN) */
int digiwave_fb = 0;            /* flanger feedback (capped at 75%) */
int digiwave_pmix = 127;        /* PITCH group dry/wet (WOB/SHFT); 127 = full wet */

#define WV_WOB 0
#define WV_SHF 1
#define WV_FLA 2
#define WV_PAN 3
#define WV_WID 4

#define WV_N     1024           /* WOB/SHFT dry line (was 2048): 2048 covers
                                 * 765 samples (WOB) and ~756 (SHFT grain), so
                                 * 1024 (~21 ms) is enough and halves the RAM */
#define WV_MASK  (WV_N - 1)
#define WV_GRAIN 512            /* SHFT grain length, samples (~10.7 ms) */
#define WV_LEAD  256            /* SHFT restart delay */
#define WV_FN    256            /* FLAN's own feedback line (~5.3 ms): the flanger
                                 * needs ~255 samples (base 128 + span 127) */
#define WV_FMASK (WV_FN - 1)

#define WV_K 89478              /* 2^32 / 48000: Hz -> Q32 phase inc */

static volatile unsigned wv_off;

/* +1 guard sample mirrors index 0, so a read at idx+1 never needs a mask.
 * WOB and SHFT share the dry line; FLAN keeps its OWN line because it writes
 * feedback into it -- that feedback must never reach WOB/SHFT (the
 * flanger/pitch "link"): with separate buffers there is no cross-talk to flush.
 * (DT1: 3.2a3 has no boot splash, so no digiwave_arena scratch; render.c is
 * kept but not built. The FLAN line is new in 3.5p, replacing the shared one.) */
static int wv_buf[2][WV_N + 1];
static unsigned wv_w;           /* dry write index (WOB/SHFT) */
static int wv_fbuf[2][WV_FN + 1];
static unsigned wv_fw;          /* FLAN feedback write index */
static unsigned wv_phase;       /* LFO phase, Q32 */

/* ramped (de-zipper) state, Q15: each mode's neutral value = true bypass */
static int wv_wob_wet;
static int wv_fla_mix;
static int wv_shf_wet;
static int wv_shf_dep = 64;     /* ramped SHFT ratio index (glide) */
static int wv_pan_dep;
static int wv_wide_cur = 64;

/* SHFT grain state */
static unsigned wv_gpos[2];     /* read position, Q16.16 */
static int wv_age[2];
static int wv_primed;

/* ratio: depth 0..127 -> 2^(semitones/12), semitones -12..+12, Q16.16 */
static const int wv_ratio[128] = {
    32768, 33125, 33486, 33850, 34219, 34591, 34968, 35349,
    35734, 36123, 36516, 36914, 37316, 37722, 38133, 38548,
    38968, 39392, 39821, 40255, 40693, 41136, 41584, 42037,
    42495, 42958, 43425, 43898, 44376, 44859, 45348, 45842,
    46341, 46846, 47356, 47871, 48393, 48920, 49452, 49991,
    50535, 51085, 51642, 52204, 52773, 53347, 53928, 54515,
    55109, 55709, 56316, 56929, 57549, 58176, 58809, 59449,
    60097, 60751, 61413, 62081, 62757, 63441, 64132, 64830,
    65536, 66250, 66971, 67700, 68438, 69183, 69936, 70698,
    71468, 72246, 73032, 73828, 74632, 75444, 76266, 77096,
    77936, 78785, 79642, 80510, 81386, 82273, 83169, 84074,
    84990, 85915, 86851, 87796, 88752, 89719, 90696, 91684,
    92682, 93691, 94711, 95743, 96785, 97839, 98905, 99982,
    101070, 102171, 103283, 104408, 105545, 106694, 107856, 109031,
    110218, 111418, 112631, 113858, 115098, 116351, 117618, 118899,
    120194, 121502, 122825, 124163, 125515, 126882, 128263, 129660,
};

/* triangle window, 256 entries (Q15), over WV_GRAIN; w[(i+128)&255] = 2^15-1-w[i] */
static const short wv_win[256] = {
    0, 256, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816,
    3072, 3328, 3584, 3840, 4096, 4352, 4608, 4864, 5120, 5376, 5632, 5888,
    6144, 6400, 6656, 6912, 7168, 7424, 7680, 7936, 8192, 8448, 8704, 8960,
    9216, 9472, 9728, 9984, 10240, 10496, 10752, 11008, 11264, 11520, 11776, 12032,
    12288, 12544, 12800, 13056, 13312, 13568, 13824, 14080, 14336, 14592, 14848, 15104,
    15360, 15616, 15872, 16128, 16384, 16639, 16895, 17151, 17407, 17663, 17919, 18175,
    18431, 18687, 18943, 19199, 19455, 19711, 19967, 20223, 20479, 20735, 20991, 21247,
    21503, 21759, 22015, 22271, 22527, 22783, 23039, 23295, 23551, 23807, 24063, 24319,
    24575, 24831, 25087, 25343, 25599, 25855, 26111, 26367, 26623, 26879, 27135, 27391,
    27647, 27903, 28159, 28415, 28671, 28927, 29183, 29439, 29695, 29951, 30207, 30463,
    30719, 30975, 31231, 31487, 31743, 31999, 32255, 32511, 32767, 32511, 32255, 31999,
    31743, 31487, 31231, 30975, 30719, 30463, 30207, 29951, 29695, 29439, 29183, 28927,
    28671, 28415, 28159, 27903, 27647, 27391, 27135, 26879, 26623, 26367, 26111, 25855,
    25599, 25343, 25087, 24831, 24575, 24319, 24063, 23807, 23551, 23295, 23039, 22783,
    22527, 22271, 22015, 21759, 21503, 21247, 20991, 20735, 20479, 20223, 19967, 19711,
    19455, 19199, 18943, 18687, 18431, 18175, 17919, 17663, 17407, 17151, 16895, 16639,
    16384, 16128, 15872, 15616, 15360, 15104, 14848, 14592, 14336, 14080, 13824, 13568,
    13312, 13056, 12800, 12544, 12288, 12032, 11776, 11520, 11264, 11008, 10752, 10496,
    10240, 9984, 9728, 9472, 9216, 8960, 8704, 8448, 8192, 7936, 7680, 7424,
    7168, 6912, 6656, 6400, 6144, 5888, 5632, 5376, 5120, 4864, 4608, 4352,
    4096, 3840, 3584, 3328, 3072, 2816, 2560, 2304, 2048, 1792, 1536, 1280,
    1024, 768, 512, 256,
};

#define WV_PUT(ch, v) do { wv_buf[ch][wv_w] = (v); \
    if (wv_w == 0) wv_buf[ch][WV_N] = (v); } while (0)
#define WV_FPUT(ch, v) do { wv_fbuf[ch][wv_fw] = (v); \
    if (wv_fw == 0) wv_fbuf[ch][WV_FN] = (v); } while (0)

void digiwave_render_in(void)
{
    wv_off = dn_out_off();
}

/* interpolated read at pos (Q16.16), one channel (guard sample => no mask beyond
 * `mask`); `mask` is WV_MASK for the dry line and WV_FMASK for FLAN's line */
static inline int wv_read(const int *line, unsigned pos, unsigned mask)
{
    unsigned idx = (pos >> 16) & mask;
    int frac = (int)((pos >> 1) & 0x7fff);          /* Q15 */
    int a = line[idx];
    return a + dn_mul15(line[idx + 1] - a, frac);
}

/* A ramp step that always reaches the target: (t-c)>>5 stalls at small
 * differences (e.g. 8>>5 == 0), which left a mode "off" but still running (the
 * flanger kept writing feedback). Snap to 1 near the target instead. */
static inline int wv_step(int cur, int tgt)
{
    int s = (tgt - cur) >> 5;
    if (s == 0 && cur != tgt)
        s = (tgt > cur) ? 1 : -1;
    return s;
}

/* one sine sample (Q15) at a Q32 phase */
static inline int wv_sin(unsigned ph)
{
    unsigned hi = ph >> 24;
    int fr = (int)((ph >> 16) & 0xffu);
    int s0 = sin256[hi & 0xffu];
    int s1 = sin256[(hi + 1) & 0xffu];
    return s0 + (((s1 - s0) * fr) >> 8);
}

/* The delay line is shared by WOB/SHFT/FLAN. FLAN writes its feedback into it,
 * so on a mode change or a fresh enable the line must be flushed or WOB/SHFT
 * read the flanger's fed-back residue and distort. Only runs on the change. */
static int wv_seq = -1;

static void wv_flush(void)
{
    int i, j;
    for (i = 0; i < 2; i++) {
        for (j = 0; j <= WV_N; j++)
            wv_buf[i][j] = 0;
        for (j = 0; j <= WV_FN; j++)
            wv_fbuf[i][j] = 0;
    }
    wv_w = 0;
    wv_fw = 0;
    wv_gpos[0] = wv_gpos[1] = 0;
    wv_age[0] = wv_age[1] = 0;
    wv_primed = 0;
}

void digiwave_render_out(void)
{
    int *p, i;
    int mode;
    int depth = digiwave_page ? digiwave_sdepth : digiwave_pdepth;
    int on = (digiwave_page ? digiwave_son : digiwave_on) ? 1 : 0;

    if (digiwave_page) {                        /* STEREO group */
        int b = 2 + ((digiwave_smode * 3) >> 7);
        mode = b > WV_WID ? WV_WID : b;
    } else {                                    /* PITCH group */
        int b = (digiwave_pmode * 2) >> 7;
        mode = b > 1 ? 1 : b;
    }

    if (!on) {
        wv_seq = -1;                            /* flush on the next enable */
    } else if (wv_seq != mode) {
        wv_flush();                             /* mode change: drop the residue */
        wv_seq = mode;
    }

    {
        unsigned hz = 1u + (((unsigned)(digiwave_speed * 12)) >> 7);
        unsigned inc = hz * WV_K * DN_OUT_FRAMES;       /* per block */
        int mix = digiwave_mix * 258;                   /* Q15 */
        int fb = (digiwave_fb * 24575) / 127;           /* Q15, capped at 0.75 */
        unsigned ph = wv_phase;
        int l0 = wv_sin(ph);                            /* block-rate LFO: */
        int lstep = (wv_sin(ph + inc) - l0) >> 5;       /* linear ramp      */
        p = (int *)(DN_OUT_BASE + wv_off);

        if (mode == WV_WOB) {
            int base = 384;                             /* ~8 ms */
            int span = depth * 3;                       /* up to ~8 ms */
            int wet = wv_wob_wet;
            int wet_t = (on && depth > 0) ?
                        (32767 * digiwave_pmix / 127) : 0;  /* neutral = dry */
            int wstep;
            if (wet_t == 0 && wet == 0) return;         /* true bypass */
            wstep = wv_step(wet, wet_t);
            for (i = 0; i < DN_OUT_FRAMES; i++) {
                int l = p[0], r = p[1];
                int dq = (base << 16) + span * l0 * 2;  /* Q16.16 samples */
                unsigned rp;
                int wl, wr;
                if (dq < (2 << 16)) dq = (2 << 16);     /* never wrap negative */
                rp = ((wv_w << 16) + (WV_N << 16) - (unsigned)dq) &
                     ((WV_N << 16) - 1);
                wl = wv_read(wv_buf[0], rp, WV_MASK);
                wr = wv_read(wv_buf[1], rp, WV_MASK);
                p[0] = l + dn_mul15(wl - l, wet);
                p[1] = r + dn_mul15(wr - r, wet);
                WV_PUT(0, l);
                WV_PUT(1, r);
                wv_w = (wv_w + 1) & WV_MASK;
                l0 += lstep;
                wet += wstep;
                p += 2;
            }
            wv_wob_wet = wet;
        } else if (mode == WV_FLA) {
            int base = 128;                             /* ~2.7 ms */
            int span = depth;                           /* up to ~2.7 ms */
            int m = wv_fla_mix;
            int m_t = on ? mix : 0;                     /* neutral = dry */
            int mstep;
            if (m_t == 0 && m == 0) return;
            mstep = wv_step(m, m_t);
            for (i = 0; i < DN_OUT_FRAMES; i++) {
                int l = p[0], r = p[1];
                int dq = (base << 16) + span * l0 * 2;
                unsigned rp;
                int wl, wr;
                if (dq < (2 << 16)) dq = (2 << 16);
                rp = ((wv_fw << 16) + (WV_FN << 16) - (unsigned)dq) &
                     ((WV_FN << 16) - 1);
                wl = wv_read(wv_fbuf[0], rp, WV_FMASK); /* FLAN's own line */
                wr = wv_read(wv_fbuf[1], rp, WV_FMASK);
                int fl, fr;
                p[0] = l + dn_mul15(wl - l, m);
                p[1] = r + dn_mul15(wr - r, m);
                fl = l + dn_mul15(wl, fb);              /* clamp feedback so it
                                                         * cannot run away */
                fr = r + dn_mul15(wr, fb);
                if (fl > 0x700000) fl = 0x700000;
                else if (fl < -0x700000) fl = -0x700000;
                if (fr > 0x700000) fr = 0x700000;
                else if (fr < -0x700000) fr = -0x700000;
                WV_FPUT(0, fl);                         /* never the dry line */
                WV_FPUT(1, fr);
                wv_fw = (wv_fw + 1) & WV_FMASK;
                l0 += lstep;
                m += mstep;
                p += 2;
            }
            wv_fla_mix = m;             /* store the RAMped mix, not the target:
                                         * storing `mix` restarted the ramp at
                                         * full mix every block, so FLAN never
                                         * bypassed and buzzed when switched off */
        } else if (mode == WV_SHF) {
            const int *bl = wv_buf[0], *br = wv_buf[1];
            unsigned r0, r1, inc2;
            int a0, a1, ed;
            int w = wv_shf_wet;
            int w_t, wstep;
            wv_shf_dep += (depth - wv_shf_dep) >> 5;    /* glide the ratio */
            ed = wv_shf_dep < 0 ? 0 : wv_shf_dep > 127 ? 127 : wv_shf_dep;
            w_t = (on && ed != 64) ? (32767 * digiwave_pmix / 127) : 0;  /* neutral = dry */
            if (w_t == 0 && w == 0) { wv_primed = 0; return; }
            wstep = wv_step(w, w_t);
            inc2 = (unsigned)wv_ratio[ed];
            if (!wv_primed) {
                r0 = r1 = ((wv_w + WV_N - WV_LEAD) & WV_MASK) << 16;
                a0 = 0;
                a1 = WV_GRAIN / 2;
                wv_primed = 1;
            } else {
                r0 = wv_gpos[0];
                r1 = wv_gpos[1];
                a0 = wv_age[0];
                a1 = wv_age[1];
            }
            for (i = 0; i < DN_OUT_FRAMES; i++) {
                int l = p[0], r = p[1];
                unsigned i0 = (r0 >> 16) & WV_MASK;
                unsigned i1 = (r1 >> 16) & WV_MASK;
                int f0 = (int)((r0 >> 1) & 0x7fff);
                int f1 = (int)((r1 >> 1) & 0x7fff);
                int a_l = bl[i0], b_l = bl[i1];
                int a_r = br[i0], b_r = br[i1];
                int sl0 = a_l + dn_mul15(bl[i0 + 1] - a_l, f0);
                int sr0 = a_r + dn_mul15(br[i0 + 1] - a_r, f0);
                int sl1 = b_l + dn_mul15(bl[i1 + 1] - b_l, f1);
                int sr1 = b_r + dn_mul15(br[i1 + 1] - b_r, f1);
                int w0 = wv_win[a0 >> 1];
                int sl = sl1 + dn_mul15(sl0 - sl1, w0);  /* crossfaded grains */
                int sr = sr1 + dn_mul15(sr0 - sr1, w0);
                /* complementary windows -> one mul; then dry<->shifted */
                p[0] = l + dn_mul15(sl - l, w);
                p[1] = r + dn_mul15(sr - r, w);
                WV_PUT(0, l);
                WV_PUT(1, r);
                wv_w = (wv_w + 1) & WV_MASK;

                r0 += inc2; a0++;
                if (a0 >= WV_GRAIN) {
                    a0 = 0;
                    r0 = ((wv_w + WV_N - WV_LEAD) & WV_MASK) << 16;
                }
                r1 += inc2; a1++;
                if (a1 >= WV_GRAIN) {
                    a1 = 0;
                    r1 = ((wv_w + WV_N - WV_LEAD) & WV_MASK) << 16;
                }
                w += wstep;
                p += 2;
            }
            wv_gpos[0] = r0;
            wv_gpos[1] = r1;
            wv_age[0] = a0;
            wv_age[1] = a1;
            wv_shf_wet = w;
        } else if (mode == WV_PAN) {
            int dep = wv_pan_dep;
            int d_t = on ? depth * 258 : 0;             /* neutral = centre */
            int dstep;
            if (d_t == 0 && dep == 0) return;           /* true bypass */
            dstep = wv_step(dep, d_t);
            for (i = 0; i < DN_OUT_FRAMES; i++) {
                int att = dn_mul15(l0, dep);
                int gl = 32767, gr = 32767;
                if (att > 0) gl = 32767 - att;
                else gr = 32767 + att;
                p[0] = dn_mul15(p[0], gl);
                p[1] = dn_mul15(p[1], gr);
                l0 += lstep;
                dep += dstep;
                p += 2;
            }
            wv_pan_dep = dep;
        } else {                                        /* WV_WID */
            int d = wv_wide_cur;
            int d_t = on ? depth : 64;                  /* neutral = 64 (1.0) */
            int dstep;
            if (d_t == 64 && d == 64) return;           /* true bypass */
            dstep = wv_step(d, d_t);
            for (i = 0; i < DN_OUT_FRAMES; i++) {
                int l = p[0], r = p[1];
                int m = (l + r) >> 1;
                int s = dn_mul14((l - r) >> 1, d * 256);
                p[0] = m + s;
                p[1] = m - s;
                d += dstep;
                p += 2;
            }
            wv_wide_cur = d;
        }
        wv_phase = ph + inc;
    }
}
