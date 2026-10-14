| SPDX-License-Identifier: GPL-2.0-or-later
| Tone+FX, DT1 port: where the effects meet the render (0x400721e6, OS 1.53 and 1.54).
| Replaces "move.l #0x8000ea70,d4", the first instruction of the stage that spreads the master pair over
| the outputs (the same site as Digi EQ's): from here it goes to the analog conversion for the main
| outputs and the headphones, and into the 12-channel bus the USB stream is built from. The stage's
| registers are live here (d0-d2 the mix coefficients, d3, a2, a3), so everything the C code may use is
| kept; it touches neither the EMAC nor its accumulators. The replaced instruction sets d4 and the CCR
| at the end, as it did.
        .section .run, "ax"
        .balign 2
        .globl  tonefx_master_glue
tonefx_master_glue:
        lea     -16(%sp), %sp
        movem.l %d0-%d1/%a0-%a1, (%sp)
        pea     0x8000ea70
        jsr     tonefx_master
        addq.l  #4, %sp
        movem.l (%sp), %d0-%d1/%a0-%a1
        lea     16(%sp), %sp
        move.l  #0x8000ea70, %d4                | the instruction this replaced
        rts
