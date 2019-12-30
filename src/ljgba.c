/* GBA frontend for LOCKJAW, an implementation of the Soviet Mind Game

Copyright (C) 2006-2007 Damian Yerrick <tepples+lj@spamcop.net>

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

#include <stdio.h>
#include <string.h>
#include "ljgba.h"
#include "ljplay.h"

#if 1
  #define LJ_VERSION "0.34 ("__DATE__")"
#else
  #define LJ_VERSION "WIP ("__DATE__")"
#endif

#define SCREEN_W 30
#define SCREEN_H 20
#define DS_PFTOP 0
#define DS_PFLEFT 9
#include "ljgbads.inc"

static unsigned int nSprites;

void finishSprites() {
  for (int i = nSprites - 1; i >= 0; --i) {
    OAM[i].attr0 = 512;
  }
  nSprites = 128;
}

void openWindow(void) {
  REG_DISPCNT = MODE_0 | BG0_ON | OBJ_ON;
  BGCTRL[0] = BG_TILE_BASE(0) | BG_MAP_BASE(31);
  BG_COLORS[0] = RGB5(31, 31, 31);
  BG_COLORS[1] = RGB5(0, 0, 0);
  OBJ_COLORS[0] = RGB5(31, 0, 31);
  OBJ_COLORS[1] = RGB5(0, 0, 0);
  load_font();
  setupPalette(srsColors);
  cls();
}

void playSoundEffects(LJView *v, LJBits sounds, int countdown) {
  if (sounds & LJSND_IRS) {
    gba_play_sound(v->plat, 6);
  } else if (sounds & LJSND_ROTATE) {
    gba_play_sound(v->plat, 1);
  }
  if (sounds & LJSND_SHIFT) {
    gba_play_sound(v->plat, 0);
  }
  if (sounds & LJSND_LAND) {
    gba_play_sound(v->plat, 2);
  }
  if (sounds & LJSND_LOCK) {
    gba_play_sound(v->plat, 3);
  }
  if (sounds & LJSND_B2B) {
    gba_play_sound(v->plat, 8);
  } else if (sounds & LJSND_SETB2B) {
    gba_play_sound(v->plat, 7);
  } else if (sounds & LJSND_LINE) {
    gba_play_sound(v->plat, 4);
  }
  if (sounds & LJSND_HOLD) {
    gba_play_sound(v->plat, 5);
  }
  if (sounds & LJSND_SECTIONUP) {
    gba_play_sound(v->plat, 9);
  }
  gba_poll_sound(v->plat);
}

/**
 * Draws a tetromino whose lower left corner of the bounding box is at (x, y)
 * @param b the bitmap to draw to
 * @param piece the piece to be drawn
 * @param x distance from to left side of 4x4 box
 * @param y distance from top of bitmap to bottom of 4x4 box
 * @param the rotation state (0: U; 1: R; 2: D; 3: L; 4: Initial position)
 * @param w width of each block
 * @param h height of each block
 * @param color Drawing style
 * color == 0: draw shadow
 * color == 0x10 through 0x70: draw in that color
 * color == 0x80: draw as garbage
 * color == -255 through -1: draw with 255 through 1 percent lighting
 */
LJBits drawPiece(LJView *const v, void *const b,
                 int piece, int x, int y, int theta,
                 int color, int w, int h) {
  // Don't try to draw the -1 that's the sentinel for no hold piece
  if (piece < 0)
    return 0;

  LJBits rows = 0;
  LJBlkSpec blocks[4];

  expandPieceToBlocks(blocks, v->field, piece, 0, 0, theta);
  
  for (int blk = 0; blk < 4; ++blk) {
    int blkValue = blocks[blk].conn;
    if (blkValue) {
      int blkX = blocks[blk].x;
      int blkY = blocks[blk].y;
      const int dstX = x + w * blkX;
      const int dstY = y + h * (-1 - blkY);
    
      if (color == 0x80) {
        blkValue = 0x8001;  // garbage
      } else if (color != 0) {
        blkValue = ((blkValue & 0xF0) << 8) | 1;
      } else if (color == 0) {
        if (v->hideShadow == LJSHADOW_COLORED) {
          blkValue = ((blkValue & 0xF0) << 8) | 2;
        } else {
          blkValue = 0x0002;
        }
      }
    
      if (dstY > -8 && dstY < 160) {
        --nSprites;
        OAM[nSprites].attr0 = dstY & 0x00FF;
        OAM[nSprites].attr1 = dstX & 0x01FF;
        OAM[nSprites].attr2 = blkValue;
      }

      rows |= 1 << blkY;
    }
  }

  return rows;
}

int main(void) {
  LJField p = {
    .maxUpwardKicks = 127,
    .lockReset = LJLOCK_MOVE,
    .lockDelay = 0,
    .randomizer = 0,
    .tSpinAlgo = LJTS_TDS,
    .speedCurve = LJSPD_EXP,
    .clearGravity = 0,
    .squareMode = 0,
    .scoreStyle = LJSCORE_LJ,
    .areStyle = 0,
    .leftWall = 0,
    .rightWall = 10,
    .ceiling = 20
  };
  LJControl control = {
    .dasSpeed = 1,
    .dasDelay = 10,
    .initialDAS = 1,
    .allowDiagonals = 0,
    .softDropSpeed = 0,
    .softDropLock = 0,
    .hardDropLock = 1
  };
  struct LJPCView platView;
  LJView mainView = {
    .field = &p,
    .control = &control,
    .smoothGravity = 1,
    .nextPieces = 3,
    .plat = &platView,
    .backDirty = ~0
  };

  install_timer();
  openWindow();
  install_sound(&platView);
  initOptions();
  coprNotice();
  
  while (1) {
    BG_COLORS[0] = RGB5(31, 31, 31);
    REG_DISPCNT = MODE_0 | BG0_ON;
    setupPalette(srsColors);
    options(&mainView);

    setupPalette(wkColorScheme[p.rotationSystem] ? arsColors : srsColors);
    p.seed = curTime ^ (curTime << 16);
    play(&mainView);
    BG_COLORS[0] = (control.countdown > 0)
                   ? RGB5(31, 15, 15) 
                   : RGB5(15, 31, 15);
    for (int i = 0; i < 90; ++i) {
      vsync();
      gba_poll_sound(&platView);
    }
  }
}
