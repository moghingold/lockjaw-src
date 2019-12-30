/* PC frontend for LOCKJAW, an implementation of the Soviet Mind Game

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

#include "ljpc.h"
#include "ljreplay.h"
#include <jpgalleg.h>
#include <time.h>

#if 1
  #define LJ_VERSION "0.34a ("__DATE__")"
#else
  #define LJ_VERSION "WIP ("__DATE__")"
#endif


int bgColor, fgColor = 0, hiliteColor, pfBgColor = 0, pfFgColor;
static volatile int curTime = 0;
volatile int redrawWholeScreen = 1;
static volatile int wantPause = 0;
const DATAFILE *dat;
const FONT *aver32 = NULL, *aver16 = NULL;
char withSound = 0;
char autoPause;
int refreshRate = 0;
char demoFilename[512];

static void incCurTime(void) {
  ++curTime;
} END_OF_FUNCTION(incCurTime);

static void amnesia(void) {
  redrawWholeScreen = 1;
} END_OF_FUNCTION(amnesia);

static void requestPause(void) {
  wantPause |= autoPause;
}


int getTime(void) {
  return curTime;
}

void yieldCPU(void) {
  rest(5);  
}

void ljBeginDraw(LJView *v, int sync) {
  vsync();
  acquire_screen();
  if (redrawWholeScreen) {
    redrawWholeScreen = 0;
    playRedrawScreen(v);
  }

  // Draw shape
  blit(v->plat->bg, screen, 0, 0, 0, 0, 16, 16);
  if (v->control->replayDst) {
    // circle: record
    circlefill(screen, 8, 8, 7, fgColor);
  } else if (v->control->replaySrc) {
    for (int wid = 0; wid < 8; ++wid) {
      hline(screen, 1, 1 + wid, wid * 2 + 1, fgColor);
      hline(screen, 1, 15 - wid, wid * 2 + 1, fgColor);
    }
  } else {
    // square: stop
    rectfill(screen, 2, 2, 14, 14, fgColor);
  }
}

void ljEndDraw(LJView *v) {
  release_screen();
}

static void startRecording(LJView *v) {
  // close existing playback
  if (v->control->replaySrc) {
    replayClose(v->control->replaySrc);
    v->control->replaySrc = NULL;
  }
      
  // toggle recording
  if (v->control->replayDst) {
    replayClose(v->control->replayDst);
    v->control->replayDst = NULL;
  } else {
    v->control->replayDst = newReplay("demo.ljm", v->field);
  }
}

static void startPlaying(LJView *v) {
      // close existing recording
      if (v->control->replayDst) {
        replayClose(v->control->replayDst);
        v->control->replayDst = NULL;
      }
      
      // toggle playback
      if (v->control->replaySrc) {
        replayClose(v->control->replaySrc);
        v->control->replaySrc = NULL;
      } else {
        v->control->replaySrc = openReplay("demo.ljm", v->field);
        v->nLockTimes = 0;
      }
      v->backDirty = ~0;
      v->field->reloaded = 1;
}

int ljHandleConsoleButtons(LJView *v) {
  int canceled = 0;
  
  while (keypressed()) {
    int scancode;
    int codepoint = ureadkey(&scancode);

    if (scancode == KEY_ESC) {
      wantPause = 1;
    } else if (scancode == KEY_PRTSCR) {
      saveScreen();
    } else if (codepoint == '[') {
      v->plat->wantRecord = 1;
    } else if (codepoint == ']') {
      v->plat->wantRecord = 0;
      startPlaying(v);
    }
  }
  if (v->plat->wantRecord) {
    v->plat->wantRecord = 0;
    startRecording(v);
  }
  if (wantPause) {
    canceled = pauseGame(v->plat);
    wantPause = 0;
  }
  
  if (wantsClose) {
    canceled = 1;
  }
  
  return canceled;
}


static void drawBlock(const LJPCView *const v, int x, int y, int blk) {

  if (x >= 0 && x < LJ_PF_WID && y >= 0 && y < LJ_PF_VIS_HT) {
    const int w = v->blkW;
    const int h = v->blkH;
    const int dstX = w * x;
    const int dstY = h * (LJ_PF_VIS_HT - 1 - y);
    
    if (blk && v->connBlocks) {
      const unsigned int srcX = ((blk >> 0) & 15) * w;
      const unsigned int srcY = ((blk >> 4) & 15) * h;
      blit(v->connBlocks, v->back, srcX, srcY, dstX, dstY, w, h);
    } else {
      const unsigned int srcX = ((blk >> 4) & 7) * w;
      const unsigned int srcY = ((blk >> 7) & 1) * h;
      blit(v->blocks, v->back, srcX, srcY, dstX, dstY, w, h);
    }
    
    // 0x100: hotline
    if (blk & 0x100) {
      hline(v->back, dstX, dstY + h / 2 - 1, dstX + w - 1, pfFgColor);
      hline(v->back, dstX, dstY + h / 2    , dstX + w - 1, pfFgColor);
      hline(v->back, dstX, dstY + h / 2 + 1, dstX + w - 1, pfBgColor);
    }
  }
}

/**
 * Draws multiple rows of the playfield
 * @param p     the playfield
 * @param rows  the rows to be updated (0x00001 on bottom, 0x80000 on top)
 */
void updField(const LJView *const v, LJBits rows) {
  const LJField *const p = v->field;

  acquire_bitmap(v->plat->back);
  for (int y = 0;
       y < LJ_PF_VIS_HT && rows != 0;
       ++y, rows >>= 1) {
    int usingHotline = hotlineRows[y]
                       && v->field->scoreStyle == LJSCORE_HOTLINE;
    if (rows & 1) {
      for (int x = 0; x < LJ_PF_WID; x++) {
        int b = v->hidePF ? 0 : p->b[y][x];
        if (b == 0 && usingHotline) {
          b = 0x100;
        }
        drawBlock(v->plat, x, y, b);
      }
    }
  }
  release_bitmap(v->plat->back);
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
LJBits drawPiece(LJView *const v, BITMAP *const b,
                 int piece, int x, int y, int theta,
                 int color, int w, int h) {
  // Don't try to draw the -1 that's the sentinel for no hold piece
  if (piece < 0)
    return 0;

  LJBits rows = 0;
  LJBlkSpec blocks[4];
  const int srcW = v->plat->blkW;
  const int srcH = v->plat->blkH;
  BITMAP *singleBlocks = v->plat->blocks;
  int singleSeries = (color == 0
                      && singleBlocks->h >= 6 * srcH)
                     ? 4 : 2;

  // color: 0 for shadow, >0 for piece
  BITMAP *connBlocks = (color != SHADOW_BLOCK)
                       ? v->plat->connBlocks : NULL;

  BITMAP *transTemp = color < 0
                      || (color == SHADOW_BLOCK && v->hideShadow < LJSHADOW_COLORED)
                      ? create_bitmap(w, h) : 0;

  if (transTemp) {
    set_trans_blender(0, 0, 0, v->hideShadow ? 128 : 64);
  }

  expandPieceToBlocks(blocks, v->field, piece, 0, 0, theta);
  acquire_bitmap(b);
  for (int blk = 0; blk < 4; ++blk) {
    int blkValue = blocks[blk].conn;
    
    // If blkValue == 0 then the block is not part of the piece,
    // such as if it's a domino or tromino or (once pentominoes
    // are added) a tetromino.
    if (blkValue) {
      int blkX = blocks[blk].x;
      int blkY = blocks[blk].y;
      const int dstX = x + w * blkX;
      const int dstY = y + h * (-1 - blkY);

      if (color > 0) {
        blkValue = color | (blkValue & CONNECT_MASK);
      } else if (color == 0 && v->hideShadow == LJSHADOW_COLORLESS) {
        blkValue = 0;
      }
    
      if (connBlocks) {
        const unsigned int srcX = ((blkValue >> 0) & 15) * srcW;
        const unsigned int srcY = ((blkValue >> 4) & 15) * srcH;

        if (transTemp) {
          stretch_blit(connBlocks, transTemp,
                       srcX, srcY, srcW, srcH,
                       0, 0, w, h);
          if (color < 0) {
            draw_lit_sprite(b, transTemp, dstX, dstY, color + 256);
          } else {
            draw_trans_sprite(b, transTemp, dstX, dstY);
          }
        } else {
          stretch_blit(connBlocks, b,
                       srcX, srcY, srcW, srcH,
                       dstX, dstY, w, h);
        }
      } else {
        const unsigned int srcX = ((blkValue >> 4) & 7) * srcW;
        const unsigned int srcY = (((blkValue >> 7) & 1) + singleSeries) * srcH;

        if (transTemp) {
          stretch_blit(singleBlocks, transTemp,
                       srcX, srcY, srcW, srcH,
                       0, 0, w, h);
          if (color < 0) {
            draw_lit_sprite(b, transTemp, dstX, dstY, color + 256);
          } else {
            draw_trans_sprite(b, transTemp, dstX, dstY);
          }
        } else {
          stretch_blit(singleBlocks, b,
                       srcX, srcY, srcW, srcH,
                       dstX, dstY, w, h);
        }
      }
      rows |= 1 << blkY;
    }
  }
  release_bitmap(b);
  if (transTemp) {
    solid_mode();
    destroy_bitmap(transTemp);
  }

  return rows;
}

void drawPieceInPF(LJView *v, int dstX, LJFixed dstY, int theta, int color) {
  LJBits bits = 0;
  BITMAP *b = v->plat->back;
  const LJField *const p = v->field;
  int y = ljfixfloor(dstY);
  const int w = v->plat->blkW;
  const int h = v->plat->blkH;
  int drawnY = fixmul(h, dstY);

  bits = drawPiece(v, b, p->curPiece[0],
                   w * dstX, h * LJ_PF_VIS_HT - drawnY,
                   theta,
                   color, w, h);
  bits = (y >= 0) ? bits << y : bits >> -y;
  bits &= (1 << LJ_PF_VIS_HT) - 1;
  if (dstY & 0xFFFF) {
    bits |= bits << 1;
  }

  v->backDirty |= bits;
  v->frontDirty |= bits;
}


void drawFallingPiece(LJView *v) {
  const LJField *const p = v->field;
  int piece = p->curPiece[0];
  int y = v->smoothGravity
          ? p->y
          : ljitofix(ljfixfloor(p->y));
  const int color = (p->state == LJS_LANDED)
                    ? -128 - ((p->stateTime + 1) * 128 / (p->lockDelay + 1))
                    : pieceColors[piece];

  // Draw trails
  if (v->showTrails) {

    // trail going up
    while (v->trailY - y < ljitofix(-1)) {
      v->trailY += ljitofix(1);
      drawPieceInPF(v, p->x, v->trailY, p->theta, color);
    }

    // trail going down
    while (v->trailY - y > ljitofix(1)) {
      v->trailY -= ljitofix(1);
      drawPieceInPF(v, p->x, v->trailY, p->theta, color);
    }
  }
  drawPieceInPF(v, p->x, y, p->theta, color);
  v->trailY = y;
}

void drawShadow(LJView *v) {
  const LJField *const p = v->field;
  int y = ljitofix(p->hardDropY);
  const int color = SHADOW_BLOCK;
  drawPieceInPF(v, p->x, y, p->theta, color);
}

const char *const optionsNextStyleNames[LJNEXT_N_STYLES] = {
  "Next at right",
  "Next at right (taper)",
  "Next at top"
};

void drawNextPieces(LJView *v) {
  int baseX = v->plat->baseX;
  int baseY = v->plat->baseY;
  int blkW = v->plat->blkW;
  int blkH = v->plat->blkH;
  int holdPieceColor = v->field->alreadyHeld
                       ? 0x80
                       : pieceColors[v->field->holdPiece];
  
  acquire_screen();

  if (v->frontDirty & LJ_DIRTY_NEXT) {
    int holdX = baseX + blkW;
  
    // Draw hold piece
    blit(v->plat->bg, screen,
         holdX, baseY - 19 * blkH - 64,
         holdX, baseY - 19 * blkH - 64,
         64, 32);
    drawPiece(v, screen,
              v->field->holdPiece, holdX, baseY - 19 * blkH, 4,
              holdPieceColor, 16, 16);
  }
  // Draw next pieces

  switch (v->plat->nextPos) {
  case LJNEXT_RIGHT:
  case LJNEXT_RIGHT_TAPER:
    if (v->frontDirty & LJ_DIRTY_NEXT) {
      int y = baseY - LJ_PF_VIS_HT * blkH;
      int x = baseX + LJ_PF_WID * blkW;
      int w = blkW;
      int h = blkH;
      for (int i = 1; i <= v->nextPieces; ++i) {
        int piece = v->field->curPiece[i];

        blit(v->plat->bg, screen, x, y, x, y, w * 4, h * 2);
        if (!v->hideNext) {
          drawPiece(v, screen,
                    piece, x, y + 4 * h, 4,
                    pieceColors[piece], w, h);
        }
        y += 8 + h * 2;
        if (v->plat->nextPos == LJNEXT_RIGHT_TAPER) {
          --h;
          --w;
        }
      }
    }
    break;
    
  case LJNEXT_TOP:
    if (v->frontDirty & LJ_DIRTY_NEXT) {
      int y = baseY - (LJ_PF_VIS_HT + 2) * blkH - 8;
      int x = baseX + 4 * blkW;

      blit(v->plat->bg, screen, x, y, x, y, blkW * 8, blkH * 2);
      if (!v->hideNext) {
        if (v->nextPieces >= 1) {
          int piece = v->field->curPiece[1];
          drawPiece(v, screen,
                    piece, x, y + 4 * blkH, 4,
                    pieceColors[piece], blkW, blkH);
        }
        if (v->nextPieces >= 2) {
          int piece = v->field->curPiece[2];
          x += 4 * blkW;
          drawPiece(v, screen,
                    piece, x, y + 3 * blkH, 4,
                    pieceColors[piece], blkW/ 2, blkH / 2);
        }
        if (v->nextPieces >= 3) {
          int piece = v->field->curPiece[3];
          x += 2 * blkW;
          drawPiece(v, screen,
                    piece, x, y + 3 * blkH, 4,
                    pieceColors[piece], blkW / 2, blkH / 2);
        }
      }
    }
    break;
  }

  if (v->plat->nextAbove && !v->hideNext) {
    int row = (v->field->hardDropY + 4);
    int x = (1 + v->field->x) * blkW;
    for (int i = 1;
         i <= v->plat->nextAbove
         && row <= v->field->ceiling - 2;
         ++i) {
      int y = (LJ_PF_VIS_HT - row) * blkH;
      int piece = v->field->curPiece[i];

      drawPiece(v, v->plat->back,
                piece, x, y, 4,
                pieceColors[piece], blkW / 2, blkH / 2);
      v->backDirty |= 3 << row;
      row += 2;
    }
  }
  release_screen();
  v->frontDirty &= ~LJ_DIRTY_NEXT;
}

void drawScore(LJView *v) {
  int curve = v->field->speedCurve;
  int drawLevel = (curve == LJSPD_RHYTHM || curve == LJSPD_RHYTHMZERO
                   || curve == LJSPD_TGM || curve == LJSPD_DEATH
                   || v->field->gimmick == LJGM_BABY);
  int gameTime = v->field->gameTime;
  int seconds = gameTime / 60;
  int minutes = seconds / 60;
  int baseX = v->plat->baseX;
  int tpm = -1;
  int spawnLeft = v->plat->blkW * LJ_SPAWN_X + baseX;
  int pfRight = v->plat->blkW * LJ_PF_WID + baseX;
  
  if (v->nLockTimes >= 2 ) {
    int time = v->lockTime[0] - v->lockTime[v->nLockTimes - 1];
    if (time > 0) {
      tpm = 3600 * (v->nLockTimes - 1) / time;
    }
  }

  if (v->frontDirty & LJ_DIRTY_SCORE) {
    switch (v->plat->nextPos) {
    case LJNEXT_TOP:
      blit(v->plat->bg, screen,
           pfRight, 72,
           pfRight, 72,
           112, 380);
      
      textout_ex(screen, aver32, "Lines:", pfRight, 72, fgColor, -1);
      textprintf_right_ex(screen, aver32, pfRight + 104, 102, fgColor, -1,
                          "%d", v->field->lines);
      textout_ex(screen, aver32, "Score:", pfRight, 142, fgColor, -1);
      textprintf_right_ex(screen, aver32, pfRight + 104, 172, fgColor, -1,
                          "%d", v->field->score);
      if (drawLevel) {
        textout_ex(screen, aver32, "Level:", pfRight, 212, fgColor, -1);
        textprintf_right_ex(screen, aver32, pfRight + 104, 242, fgColor, -1,
                            "%d", v->field->speedLevel);
      }
      if (tpm > 0) {
        textout_ex(screen, aver32, "Speed:", pfRight, 282, fgColor, -1);
        textprintf_right_ex(screen, aver32, pfRight + 104, 312, fgColor, -1,
                            "%d", tpm);
      }
      break;

    default:
      blit(v->plat->bg, screen, spawnLeft, 12, spawnLeft, 12, 288, 60);
      if (drawLevel) {
        textprintf_right_ex(screen, aver32, spawnLeft + 288, 12, 0, -1,
                            "Lv. %d", v->field->speedLevel);
      }
      textprintf_ex(screen, aver32, spawnLeft, 12, fgColor, -1,
                    "Lines: %d", v->field->lines);
      textprintf_ex(screen, aver32, spawnLeft, 42, fgColor, -1,
                    "Score: %d", v->field->score);
      break;
    }
  }
  
  if (v->frontDirty & LJ_DIRTY_NEXT) {

    // Erase gimmick
    blit(v->plat->bg, screen,
         baseX + (LJ_PF_WID / 2) * v->plat->blkW, v->plat->baseY + 8,
         baseX + (LJ_PF_WID / 2) * v->plat->blkW, v->plat->baseY + 8,
         v->plat->blkW, 30);
    // Draw gimmick
    if (v->field->gimmick >= 0 && v->field->gimmick < LJGM_N_GIMMICKS) {
      textout_centre_ex(screen, aver32,
                        gimmickNames[v->field->gimmick],
                        baseX + (LJ_PF_WID / 2) * v->plat->blkW, v->plat->baseY + 8,
                        0, -1);
    }
  }
  drawNextPieces(v);

  blit(v->plat->bg, screen, pfRight, 42, pfRight, 42, 96, 30);
#if 0
  // Use this for DEBUG inspection into a variable
  textprintf_right_ex(screen, aver16, pfRight + 96, 8, fgColor, -1,
                      "%d", v->field->bpmCounter);
#endif
  textprintf_right_ex(screen, aver32, pfRight + 96, 42, fgColor, -1,
                      "%d:%02d", minutes, seconds - 60 * minutes);
}

/**
 * Plays a sample from datafile by name, with the specified volume
 * and the specified rate scale factor, panned center, without loop.
 * @param filename name of object in datafile
 * @param vol scale factor for volume (0-255); if set to 0,
 * stops all voices playing the sample
 * @param pitch scale factor for playback rate (1000 = normal; 2000 = double speed)
 */
static void playPitchSample(const char *filename, int vol, int pitch) {
  if (withSound) {
    const DATAFILE *entry = find_datafile_object(dat, filename);

    if (entry) {
      if (vol < 1) {
        stop_sample(entry->dat);
      } else {
        play_sample(entry->dat, vol, 128, pitch, 0);
      }
    }
  }
}

void ezPlaySample(const char *filename, int vol) {
  playPitchSample(filename, vol, 1000);
}

void playSampleForTetromino(int piece) {
  static const char tetrominoNames[] = "IJLOSTZ22V";
  
  piece &= LJP_MASK;
  if (piece >= 0 && piece < 10) {
    char filename[32];
    usprintf(filename, "next%c_wav", tetrominoNames[piece]);
    ezPlaySample(filename, 128);
  }
}

void blitField(LJView *v) {
  int blkH = v->plat->blkH;
  int rowY = v->plat->baseY
             + (v->field->ceiling - LJ_PF_VIS_HT - 1) * blkH;
  int x = v->plat->blkW * v->field->leftWall;
  int w = v->plat->blkW * (v->field->rightWall - v->field->leftWall);

  // Copy each dirty row
  for (int y = 0; y < v->field->ceiling; ++y) {
    if (v->frontDirty & (1 << y)) {
      blit(v->plat->back, screen,
           x, (LJ_PF_VIS_HT - y - 1) * blkH,
           x + v->plat->baseX, rowY,
           w, blkH);
    }
    rowY -= 24;
  }

  v->frontDirty &= (~0) << LJ_PF_HT;
}

void saveScreen(void) {
  BITMAP *b = create_bitmap(SCREEN_W, SCREEN_H);
  if (b) {
    PALETTE pal;

    get_palette(pal);
    blit(screen, b, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
    save_bitmap("ljsnap.bmp", b, pal);
    destroy_bitmap(b);
  }
}

#define GIMMICKS_PER_COL 5
#define N_NONGIMMICKS 2

static const char *const nonGimmickNames[N_NONGIMMICKS] = {
  "Play .ljm", "Back"
};

static const char *const gimmickHelp[LJGM_N_GIMMICKS + N_NONGIMMICKS][2] = {
  { "Move and rotate the falling tetrominoes to pack them",
    "tightly into the well. Play until you top out!" },
  { "Game ends after you clear 40 lines.",
    "Some play for speed; others for points." },
  { "Game ends after three minutes.",
    "" },
  { "Three opponents use items to constantly send you",
    "disabling attacks. (This gimmick is a joke.)" },
  { "You have 300 keystrokes.",
    "Make them count." },
  { "Watch a recorded demo.",
    "" },
  { "Return to title screen",
    "" }
};

static void getGimmickDrawRow(unsigned int gimmick, int hilite) {
  unsigned int ht = text_height(aver32);
  const char *txt = gimmick < LJGM_N_GIMMICKS
                    ? gimmickNames[gimmick]
                    : nonGimmickNames[gimmick - LJGM_N_GIMMICKS];
  unsigned int wid = text_length(aver32, txt);

  int buttonCol = gimmick / GIMMICKS_PER_COL;
  int buttonX = 20 + buttonCol * 200;
  int buttonY = 140
                + 40 * (gimmick - buttonCol * GIMMICKS_PER_COL);
  unsigned int buttonWidth = wid + 16;
  unsigned int descriptionY = 160 + 40 * GIMMICKS_PER_COL;

  rectfill(screen,
           buttonX, buttonY, buttonX + buttonWidth - 1, buttonY + 39,
           hilite ? hiliteColor : bgColor);
  if (hilite) {
    rect(screen,
         buttonX, buttonY, buttonX + buttonWidth - 1, buttonY + 39,
         fgColor);
    rectfill(screen,
             8, descriptionY, SCREEN_W - 9, descriptionY + 59,
             bgColor);
    textout_ex(screen, aver32, gimmickHelp[gimmick][0], 16, descriptionY, fgColor, -1);
    textout_ex(screen, aver32, gimmickHelp[gimmick][1], 16, descriptionY + 30, fgColor, -1);
  }
  textout_ex(screen, aver32, txt,
             8 + buttonX,
             20 + buttonY - ht / 2,
             fgColor, -1);
}

int getGimmick(int lastGimmick) {
  LJBits lastKeys = ~0;
  redrawWholeScreen = 1;

  clear_keybuf();
  if (lastGimmick < 0) {
    lastGimmick += LJGM_N_GIMMICKS + N_NONGIMMICKS;
  }

  for(int done = 0; done == 0; ) {
    if (redrawWholeScreen) {
      redrawWholeScreen = 0;
      acquire_screen();
      clear_to_color(screen, bgColor);
      textout_ex(screen, aver32, "LOCKJAW > Play", 16, 32, fgColor, -1);
      textout_ex(screen, aver32, "Select a gimmick:", 16, 90, fgColor, -1);
      
      for (int gimmick = 0;
            gimmick < N_GIMMICKS + N_NONGIMMICKS;
            ++gimmick) {
        getGimmickDrawRow(gimmick, gimmick == lastGimmick);
      }
      textout_ex(screen, aver16, "Arrows: move; Rotate Right: start",
                 16, 240 + 40 * (GIMMICKS_PER_COL + 1), fgColor, -1);
      release_screen();
    }
    while (keypressed()) {
      int scancode;
      ureadkey(&scancode);
      if (scancode == KEY_PRTSCR) {
        saveScreen();
      }
    }

    int gimmick = lastGimmick;    
    LJBits keys = menuReadPad();
    LJBits newKeys = keys & ~lastKeys;

    if ((newKeys & VKEY_ROTL) || wantsClose) {
      gimmick = N_GIMMICKS + N_NONGIMMICKS - 1;
      ezPlaySample("rotate_wav", 128);
    }
    if ((newKeys & VKEY_ROTR) || wantsClose) {
      done = 1;
      ezPlaySample("line_wav", 128);
    }

    if (newKeys & VKEY_UP) {
      if (gimmick <= 0) {
        gimmick = N_GIMMICKS + N_NONGIMMICKS - 1;
      } else {
        --gimmick;
      }
      ezPlaySample("shift_wav", 128);
    }
    if (newKeys & VKEY_DOWN) {
      ++gimmick;
      if (gimmick >= N_GIMMICKS + N_NONGIMMICKS) {
        gimmick = 0;
      }
      ezPlaySample("shift_wav", 128);
    }

    if (newKeys & VKEY_LEFT) {
      if (gimmick < GIMMICKS_PER_COL) {
        gimmick += (((N_GIMMICKS + N_NONGIMMICKS)
                     / GIMMICKS_PER_COL)
                    )
                   * GIMMICKS_PER_COL;
        if (gimmick >= N_GIMMICKS + N_NONGIMMICKS) {
          gimmick -= GIMMICKS_PER_COL;
        }
      } else {
        gimmick -= GIMMICKS_PER_COL;
      }
      ezPlaySample("shift_wav", 128);
    }
    if (newKeys & VKEY_RIGHT) {
      gimmick += GIMMICKS_PER_COL;
      if (gimmick >= N_GIMMICKS + N_NONGIMMICKS) {
        gimmick %= GIMMICKS_PER_COL;
      }
      ezPlaySample("shift_wav", 128);
    }

    if (gimmick != lastGimmick) {
      vsync();
      acquire_screen();
      getGimmickDrawRow(lastGimmick, 0);
      getGimmickDrawRow(gimmick, 1);
      release_screen();
      lastGimmick = gimmick;
    } else {
      rest(30);
    }
    lastKeys = keys;
  }

  // Wrap the nongimmicks into the negative numbers.
  if (lastGimmick >= N_GIMMICKS) {
    lastGimmick -= (N_GIMMICKS + N_NONGIMMICKS);
  }

  return lastGimmick;
}

/**
 * Pauses the game, returning nonzero if the player wants to quit.
 */
int pauseGame(LJPCView *v) {
  int escCount = 0;
  int quit = 0;
  int escHold = 1;
  redrawWholeScreen = 1;
  
  LJMusic_pause(v->bgm, 1);
  while (escCount < 2 && !quit) {
    LJMusic_poll(v->bgm);
    if (redrawWholeScreen) {
      redrawWholeScreen = 0;
      acquire_screen();
      clear_to_color(screen, pfBgColor);
      textout_centre_ex(screen, aver32, "LOCKJAW: GAME PAUSED",
                        SCREEN_W / 2, 200, pfFgColor, -1);
      textout_centre_ex(screen, aver32, "Press Esc to continue",
                        SCREEN_W / 2, 250, pfFgColor, -1);
      textout_centre_ex(screen, aver32, "or hold Esc to quit",
                        SCREEN_W / 2, 300, pfFgColor, -1);
      release_screen();
    }
    
    if (key[KEY_ESC]) {
      vsync();
      ++escHold;
      if (escHold >= 60) {
        quit = 1;
      }
    } else {
      if (escHold) {
        ++escCount;
      }
      escHold = 0;
      rest(30);
    }
    if (wantsClose) {
      quit = 1;
    }
    
  }
  LJMusic_pause(v->bgm, 0);
  redrawWholeScreen = 1;
  clear_keybuf();
  return quit;
}

const char *const optionsSideNames[] = {
  "Left", "Middle", "Right"
};


void playSoundEffects(LJView *v, LJBits sounds, int countdown) {
  // Handle sound
  if ((sounds & LJSND_SPAWN)
       && !(v->hideNext)) {
    playSampleForTetromino(v->field->curPiece[1]);
  }
  if (sounds & LJSND_HOLD) {
    ezPlaySample("hold_wav", 128);
  }
  if (sounds & LJSND_ROTATE) {
    ezPlaySample("rotate_wav", 128);
  }
  if (sounds & LJSND_IRS) {
    ezPlaySample("irs_wav", 128);
  }
  if (sounds & LJSND_SHIFT) {
    ezPlaySample("shift_wav", 128);
  }
  if (sounds & LJSND_LAND) {
    ezPlaySample("land_wav", 192);
  }
  if (sounds & LJSND_LOCK) {
    ezPlaySample("lock_wav", 192);
  }
  if (sounds & LJSND_LINE) {
    ezPlaySample("line_wav", 128);
  }
  if (sounds & LJSND_SECTIONUP) {
    ezPlaySample("sectionup_wav", 128);
  }
  if (v->plat->b2bcd1 > 0) {
    if (--v->plat->b2bcd1 == 0) {
      playPitchSample("line_wav", 148, 1200);
    }
  }
  if (sounds & LJSND_SETB2B) {
    v->plat->b2bcd1 = 10;
  }
  if (v->plat->b2bcd2 > 0) {
    if (--v->plat->b2bcd2 == 0) {
      playPitchSample("line_wav", 168, 1440);
    }
  }
  if (sounds & LJSND_B2B) {
    v->plat->b2bcd2 = 20;
  }
  if (countdown < v->control->countdown) {
    char name[32];
    usprintf(name, "count_%d_wav", countdown);
    ezPlaySample(name, 128);
  }
  LJMusic_poll(v->plat->bgm);
}

void pcInit(LJView *v, union LJPrefs *prefs) {
  v->plat->b2bcd1 = 0;
  v->plat->b2bcd2 = 0;
  v->plat->baseX = 200 * prefs->number[LJPREFS_SIDE];
  
  // If the player has chosen to use more next pieces than the
  // next piece position can handle, set the number of
  // next pieces for correctness of debrief().
  if (v->nextPieces > 3 && v->plat->nextPos == LJNEXT_TOP) {
    v->nextPieces = 3;
  }
}

/**
 * Redraws everything on the screen.
 * Called when needs redraw.
 */
void playRedrawScreen(LJView *v) {
  blit(v->plat->bg, screen,
       0, 0,
       0, 0,
       SCREEN_W, SCREEN_H);
  v->frontDirty = ~0;
}

void startingAniWantsSkip(LJView *v) {
  LJInput unusedIn;
  addKeysToInput(&unusedIn, readPad(), v->field, v->control);
}

void startingAnimation(LJView *v) {
  int readyGoX = v->plat->baseX + v->plat->blkW * LJ_PF_WID / 2;
  int readyGoY = v->plat->baseY
                 - v->plat->blkH
                   * (LJ_PF_VIS_HT - v->field->ceiling * 2 / 5);

  clear_keybuf();
  v->backDirty = 0;
  
  acquire_screen();
  playRedrawScreen(v);
  blitField(v);
  textout_centre_ex(screen, aver32, "Ready",
                    readyGoX, readyGoY, pfFgColor, -1);
  release_screen();
  
  ezPlaySample("ready_wav", 128);
  rest(600);
  v->frontDirty = ~0;
  
  if (!wantsClose) {
    acquire_screen();
    blitField(v);
    textout_centre_ex(screen, aver32, "GO!",
                    readyGoX, readyGoY, pfFgColor, -1);
    drawScore(v);
    release_screen();

    ezPlaySample("go_wav", 128);
    v->frontDirty = ~0;
    rest(200);
  }
  if (!wantsClose) {
    playSampleForTetromino(v->field->curPiece[1]);
    rest(400);
  }
}


static void gameOverAnimation(const LJPCView *const v, const LJField *p, int won) {
  int ceiling = p->ceiling;
  int left = v->baseX + p->leftWall * v->blkW;
  int right = v->baseX + p->rightWall * v->blkW - 1;

  ezPlaySample("sectionup_wav", 0);
  if (!won) {
    ezPlaySample("gameover_wav", 256);
  } else {
    ezPlaySample("win_wav", 256);
  }

  int minRow = LJ_PF_VIS_HT - ceiling;

  for (int t = LJ_PF_VIS_HT + v->blkH - 2; t >= minRow; --t) {
    acquire_screen();
    for (int row = LJ_PF_VIS_HT - 1; row >= minRow; --row) {
      int ysub = t - row;

      if (ysub >= 0 && ysub < v->blkH) {
        int y = v->baseY - row * v->blkH - ysub - 1;
        hline(screen, left, y, right, pfBgColor);
      }
    }
    release_screen();
    vsync();
    if (wantsClose) {
      t = 0;
    }
  }
}

#define MENU_COPR_NOTICE_LINES 4
const char *const menuCoprNotice[MENU_COPR_NOTICE_LINES] = {
  "Copr. 2006-2007 Damian Yerrick",
  "Not sponsored or endorsed by The Tetris Company.",
  "LOCKJAW comes with ABSOLUTELY NO WARRANTY.  This is free software, and you are",
  "welcome to redistribute it under certain conditions as described in GPL.txt."
};

static BITMAP *buildTitleScreen(void) {
  BITMAP *back = create_system_bitmap(800, 600);
  if (!back) {
    return NULL;
  }

  // Gradient from (0, 0, 0) to (0, 0, 153)
  for (int y = 0; y < 192; ++y) {
    for (int x = -((y * 13) & 0x1F);
          x < 800;
          x += 32) {
      int colValue = y + ((rand() & 0x7000) >> 12);
      int c = makecol(0, 0, 153 * colValue / 192);
      hline(back, x, y, x + 31, c);
    }
  }
  
  // Gradient from (102, 51, 0) to (204, 102, 0)
  for (int y = 192; y < 384; ++y) {
    for (int x = -((y * 13) & 0x1F);
          x < 800;
          x += 32) {
      int colValue = y + ((rand() & 0x7800) >> 11);
      int c = makecol(102 * colValue / 192, 51 * colValue / 192, 0);
      hline(back, x, y, x + 31, c);
    }
  }
  
  // Gradient from (204, 102, 0) to (255, 128, 0)
  for (int y = 384; y < 600; ++y) {
    for (int x = -((y * 13) & 0x1F);
          x < 800;
          x += 32) {
      int colValue = y - 400 + ((rand() & 0x7C00) >> 10);
      if (colValue > 600 - 384) {
        colValue = 600 - 384;
      }
      int c = makecol(204 + 50 * colValue / (600 - 384),
                      102 + 25 * colValue / (600 - 384),
                      0);
      hline(back, x, y, x + 31, c);
    }
  }

  DATAFILE *obj = find_datafile_object(dat, "arttitle_bmp");
  BITMAP *logo = obj ? obj->dat : NULL;
  obj = find_datafile_object(dat, "arttitle_pal");
  const RGB *pal = obj ? obj->dat : NULL;
  
  if (logo && pal) {
    set_palette(pal);
    draw_sprite(back, logo, 400 - logo->w / 2, 192 - logo->h / 2);
    //unselect_palette();
  }

  textout_centre_ex(back, aver32, "Arrows: change; Enter: choose",
                    400, 440,
                    0, -1);
                    
  textout_centre_ex(back, aver32, "LOCKJAW: The Reference "LJ_VERSION,
                    400, 550,
                    0, -1);

  return back;
}

#define N_TITLE_ACTIONS 4
static const char *titleActions[N_TITLE_ACTIONS] = {
  "Exit", "Play", "Options", "Game Keys"
};
/*
  0: Exit
  1: Play
  2
*/
int title(void) {
  BITMAP *back = buildTitleScreen();
  LJBits lastKeys = ~0;
  int redraw = 1;
  int choice = 1;

  if (!back) {
    alert("Not enough memory to display the title screen.",
          "(If you don't even have RAM for a title screen,",
          "then what do you have RAM for?)",
          "Exit", 0, 13, 0);
    return 0;
  }
  
  redrawWholeScreen = 1;

  for(int done = 0; done == 0;) {
    if (redrawWholeScreen) {
      redrawWholeScreen = 0;
      blit(back, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      redraw = 1;
    }
    if (redraw) {
      int dehilite = makecol(221, 153, 85);
      int white = makecol(255, 255, 255);
      redraw = 0;
      vsync();
      blit(back, screen, 0, 400, 0, 400, SCREEN_W, 30);
      textout_centre_ex(screen, aver32,
                        titleActions[choice],
                        400, 400, white, -1);
      textout_centre_ex(screen, aver32,
                        titleActions[choice == 0 ? N_TITLE_ACTIONS - 1 : choice - 1],
                        200, 400, dehilite, -1);
      textout_centre_ex(screen, aver32,
                        titleActions[choice == N_TITLE_ACTIONS - 1 ? 0 : choice + 1],
                        600, 400, dehilite, -1);
    }

    while (keypressed()) {
      int scancode;
      ureadkey(&scancode);
      if (scancode == KEY_PRTSCR) {
        saveScreen();
      }
    }

    LJBits keys = menuReadPad();
    LJBits newKeys = keys & ~lastKeys;

    if (newKeys & VKEY_LEFT) {
      --choice;
      redraw = 1;
      ezPlaySample("shift_wav", 128);
    }
    if (newKeys & VKEY_RIGHT) {
      ++choice;
      redraw = 1;
      ezPlaySample("shift_wav", 128);
    }
    if (newKeys & VKEY_ROTL) {
      choice = 0;
      redraw = 1;
      ezPlaySample("rotate_wav", 128);
    }
    if (newKeys & VKEY_ROTR) {
      done = 1;
      ezPlaySample("line_wav", 128);
    }
    if (choice < 0) {
      choice += 4;
    }
    if (choice >= 4) {
      choice -= 4;
    }

    lastKeys = keys;
    
    if (!redraw) {
      rest(30);
    }
    if (wantsClose) {
      done = 1;
      choice = 0;
    }
  }
  destroy_bitmap(back);
  return choice;
}


void setupWindow(void) {
  set_window_title("LOCKJAW");
  bgColor = makecol(255, 255, 255);
  pfFgColor = bgColor;
  hiliteColor = makecol(255, 255, 204);
  

  acquire_screen();
  clear_to_color(screen, pfBgColor);
  for (int i = 0; i < MENU_COPR_NOTICE_LINES; ++i) {
    textout_ex(screen, font, menuCoprNotice[i],
               16, 580 + 12 * (i - MENU_COPR_NOTICE_LINES),
               pfFgColor, -1);
  }
  release_screen();
  refreshRate = get_refresh_rate();
}

#define TRY_OVERLAY 0
int openWindow(int windowed)
{
  int depth = desktop_color_depth();
  int card = windowed ? GFX_AUTODETECT_WINDOWED : GFX_AUTODETECT_FULLSCREEN;

  if (depth < 15) {
    depth = 16;
  }

  // Full screen procedure
  set_color_depth(depth);
  if (set_gfx_mode(card, 800, 600, 0, 0) == 0) {
    setupWindow();
    return 0;
  }
    
  // Windows can't tell 16 bit from 15 bit. If desktop color depth is reported as 16, try 15 too.
  if (depth == 16) {
    set_color_depth(15);
    if (set_gfx_mode(card, 800, 600, 0, 0) == 0) {
      setupWindow();
      return 0;
    }
  }

  return -1;
}

BITMAP *loadConnections(const char *filename) {
  BITMAP *src = load_bitmap(filename, NULL);
  if (!src) {
    return NULL;
  }
  BITMAP *dst = create_system_bitmap(24*16, 24*16);
  if (!dst) {
    destroy_bitmap(src);
    return NULL;
  }
  acquire_bitmap(dst);
  for (unsigned int col = 0; col < 16; ++col) {
    unsigned int srcXBase = (col & 0x03) * 48;
    unsigned int srcYBase = (col >> 2) * 48;
    unsigned int dstYBase = col * 24;
    for (unsigned int conn = 0; conn < 16; ++conn) {
      unsigned int dstXBase = conn * 24;
      unsigned int topSegY = (conn & CONNECT_U) ? 24 : 0;
      unsigned int botSegY = (conn & CONNECT_D) ? 12 : 36;
      unsigned int leftSegX = (conn & CONNECT_L) ? 24 : 0;
      unsigned int rightSegX = (conn & CONNECT_R) ? 12 : 36;
      blit(src, dst,
           srcXBase + leftSegX, srcYBase + topSegY,
           dstXBase + 0, dstYBase + 0,
           12, 12);
      blit(src, dst,
           srcXBase + rightSegX, srcYBase + topSegY,
           dstXBase + 12, dstYBase + 0,
           12, 12);
      blit(src, dst,
           srcXBase + leftSegX, srcYBase + botSegY,
           dstXBase + 0, dstYBase + 12,
           12, 12);
      blit(src, dst,
           srcXBase + rightSegX, srcYBase + botSegY,
           dstXBase + 12, dstYBase + 12,
           12, 12);
    }
  }
  release_bitmap(dst);
  destroy_bitmap(src);
  return dst;
}


void closeWindow(void) {
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

static void mainCleanup(LJPCView *v) {
  if (v->back) {
    destroy_bitmap(v->back);
  }
  if (v->bg) {
    destroy_bitmap(v->bg);
  }
  if (v->blocks) {
    destroy_bitmap(v->blocks);
  }
  if (v->connBlocks) {
    destroy_bitmap(v->connBlocks);
  }
  LJMusic_delete(v->bgm);
  if (withSound) {
    remove_sound();
  }
  closeWindow();
}

char skinName[PATH_MAX] = "skin.ini";
char ljblocksSRSName[PATH_MAX] = "ljblocks.bmp";
char ljblocksSegaName[PATH_MAX] = "ljblocks-sega.bmp";
char ljconnSRSName[PATH_MAX] = "ljconn.bmp";
char ljconnSegaName[PATH_MAX] = "ljconn-sega.bmp";
char ljbgName[PATH_MAX] = "ljbg.jpg";
char bgmName[PATH_MAX] = "bgm.s3m";


/**
 * Converts a single hexadecimal digit to its value.
 * @param in USASCII/Unicode value of hex digit character
 * @return value
*/
int hexDigitValue(int in) {
  if (in >= '0' && in <= '9') {
    return in - '0';
  } else if (in >= 'A' && in <= 'F') {
    return in - 'A' + 10;
  } else if (in >= 'a' && in <= 'f') {
    return in - 'a' + 10;
  } else {
    return -1;
  }
}

int translateComponent(const char **in, size_t nDigits) {
  const char *here = *in;
  int hi = hexDigitValue(*here++);
  int lo = nDigits > 3 ? hexDigitValue(*here++) : hi;
  *in = here;
  if (hi >= 0 && lo >= 0) {
    return hi * 16 + lo;
  } else {
    return -1;
  }
}

/**
 * Interprets a hexadecimal color specifier.
 * @param in hexadecimal color specifier, in #ABC or #AABBCC format
 * @return Allegro device-dependent color, in makecol() format
 */
int translateColor(const char *in) {
  size_t nDigits = 0;

  // Verify and skip #
  if (*in != '#') {
    return -1;
  }
  ++in;

  // Determine whether we have a 3-digit color or a 6-digit color
  for (const char *here = in;
       hexDigitValue(*here) >= 0;
       ++here) {
    ++nDigits;
  }
  if (nDigits != 3 && nDigits != 6) {
    return -1;
  }
  
  int red = translateComponent(&in, nDigits);
  int green = translateComponent(&in, nDigits);
  int blue = translateComponent(&in, nDigits);
  if (red >= 0 && green >= 0 && blue >= 0) {
    return makecol(red, green, blue);
  } else {
    return -1;
  }
};


int loadSkinNames(void) {
  FILE *fp = fopen(skinName, "rt");
  char key[1024], var[1024], val[1024], input_buf[1024];

  if (!fp) return 0;

  key[0] = 0;
  var[0] = 0;
  val[0] = 0;

  while(1) {
    int rval;

    if(!fgets (input_buf, sizeof(input_buf), fp))
      break;
    rval = parse_ini_line(input_buf, key, var, val);

    if(!ustrcmp ("ljblocksSRS", var)) {
      ustrzcpy(ljblocksSRSName, sizeof(ljblocksSRSName) - 1, val);
    }
    else if(!ustrcmp ("ljblocksSega", var)) {
      ustrzcpy(ljblocksSegaName, sizeof(ljblocksSegaName) - 1, val);
    }
    else if(!ustrcmp ("ljconnSRS", var)) {
      ustrzcpy(ljconnSRSName, sizeof(ljconnSRSName) - 1, val);
    }
    else if(!ustrcmp ("ljconnSega", var)) {
      ustrzcpy(ljconnSegaName, sizeof(ljconnSegaName) - 1, val);
    }
    else if(!ustrcmp ("bgm", var)) {
      ustrzcpy(bgmName, sizeof(bgmName) - 1, val);
    }
    else if(!ustrcmp ("ljbg", var)) {
      ustrzcpy(ljbgName, sizeof(ljbgName) - 1, val);
    }
    else if(!ustrcmp ("pfbgcolor", var)) {
      int c = translateColor(val);
      if (c >= 0) {
        pfBgColor = c;
      }
    }
    else if(!ustrcmp ("pfcolor", var)) {
      int c = translateColor(val);
      if (c >= 0) {
        pfFgColor = c;
      }
    }
    else if(!ustrcmp ("bgcolor", var)) {
      int c = translateColor(val);
      if (c >= 0) {
        bgColor = c;
      }
    }
    else if(!ustrcmp ("color", var)) {
      int c = translateColor(val);
      if (c >= 0) {
        fgColor = c;
      }
    }
    else if(!ustrcmp ("hilitecolor", var)) {
      int c = translateColor(val);
      if (c >= 0) {
        hiliteColor = c;
      }
    }
  }
  fclose(fp);
  return 0;
}


static void drawProgressSegment(int min, int max) {
  int blue = makecol(0, 0, 255);
  rectfill(screen, min * 8, 592, max * 8 - 1, 595, blue);
  int orange = makecol(255, 128, 0);
  rectfill(screen, min * 8, 596, max * 8 - 1, 599, orange);
}

static void loadSkin(LJView *v) {
  BITMAP *bmp;
  int colorScheme = wkColorScheme[v->field->rotationSystem];
  
  rectfill(screen, 0, 592, 799, 599, 0);
  
  loadSkinNames();
  drawProgressSegment(0, 20);
  
  bmp = load_bitmap(ljbgName, NULL);
  if (bmp) {
    if (v->plat->bg) {
      destroy_bitmap(v->plat->bg);
    }
    
    // If the image size doesn't match the window size, resize it
    if (bmp->w != SCREEN_W || bmp->h != SCREEN_H) {
      BITMAP *resized = create_bitmap(SCREEN_W, SCREEN_H);

      if (resized) {
        stretch_blit(bmp, resized, 0, 0, bmp->w, bmp->h,
                     0, 0, SCREEN_W, SCREEN_H);
        destroy_bitmap(bmp);
      }
      bmp = resized;
    }
    v->plat->bg = bmp;
  }
  drawProgressSegment(20, 40);
  
  bmp = load_bitmap(colorScheme
                    ? ljblocksSegaName
                    : ljblocksSRSName,
                    NULL);
  if (bmp) {
    if (v->plat->blocks) {
      destroy_bitmap(v->plat->blocks);
    }
    v->plat->blocks = bmp;
  }
  drawProgressSegment(40, 60);
  
  bmp = loadConnections(colorScheme
                        ? ljconnSegaName
                        : ljconnSRSName);
  if (bmp) {
    if (v->plat->connBlocks) {
      destroy_bitmap(v->plat->connBlocks);
    }
    v->plat->connBlocks = bmp;
  }
  drawProgressSegment(60, 80);
  
  LJMusic_load(v->plat->bgm, bgmName);
  drawProgressSegment(80, 100);
}

int pickReplay(void) {
  FONT *oldFont = font;
  font = (FONT *)aver16;
  install_mouse();
  int got = file_select_ex("Choose a demo:", demoFilename, "ljm", sizeof(demoFilename), 600, 400);
  remove_mouse();
  font = oldFont;
  return got ? 0 : -1;
}

int main(void) {
  LJField p = {
  };
  LJControl control = {
    .replaySrc = 0,
    .replayDst = 0    
  };
  LJPCView platView = {
    .baseY = 552,
    .blkW = 24,
    .blkH = 24
  };
  LJView mainView = {
    .field = &p,
    .control = &control,
    .plat = &platView,
    .backDirty = ~0
  };
  
  union LJPrefs prefs = {
    .number = {
      [LJPREFS_WIDTH] = 10 - 4,
      [LJPREFS_HEIGHT] = 20 - 8,
      [LJPREFS_ROTATION_SYSTEM] = 0,
      [LJPREFS_KICK_LIMIT] = 6,
      [LJPREFS_LOCK_RESET] = LJLOCK_MOVE,
      [LJPREFS_LOCK_DELAY] = 0,
      [LJPREFS_RANDOMIZER] = 0,

      [LJPREFS_T_SPIN_ALGO] = LJTS_TDS,
      [LJPREFS_SPEED_CURVE] = LJSPD_EXP,
      [LJPREFS_CLEAR_GRAVITY] = 0,
      [LJPREFS_SQUARE_MODE] = 0,
      [LJPREFS_SCORE_STYLE] = 0,

      [LJPREFS_ARE_STYLE] = 0,
      [LJPREFS_DAS_SPEED] = 1,
      [LJPREFS_DAS_DELAY] = 10,
      [LJPREFS_INITIAL_DAS] = 1,
      [LJPREFS_IRS] = 1,
      [LJPREFS_ALLOW_DIAGONALS] = 0,

      [LJPREFS_SOFT_DROP_SPEED] = 0,
      [LJPREFS_SOFT_DROP_LOCK] = 0,
      [LJPREFS_HARD_DROP_LOCK] = 1,
      [LJPREFS_NEXT_PIECES] = 8,
      [LJPREFS_HIDE_SHADOW] = LJSHADOW_COLORED_50,

      [LJPREFS_SMOOTH_GRAVITY] = 1,
      [LJPREFS_TRAILS] = 1,
      [LJPREFS_SIDE] = 0,
      [LJPREFS_NEXT_POS] = 0,
      [LJPREFS_AUTO_PAUSE] = 1,
      [LJPREFS_AUTO_RECORD] = 0,
      [LJPREFS_WINDOWED] = 1
    }
  };
  
  allegro_init();
  install_timer();

  loadOptions(&prefs);

  if (openWindow(prefs.number[LJPREFS_WINDOWED]) != 0) {
    allegro_message("LOCKJAW fatal error: Could not open an 800x600 pixel %s.\n"
                    "Trying %s next time.\n",
                    prefs.number[LJPREFS_WINDOWED] ? "window": "screen mode",
                    prefs.number[LJPREFS_WINDOWED] ? "the full screen": "a window");
    prefs.number[LJPREFS_WINDOWED] = !prefs.number[LJPREFS_WINDOWED];
    saveOptions(&prefs);
    return EXIT_FAILURE;
  }
  LOCK_FUNCTION(incCurTime);
  LOCK_VARIABLE(curTime);
  install_int_ex(incCurTime, BPM_TO_TIMER(3600));
  
  jpgalleg_init();
  
  set_color_conversion(COLORCONV_NONE);
  dat = load_datafile("lj.dat");
  set_color_conversion(COLORCONV_TOTAL);
  if(!dat) {
    closeWindow();
    allegro_message("LOCKJAW fatal error: Could not load datafile lj.dat\n");
    return 1;
  }

  {
    const DATAFILE *aver16dat = find_datafile_object(dat, "Aver16_bmp");
    aver16 = aver16dat ? aver16dat->dat : font;
    const DATAFILE *aver32dat = find_datafile_object(dat, "Aver32_bmp");
    aver32 = aver32dat ? aver32dat->dat : aver16;
  }

  LOCK_FUNCTION(amnesia);
  LOCK_VARIABLE(redrawWholeScreen);
  
  // If we can be notified on switching out, take this notification. 
  if (set_display_switch_mode(SWITCH_BACKGROUND) >= 0
      || set_display_switch_mode(SWITCH_BACKAMNESIA) >= 0) {
    set_display_switch_callback(SWITCH_OUT, requestPause);
  }
  set_display_switch_callback(SWITCH_IN, amnesia);

  install_keyboard();
  initKeys();

  p.seed = time(NULL);

  reserve_voices(8, 0);
  set_volume_per_voice(0);
#ifdef ALLEGRO_WINDOWS
  // Under Windows, use the Allegro mixer because on my machine
  // and probably others, the built-in mixer will replace the very
  // beginning of one sound with the end of the last sound played
  // on that voice.
  withSound = !install_sound(DIGI_DIRECTAMX(0), MIDI_NONE, NULL);
#else
  withSound = !install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
#endif
  platView.bgm = LJMusic_new();

  unpackOptions(&mainView, &prefs);
  loadSkin(&mainView);
  
  // Needs to be created after mainView fields are constructed
  // because they would otherwise depend on fields that had
  // not yet been initialized.
  if(!platView.bg) {
    platView.bg = create_bitmap(SCREEN_W, SCREEN_H);
    if (platView.bg) {
      allegro_message("Background image \"%s\" not found.\n"
                      "Using plain background instead.\n",
                      ljbgName);
      clear_to_color(platView.bg, bgColor);
    } else {
      mainCleanup(&platView);
      allegro_message("Background image \"%s\" not found.\n",
                      ljbgName);
      return 1;
    }
  }
  if(!platView.blocks) {
    mainCleanup(&platView);
    allegro_message("Blocks image \"%s\" not found.\n",
                    p.rotationSystem
                    ? ljblocksSegaName
                    : ljblocksSRSName);
    return 1;
  }

  platView.back = create_system_bitmap(platView.blkW * LJ_PF_WID,
                                       platView.blkH * LJ_PF_VIS_HT);
  if(!platView.back) {
    mainCleanup(&platView);
    allegro_message("Could not create back buffer.\n");
    return 1;
  }
  
  srand(time(NULL));
  p.gimmick = 0;

  // Wait for copyright notice to be displayed "conspicuously"
  if (curTime < 180) {
    rest(3100 - curTime * 16);
  }

  for (int action = title();
        action > 0;
        action = title()) {
    switch (action) {
    case 1:
      for (int gimmick = getGimmick(p.gimmick);
           gimmick != -1 && !wantsClose;
           gimmick = getGimmick(p.gimmick))
      {
        if (gimmick >= 0 || gimmick == -2) {
          if (gimmick == -2 && pickReplay() < 0) {
            break;
          }
          p.gimmick = gimmick;
          unpackOptions(&mainView, &prefs);
          pcInit(&mainView, &prefs);
          wantPause = 0;
          platView.wantRecord = gimmick >= 0
                                && prefs.number[LJPREFS_AUTO_RECORD];
          LJMusic_start(platView.bgm,
                        4096,  // mix buffer size
                        128);  // volume scale
          play(&mainView);
          LJMusic_stop(platView.bgm);
          if (!wantsClose) {
            gameOverAnimation(&platView, &p, control.countdown <= 0);
            debrief(&mainView);
          }
        }
      }
      break;

    case 2:
      options(&prefs);

      // reload the skin if the player changed the gametype
      // or (in future versions) chose a different skin.ini
      unpackOptions(&mainView, &prefs);
      loadSkin(&mainView);
      break;
      
    case 3:
      configureKeys();
      break;
    
    }
  }
  
  mainCleanup(&platView);
  return 0;
} END_OF_MAIN();
