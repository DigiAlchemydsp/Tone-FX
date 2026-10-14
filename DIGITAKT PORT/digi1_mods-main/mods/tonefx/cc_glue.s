| SPDX-License-Identifier: GPL-2.0-or-later
| Digi FX MIDI CC entry - the DT1 port (OS 1.53; tools/port_os.py names the 1.54 addresses).
| Based on Digital Alchemy DSP's cc_glue.s for the original (github.com/DigiAlchemydsp/RingTone).
|
| Every incoming MIDI Control Change message reaches the DT1's CC handler 0x400d4a90(msg) once,
| before it is matched to the tracks' channels: msg+0 the status byte, +1 the CC number, +2 the value.
| It first tests the CC receive gate 0x4197b77c and does nothing while it is clear. Its first two
| instructions are
|     0x400d4a90  lea      -32(%sp),%sp
|     0x400d4a94  movem.l  %d2-%d7/%a2-%a3,(%sp)
| mod.json replaces those 8 bytes with `jmp digictl_cc_disp` (+ a nop), so on entry the stack is the
| stock one: sp@(0) the return address, sp@(4) msg. When the gate is open we call digictl_cc_apply
| (digictl.c) with the CC number and value; it ignores the numbers it does not own. Then the two
| displaced instructions, and the stock handler goes on at 0x400d4a98 (so stock sees every CC too).

| ---- firmware addresses that moved in OS 1.54 (tools/port_os.py, tools/os154.json) ----
        .ifdef  OS154
        .equ    .LF_400d4a98, 0x400d4cc0
        .equ    .LF_4197b77c, 0x4197c77c
        .else
        .equ    .LF_400d4a98, 0x400d4a98
        .equ    .LF_4197b77c, 0x4197b77c
        .endif
| ---- end of the moved addresses ----


        .section .run, "ax"
        .balign 2
        .globl  digictl_cc_disp
digictl_cc_disp:
        tst.l   .LF_4197b77c                  | CC receive off: leave it all to stock
        beq.s   1f
        move.l  4(%sp), %a0                 | msg
        moveq   #0, %d0
        move.b  2(%a0), %d0                 | value
        move.l  %d0, -(%sp)
        moveq   #0, %d0
        move.b  1(%a0), %d0                 | CC number
        move.l  %d0, -(%sp)
        jsr     digictl_cc_apply            | digictl.c (keeps d2-d7/a2-a6)
        addq.l  #8, %sp
1:      lea     -32(%sp), %sp               | displaced instruction 1
        movem.l %d2-%d7/%a2-%a3, (%sp)      | displaced instruction 2
        jmp     .LF_400d4a98                  | the stock handler goes on
