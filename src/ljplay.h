/* Platform hooks for LOCKJAW, an implementation of the Soviet Mind Game

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

#ifndef LJPLAY_H
#define LJPLAY_H
#include "lj.h"
#include "ljcontrol.h"

/**
 * Plays Lockjaw.
 * @param v view
 */
void play(LJView *v);

/*
 * Platform-native code must implement the following callbacks,
 * which will be described later:
 */
LJBits readPad(void);
void updField(const LJView *const v, LJBits rows);
void startingAnimation(LJView *v);
void blitField(LJView *v);
int pauseGame(struct LJPCView *v);
void playSoundEffects(LJView *v, LJBits sounds, int countdown);
void drawShadow(LJView *v);
void drawFallingPiece(LJView *v);
void drawScore(LJView *v);
int getTime(void);
void yieldCPU(void);
void ljBeginDraw(LJView *v, int sync);
void ljEndDraw(LJView *v);
int ljHandleConsoleButtons(LJView *v);
void playRedrawScreen(LJView *v);


#endif
