/* Gimmick code for LOCKJAW, an implementation of the Soviet Mind Game

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

const char *const gimmickNames[LJGM_N_GIMMICKS] = {
  "Marathon",
  "40 lines",
  "180 seconds",
  "Vs. w/Items",
  "Baboo!"
};

static const LJFixed initialGravity[LJSPD_N_CURVES] = {
  [LJSPD_ZERO] = 0,
  [LJSPD_RHYTHMZERO] = 0,
  [LJSPD_EXP] = LJITOFIX(1) / 60,
  [LJSPD_RHYTHM] = LJITOFIX(20)
};

const char *const optionsSpeedCurveNames[] = {
  "Zero", "Rhythm Zero", "Exponential", "Rhythm", "Master", "Death",
  "NES", "Game Boy"
};

/**
 * The default speed curve for each gimmick.  A negative value
 * means that the gimmick uses the player's chosen speed curve.
 */
static const signed char defaultSpeedCurve[LJGM_N_GIMMICKS] = {
  [LJGM_ATYPE] = -1,
  [LJGM_BTYPE] = -1,
  [LJGM_ULTRA] = -1,
  [LJGM_ITEMS] = -1,
  [LJGM_BABY] = LJSPD_ZERO
};

void initSpeed(LJField *p) {
  p->gravity = initialGravity[p->speedCurve];
  
  switch (p->speedCurve) {
  case LJSPD_RHYTHM:
  case LJSPD_RHYTHMZERO:
    p->speedLevel = 60;
    p->bpmCounter = 0;
    p->speedupCounter = 0;
    break;
  case LJSPD_TGM:
  case LJSPD_DEATH:
    p->speedLevel = -1;
    break;
  default:
    p->speedLevel = 0;
    break;
  }
}

struct TGMSectionMap {
  int startGravity;
  int endGravity;
  signed char areTime, dasDelay, lockDelay, lineDelay;
};

#define TGM_N_SECTIONS 12
static const struct TGMSectionMap tgmSections[TGM_N_SECTIONS] = {
  { LJITOFIX(1)/60, LJITOFIX(1)/10, 30, 15, 30, 40 }, // Master start
  { LJITOFIX(1)/10, LJITOFIX(1)/ 2, 25, 15, 30, 40 },
  { LJITOFIX(1)/60, LJITOFIX(1),    20, 15, 30, 40 },
  { LJITOFIX(1),    LJITOFIX(3),    20, 15, 30, 30 },
  { LJITOFIX(3),    LJITOFIX(5),    20, 14, 30, 20 },
  { LJITOFIX(20),   LJITOFIX(20),   20, 13, 30, 15 },
  { LJITOFIX(20),   LJITOFIX(20),   15, 12, 30,  8 }, // Death start
  { LJITOFIX(20),   LJITOFIX(20),   14, 12, 26,  0 },
  { LJITOFIX(20),   LJITOFIX(20),   14, 11, 22,  0 },
  { LJITOFIX(20),   LJITOFIX(20),    8, 10, 18,  6 }, // major ARE speedup
  { LJITOFIX(20),   LJITOFIX(20),    7,  8, 15,  5 },
  { LJITOFIX(20),   LJITOFIX(20),    6,  8, 15,  4 }
};

// Reference for major ARE speedup at master 900:
// http://www.tetrisconcept.com/forum/viewtopic.php?p=4927#4927
// If master 900 speeds up like death 300, then master 600 must
// be similar to death 0. Thus, we start the (well-known) death table at 600.
#define TGM_DEATH_FIRST_SECTION 6


#define NES_N_SECTIONS 30
static const char nesSections[NES_N_SECTIONS] = {
 48, 43, 38, 33, 28,
 23, 18, 13,  8,  6,
  5,  5,  5,  4,  4,
  4,  3,  3,  3,  2,
  2,  2,  2,  2,  2, 
  2,  2,  2,  2,  1
};

#define GB_N_SECTIONS 21
static const char gbSections[GB_N_SECTIONS] = {
 53, 49, 45, 41, 37,
 33, 28, 22, 17, 11,
 10,  9,  8,  7,  6,
  6,  5,  5,  4,  4,
  3
};

void setSpeed(LJField *p, LJControl *c) {
  switch (p->speedCurve) {
  case LJSPD_ZERO:
  case LJSPD_EXP:
    if (p->gravity < ljitofix(20)) {
      p->gravity += p->gravity * 3 / 256;
      p->lockDelay = 40;
    } else {
      p->gravity += ljitofix(20) * 3 / 256;
      p->lockDelay = ljitofix(800) / p->gravity;
    }
    if (p->lockReset == LJLOCK_NOW) {
      p->lockDelay = 0;
    }
    if (p->gimmick == LJGM_BABY) {
      p->speedLevel = c->presses;
    }
    break;

  case LJSPD_RHYTHM:
  case LJSPD_RHYTHMZERO:
    // If we've already banked five pieces' worth of time,
    // add 20 points instead of banking another.
    if (p->bpmCounter <= -18000) {
      // removed in 0.21 because other curves don't reward for drops
      // p->score += 20;
    } else {
      p->bpmCounter -= 3600;  // number of frames per minute
    }
    p->lockDelay = 3600 / p->speedLevel;
    p->gravity = (p->speedCurve == LJSPD_RHYTHM) ? ljitofix(20) : 0;
    break;

  case LJSPD_TGM:
  case LJSPD_DEATH:
    {
      int section = p->speedLevel / 100;
      int sectionPos = p->speedLevel % 100;
      if (p->speedCurve == LJSPD_DEATH) {
        section += TGM_DEATH_FIRST_SECTION;
      }
      if (section >= TGM_N_SECTIONS) {
        section = TGM_N_SECTIONS - 1;
      }
      
      p->gravity = tgmSections[section].startGravity
                   + (tgmSections[section].endGravity - tgmSections[section].startGravity)
                     * sectionPos / 100;
      
      p->areTime = tgmSections[section].areTime;
      if (p->areTime > p->areStyle) {
        p->areTime = p->areStyle;
      }
      if (c->dasDelay > tgmSections[section].dasDelay) {
        c->dasDelay = tgmSections[section].dasDelay;
      };
      p->lockDelay = tgmSections[section].lockDelay;
      p->lineDelay = tgmSections[section].lineDelay;
      
      if (sectionPos < 99) {
        ++p->speedLevel;
      }
    } break;
    
  case LJSPD_NES:
    {
      int section = p->lines / 10;

      if (section >= NES_N_SECTIONS) {
        section = NES_N_SECTIONS - 1;
      }
      p->gravity = ljitofix(1) / nesSections[section];
      p->lockDelay = 10;
      if (p->lockReset == LJLOCK_NOW) {
        p->lockDelay = 0;
      }
    } break;

  case LJSPD_GB:
    {
      int section = p->lines / 10;

      if (section >= GB_N_SECTIONS) {
        section = GB_N_SECTIONS - 1;
      }
      p->gravity = ljitofix(1) / gbSections[section];
      p->lockDelay = 10;
      if (p->lockReset == LJLOCK_NOW) {
        p->lockDelay = 0;
      }
    } break;

  }
  
  if (p->setLockDelay >= 128) {
    p->lockDelay = 127;
  } else if (p->setLockDelay > 0) {
    p->lockDelay = p->setLockDelay;
  }
  if (p->setLineDelay > 0) {
    p->lineDelay = p->setLineDelay;
  }
}

const char *const optionsGarbageNames[LJGARBAGE_N_STYLES] = {
  "None", "Level 1", "Level 2", "Level 3", "Level 4",
  "Home Run Derby", "Drill"
};

void initGimmicks(LJField *p) {
  if (defaultSpeedCurve[p->gimmick] >= 0) {
    p->speedCurve = defaultSpeedCurve[p->gimmick];
  }
  initSpeed(p);
  p->areTime = 0;  /* subsequent won't */
  p->lineDelay = 0;
  if (p->garbageStyle == LJGARBAGE_DRILL) {
    p->garbageRandomness = 255;
    p->garbage = p->ceiling - 2;
  }
}

LJBits gimmicks(LJField *p, LJControl *c) {
  LJBits changed = 0;

  // In rhythm, lock the tetromino if the player
  // isn't keeping up with the rate
  if (p->speedCurve == LJSPD_RHYTHM
      || p->speedCurve == LJSPD_RHYTHMZERO) {
    p->bpmCounter += p->speedLevel;

    if (p->bpmCounter >= 0) {
      if (p->state == LJS_LANDED) {
        p->stateTime = 0;
      } else if (p->state == LJS_FALLING) {
        p->gravity = ljitofix(LJ_PF_HT);
      }
    }

    // In rhythm, increase BPM periodically
    p->speedupCounter += p->speedLevel;
    if(p->speedupCounter++ >= 60 * 60 * 64) {
      p->speedupCounter -= 60 * 60 * 64;
      p->speedLevel += 10;
      p->sounds |= LJSND_SECTIONUP;
    }
  }

  // For each piece, set the entry and line delays.
  // Don't set it twice when spawning to replace the
  // first held piece (both LJSND_SPAWN and LJSND_HOLD
  // on the same piece).
  if ((p->sounds & (LJSND_SPAWN | LJSND_HOLD))
      == LJSND_SPAWN) {
    p->areTime = p->areStyle;

    // Default line delay is equal to the entry delay,
    // but speed curves and setLineDelay can override this
    p->lineDelay = p->areTime;
    setSpeed(p, c);
  }

  if (p->sounds & LJSND_LOCK) {
    if (p->gimmick == LJGM_ITEMS) {
      if (p->nPieces >= 7) {
        p->canRotate = 0;
        p->gravity = ljitofix(1);
      }
    }
    
    // Garbage in simulated multiplayer
    int simGarbage = p->nPieces >= 7
                     && p->garbageStyle >= LJGARBAGE_1
                     && p->garbageStyle <= LJGARBAGE_4
                     && (p->curPiece[1] == LJP_I
                         || ((p->randomizer == LJRAND_2
                              || p->randomizer == LJRAND_6BAG)
                             && p->nPieces % 7 == 3));
    if (simGarbage) {
      p->garbage += p->garbageStyle;
    }

    // Banana attack in "Vs. with Items" gimmick    
    if (p->gimmick == LJGM_ITEMS
        && (ljRand(p) & 0xF00) == 0xF00) {
      shuffleColumns(p);
      changed |= (1 << LJ_PF_HT) - 1;
    }
  }
  return changed;
}


static void bagInitRandomize(LJField *p) {
  p->permuPhase = 0;
}

static const signed char rand6Bag[] = {
  1, 2, 3, 4, 5, 6, -1
};

static const signed char rand7Bag[] = {
  0, 1, 2, 3, 4, 5, 6, -1
};

static const signed char rand10Bag[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1
};

static const signed char rand14Bag[] = {
  0, 1, 2, 3, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, -1
};

static unsigned int bagRandomize(LJField *p) {
  int piece;

  // If we're out of permutation pieces, make new ones by copying
  // from one of the rand#Bag arrays until we hit the -1 sentinel.
  if (p->permuPhase == 0) {
    const signed char *src = rand7Bag;
    int pos;
  
    if (p->randomizer == LJRAND_14BAG) {
      src = rand14Bag;
    } else if (p->randomizer == LJRAND_6BAG) {
      src = rand6Bag;
    } else if (p->randomizer == LJRAND_10BAG) {
      src = rand10Bag;
    }
    for (pos = 0;
         pos < sizeof(p->permuPiece) / sizeof(p->permuPiece[0])
         && src[pos] >= 0;
         ++pos) {
      p->permuPiece[pos] = src[pos];
    }
    p->permuPhase = pos;
  }

  // Choose a position in the remainder of the deck
  int r = (p->permuPhase > 1) ? ljRand(p) % p->permuPhase : 0;
  
  // Swap the top card with the card at this position
  piece = p->permuPiece[r];
  p->permuPiece[r] = p->permuPiece[--p->permuPhase];
  return piece;
}

static void mtbInitRandomize(LJField *p) {
  // At game start, front three are I, J, L
  // Back four (never dealt as first tetromino) are O, S, T, Z
  for (int i = 0; i < 7; ++i) {
    p->permuPiece[i] = i;
  }
}

static unsigned int mtbRandomize(LJField *p) {

  // Choose a piece from the three in front
  int r = ljRand(p) % 3;
  int piece = p->permuPiece[r];
  
  // Move it to the back
  for (; r < 6; ++r) {
    p->permuPiece[r] = p->permuPiece[r + 1];
  }
  p->permuPiece[6] = piece;
  return piece;
}

static void tgmInitRandomize(LJField *p) {
  p->permuPiece[2] = 0;
  p->permuPiece[3] = LJP_Z;
  p->permuPiece[4] = LJP_S;
  p->permuPiece[5] = LJP_Z;
  p->permuPiece[6] = LJP_S;
}

/* State of TGM randomizer:
 * permuPiece[3..6]: history, [3] most recent
 * permuPiece[2]: 0 for first piece (use equal probability I, J, L, T)
 *   or 1 for subsequent pieces
 */
static unsigned int tgmRandomize(LJField *p) {
  int r = ljRand(p) ^ (ljRand(p) << 15);
  int piece;
  
  if (p->permuPiece[2]) {

    // Roll up to 6 times for pieces
    for (int rolls = 6;
          rolls > 0;
          --rolls, r /= 7) {
      piece = r % 7;

      // If the piece is not in the history, use it now
      if (piece != p->permuPiece[3]
          && piece != p->permuPiece[4]
          && piece != p->permuPiece[5]
          && piece != p->permuPiece[6])
        break;
    }
  } else {
    p->permuPiece[2] = 1;
    // Generate only T, L, J, and I
    piece = (r >> 12) & 0x03;
    if (piece == LJP_O) {
      piece = LJP_T;
    }
  }
  
  // Move it to the back
  for (r = 3; r < 6; ++r) {
    p->permuPiece[r] = p->permuPiece[r + 1];
  }
  p->permuPiece[6] = piece;
  return piece;
}


const char *const optionsRandNames[LJRAND_N_RANDS] = {
  "7-piece Bag",
  "14-piece Bag",
  "Move to Back",
  "History 6 Rolls",
  "Memoryless",
  "SZSZ",
  "6-piece Bag",
  "10-piece Bag"
};

void initRandomize(LJField *p) {
  switch (p->randomizer) {
  case LJRAND_6BAG:
  case LJRAND_7BAG:
  case LJRAND_10BAG:
  case LJRAND_14BAG:
    bagInitRandomize(p);
    break;
  case LJRAND_HIST_INF:
    mtbInitRandomize(p);
    break;
  case LJRAND_HIST_6:
    tgmInitRandomize(p);
    break;
  }
}

unsigned int randomize(LJField *p) {
  switch (p->randomizer) {
  case LJRAND_6BAG:
  case LJRAND_7BAG:
  case LJRAND_10BAG:
  case LJRAND_14BAG:
    return bagRandomize(p);
  case LJRAND_HIST_INF:
    return mtbRandomize(p);
  case LJRAND_HIST_6:
    return tgmRandomize(p);
  case LJRAND_2:
    return (ljRand(p) & 0x1000) ? LJP_Z : LJP_S;
  default:
    return ljRand(p) % 7;
  }
}


const char *const optionsScoringNames[LJSCORE_N_STYLES] = {
  "LJ", "TNT64", "Hotline", "TDS", "NES", "LJ (nerfed spin)"
};


static const char garbageScoreTable[] = { 0, 0, 1, 2, 4 };
static const char tSpinGarbageScoreTable[] = { 0, 2, 4, 6, 6 };
static const char squareScoreTable[] = {0, 1, 1, 1, 2, 3, 5, 8};

const char hotlineRows[LJ_PF_HT] =
{ 
  0, 0, 0, 0, 1,
  0, 0, 0, 0, 2,
  0, 0, 0, 3, 0,
  0, 4, 0, 5, 6,
  0, 0, 0, 0
};

static const char tdsScoreTable[] = {0, 1, 3, 5, 8};
static const char tdsTSScoreTable[] = {4, 8, 12, 16};
static const char nesScoreTable[] = {0, 4, 10, 30, 120};


/**
 * Computes the score and outgoing garbage for lines
 * and adds them to the player's total.
 * @param p The playfield
 * @param lines Bit array where 1 means a line clear on this row.
 */
void addLinesScore(LJField *p, LJBits lines) {
  const int nLines = countOnes(lines);
  int oldLines = p->nLinesThisPiece;
  int tdsSection = p->lines / 10 + 1;

  p->lines += nLines;
  if (p->speedCurve == LJSPD_TGM || p->speedCurve == LJSPD_DEATH) {
    int oldTgmSection = p->speedLevel / 100;
    p->speedLevel += nLines;
    int newTgmSection = p->speedLevel / 100;
    if (newTgmSection > oldTgmSection) {
      p->sounds |= LJSND_SECTIONUP;
    }
  }
  else if (p->speedCurve == LJSPD_NES || p->speedCurve == LJSPD_GB) {
    int newTdsSection = p->lines / 10 + 1;
    if (newTdsSection > tdsSection) {
      p->sounds |= LJSND_SECTIONUP;
    }
  }

  switch (p->scoreStyle) {
  
  case LJSCORE_TNT64:
    if (nLines < 1) {
      return;
    }
    for (int i = nLines; i > 0; --i) {
      oldLines += (oldLines < 7);
      p->score += 100 * squareScoreTable[oldLines];
    } break;
    
  case LJSCORE_NES:
    {
      int garbageLevel = (nLines > 4) ? 4 : nLines;
      int value = nesScoreTable[garbageLevel];

      p->score += 10 * tdsSection * value;
      if (nLines >= 4) {
        p->sounds |= LJSND_SETB2B;
      }
    } break;

  case LJSCORE_TDS:
    {
      // Garbage based scoring system.
      int garbageLevel = (nLines > 4) ? 4 : nLines;
      int chain, value;
      
      if (p->isSpin) {
        chain = (nLines >= 1);
        value = tdsTSScoreTable[garbageLevel];
        
        // TDS gives fewer points for a T-spin single
        // that involves a wall kick.
        if (p->isSpin == 2 && nLines < 2) {
          value >>= 2;
        }
      } else {
        chain = (nLines >= 4);
        value = tdsScoreTable[garbageLevel];
      }
      if (chain && p->chain && nLines >= 1) {  // b2b
        value = value * 3 / 2;
      }

      if (tdsSection > 20) {
        tdsSection = 20;
      }
      p->score += 100 * tdsSection * value;
      if (nLines >= 1) {
        if (chain) {
          p->sounds |= LJSND_SETB2B;
          if (p->chain) {
            p->sounds |= LJSND_B2B;
          }
        }
        p->chain = chain;
      }
    } break;

  case LJSCORE_LJ:
  case LJSCORE_LJ_NERF:
    if (nLines >= 1) {
      // Garbage based scoring system.
      int garbageLevel = (nLines > 4) ? 4 : nLines;
      unsigned int chain, garbage;

      if (p->isSpin) {
        chain = (nLines >= 1);
        garbage = tSpinGarbageScoreTable[garbageLevel];
        if (p->scoreStyle == LJSCORE_LJ_NERF) {
          garbage /= 2;
        }
        garbage += (chain && p->chain);
      } else {
        chain = (nLines >= 4);
        garbage = garbageScoreTable[garbageLevel]
                      + (chain && p->chain);
      }
      p->score += 100 * nLines + 200 * garbage;
      p->outGarbage += garbage;
      if (chain) {
        p->sounds |= LJSND_SETB2B;
        if (p->chain) {
          p->sounds |= LJSND_B2B;
        }
      }
      p->chain = chain;
    } break;
  
  case LJSCORE_HOTLINE:
    for (int y = 0; y < 24; ++y) {
      if (lines & (1 << y)) {
        p->score += 100 * hotlineRows[y];
      }
    } break;

  }

  p->nLinesThisPiece += nLines;
}

