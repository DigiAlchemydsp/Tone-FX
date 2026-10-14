/* Digi Macro's MACRO machine (once part of Digi Mono): synthesis engines ported from Plaits (Emilie Gillet, MIT licence), in 32-bit
 * integer arithmetic for the ColdFire (no FPU, no 64-bit maths, no library calls). See macro.c for the
 * licence text and mods/digimacro/MACRO.md for the engines and how each was checked against the original.
 *
 * One machine, several engines: knob B picks the engine. Each engine plays Plaits' main output (OUT); its
 * second output (AUX) is left out, for room and speed. It is read at each note start (and while a voice
 * has not started yet), so a p-lock on B changes the engine on that trig and a turning knob on the next.
 */
#ifndef MACRO_H
#define MACRO_H

#include <stdint.h>

enum {
    MACRO_WSH = 0,          /* waveshaping: a slope oscillator through a waveshaper and a wavefolder */
    MACRO_FM  = 1,          /* 2-operator FM with feedback, 4x oversampled (2x without feedback)    */
    MACRO_NOISE = 2,        /* clocked noise through a LP-to-HP filter                              */
    MACRO_PARTICLE = 3,     /* random impulses through resonant band-passes                        */
    MACRO_BD = 4,           /* an analog-style bass drum                                            */
    MACRO_SD = 5,           /* an analog-style snare drum                                           */
    MACRO_HH = 6,           /* an 808-style hi-hat: six square oscillators                         */
    MACRO_GRAIN = 7,        /* granular formants: two grainlets                                     */
    MACRO_ENGINES
};

/* Knob B has 16 zones of 8 values, one an engine, so a saved ENGN value keeps its engine as engines are
 * added; the zones past the last engine play the last one. */
#define MACRO_ZONE_SHIFT 3

/* The parameters, raw 0..127: */
enum {
    MACRO_P_ENGINE = 0,     /* B  ENGN  the engine (MACRO_ENGINES zones over 0..127)               */
    MACRO_P_HARM   = 1,     /* C  HARM  Plaits' HARMONICS                                          */
    MACRO_P_TIMB   = 2,     /* D  TIMB  Plaits' TIMBRE                                             */
    MACRO_P_MORPH  = 3,     /* E  MORP  Plaits' MORPH                                              */
                            /* (4: F, unused since 0.18; it was AUX)                               */
    MACRO_PARAMS   = 7
};

/* Every engine's state below is made of 32-bit words only (the tests turn a voice's state round between
 * the ColdFire's byte order and a PC's word by word). */

/* The slope oscillator (Plaits' Oscillator, shape SLOPE), Q16 samples. */
struct macro_slope {
    uint32_t phase;
    int32_t  next;                  /* the next sample's value so far, Q16 (0..1 = 0..65536)          */
    int32_t  high;                  /* 1 while on the rising slope                                   */
};

struct macro_wsh {
    struct macro_slope slope;
    int32_t prev_shape, prev_gain;                  /* Q15, the values the last block ended on        */
};

struct macro_fm {
    uint32_t carrier, modulator;                    /* phases                                        */
    int32_t  prev;                                  /* the carrier, one-pole filtered (the feedback)  */
    int32_t  hc[6];                                 /* 2x: the carrier's last six samples             */
    int32_t  car_fir;                               /* 4x: the downsampler's carried half, Q30        */
    int32_t  prev_amount, prev_feedback;            /* Q15 (amount: Q14)                              */
    int32_t  os4;                                   /* this note runs 4x (it started with feedback)   */
};

/* Plaits' ClockedNoise: random values held at a clock's rate, band-limited steps (polyBLEP) */
struct macro_cnoise {
    uint32_t phase;
    int32_t  sample, next;                          /* Q15                                           */
};

/* stmlib's Svf (a trapezoidal state-variable filter), states Q24 */
struct macro_svf {
    int32_t s1, s2;
};

struct macro_noise {
    struct macro_cnoise src;
    struct macro_svf mm;                            /* the LP-to-HP filter                            */
    int32_t sync;                                   /* a note started: restart both clocks           */
};

/* Plaits' Particle: impulses at random times, each particle's band-pass (stmlib's Svf, Q24) */
#define MACRO_PARTICLES 6
struct macro_particle {
    int32_t e;                                      /* the time to the next impulse, in exponential units, Q26 */
    int32_t s1, s2;                                 /* the band-pass's states                          */
    int32_t a1, a2, a3;                             /* its coefficients (struct svf_c in macro.c)      */
    int32_t c1m, c1s, c2m, c2s;                     /* an impulse's step into bp (and s1), lp (and s2)  */
};

struct macro_particles {
    struct macro_particle p[MACRO_PARTICLES];
    struct macro_svf post;                          /* the low-pass after them                         */
    int32_t sync;
};

/* Plaits' bass drum engine, OUT: AnalogBassDrum, Q24 */
struct macro_bd {
    int32_t trig;
    int32_t idle;                                   /* the drum has died away: nothing computed       */
    int32_t pulse_left, fm_left, pulse, pulse_lp, fm_lp, retrig, lp_out, tone_lp;
    struct macro_svf res;
    int32_t a1, a2, a3;                             /* the resonator's coefficients now               */
};

/* Plaits' snare drum engine, OUT: AnalogSnareDrum, Q24 */
struct macro_sd {
    int32_t trig;
    int32_t idle;
    /* five modes' resonators, the noise's band-pass; coefficients kept for the block's knobs */
    int32_t pulse_left, pulse, pulse_lp, noise_env;
    struct macro_svf mode[5], nf;
    int32_t ma1[5], ma2[5], ma3[5], na1, na2, na3;
    uint32_t key_inc;
    int32_t key_knobs;
};

/* Plaits' hi-hat engine, OUT: a HiHat with SquareNoise, Q24 */
struct macro_hh {
    int32_t trig;
    int32_t idle;
    int32_t env;
    uint32_t nclk;
    int32_t nsmp;
    struct macro_svf bp, hp;
    uint32_t sq[6];                                 /* SquareNoise's phases                             */
    int32_t key_timb;                               /* the filters' coefficients for this TIMBRE:      */
    int32_t kc[2][4];                               /* the band-pass, the high-pass (svf_c)             */
};

/* Plaits' grain engine, OUT: GrainletOscillator x 2, Q24 samples */
struct macro_grain {
    uint32_t gc[2], gf[2];                          /* the grainlets' carrier and formant phases (Q32)  */
    int32_t gnext[2];
    int32_t dc;                                     /* the DC blocker's state                          */
};

struct macro_voice {
    uint8_t engine;                 /* the engine playing                                            */
    uint8_t latch;                  /* 1: take the engine from knob B at the next block              */
    uint8_t pad[2];                 /* (four bytes, then 32-bit words)                               */
    uint32_t rng;                   /* the voice's random numbers (stmlib's Random)                  */
    int32_t lim_out;                /* the limiter's peak (engines Plaits limits), Q17               */
    union {
        struct macro_wsh wsh;
        struct macro_fm fm;
        struct macro_noise noise;
        struct macro_particles part;
        struct macro_bd bd;
        struct macro_sd sd;
        struct macro_hh hh;
        struct macro_grain grain;
    } e;
};

void macro_init(struct macro_voice *m);
void macro_trig(struct macro_voice *m);
/* Render n samples (n <= 32) of 16-bit mono with parameters p[0..6] at phase increment inc (a 32-bit
 * phase step per 48 kHz sample). */
void macro_render(struct macro_voice *m, const uint8_t *p, uint32_t inc, int16_t *out, int n);
/* The engine knob B's value -> its engine, and the engines' names as the knob shows them. */
int macro_engine_of(int b);
extern const char *const macro_engine_name[MACRO_ENGINES];
/* one cycle of a sine in 512 steps, Q15 (+ 128 more: a cosine reads it a quarter on): Digi Mono's (its sine.c),
 * which Digi Macro imports */
extern const int16_t MACRO_SINE[641];

/* Digi Mono's: 1 while the audio render is short of time (its overload guard). 2OP FM then starts its notes
 * at 2x, PARTCL fires four of its six particles. */
extern uint8_t digimono_eco;

#endif
