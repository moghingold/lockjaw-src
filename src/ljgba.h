/* GBA frontend for LOCKJAW, an implementation of the Soviet Mind Game

Copyright (C) 2006 Damian Yerrick <tepples+lj@spamcop.net>

This work is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Original game concept and design by Alexey Pajitnov.
The Software is not sponsored or endorsed by Alexey Pajitnov, Elorg,
or The Tetris Company LLC.

*/

#ifndef LJGBA_H
#define LJGBA_H

#include <gba.h>
#include <stdlib.h>
#include "ljcontrol.h"

struct LJPCView {
  const u16 *sndData[4];
  u8 sndLeft[4];
};

void textout(const char *str, int x, int y, int c);
void isr(void);
void cls(void);
void vsync(void);
LJBits readPad(void);
extern volatile int curTime;

void install_sound(struct LJPCView *v);
void gba_poll_sound(struct LJPCView *v);
void gba_play_sound(struct LJPCView *v, int effect);

extern unsigned char prefs[];
void initOptions(void);
void options(LJView *view);
#endif
