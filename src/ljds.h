/* DS frontend for LOCKJAW, an implementation of the Soviet Mind Game

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

#include <nds.h>
#include <stdlib.h>
#include "ljcontrol.h"

//#define SPR_VRAM(x) ((u32 *)(0x06420000 + 32 * x))
#define SPR_VRAM(x) ((u32 *)(0x06400000 + 32 * x))
#define MAINOAM ((SpriteEntry *)0x07000000)

#define PATRAM4(x, tn) ((u32 *)(BG_TILE_RAM(0) | (((x) << 14) + ((tn) << 5)) ))

typedef u16 NAMETABLE[32][32];

#define MAP ((NAMETABLE *)BG_MAP_RAM(0))
#define MAP_HFLIP      0x0400
#define MAP_VFLIP      0x0800
#define MAP_FLIP       0x0c00
#define MAP_PALETTE(x) ((x) << 12)

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
