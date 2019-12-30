/* Header for PC frontend for LOCKJAW, an implementation of the Soviet Mind Game

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

#ifndef LJPC_H
#define LJPC_H
#include <allegro.h>
#include "ljmusic.h"
#include "ljplay.h"
#include "pcjoy.h"
#include "ljvorbis.h"

typedef struct LJPCView
{
  // Platform-dependent graphics
  unsigned int baseX, baseY, blkW, blkH;
  BITMAP *back;
  BITMAP *bg;
  BITMAP *blocks;
  BITMAP *connBlocks;
  
  // Platform-dependent sound
  LJMusic *bgm;
  unsigned char b2bcd1, b2bcd2;
  unsigned char wantRecord;
  unsigned char nextPos;
  unsigned char nextAbove;
} LJPCView;

extern char skinName[PATH_MAX];
extern char ljblocksSRSName[PATH_MAX];
extern char ljblocksSegaName[PATH_MAX];
extern char ljconnSRSName[PATH_MAX];
extern char ljconnSegaName[PATH_MAX];
extern char ljbgName[PATH_MAX];
extern char bgmName[PATH_MAX];


#define N_GIMMICKS LJGM_N_GIMMICKS
extern const char *const gimmickNames[N_GIMMICKS];

extern const char *const optionsSideNames[];


enum {
  LJNEXT_RIGHT = 0,
  LJNEXT_RIGHT_TAPER,
  LJNEXT_TOP,
  LJNEXT_N_STYLES
};
extern const char *const optionsNextStyleNames[];

/**
 * The number of persistent preferences.  Must match the number of
 * fields in struct LJPrefsNamed.
 */

/**
 * Names of persistent preferences.
 * Order must be exactly the same as in
 * optionsMenu[] (options.c)
 */
enum {
  LJPREFS_WIDTH,
  LJPREFS_HEIGHT,
  LJPREFS_SPEED_CURVE,
  LJPREFS_ARE_STYLE,
  LJPREFS_RANDOMIZER,

  LJPREFS_ROTATION_SYSTEM,
  LJPREFS_KICK_LIMIT,
  LJPREFS_LOCK_RESET,
  LJPREFS_LOCK_DELAY,

  LJPREFS_LINE_DELAY,
  LJPREFS_CLEAR_GRAVITY,
  LJPREFS_SQUARE_MODE,
  LJPREFS_SCORE_STYLE,
  LJPREFS_DROP_SCORE_STYLE,
  LJPREFS_T_SPIN_ALGO,
  LJPREFS_GARBAGE,

  LJPREFS_DAS_SPEED,
  LJPREFS_DAS_DELAY,
  LJPREFS_INITIAL_DAS,
  LJPREFS_IRS,
  LJPREFS_ALLOW_DIAGONALS,

  LJPREFS_SOFT_DROP_SPEED,
  LJPREFS_SOFT_DROP_LOCK,
  LJPREFS_HARD_DROP_LOCK,
  LJPREFS_NEXT_PIECES,
  LJPREFS_NEXT_ABOVE,
  LJPREFS_HIDE_SHADOW,
  
  LJPREFS_HIDE_PF,
  LJPREFS_SMOOTH_GRAVITY,
  LJPREFS_TRAILS,
  LJPREFS_SIDE,
  LJPREFS_NEXT_POS,
  LJPREFS_AUTO_PAUSE,
  LJPREFS_AUTO_RECORD,
  LJPREFS_WINDOWED,
  OPTIONS_MENU_LEN
};
union LJPrefs {
  int number[OPTIONS_MENU_LEN];
};


/* set by display mode */
extern int pfBgColor, pfFgColor, bgColor, fgColor, hiliteColor;
extern const FONT *aver32, *aver16;
extern volatile int redrawWholeScreen;
extern char autoPause;

void saveScreen(void);
void ezPlaySample(const char *filename, int vol);
int parse_ini_line(const char *in, char *key, char *var, char *val);
int loadOptions(union LJPrefs *prefs);
void saveOptions(const union LJPrefs *prefs);
void options(union LJPrefs *prefs);
void unpackOptions(LJView *v, const union LJPrefs *prefs);
void debrief(const LJView *v);



#endif
