/* DS frontend for LOCKJAW, an implementation of the Soviet Mind Game

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
#include "ljds.h"
#include "ljplay.h"

#if 1
  #define LJ_VERSION "0.34a ("__DATE__")"
#else
  #define LJ_VERSION "WIP ("__DATE__")"
#endif

#define SCREEN_W 32
#define SCREEN_H 24
#define DS_PFTOP 3
#define DS_PFLEFT 10
#include "ljgbads.inc"

unsigned int nSprites = 0;
volatile int curTime;

void gba_play_sound(struct LJPCView *v, int n) {

}

void gba_poll_sound(struct LJPCView *plat) {
  
}

#if 0
void rectfill(int x, int y, int width, int height, uint16* buffer, uint16 color)
{
  if(x < 0) {
    width += x;
    x = 0;
  }
  if(y < 0) {
    height += y;
    y = 0;
  }
  if(width > 256 - x) {
    width = 256 - x;
  }
  if(height > 192 - y) {
    height = 192 - y;
  }
  if(width > 0 && height > 0) {
    buffer += y * 256 + x;
    for(;
        height > 0;
        --height, buffer += 256) {
      uint16* line = buffer;
      for(int j = width; j > 0; --j) {
        *line++ = color;
      }
    }
  }
}
#endif

void debugMainOAM(void) {

}


void finishSprites(void) {
  for (int i = nSprites - 1; i >= 0; --i) {
    MAINOAM[i].attribute[0] = 512;
  }
  nSprites = 128;
}

void vsync(void) {
  swiWaitForVBlank();
}

void isr(void) 
{
  int interrupts = REG_IF;

  VBLANK_INTR_WAIT_FLAGS |= interrupts;
  REG_IF = interrupts;
  ++curTime;
}

#define KEY_X (IPC_X << 16)
#define KEY_Y (IPC_Y << 16)
#define KEY_PEN (IPC_PEN_DOWN << 16)
#define VRAM_MAIN ((uint16 *)0x06000000)
#define VRAM_SUB ((uint16 *)0x06200000)



void playSoundEffects(LJView *v, LJBits sounds, int countdown) {

}

#define SHADOW_BLOCK 0x00

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
    
      if (color != 0) {
        blkValue = ((blkValue & 0xF0) << 8) | 1;
      } else if (color == 0) {
        if (v->hideShadow == LJSHADOW_COLORED) {
          blkValue = ((blkValue & 0xF0) << 8) | 2;
        } else {
          blkValue = 0x0002;
        }
      }
    
      if (dstY > -8 && dstY < 192) {
        --nSprites;
        MAINOAM[nSprites].attribute[0] = dstY & 0x00FF;
        MAINOAM[nSprites].attribute[1] = dstX & 0x01FF;
        MAINOAM[nSprites].attribute[2] = blkValue;
      }

      rows |= 1 << blkY;
    }
  }

  return rows;
}

void openWindow(void) {
  videoSetMode(MODE_0_2D
               | DISPLAY_BG0_ACTIVE
               | DISPLAY_SPR_1D_LAYOUT 
               | DISPLAY_SPR_ACTIVE);
  videoSetModeSub(MODE_5_2D
                  | DISPLAY_BG2_ACTIVE);
  BGCTRL[0] = BG_16_COLOR | BG_TILE_BASE(0) | BG_MAP_BASE(31);
  BGCTRL_SUB[2] = BG_BMP16_256x256;
  
  vramSetMainBanks(VRAM_A_MAIN_BG, 0x82, 
                   VRAM_C_SUB_BG, VRAM_D_SUB_SPRITE);
  load_font();
  // Load palette
  BG_PALETTE[0] = RGB5(31,31,31);
  BG_PALETTE[1] = RGB5( 0, 0,15);
  setupPalette(srsColors);

  // Set scrolling
  BG_OFFSET[0].x = 0;
  BG_OFFSET[0].y = 0;

  SUB_BG2_XDX = 0x100;
  SUB_BG2_XDY = 0;
  SUB_BG2_YDX = 0;
  SUB_BG2_YDY = 0x100;
  SUB_BG2_CY = 0;
  SUB_BG2_CX = 0;

  lcdMainOnTop();
}

int main(void)
{
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

  powerON(POWER_ALL_2D);
  install_timer();
  openWindow();
  coprNotice();
  initOptions();
  
  // Initialize and enable interrupts
  while (1) {
    BG_PALETTE[0] = RGB5(31, 31, 31);
    videoSetMode(MODE_0_2D
               | DISPLAY_BG0_ACTIVE);
    setupPalette(srsColors);
    options(&mainView);
    setupPalette(wkColorScheme[p.rotationSystem] ? arsColors : srsColors);
    p.seed = curTime ^ (curTime << 16);
    play(&mainView);
    BG_PALETTE[0] = (control.countdown > 0)
                    ? RGB5(31, 15, 15) 
                    : RGB5(15, 31, 15);
    for (int i = 0; i < 60; ++i) {
      vsync();
      //gba_poll_sound(&platView);
    }
  }
}
