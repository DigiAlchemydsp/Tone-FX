/* Digi Mono: synth machines for the audio tracks (DESIGN.md). The render side.
 *
 * digimono_blocks() runs once a render block, after the voice loop has left each voice's 32 Q31 samples at
 * 0x80001a18 + 128 v and before the filter, amp envelope and mixer stages work on them. For every voice
 * playing one of our machines (core_track_machine 20..28; they render as ONESHOT) it writes the engine's
 * block there, so the voice's filter, amp envelope, VOL, LFOs, sends and level follow as for a sample. Firmware addresses are OS 1.53's; how each was found: docs/TECHNICAL_NOTES.md.
 */

/* ---- firmware addresses that moved in OS 1.54 (tools/port_os.py, tools/os154.json) ---- */
#ifdef OS154
#define F_40151cc2 0x40151eea
#define F_4197b6b4 0x4197c6b4
#define F_4197c7fc 0x4197d7fc
#define F_4197ce98 0x4197de98
#define F_4197cf5c 0x4197df5c
#define F_4199dc44 0x4199ec44
#define F_4199df54 0x4199ef54
#define F_4199df58 0x4199ef58
#else
#define F_40151cc2 0x40151cc2
#define F_4197b6b4 0x4197b6b4
#define F_4197c7fc 0x4197c7fc
#define F_4197ce98 0x4197ce98
#define F_4197cf5c 0x4197cf5c
#define F_4199dc44 0x4199dc44
#define F_4199df54 0x4199df54
#define F_4199df58 0x4199df58
#endif
/* ---- end of the moved addresses ---- */

#include "mono.h"
#include "digimono_ext.h"

/* the UI and the per-block glue: built for size (the samples are made in mono.c, and in other mods for
 * their machines: mods/digimono/digimono_ext.h) */
#pragma GCC optimize("Os", "no-tree-loop-distribute-patterns")

#define MACH_FIRST   20                      /* digimono_m20..m28 in glue.s: SIN NOIS SAW PULS ENS VO PSIN, 28 CHORDS */
#define MACH_SLOTS   10                      /* 20..29: Digi Mono's and other mods' (27: Digi Macro's MACRO) */
extern volatile uint8_t core_track_machine[8];                  /* core 2.1: the machine each voice plays */
#define VOICE_MACH   core_track_machine
#define VOICE_START  (*(volatile const uint32_t *)0x80001228) /* bit v: voice v (re)started this block */
#define VOICE_NOTE   ((volatile const int32_t *)0x80001f28)   /* per voice: MIDI note << 16 */
#define VOICE_WORDS  0x80002772                /* smoothed parameter words, 106 bytes a voice */
#define BLOCK(v)     ((int32_t *)(0x80001a18 + 128 * (v)))   /* voice v's 32 samples, Q31 */
/* The amp envelope's phase (0: idle) and level, per track, as the AMP stage leaves them (digisophie reads
 * the same). Not the voice gain at 0x8000edc4 + 94 v + 16: that is LEV^2 x the track level, and LEV is
 * knob H, an engine parameter here. */
#define AMP_PHASE(v) (*(volatile const int32_t *)(F_4199df54 + 12 * (v)))
#define AMP_LEVEL(v) (*(volatile const int32_t *)(F_4199df58 + 12 * (v)))
#define QUIET_BLOCKS 32                      /* 21 ms of a silent envelope: the voice sleeps */

#define SLOT_TUNE    17                        /* SRC knob A */
#define KNOBS        7
static const uint8_t knob_slot[KNOBS] = {18, 19, 21, 22, 23, 24, 20};  /* SRC knobs B C E F G H, then D */

/* Which engine parameter each knob (B C E F G H D) sets, or -1: the knob is not on the machine's page. */
static const int8_t knob_p[MONO_MACHINES][KNOBS] = {
    {-1, -1, -1, -1, -1, -1, -1},       /* SIN                                            */
    { 0,  1,  2, -1, -1, -1, -1},       /* NOIS  ST RED STON                              */
    { 0,  1,  2,  4,  5,  6, -1},       /* SAW   UNIL UNIW UNIX SUBX SUB1 SUB2            */
    { 0,  1,  2,  4,  5,  6,  3},       /* PULS  UNIL UNIW SUB1 PW PWAD PWRS, D SUB2      */
    { 0,  1,  2,  3,  5,  6,  4},       /* ENS   PCH2 PCH3 PCH4 WAVE CHRL CHRW, D PW      */
    { 0,  1,  2,  4,  5,  6,  3},       /* VO    VOC1 VOC2 V-SW CONS CLEN CVOL, D VOIC    */
    { 0,  1,  3,  4, -1, -1,  2},       /* PSIN  NOT1 NOT2 EDEP ESPD, D NOT3              */
    {-1, -1, -1, -1, -1, -1, -1},       /* (27: another mod's: digimono_ext)              */
    { 0,  1,  2,  3,  5,  6,  4},       /* CHORD TYPE INV SPRD WAVE CHRL CHRW, D PW       */
};

struct digimono_voice {
    struct mono_voice mv;
    uint8_t live;
    uint8_t quiet;                          /* blocks the amp envelope has been silent; QUIET_BLOCKS: asleep */
    uint8_t mach;                           /* the machine it last played (an id; 0: none yet)        */
    uint8_t strikes;                        /* times the guard has rested it since the render was calm */
    uint16_t rest;                          /* blocks left in which it rests (the overload guard)      */
    uint16_t cost;                          /* its last block's time, DTIM ticks (the guard)           */
    uint16_t want;                          /* resting: the time its block took before                 */
};

/* A machine's knobs: Digi Mono's own (tables below) or another mod's (its struct digimono_ext). */
struct kinfo {
    const int8_t *kp;                       /* knob -> parameter, -1: not on the page */
    const uint8_t *def;                     /* the knobs' values on a switch to it */
    const char *const *sn, *const *ln;      /* their names */
    const struct digimono_ext *x;           /* another mod's machine, or 0 */
    int m;                                  /* id - MACH_FIRST */
};
static int kinfo_of(int id, struct kinfo *k);
static struct digimono_voice voices[8];
static int shed;                            /* blocks left in which every voice rests (the overload guard) */

static int16_t word(int v, int slot)
{
    return *(volatile const int16_t *)(VOICE_WORDS + 106 * v + 2 * slot);
}

/* The engine's seven parameters from the SRC knobs B..H (DESIGN.md, "The SRC page"). */
static void params(int v, const struct kinfo *k, uint8_t *p)
{
    int i, model = k->m;
    for (i = 0; i < MONO_PARAMS; i++)
        p[i] = 0;
    for (i = 0; i < KNOBS; i++) {
        int w, j = k->kp[i];
        if (j < 0)
            continue;
        w = word(v, knob_slot[i]) >> 8;
        p[j] = w < 0 ? 0 : w > 127 ? 127 : (uint8_t)w;
    }
    if (!k->x && (model == MONO_ENS || model == MONO_CHORD))   /* PW on D: 0 = square (the engine's 64) .. 127 = thinnest */
        p[4] = (uint8_t)(64 + (p[4] >> 1));
}

static void digimono_block(int v)
{
    struct digimono_voice *d;
    struct kinfo k;
    int id, i;
    int32_t pitch;
    uint8_t p[MONO_PARAMS];
    int16_t out[32];
    if ((unsigned)v > 7)
        return;
    id = VOICE_MACH[v];
    if (!kinfo_of(id, &k))
        return;
    d = &voices[v];
    if (!d->live) {
        mono_init(&d->mv);
        d->mv.rng ^= (uint32_t)v * 0x9e3779b9u;
        d->live = 1;
        d->quiet = QUIET_BLOCKS;
    }
    if (d->mach != id) {                    /* turned to this machine: another mod's starts afresh (its
                                             * state in the chorus line's memory), and the line is
                                             * cleared for Digi Mono's after one of those */
        struct kinfo pk;
        int was_ext = d->mach && kinfo_of(d->mach, &pk) && pk.x;
        if (k.x)
            k.x->init(d->mv.dl);
        else if (was_ext)
            d->mv.own = 0;
        d->mach = (uint8_t)id;
    }
    if ((VOICE_START >> v) & 1) {
        if (k.x)
            k.x->trig(d->mv.dl);
        else
            mono_trig(&d->mv, k.m);
        d->quiet = 0;
    } else {
        int32_t l = AMP_LEVEL(v);
        if (AMP_PHASE(v) != 0 || (l < 0 ? -l : l) > (1 << 19))
            d->quiet = 0;
        else if (d->quiet < QUIET_BLOCKS)
            d->quiet++;
        if (d->quiet >= QUIET_BLOCKS)
            return;                         /* asleep: the stock block is silence already */
    }
    if (shed || d->rest)
        return;                             /* the CPU is overloaded: rest (see the overload guard) */
    /* the pitch as the render computes a sample's: the note plus TUNE (after the LFOs), in Q16 semitones */
    pitch = VOICE_NOTE[v] + ((int32_t)(word(v, SLOT_TUNE) - 16384) << 8);
    params(v, &k, p);
    if (k.x)
        k.x->render(d->mv.dl, p, mono_pitch_inc(pitch >> 9), out, 32);
    else
        mono_render(&d->mv, k.m, p, mono_pitch_inc(pitch >> 9), out, 32);
    for (i = 0; i < 32; i++)
        BLOCK(v)[i] = (int32_t)out[i] << 16;
}

/* ---- the overload guard ---------------------------------------------------------------------------- *
 * The audio render runs at a higher priority than the UI. If a render takes longer than the time between
 * two renders, the next one starts at once, the UI never runs again (no knob, no key release reaches the
 * firmware) and the outputs repeat their last buffers: a freeze only a power cycle ends. So each render is
 * timed (core's ev_render_in / ev_render_out, DTIM0: core starts it free-running at the 132 MHz bus clock;
 * a 32-frame render period at 48 kHz is 88,000 ticks), and so is each Digi Mono voice's block. In steps:
 *   1. eco: a render over 7/8 of the period sets digimono_eco: the machines take their cheaper paths (ENS
 *      and CHORDS fade their chorus out; MACRO's 2OP FM starts its notes at 2x instead of 4x, PARTCL
 *      fires four of its six particles). Two seconds of renders under 5/8 of the period end it.
 *   2. one voice: three renders in a row over 94 % (a single long one is a spike, not an overload) rest the
 *      voice whose block took longest (its block stays the stock silence, its state waits): 32 ms, twice as
 *      long each time again (up to 1 s) until the render is calm, and after that until the render has
 *      room for its block again (no blips of a voice that does not fit). The other voices play on.
 *   3. every voice: twelve renders in a row over 94 % (the load is not Digi Mono's, or resting voices
 *      did not help) rest them all for 16 ms, which gives the CPU back: a dropout instead of a freeze. */
#define DTCN0        (*(volatile const uint32_t *)0xFC07000C)
#define RENDER_TICKS 88000u
#define ECO_LIMIT    (RENDER_TICKS - RENDER_TICKS / 8)
#define CALM_LIMIT   (RENDER_TICKS * 5 / 8)
#define CALM_BLOCKS  3000                       /* 2 s */
#define SHED_LIMIT   (RENDER_TICKS - RENDER_TICKS / 16)
#define REST_BLOCKS  48                         /* 32 ms, doubled with each strike */
#define REST_STRIKES 5                          /* up to 1 s */
#define SHED_BLOCKS  24                         /* 16 ms */
static uint32_t rin_t;
static int over;                                /* renders over the limit in a row */
static int calm;                                /* renders under CALM_LIMIT in a row */
uint32_t digimono_overloads;                    /* voices rested since power-on (diagnostics) */

void digimono_rin(void)
{
    rin_t = DTCN0;
}

/* rest the voice whose last block took longest; 0 if none is playing */
static int rest_heaviest(void)
{
    struct digimono_voice *h = 0;
    int v;
    for (v = 0; v < 8; v++) {
        struct digimono_voice *d = &voices[v];
        if (!d->rest && d->cost > 400 && (!h || d->cost > h->cost))   /* (400: a voice asleep) */
            h = d;
    }
    if (!h)
        return 0;
    h->rest = (uint16_t)(REST_BLOCKS << h->strikes);
    h->want = h->cost;
    if (h->strikes < REST_STRIKES)
        h->strikes++;
    digimono_overloads++;
    return 1;
}

void digimono_rout(void)
{
    uint32_t dur = DTCN0 - rin_t;
    int v;
    if (dur >= 0x80000000u)
        dur = 0;
    if (dur > ECO_LIMIT) {
        digimono_eco = 1;
        calm = 0;
    } else if (dur < CALM_LIMIT) {
        if (++calm >= CALM_BLOCKS) {
            calm = 0;
            digimono_eco = 0;
            for (v = 0; v < 8; v++)
                voices[v].strikes = 0;
        }
    } else {
        calm = 0;
    }
    if (dur > SHED_LIMIT) {
        ++over;
        if (over >= 12) {
            shed = SHED_BLOCKS;
            over = 0;
            digimono_overloads++;
        } else if (over % 3 == 0) {
            rest_heaviest();
        }
    } else {
        int back = 0;                           /* one voice back a render, if there is room for it */
        over = 0;
        if (shed)
            shed--;
        for (v = 0; v < 8; v++) {
            struct digimono_voice *d = &voices[v];
            if (d->rest > 1)
                d->rest--;
            else if (d->rest && !back && dur + d->want < SHED_LIMIT)
                d->rest = 0, back = 1;
        }
    }
}

/* The render's own stack (glue.s digimono_rblock): the engines' deepest calls take about 2 KB (MACRO's
 * drums 1.9 KB), the rest is room for the interrupts above the render's level. */
#define DIGIMONO_STACK 4096
uint32_t digimono_stack[DIGIMONO_STACK / 4] __attribute__((aligned(16)));
__asm__(".globl digimono_stack_top\n\t.set digimono_stack_top, digimono_stack + " "4096");

void digimono_blocks(void)
{
    int v;
    for (v = 0; v < 8; v++) {
        uint32_t t = DTCN0;
        digimono_block(v);
        t = DTCN0 - t;
        voices[v].cost = (uint16_t)(t > 0xffffu ? 0xffffu : t);
    }
}

/* ---- the SRC page ------------------------------------------------------------------------------- */

#define ACTIVE_TRACK (*(volatile const uint32_t *)F_4197b6b4)
#define UI_KIT       (*(uint8_t *volatile const *)F_4199dc44)   /* its sounds: + 0x20 + 0xa2 t */
#define SLICE_ID_B   0x85                      /* SLICE's SRC page: ids 0x84..0x8b = knobs A..H */
#define FMT_INT      ((void *)F_4197c7fc)      /* the firmware's plain-number formatter (BR's) */
#define FMT_BUF      ((char *)F_4197ce98)      /* where 0x400657ee writes a value's text */
typedef void (*fmt_t)(void *fmt, int32_t value, char *buf);
#define FORMAT       ((fmt_t)F_40151cc2)

/* the knobs: B C E F G H D ("-": not on the page) */
static const char *const sname[MONO_MACHINES][KNOBS] = {
    {"-",    "-",    "-",    "-",    "-",    "-",    "-"},
    {"ST",   "RED",  "STON", "-",    "-",    "-",    "-"},
    {"UNIL", "UNIW", "UNIX", "SUBX", "SUB1", "SUB2", "-"},
    {"UNIL", "UNIW", "SUB1", "PW",   "PWAD", "PWRS", "SUB2"},
    {"PCH2", "PCH3", "PCH4", "WAVE", "CHRL", "CHRW", "PW"},
    {"VOC1", "VOC2", "V-SW", "CONS", "CLEN", "CVOL", "VOIC"},
    {"NOT1", "NOT2", "EDEP", "ESPD", "-",    "-",    "NOT3"},
    {"-",    "-",    "-",    "-",    "-",    "-",    "-"},     /* (27: another mod's) */
    {"TYPE", "INV",  "SPRD", "WAVE", "CHRL", "CHRW", "PW"},
};
static const char *const lname[MONO_MACHINES][KNOBS] = {
    {"-", "-", "-", "-", "-", "-", "-"},
    {"Sample Hold Rate", "Red Noise", "Tuned Noise", "-", "-", "-", "-"},
    {"Unison Level", "Unison Detune", "Unison Voices", "Sub Shape", "Sub 1 Oct Lev", "Sub 2 Oct Lev", "-"},
    {"Unison Level", "Unison Detune", "Sub 1 Oct Lev", "Pulse Width", "PWM Depth", "PWM Rate", "Sub 2 Oct Lev"},
    {"Pitch Osc 2", "Pitch Osc 3", "Pitch Osc 4", "Saw-Pulse", "Chorus Level", "Chorus Width", "Pulse Width"},
    {"Vowel 1", "Vowel 2", "Vowel Glide", "Consonant", "Cons. Length", "Cons. Level", "Breath"},
    {"Note 1", "Note 2", "Env Depth", "Env Speed", "-", "-", "Note 3"},
    {"-", "-", "-", "-", "-", "-", "-"},
    {"Chord Type", "Inversion", "Spread", "Sine-Saw-Pulse", "Chorus Level", "Chorus Width", "Pulse Width"},
};

/* how each knob's value reads */
enum { F_NUM, F_VOICES, F_SEMI, F_PW, F_PWENS, F_VOWEL, F_CONS, F_MS, F_SHAPE, F_BIPST, F_BIPOFF,
       F_CHORD, F_INV, F_SPRD, F_CHWAVE };
static const uint8_t knob_fmt[MONO_MACHINES][KNOBS] = {
    {F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM},
    {F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM},
    {F_NUM, F_NUM, F_VOICES, F_SHAPE, F_NUM, F_NUM, F_NUM},
    {F_NUM, F_NUM, F_NUM, F_PW, F_NUM, F_NUM, F_NUM},
    {F_SEMI, F_SEMI, F_SEMI, F_SHAPE, F_NUM, F_NUM, F_PWENS},
    {F_VOWEL, F_VOWEL, F_NUM, F_CONS, F_MS, F_NUM, F_NUM},
    {F_SEMI, F_SEMI, F_BIPST, F_BIPOFF, F_NUM, F_NUM, F_SEMI},
    {F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM, F_NUM},
    {F_CHORD, F_INV, F_SPRD, F_CHWAVE, F_NUM, F_NUM, F_PWENS},
};

/* The active track's machine's knobs, if it is one here (Digi Mono's or another mod's); else 0. */
static int active_model(struct kinfo *ki)
{
    uint32_t t = ACTIVE_TRACK;
    const uint8_t *kit = UI_KIT;
    if (t > 7 || !kit)
        return 0;
    return kinfo_of(kit[0x9e + 0xa2 * t], ki);
}

/* A SLICE page id -> our knob 0..6 (B C E F G H D), or -1 (A = TUNE, anything else). */
static int knob_of(uint32_t id)
{
    static const int8_t k[7] = {0, 1, 6, 2, 3, 4, 5};      /* ids 0x85..0x8b: B C D E F G H */
    return id >= SLICE_ID_B && id < SLICE_ID_B + 7 ? k[id - SLICE_ID_B] : -1;
}

/* The knob of id on the active Digi Mono page, if the machine has it there; else -1. */
static int our_knob(uint32_t id, struct kinfo *ki)
{
    int k = knob_of(id);
    if (!active_model(ki) || k < 0 || ki->kp[k] < 0)
        return -1;
    return k;
}

int digimono_ours(uint32_t id)
{
    struct kinfo ki;
    return our_knob(id, &ki) >= 0;
}

const char *digimono_name(uint32_t id, int shortname)
{
    struct kinfo ki;
    int k = our_knob(id, &ki);
    if (k < 0)
        return 0;
    return shortname ? ki.sn[k] : ki.ln[k];
}

/* ---- the LFO page: DEST's list and box name the SRC knobs as the SRC page does ------------------
 * These machines take ONESHOT's parameters, ids 0x6c..0x73 = knobs A..H, sound slots 17..24; an LFO's
 * DEST is the slot, so "SAMP:Play Mode" is knob B, the one the engine reads as UNIL on MONO SAW. The
 * firmware names a destination from its parameter descriptor (0x401a9d9c + 52 id: long name +40, group
 * +44, short name +48); glue.s hands those reads here with the descriptor's offset (52 id). */
#define ONESHOT_ID_A 0x6c
static const char *const mach_short[MONO_MACHINES] = {"MSIN", "MNOI", "MSAW", "MPLS", "MENS", "MVO", "PSIN",
                                                      "-", "CHRD"};

/* what: 0 the group ("SAMP"), 1 the long name, 2 the short one; stock: the firmware's own */
const char *digimono_dest(uint32_t off, int what, const char *stock)
{
    struct kinfo ki;
    int k, j;
    for (k = 0; k < 8 && off != (uint32_t)(ONESHOT_ID_A + k) * 52; k++)
        ;
    if (!active_model(&ki) || k == 8)
        return stock;
    if (what == 0)
        return ki.x ? ki.x->mshort : mach_short[ki.m];
    if (k == 0)                                 /* A: TUNE, as on the SRC page */
        return stock;
    j = our_knob(SLICE_ID_B - 1 + k, &ki);      /* SLICE's id of the same knob */
    if (j < 0)
        return what == 1 ? "Unused" : "-";
    return what == 1 ? ki.ln[j] : ki.sn[j];
}

int digimono_range(uint32_t id, int32_t *out)
{
    struct kinfo ki;
    if (our_knob(id, &ki) < 0)
        return 0;
    out[0] = 0;
    out[1] = 127 << 8;
    out[2] = 0;
    return 1;
}

/* CHORDS: inversion and spread (mono.c mono_chord_notes: 32 values a step) */
static const char *const inv_name[4] = {"ROOT", "1ST", "2ND", "3RD"};
static const char *const sprd_name[4] = {"CLOSE", "OPEN", "WIDE", "XWIDE"};

/* VO's vowels (VOC1, VOC2: the continuum of mono.c's VOWEL, nearest) and consonants (CONS: 8 zones) */
static const char *const vowel_name[10] = {"OO", "U", "AW", "AH", "UH", "AE", "EH", "IH", "EE", "ER"};
static const char *const cons_name[8] = {"-", "S", "SH", "F", "H", "T", "K", "P"};

static char *put_s(char *o, const char *t)
{
    while ((*o = *t++))
        o++;
    return o;
}

static char *put_u(char *o, uint32_t n)
{
    char d[10];
    int i = 0;
    do {
        d[i++] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    while (i)
        *o++ = d[--i];
    *o = 0;
    return o;
}

/* A knob's value v (0..127) as text: units where it has them. */
static void text_into(const struct kinfo *ki, int k, int v, char *buf)
{
    char *o = buf;
    if (ki->x) {                            /* another mod's machine: its text, or the number */
        if (!ki->x->text(k, v, buf))
            put_u(o, (uint32_t)v);
        return;
    }
    switch (knob_fmt[ki->m][k]) {
    case F_VOICES:                          /* mono.c render_saw: 1 to 3 unison saws */
        o = put_u(o, v < 43 ? 1 : v < 86 ? 2 : 3);
        put_s(o, v < 43 ? " SAW" : " SAWS");
        break;
    case F_SEMI: {                          /* render_ens: 63 = the main oscillator's pitch */
        int s = v - 63;
        s = s < -36 ? -36 : s > 36 ? 36 : s;
        if (s > 0)
            *o++ = '+';
        if (s < 0) {
            *o++ = '-';
            s = -s;
        }
        o = put_u(o, (uint32_t)s);
        put_s(o, "st");
        break;
    }
    case F_PW:                              /* duty(): 50 % at 64, 0.763 % a step */
        o = put_u(o, (uint32_t)((50 * 131 + (v - 64) * 100 + 65) / 131));
        put_s(o, "%");
        break;
    case F_PWENS:                           /* D 0..127 -> the engine's 64..127 */
        v = 64 + (v >> 1);
        o = put_u(o, (uint32_t)((50 * 131 + (v - 64) * 100 + 65) / 131));
        put_s(o, "%");
        break;
    case F_VOWEL:
        put_s(o, vowel_name[(v * 9 * 2 + 127) / 254]);
        break;
    case F_CONS:
        put_s(o, cons_name[v >> 4]);
        break;
    case F_CHORD:
        put_s(o, mono_chord_name[mono_chord_type(v)]);
        break;
    case F_INV:
        put_s(o, inv_name[v >> 5]);
        break;
    case F_SPRD:
        put_s(o, sprd_name[v >> 5]);
        break;
    case F_CHWAVE:                          /* render_chord: 0..15 sines, then saw..pulse in % */
        if (v < 16) {
            put_s(o, "SINE");
            break;
        }
        o = put_u(o, (uint32_t)(((v - 16) * 100 + 55) / 111));
        put_s(o, "%");
        break;
    case F_MS:                              /* render_vo: CLEN 96 + v^2 x 1188 / 1024 samples */
        o = put_u(o, (uint32_t)((96 + ((v * v * 1188) >> 10)) / 48));
        put_s(o, "ms");
        break;
    case F_BIPST:                           /* render_psin: EDEP, 64 = none, in semitones */
    case F_BIPOFF: {                        /* render_psin: ESPD, 64 = off, - rises, + decays */
        int s = v - 64;
        if (s == 0 && knob_fmt[ki->m][k] == F_BIPOFF) {
            put_s(o, "OFF");
            break;
        }
        if (s > 0)
            *o++ = '+';
        if (s < 0) {
            *o++ = '-';
            s = -s;
        }
        o = put_u(o, (uint32_t)s);
        if (knob_fmt[ki->m][k] == F_BIPST)
            put_s(o, "st");
        break;
    }
    case F_SHAPE:                           /* SUBX square..saw, WAVE saw..pulse: a mix, in % */
        o = put_u(o, (uint32_t)((v * 100 + 63) / 127));
        put_s(o, "%");
        break;
    default:
        put_u(o, (uint32_t)v);
    }
}

/* The value pop-up's text (0x400657ee's callers): FMT_BUF, as the firmware's own. */
char *digimono_text(uint32_t id, int32_t value)
{
    struct kinfo ki;
    int k = our_knob(id, &ki);
    if (k < 0)
        return 0;
    text_into(&ki, k, (value >> 8) & 0x7f, FMT_BUF);
    return FMT_BUF;
}

/* A knob's value under it while it turns (0x4000f324, through digichain): into buf; 0 if not ours. */
int digimono_knob_text(uint32_t id, int32_t value, char *buf)
{
    struct kinfo ki;
    int k = our_knob(id, &ki);
    if (k < 0)
        return 0;
    text_into(&ki, k, (value >> 8) & 0x7f, buf);
    return 1;
}

/* The SRC page's layout for a Digi Mono machine (0x400657cc, through digichain): SLICE's, with the
 * knobs the machine does not have emptied (id 0), so they show nothing and turn nothing. 0: not ours. */
#define LAY_SLICE    ((const uint32_t *)F_4197cf5c)    /* 44 bytes; the knobs' ids at +8 (A..H) */
static uint32_t lay[MACH_SLOTS][11];
static uint8_t lay_ok[MACH_SLOTS];

uint32_t *digimono_layout_for(uint32_t machine)
{
    static const uint8_t at[KNOBS] = {3, 4, 6, 7, 8, 9, 5};   /* B C E F G H D: longs at +12 .. +36 */
    struct kinfo ki;
    int m, i;
    if (!kinfo_of((int)machine, &ki))
        return 0;
    m = ki.m;
    if (!lay_ok[m]) {
        for (i = 0; i < 11; i++)
            lay[m][i] = LAY_SLICE[i];
        for (i = 0; i < KNOBS; i++)
            if (ki.kp[i] < 0)
                lay[m][at[i]] = 0;
        lay_ok[m] = 1;
    }
    return lay[m];
}

/* ---- defaults on a switch ------------------------------------------------------------------------ */
/* Switching a track's machine gives it the parameters' machine's defaults (ONESHOT's: core 2.1), which
 * mean nothing here (ENS would start 36 semitones down). digimono_tick (ev_tick, 30 Hz) watches the UI
 * kit: when a track of the same kit turns into a Digi Mono machine while its knobs still hold switch
 * defaults (ONESHOT's for E F G H, which a stock switch resets, or the Digi Mono machine it was before: the machine
 * list switches as its cursor moves), it sets the new machine's own, through the firmware's knob path
 * (0x400771e8, the engine's copy) and in the kit (what is saved and shown). A kit or sound load that
 * brings its own values matches neither, or comes with a new kit, and is left alone. */
typedef void (*setparam_t)(int32_t value, int32_t voice, int32_t slot);
#define SETPARAM     ((setparam_t)0x400771e8)

static const uint16_t oneshot_def[6] = {0x0300, 0x0000, 0x0000, 0x7800, 0x0000, 0x6400};  /* B C E F G H */
static const uint8_t mono_def[MONO_MACHINES][KNOBS] = {   /* B C E F G H D */
    {0, 0, 0, 0, 0, 0, 0},              /* SIN                                         */
    {0, 0, 0, 0, 0, 0, 0},              /* NOIS  ST RED STON                           */
    {0, 40, 0, 0, 0, 0, 0},             /* SAW   UNIL UNIW UNIX SUBX SUB1 SUB2         */
    {0, 40, 0, 64, 0, 40, 0},           /* PULS  UNIL UNIW SUB1 PW PWAD PWRS, SUB2     */
    {63, 63, 63, 0, 0, 127, 0},         /* ENS   PCH2 PCH3 PCH4 WAVE CHRL CHRW, PW (square) */
    {43, 113, 64, 0, 40, 100, 0},       /* VO    VOC1 (AH) VOC2 (EE) V-SW CONS CLEN CVOL, VOIC */
    {63, 63, 64, 64, 0, 0, 63},         /* PSIN  NOT1 NOT2, EDEP ESPD off, D NOT3: MONO SIN */
    {0, 0, 0, 0, 0, 0, 0},              /* (27: another mod's: digimono_ext)           */
    {0, 0, 0, 16, 0, 127, 0},           /* CHORD MAJ, ROOT, CLOSE, saw, chorus off, PW (square) */
};

/* A machine's knobs, if it is one here: Digi Mono's own, or another mod's (digimono_ext). */
static int kinfo_of(int id, struct kinfo *k)
{
    const struct digimono_ext *const *e;
    int m = id - MACH_FIRST;
    if ((unsigned)m >= MACH_SLOTS)
        return 0;
    k->m = m;
    if (m < MONO_MACHINES && m != MONO_EXT7) {
        k->kp = knob_p[m];
        k->def = mono_def[m];
        k->sn = sname[m];
        k->ln = lname[m];
        k->x = 0;
        return 1;
    }
    for (e = digimono_ext; *e; e++)
        if ((*e)->id == (uint32_t)id && (*e)->state_size <= DIGIMONO_EXT_STATE) {
            k->x = *e;
            k->kp = k->x->knob_p;
            k->def = k->x->def;
            k->sn = k->x->sname;
            k->ln = k->x->lname;
            return 1;
        }
    return 0;
}

static const uint8_t *seen_kit;
static uint8_t seen_mach[8];

static uint16_t knobw(const uint8_t *snd, int i)
{
    return *(const uint16_t *)(snd + 0x14 + 2 * knob_slot[i]);
}

/* Do knobs B..H hold what a machine switch leaves there? prev: the machine before (id). */
static int switch_defaults(const uint8_t *snd, int prev)
{
    struct kinfo ki;
    int i;
    if (kinfo_of(prev, &ki)) {
        for (i = 0; i < KNOBS; i++)
            if (knobw(snd, i) != (uint16_t)(ki.def[i] << 8))
                break;
        if (i == KNOBS)
            return 1;
    }
    for (i = 2; i < 6; i++)                 /* a stock switch resets E..H; B and C it leaves or clears */
        if (knobw(snd, i) != oneshot_def[i])
            return 0;
    return 1;
}

void digimono_tick(void *ctrl)
{
    uint8_t *kit = UI_KIT;
    int t, i;
    (void)ctrl;
    if (!kit)
        return;
    if (kit != seen_kit) {                  /* another pattern's kit: take it as it is */
        for (t = 0; t < 8; t++)
            seen_mach[t] = kit[0x9e + 0xa2 * t];
        seen_kit = kit;
        return;
    }
    for (t = 0; t < 8; t++) {
        uint8_t *snd = kit + 0x20 + 0xa2 * t;
        struct kinfo ki;
        int m = snd[0x7e], prev = seen_mach[t];
        if (m == prev)
            continue;
        seen_mach[t] = m;
        if (!kinfo_of(m, &ki) || !switch_defaults(snd, prev))
            continue;
        for (i = 0; i < KNOBS; i++) {
            int32_t w = (int32_t)ki.def[i] << 8;
            *(uint16_t *)(snd + 0x14 + 2 * knob_slot[i]) = (uint16_t)w;
            SETPARAM(w, t, knob_slot[i]);
        }
    }
}
