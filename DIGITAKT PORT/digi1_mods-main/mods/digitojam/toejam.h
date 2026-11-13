/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef TOEJAM_H
#define TOEJAM_H

void toejam_render_in(void);
void toejam_render_out(void);
void toejam_down(int t);
void toejam_up(int t);
int toejam_held(int t);
void toejam_all_off(void);
int toejam_pad(void);
int toejam_bpm(void);
void toejam_enc(int id, int d);
void toejam_draw(void *bmp);

#endif
