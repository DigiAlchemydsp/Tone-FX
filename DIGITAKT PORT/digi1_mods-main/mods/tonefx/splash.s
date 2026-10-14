| SPDX-License-Identifier: GPL-2.0-or-later
| TONE+FX: at each of the intro's panel presents, draw our animation over
| the frame the intro just composed, then run the stock present.
| Based on Digital Alchemy DSP's splash.s for the original (github.com/DigiAlchemydsp/RingTone).
|
| DT1 port (OS 1.53; tools/port_os.py names the 1.54 addresses): the sites are the
| intro's own `jsr <present>` calls (0x400e60e2 is the delta present, panel_diff, called from the
| intro at 0x4006c2d0, 0x4006c3ac, 0x4006c864 and 0x4006cb68; 0x400e6064 the full flush, called at
| 0x4006c3a4, 0x4006c85c and 0x4006cb60), so the stock present functions are left intact.
| See render.c and mod.json.

| ---- firmware addresses that moved in OS 1.54 (tools/port_os.py, tools/os154.json) ----
        .ifdef  OS154
        .equ    .LF_400e6064, 0x400e628c
        .equ    .LF_400e60e2, 0x400e630a
        .else
        .equ    .LF_400e6064, 0x400e6064
        .equ    .LF_400e60e2, 0x400e60e2
        .endif
| ---- end of the moved addresses ----


        .section .run, "ax"

        .globl  tonesplash_diff
        .globl  tonesplash_flush

tonesplash_diff:
        jsr     tonesplash_draw
        jmp     .LF_400e60e2

tonesplash_flush:
        jsr     tonesplash_draw
        jmp     .LF_400e6064
