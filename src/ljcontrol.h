/* Control for LOCKJAW, an implementation of the Soviet Mind Game

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

#ifndef LJCONTROL_H
#define LJCONTROL_H

#include "ljtypes.h"

// The cross-platform parts of the view are stored here
typedef struct LJControl {
  LJBits lastKeys, repressKeys;
  unsigned int presses;
  unsigned char dasSpeed;
  unsigned char dasDelay;
  signed char dasCounter;
  unsigned char allowDiagonals;
  unsigned char softDropSpeed;
  unsigned char softDropLock;
  unsigned char hardDropLock;
  unsigned char initialDAS;
  unsigned char initialRotate;
  signed char countdown;
  struct LJReplay *replaySrc;
  struct LJReplay *replayDst;
} LJControl;

struct LJField;
struct LJInput;

#define N_SPEED_METER_PIECES 10

typedef struct LJView {
  struct LJField *field;
  LJControl *control;
  LJBits backDirty;
  LJBits frontDirty;
  struct LJPCView *plat;
  unsigned char smoothGravity;
  unsigned char hideNext;
  unsigned char hideShadow;
  unsigned char nextPieces;
  unsigned char hidePF;
  unsigned char showTrails;
  unsigned char nLockTimes;
  unsigned int lockTime[N_SPEED_METER_PIECES];
  LJFixed trailY;
} LJView;

enum {
  LJSHADOW_COLORED_25 = 0,
  LJSHADOW_COLORED_50,
  LJSHADOW_COLORED,
  LJSHADOW_COLORLESS,
  LJSHADOW_NONE,
  LJSHADOW_NO_FALLING,
  LJSHADOW_N_STYLES
};
extern const char *const optionsShadowStyleNames[];

#define VKEY_UP        0x0001
#define VKEY_DOWN      0x0002
#define VKEY_LEFT      0x0004
#define VKEY_RIGHT     0x0008
#define VKEY_ROTL      0x0010
#define VKEY_ROTR      0x0020
#define VKEY_HOLD      0x0040
#define VKEY_ITEM      0x0080
#define VKEY_MACRO(x)  (0x100 << (x))
#define VKEY_MACROS    0xFF00

enum {
  LJZANGI_SLIDE,
  LJZANGI_LOCK,
  LJZANGI_LOCK_RELEASE,
  LJZANGI_N_STYLES
};

/* VKEY_MACRO(0) to VKEY_MACRO(7)

Event planners can restrict how many macros a player can use,
so that keyboardists don't have an unfair advantage over
gamepad users.

*/

extern const char *const optionsShadowNames[];
extern const char *const optionsDASNames[];
extern const char *const optionsDASDelayNames[];
#define optionsSoftSpeedNames &(optionsDASNames[1])


void addKeysToInput(struct LJInput *dst, LJBits keys, const struct LJField *p, LJControl *c);

#endif
