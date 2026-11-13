/* SPDX-License-Identifier: GPL-2.0-or-later
 * digiwrmz: the buffer-FX DELAY engine (one line, WRMZ).
 * Exports its parameters and the render handlers; the DELAYS page owns them.
 */
#ifndef DIGIWRMZ_H
#define DIGIWRMZ_H

extern int digiwrmz_on, digiwrmz_time, digiwrmz_fb, digiwrmz_tone;
extern int digiwrmz_mod, digiwrmz_rate, digiwrmz_mix;

void digiwrmz_render_in(void);
void digiwrmz_render_out(void);

#endif