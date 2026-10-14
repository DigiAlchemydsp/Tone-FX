/* SPDX-License-Identifier: GPL-2.0-or-later
 * Master-insert helpers for the Tone+FX port to the DT1 (OS 1.53 / 1.54).
 * Based on Digital Alchemy DSP's corea.h for the original (github.com/DigiAlchemydsp/RingTone);
 * the names are kept so the effect sources stay as they are. The drawing routines (DN_FILLRECT,
 * DN_FRAMERECT) are defined in digictl.c, the only file that draws, where tools/port_os.py finds them
 * (it reads the mod's sources, not their headers).
 *
 * Where the effects run (DT1): on the master mix, at the render's step that spreads it over the
 * outputs (0x400721e6, the same place as Digi EQ). From there it goes to the analog conversion for the
 * main outputs and the headphones, and into the 12-channel bus the USB stream is built from, so every
 * output carries the effects. master.c copies the block (0x8000ea70: 32 frames of { int32 L, int32 R },
 * 8 bits hotter than the 24-bit words the codec takes) into tfx_blk as 24-bit samples, right-justified as
 * on the original, runs the effects' render_out stages on it in their order, and puts the difference
 * back. The effects' code is unchanged: their "output buffer" is tfx_blk, and the half offset is 0.
 * (On the original they run on the SSI output buffer; the DT1's is 0x4ba8f080 + half * 0x100, or
 * 0x4ba9f080 on OS 1.54, but the USB stream does not pass through it.)
 *   ev_render_in  -> dn_out_off()  (0: one block)
 *   master.c      -> process (DN_OUT_BASE, 256 bytes) in place
 */
#ifndef DT1_COREA_H
#define DT1_COREA_H

extern int tfx_blk[64];               /* master.c: this block, 24-bit */
#define DN_OUT_BASE  ((unsigned)tfx_blk)
#define DN_OUT_FRAMES 32              /* stereo frames per block (256 bytes) */

static inline unsigned dn_out_off(void)
{
    return 0u;
}

/* x*c/2^15 (c is Q15), the low 15 bits of x dropped: safe for any int32 x and
 * int16 c, no 64-bit product, no library call. Precision is ample for audio. */
static inline int dn_q15(int x, int c)
{
    return (x >> 15) * c;
}

/* x*d/2^14 for a small delta d (Q14): result is x*(1 + d/2^14), i.e. a gain
 * near 1 expressed as its deviation, which keeps the product in 32 bits. */
static inline int dn_q14d(int x, int d)
{
    return (x >> 14) * d;
}

/* Full-precision versions. The truncated ones above drop the low 15/14 bits of
 * x BEFORE the multiply, which quantizes the output in steps of c (about -36 dB
 * for a Q15 carrier) -- audible as noise on quiet material. These keep the
 * dropped bits as a second term, so the product is exact with 32-bit ops:
 *   (x >> k) * c + (((x & (2^k-1)) * c) >> k)   ==   (x * c) >> k
 * The first term still keeps x >> k times c inside 32 bits for audio levels. */
static inline int dn_mul15(int x, int c)
{
    return (x >> 15) * c + (((x & 0x7fff) * c) >> 15);
}

static inline int dn_mul14(int x, int d)
{
    return (x >> 14) * d + (((x & 0x3fff) * d) >> 14);
}

#endif
