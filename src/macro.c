/* Macro handling for LOCKJAW, an implementation of the Soviet Mind Game

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
#include "lj.h"
#include "ljcontrol.h"
#include "ljreplay.h"
#include "pcjoy.h"

// first is rotation (+1 = 90 deg clockwise)
// second is movement (+1 = right 1 block)
// third is gravity (+1 = down 1/8 block)
// fourth is extra actions (hold, lock)
LJInput macros[8] = {
  { -1,  0,    0, 0 },  // default: alt. rotate left
  { -2,  0,    0, 0 },  // default: rotate left twice
  {  0, -9,    0, 0 },  // default: far left
  {  0,  9,    0, 0 },  // default: far right
  {  0,  0, 20*8, 0 },  // default: firm drop
  {  0,  0,    0, LJI_HOLD },  // default: alternate hold
  {  0,  0,    0, 0 },
  {  0,  0,    0, 0 }  
};

void addMacrosToInput(LJInput *dst, LJBits keys) {
  int rotation = dst->rotation;
  int movement = dst->movement;
  int gravity = dst->gravity;
  int other = dst->other;
  int macro;

  keys >>= 8;
  for (macro = 0;
       macro < 8;
       keys >>= 1, ++macro) {
    if (keys & 1) {
      rotation += macros[macro].rotation;
      movement += macros[macro].movement;
      gravity += macros[macro].gravity;
      other |= macros[macro].other;
    }
  }

  // Clip rotation to [-3, +3]
  rotation -= rotation / 4 * 4;

  // Clip movement to playfield width
  if (movement < (int)-LJ_PF_WID) {
    movement = -LJ_PF_WID;
  } else if (movement > (int)LJ_PF_WID) {
    movement = LJ_PF_WID;
  }

  // Clip gravity to playfield height
  if (gravity > LJ_PF_HT * 8) {
    gravity = LJ_PF_HT * 8;
  }
  
  dst->rotation = rotation;
  dst->movement = movement;
  dst->gravity = gravity;
  dst->other = other;
}

static const LJFixed softDropSpeeds[3] = {
  LJITOFIX(1),
  LJITOFIX(1)/2,
  LJITOFIX(1)/3
};

const char *const optionsDASNames[] = {
  "Instant",
  "1G (60 Hz)", "1/2G (30 Hz)", "1/3G (20 Hz)",
  "1/4G (15 Hz)", "1/5G (12 Hz)", "1/6G (10 Hz)",
  "1/7G (8.6 Hz)", "1/8G (7.5 Hz)", "1/9G (6.7 Hz)"
};

const char *const optionsDASDelayNames[] = {
  "16 ms", "33 ms", "50 ms", "66 ms", "83 ms", "100 ms",
  "116 ms", "133 ms", "150 ms", "166 ms", "183 ms", "200 ms",
  "216 ms", "233 ms", "250 ms", "266 ms", "283 ms", "300 ms"
};

const char *const optionsZangiNames[] = {
  "Slide", "Lock", "Lock on release"
};

void addKeysToInput(LJInput *dst, LJBits keys, const LJField *p, LJControl *c) {
  int actualKeys = keys;

  if (c->replaySrc) {
    keys = getReplayFrame(c->replaySrc, dst);
    if (keys == LJREPLAY_EOF) {
      keys = actualKeys;
      replayClose(c->replaySrc);
      c->replaySrc = NULL;
    }
  }

  int lastFrameKeys = c->lastKeys;

  // If diagonal presses are disabled, ignore any changes
  if (!c->allowDiagonals
       && (keys & (VKEY_UP | VKEY_DOWN))
       && (keys & (VKEY_LEFT | VKEY_RIGHT))) {
    keys &= ~(VKEY_UP | VKEY_DOWN | VKEY_LEFT | VKEY_RIGHT);
    keys |= lastFrameKeys
            & (VKEY_UP | VKEY_DOWN | VKEY_LEFT | VKEY_RIGHT);
  }

  LJBits newKeys = keys & ~lastFrameKeys;
  c->lastKeys = keys;

  // Count presses for Baboo!, excluding console buttons
  c->presses += countOnes(newKeys & 0x0000FFFF);

  // Only once the side effect of counting presses for Baboo!
  // is complete can we break out of a replay.
  if (c->replaySrc) {
    return;
  }

  LJBits releasedKeys = ~keys & lastFrameKeys;
  
  // At this point, c->lastKeys holds the keys actually held
  // by the player this frame, and lastFrameKeys holds the keys 
  // actually held by the player last frame.
  
  // Handle keys that must be re-pressed
  releasedKeys &= ~c->repressKeys;
  c->repressKeys &= keys;
  keys &= ~c->repressKeys;

  // If locking in a mode without ARE, require
  // down to be re-pressed before next piece
  if (p->sounds & LJSND_LOCK
      && p->areTime < c->dasDelay) {
    c->repressKeys |= VKEY_DOWN;
    
    // Treat up the same way when hard drop lock is set to lock on release.
    if (c->hardDropLock != LJZANGI_SLIDE) {
      c->repressKeys |= VKEY_UP;
    }
  }
  
  // Initial Rotation System (IRS):
  // When a piece spawns from next or hold, and a rotation or macro
  // key is held, treat the key as if they had just been pressed.
  // Treat hard drop the same way when ARE is turned on.
  if ((p->sounds & (LJSND_SPAWN | LJSND_HOLD))
      && c->initialRotate) {
    newKeys |= c->lastKeys
               & (VKEY_ROTL | VKEY_ROTR | VKEY_MACROS);
    if (p->areTime > 0) {
      newKeys |= c->lastKeys
                 & VKEY_UP;
    }
  }

  // if we're pretending that keys are not pressed,
  // pretend consistently
  newKeys &= keys;

  // TGM does not perform sideways movement on
  // the first frame after a piece is spawned.
  if (c->initialDAS == 0 &&
      (p->sounds & (LJSND_SPAWN | LJSND_HOLD))) {
    
  } else if (keys & VKEY_LEFT) {
        if (c->dasCounter > -(int)c->dasDelay) {
          if (c->dasCounter >= 0) {
            c->dasCounter = -1;
            dst->movement = -1;
          } else {
            c->dasCounter -= 1;
          }
        } else {
          int dasSpeed = c->dasSpeed;
          if (dasSpeed) {
            dst->movement = -1;
            c->dasCounter += dasSpeed - 1;
          } else {
            dst->movement = -(int)LJ_PF_WID;
          }
        }
  } else if (keys & VKEY_RIGHT) {
        if (c->dasCounter < c->dasDelay) {
          if (c->dasCounter <= 0) {
            c->dasCounter = 1;
            dst->movement = 1;
          } else {
            c->dasCounter += 1;
          }
        } else {
          int dasSpeed = c->dasSpeed;
          if (dasSpeed) {
            dst->movement = 1;
            c->dasCounter -= dasSpeed - 1;
          } else {
            dst->movement = (int)LJ_PF_WID;
          }
        }
  } else {
    c->dasCounter = 0;
  }

  if(keys & VKEY_DOWN) {
    int g = softDropSpeeds[c->softDropSpeed];
    
    // dither speed to 1/8G units
    g += ljitofix(p->gameTime % 3) / 24;
    dst->gravity += g >> 13;

    if ((newKeys & VKEY_DOWN)
        || c->softDropLock == LJZANGI_LOCK) {
      dst->other |= LJI_LOCK;
    }
  }

  if (newKeys & VKEY_ROTL) {
    dst->rotation -= 1;
  } 
  if (newKeys & VKEY_ROTR) {
    dst->rotation += 1;
  }
  if (newKeys & VKEY_HOLD) {
    dst->other |= LJI_HOLD;
  }
  if (newKeys & VKEY_UP) {
    dst->gravity = LJ_PF_HT << 3;
    if (p->state == LJS_LANDED
        || c->hardDropLock == LJZANGI_LOCK) {
      dst->other |= LJI_LOCK;
    }
  }
  
  if (c->hardDropLock == LJZANGI_LOCK_RELEASE) {
    if (releasedKeys & VKEY_UP) {
      dst->other |= LJI_LOCK;
    }
  }
  if (c->softDropLock == LJZANGI_LOCK_RELEASE) {
    if (releasedKeys & VKEY_DOWN) {
      dst->other |= LJI_LOCK;
    }
  }

  addMacrosToInput(dst, newKeys);
  
  // Baboo! ends with a hard drop
  if (p->gimmick == LJGM_BABY && c->presses >= 300) {
    dst->gravity = LJ_PF_HT << 3;
    dst->other |= LJI_LOCK;
  }

  if (c->replayDst) {
    replayRecord(c->replayDst, actualKeys, dst);
  }
}
