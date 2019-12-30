/* Game loop frontend for LOCKJAW, an implementation of the Soviet Mind Game

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

#include "ljplay.h"
#ifndef WITH_REPLAY
#define WITH_REPLAY 0
#endif

#if WITH_REPLAY
#include "ljreplay.h"
extern const char demoFilename[];
#endif


void play(LJView *v) {
  LJField *const p = v->field;
  int canceled = 0;

  v->control->countdown = 6;

  /* Load replay if needed */
#if WITH_REPLAY
  if (p->gimmick < 0) {
    v->control->replaySrc = openReplay(demoFilename, p);
    if (!v->control->replaySrc) {
      return;
    }
    v->backDirty = ~0;
    playRedrawScreen(v);
  } else {
    v->control->replaySrc = 0;
  }
  if (!v->control->replaySrc)
#endif
  {
    newGame(p);
    initGimmicks(p);
    v->control->presses = 0;
    v->nLockTimes = 0;
    v->hideNext = 0;

    updField(v, ~0);
    startingAnimation(v);
    v->control->lastKeys = 0;
    v->control->repressKeys = 0;
  }
  blitField(v);

  int lastTime = getTime();

  while(p->state != LJS_GAMEOVER && !canceled) {
    if (getTime() == lastTime) {
      yieldCPU();
    }
    while (getTime() - lastTime > 0) {
      LJInput in = {0, 0, 0, 0};
      int curTime = getTime();
      
      addKeysToInput(&in, readPad(), p, v->control);
      canceled |= ljHandleConsoleButtons(v);

      // when returning from pause, catch up the speed control
      if (curTime - lastTime > 10
          || curTime - lastTime < -10) {
        lastTime = curTime;
      }

      {
        int irsAttempted = (p->sounds & (LJSND_SPAWN | LJSND_HOLD))
                           && in.rotation && v->control->initialRotate;
                           
        // The next line does ALL the game logic.
        LJBits updatedRows = frame(p, &in) | gimmicks(p, v->control);
        
        v->backDirty |= updatedRows;
        if (irsAttempted && (p->sounds & LJSND_ROTATE)) {
          p->sounds |= LJSND_IRS;
        }
      }

      // items is a partly view-based gimmick
      if (p->gimmick == LJGM_ITEMS && (p->sounds & LJSND_SPAWN)) {
        v->hideNext = (v->field->nPieces > 7);
        v->frontDirty |= LJ_DIRTY_NEXT;
      }

      int curCountdown = 6;
      if (p->gimmick == LJGM_ULTRA && p->gameTime >= 10500) {
        curCountdown = (int)(10859 - p->gameTime) / 60;
      } else if (p->gimmick == LJGM_BTYPE) {
        curCountdown = 40 - p->lines;
      } else if (p->gimmick == LJGM_BABY) {
        curCountdown = (309 - v->control->presses) / 10;
      }
      
      // we have to wait for the new piece to come out
      // so that the score is credited properly
      if (curCountdown <= 0
          && (p->state == LJS_NEW_PIECE
              || p->state == LJS_FALLING
              || p->state == LJS_LANDED)) {
        p->state = LJS_GAMEOVER;
      }
      
      playSoundEffects(v, p->sounds, curCountdown);
      v->control->countdown = curCountdown;

      // Update speedometer
      if (p->sounds & LJSND_LOCK) {
        for (int i = N_SPEED_METER_PIECES - 2; i >= 0; --i) {
          v->lockTime[i + 1] = v->lockTime[i];
        }
        v->lockTime[0] = p->gameTime;
        if (v->nLockTimes < N_SPEED_METER_PIECES) {
          ++v->nLockTimes;
        }
        v->frontDirty = LJ_DIRTY_SCORE;
      }

      // If the piece was just spawned, move the trail to the piece's
      // starting position and redraw next pieces.
      if (p->sounds & (LJSND_SPAWN | LJSND_HOLD)) {
        v->trailY = p->y;
        v->frontDirty |= LJ_DIRTY_NEXT;
      }

      ++lastTime;
    }

    if (p->state == LJS_GAMEOVER && v->hidePF) {
      v->hidePF = 0;
      v->backDirty |= (1 << LJ_PF_VIS_HT) - 1;
    }
    
    updField(v, v->backDirty);
    v->frontDirty |= v->backDirty;
    v->backDirty = 0;
    
    // If piece is falling or landed, and it wasn't just spawned,
    // draw the piece and its shadow.
    
    // If the piece was just spawned, don't draw the piece.
    if (p->sounds & (LJSND_SPAWN | LJSND_HOLD)) {
    }
    // Otherwise, if the piece is falling or landed, draw it.
    else if (p->state == LJS_FALLING || p->state == LJS_LANDED
              || p->sounds & LJSND_LOCK) {
      if (v->hideShadow != LJSHADOW_NO_FALLING) {
        if (p->state == LJS_FALLING && v->hideShadow < LJSHADOW_NONE) {
          drawShadow(v);
        }
        drawFallingPiece(v);
      }
    }

    ljBeginDraw(v, getTime() - lastTime < 2);
    drawScore(v);
    blitField(v);
    ljEndDraw(v);
  }
  
#if WITH_REPLAY
  if (v->control->replaySrc) {
    replayClose(v->control->replaySrc);
    v->control->replaySrc = NULL;
  }
  if (v->control->replayDst) {
    replayClose(v->control->replayDst);
    v->control->replayDst = NULL;
  }
#endif
}
