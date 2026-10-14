/* SPDX-License-Identifier: GPL-2.0-or-later
 * TONE+FX: one boot splash chosen at random each boot (1/3 each) - written for the original by
 * Digital Alchemy DSP (github.com/DigiAlchemydsp/RingTone), ported to the DT1:
 *   mode 0 - accelerated particles converge and lock into the wordmark,
 *   mode 1 - a laser traces TONE+FX like a vector display,
 *   mode 2 - the laser draws it, then accelerated sparks bloom off it.
 *
 * Everything is generated in code (a small stroke font + an LCG), so the
 * whole mod is a few KB of RAM instead of pre-rendered frames. Called once
 * per panel present while the intro owns PIT3. Panel is 128x64 1bpp,
 * byte = page + 8*column, page = 7 - (y>>3), bit = y&7.
 */

/* ---- firmware addresses that moved in OS 1.54 (tools/port_os.py, tools/os154.json) ---- */
#ifdef OS154
#define F_4020d8f8 0x4020dca8
#else
#define F_4020d8f8 0x4020d8f8
#endif
/* ---- end of the moved addresses ---- */


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;

/* DT1 port (OS 1.53; tools/port_os.py names the 1.54 addresses): the panel's front buffer
 * pointer (digiemu's fb_front, read by panel_diff 0x400e60e2), the PIT3 vector (the same slot as on the
 * original) and the intro's PIT3 ISR, which the intro installs there at 0x4006c252. core starts DTIM0. */
#define FBP      (*(volatile u32 *)F_4020d8f8)
#define PIT3_VEC (*(volatile u32 *)0x40000340)
#define INTRO_ISR ((u32)0x4006c154)
#define DTCN0    (*(volatile u32 *)0xFC07000C)   /* DTIM0 free-running counter */

static u8 *fb;

/* ---- font: T O N E + F X as line strokes in a 14x24 cell ---------------- */
static const signed char SEG[] = {
    /* T */ 1,1,13,1,  7,1,7,23,
    /* O */ 2,2,12,2,  12,2,12,22,  12,22,2,22,  2,22,2,2,
    /* N */ 2,23,2,1,  2,1,12,23,  12,1,12,23,
    /* E */ 2,1,2,23,  2,1,12,1,  2,12,10,12,  2,23,12,23,
    /* + */ 7,7,7,17,  2,12,12,12,
    /* F */ 2,1,2,23,  2,1,12,1,  2,12,10,12,
    /* X */ 2,1,12,23,  12,1,2,23,
};
static const u8 SCNT[7] = { 2, 4, 3, 4, 2, 3, 2 };
static const u8 SOFF[7] = { 0, 2, 6, 9, 13, 15, 18 };
static const char TEXT[] = "TONE+FX";

#define CW 14
#define CH 24
#define PITCH 17
#define OX ((128 - (6 * PITCH + CW)) / 2)   /* 6 */
#define OY ((64 - CH) / 2)                 /* 20 */

/* ---- panel pixel plot ---- */
static void px(i32 x, i32 y)
{
    if (y < 0 || x < 0 || x > 127 || y > 63) return;
    fb[(7 - (y >> 3)) + (x << 3)] |= (u8)(1u << (y & 7));
}

/* ---- PRNG (ANSI LCG) ---- */
static u32 rs;
static u32 rnd(void) { rs = rs * 1103515245u + 12345u; return (rs >> 16) & 0x7fffu; }

/* ---- wordmark raster (built once): thick target pixels + centreline ---- */
#define NP 1100   /* max thick pixels */
#define TR 400    /* max centreline points */
#define NS 300    /* sparks (mode 2) */
/* DT1 port: the mods' RAM is 128 KB for every mod together, so the splash's working arrays (8.5 KB,
 * used only while the intro runs) live in digiwave's delay line (16 KB, wave.c), which nothing uses
 * before the intro is over: digiwave clears it then (tonesplash_active), before its first use. */
extern int digiwave_arena[];
static u8 *ptx, *pty, *trx, *try, *seen, *sx, *sy, *sp0, *spk_dir, *spk_t0;
static u16 *spk_src;

static void arena(void)                     /* carve the arrays out of the line, once */
{
    u8 *a = (u8 *)digiwave_arena;
    ptx = a;                 pty = ptx + NP;
    trx = pty + NP;          try = trx + TR;
    seen = try + TR;         sx = seen + 1024;
    sy = sx + NP;            sp0 = sy + NP;
    spk_dir = sp0 + NP;      spk_t0 = spk_dir + NS;
    spk_src = (u16 *)(spk_t0 + NS);         /* 5 NP + 2 TR + 1024 + 2 NS: even, 8,524 bytes in all */
}
static u16 ntarget;
static u16 ntrace;

static i32 charidx(char c)
{
    switch (c) {
        case 'T': return 0;
        case 'O': return 1;
        case 'N': return 2;
        case 'E': return 3;
        case '+': return 4;
        case 'F': return 5;
        case 'X': return 6;
    }
    return -1;
}

static void build_lists(void)
{
    i32 k, si, i;
    for (i = 0; i < 1024; i++) seen[i] = 0;
    ntarget = 0; ntrace = 0;
    for (k = 0; TEXT[k]; k++) {
        i32 ci = charidx(TEXT[k]);
        i32 cx, x0, y0, x1, y1, x, y, dx, dy, ax, ay, sx, sy, err, e2, bx, by;
        if (ci < 0) continue;
        cx = OX + k * PITCH;
        for (si = SOFF[ci]; si < SOFF[ci] + SCNT[ci]; si++) {
            x0 = cx + SEG[si * 4 + 0]; y0 = OY + SEG[si * 4 + 1];
            x1 = cx + SEG[si * 4 + 2]; y1 = OY + SEG[si * 4 + 3];
            dx = x1 - x0; dy = y1 - y0;
            ax = dx < 0 ? -dx : dx;
            ay = dy < 0 ? -dy : dy;
            sx = x0 < x1 ? 1 : -1;
            sy = y0 < y1 ? 1 : -1;
            err = ax - ay;
            x = x0; y = y0;
            for (;;) {
                /* centreline */
                trx[ntrace] = (u8)x; try[ntrace] = (u8)y; ntrace++;
                /* thick 3x3 brush, dedup into ptx/pty */
                for (by = -1; by <= 1; by++) {
                    for (bx = -1; bx <= 1; bx++) {
                        i32 qx = x + bx, qy = y + by;
                        i32 idx; u8 bit;
                        if (qx < 0 || qx > 127 || qy < 0 || qy > 63) continue;
                        idx = (7 - (qy >> 3)) + (qx << 3);
                        bit = (u8)(1u << (qy & 7));
                        if (!(seen[idx] & bit)) {
                            seen[idx] |= bit;
                            ptx[ntarget] = (u8)qx; pty[ntarget] = (u8)qy;
                            ntarget++;
                        }
                    }
                }
                if (x == x1 && y == y1) break;
                e2 = 2 * err;
                if (e2 > -ay) { err -= ay; x += sx; }
                if (e2 <  ax) { err += ax; y += sy; }
            }
        }
    }
}

static void draw_text(void)
{
    i32 i;
    for (i = 0; i < (i32)ntarget; i++) px(ptx[i], pty[i]);
}

static void dot3(i32 x, i32 y)
{
    px(x - 1, y - 1); px(x, y - 1); px(x + 1, y - 1);
    px(x - 1, y);     px(x, y);     px(x + 1, y);
    px(x - 1, y + 1); px(x, y + 1); px(x + 1, y + 1);
}

/* ---- mode 0: accelerated particles -------------------------------------- */

static void draw_particles(u32 frame)
{
    i32 i, f = (i32)frame;
    for (i = 0; i < (i32)ntarget; i++) {
        i32 p, e, x, y, t = f - sp0[i];
        if (t < 0) continue;
        if (t >= 32) { px(ptx[i], pty[i]); continue; }
        p = t << 3;                       /* (t/32) * 256, 8.8 */
        e = (p * p * p) >> 16;            /* cubic ease-in, accelerating */
        x = sx[i] + ((((i32)ptx[i] - sx[i]) * e) >> 8);
        y = sy[i] + ((((i32)pty[i] - sy[i]) * e) >> 8);
        px(x, y);
    }
}

/* ---- mode 1: laser vector draw ------------------------------------------ */
static void draw_laser(u32 frame)
{
    i32 i, head, p, e, f = (i32)frame;
    if (f >= 60) { draw_text(); return; }
    p = f * 256 / 60;
    e = (p * p * p) >> 16;                /* ease-in: slow, then fast */
    head = (e * (i32)ntrace) >> 8;
    for (i = 0; i <= head; i++) dot3(trx[i], try[i]);
}

/* ---- mode 2: laser draw, then spark bloom ------------------------------- */
static const signed char DIRX[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
static const signed char DIRY[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

static void draw_bloom(u32 frame)
{
    i32 i, f = (i32)frame;
    if (f < 44) {
        i32 p = f * 256 / 44, e = (p * p * p) >> 16, head = (e * (i32)ntrace) >> 8;
        for (i = 0; i <= head; i++) dot3(trx[i], try[i]);
        return;
    }
    draw_text();
    for (i = 0; i < NS; i++) {
        i32 age = f - spk_t0[i], disp, x, y;
        if (age < 0 || age > 14) continue;
        disp = age + (age * age) / 4;     /* accelerating outward */
        x = ptx[spk_src[i]] + DIRX[spk_dir[i]] * disp;
        y = pty[spk_src[i]] + DIRY[spk_dir[i]] * disp;
        px(x, y);
    }
}

/* the intro is running (it owns PIT3): the splash may be using the arena */
int tonesplash_active(void)
{
    return PIT3_VEC == INTRO_ISR;
}

/* ---- entry ---- */
void tonesplash_draw(void)
{
    i32 i;
    static u32 frame;
    static u8 ready;
    static u8 mode;

    if (PIT3_VEC != INTRO_ISR) return;
    fb = (u8 *)FBP;
    if (!fb) return;

    if (!ready) {
        u32 seed = DTCN0;
        arena();
        rs = seed ^ 0x9e3779b9u ^ 0x85ebca6bu;
        if (rs == 0u) rs = 1u;
        mode = (u8)(rnd() % 3u);
        build_lists();
        for (i = 0; i < (i32)ntarget; i++) {
            u32 e = rnd() & 3u;
            if (e == 0u)      { sx[i] = (u8)(rnd() & 127u); sy[i] = 0; }
            else if (e == 1u) { sx[i] = (u8)(rnd() & 127u); sy[i] = 63; }
            else if (e == 2u) { sx[i] = 0; sy[i] = (u8)(rnd() & 63u); }
            else              { sx[i] = 127; sy[i] = (u8)(rnd() & 63u); }
            sp0[i] = (u8)(rnd() % 31u);
        }
        for (i = 0; i < NS; i++) {
            spk_src[i] = (u16)(rnd() % ntarget);
            spk_dir[i] = (u8)(rnd() & 7u);
            spk_t0[i]  = (u8)(44 + (rnd() & 7u));
        }
        ready = 1;
    }

    for (i = 0; i < 1024; i++) fb[i] = 0;

    if (mode == 0u)      draw_particles(frame);
    else if (mode == 1u) draw_laser(frame);
    else                 draw_bloom(frame);

    frame++;
}
