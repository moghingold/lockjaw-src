/* Replay functionality for LOCKJAW, an implementation of the Soviet Mind Game

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
#ifndef LJREPLAY_H
#define LJREPLAY_H

#include "lj.h"
#include "ljcontrol.h"

#define LJREPLAY_EOF (-1)

typedef struct LJReplay LJReplay;

/**
 * Creates a new replay.
 * @param filename The name of the file to which the replay is recorded.
 * @param p The field that is observed.
 * @return A pointer to the replay object,
 * or NULL if allocation failed.
 */
LJReplay *newReplay(const char *filename, LJField *p);

/**
 * Records a single frame of input in the replay.
 * If spawn or hold sound is played, records the new piece.
 * @param r The replay object.
 */
void replayRecord(LJReplay *r, LJBits keys, const LJInput *in);

/**
 * Stops recording the replay and dumps it to the file.
 */
void replayClose(LJReplay *r);

/**
 * Opens an existing replay.
 * @param filename The name of the file to which the replay is recorded.
 * @param p The field that is observed.
 * @return A pointer to the replay object,
 * or NULL if allocation failed.
 */
LJReplay *openReplay(const char *filename, LJField *p);

/**
 * @param d The structure to be filled with input
 * @return The keys pressed, or REPLAY_EOF.
 */
int getReplayFrame(LJReplay *r, LJInput *d);

#endif
