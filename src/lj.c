/* Engine of LOCKJAW, an implementation of the Soviet Mind Game

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

#define LJ_INTERNAL
#include "lj.h"

unsigned int ljRand(LJField *p) {
  p->seed = p->seed * 2147001325 + 715136305;
  return p->seed >> 17;
}

static inline void ljAssert(LJField *p, int shouldBeTrue, const char *reason) {
  if (!shouldBeTrue) {
    p->state = LJS_GAMEOVER;
  }
}

static const char xShapes[10][4][4] = {
  { {0,1,2,3}, {2,2,2,2}, {3,2,1,0}, {1,1,1,1} }, // I
  { {0,1,2,0}, {1,1,1,2}, {2,1,0,2}, {1,1,1,0} }, // J
  { {0,1,2,2}, {1,1,1,2}, {2,1,0,0}, {1,1,1,0} }, // L
  { {1,1,2,2}, {1,2,2,1}, {2,2,1,1}, {2,1,1,2} }, // O
  { {0,1,1,2}, {1,1,2,2}, {2,1,1,0}, {1,1,0,0} }, // S
  { {0,1,2,1}, {1,1,1,2}, {2,1,0,1}, {1,1,1,0} }, // T
  { {0,1,1,2}, {2,2,1,1}, {2,1,1,0}, {0,0,1,1} }, // Z
  { {0,1,2,3}, {2,2,2,2}, {3,2,1,0}, {1,1,1,1} }, // I
  { {0,1,2,0}, {1,1,1,2}, {2,1,0,2}, {1,1,1,0} }, // J
  { {1,1,2,2}, {1,2,2,1}, {2,2,1,1}, {2,1,1,2} }, // O
};

static const char yShapes[10][4][4] = {
  { {2,2,2,2}, {3,2,1,0}, {1,1,1,1}, {0,1,2,3} }, // I
  { {2,2,2,3}, {3,2,1,3}, {2,2,2,1}, {1,2,3,1} }, // J
  { {2,2,2,3}, {3,2,1,1}, {2,2,2,1}, {1,2,3,3} }, // L
  { {2,3,3,2}, {3,3,2,2}, {3,2,2,3}, {2,2,3,3} }, // O
  { {2,2,3,3}, {3,2,2,1}, {2,2,1,1}, {1,2,2,3} }, // S
  { {2,2,2,3}, {3,2,1,2}, {2,2,2,1}, {1,2,3,2} }, // T
  { {3,3,2,2}, {3,2,2,1}, {1,1,2,2}, {1,2,2,3} }, // Z
  { {2,2,2,2}, {3,2,1,0}, {1,1,1,1}, {0,1,2,3} }, // I
  { {2,2,2,3}, {3,2,1,3}, {2,2,2,1}, {1,2,3,1} }, // J
  { {2,3,3,2}, {3,3,2,2}, {3,2,2,3}, {2,2,3,3} }, // O
};

const char pieceColors[10] = {
  0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
  0x40, 0x10, 0x20
};

static const signed char connShapes[10][4] = {
  {CONNECT_R,   CONNECT_LR,  CONNECT_LR,  CONNECT_L},  // I
  {CONNECT_UR,  CONNECT_LR,  CONNECT_L,   CONNECT_D},  // J
  {CONNECT_R,   CONNECT_LR,  CONNECT_UL,  CONNECT_D},  // L
  {CONNECT_UR,  CONNECT_DR,  CONNECT_DL,  CONNECT_UL}, // O
  {CONNECT_R,   CONNECT_UL,  CONNECT_DR,  CONNECT_L},  // S
  {CONNECT_R,   CONNECT_ULR, CONNECT_L,   CONNECT_D},  // T
  {CONNECT_R,   CONNECT_DL,  CONNECT_UR,  CONNECT_L},  // Z
  {-1,          CONNECT_R,   CONNECT_L,   -1},         // I2
  {CONNECT_R,   CONNECT_LR,  CONNECT_L,   -1},         // I3
  {CONNECT_UR,  CONNECT_D,   -1,          CONNECT_L},  // L3
};

static inline int pieceToFieldBlock(int piece, int conn) {
  return conn | pieceColors[piece];
}

void expandPieceToBlocks(LJBlkSpec out[],
                         const LJField *p, int piece, int xBase, int yBase, int theta) {
  int shape = piece & LJP_MASK;
  
  if (theta >= 4) {
    int kickData = wkSpawnMove[p->rotationSystem][shape];
    xBase += WKX(kickData);
    yBase += WKY(kickData);
    kickData = wkNextMove[p->rotationSystem];
    xBase += WKX(kickData);
    yBase += WKY(kickData);
    theta = wkSpawnRotate[p->rotationSystem][shape];
  }
  
  const char *xBl = xShapes[shape][theta];
  const char *yBl = yShapes[shape][theta];

  for (int blk = 0; blk < 4; ++blk) {
    if (connShapes[shape][blk] == -1) {
      out[blk].conn = 0;
    } else {
      int conn = connShapes[shape][blk] << theta;
      int connRotated = (conn | (conn >> 4)) & CONNECT_MASK;
      int color = ((0x10 << blk) & piece) ? 8 : piece;
      out[blk].y = yBl[blk] + yBase;
      out[blk].x = xBl[blk] + xBase;
      out[blk].conn = pieceToFieldBlock(color, connRotated);
    }
  }
}

int isOccupied(const LJField *p, int x, int y) {
  if (x < p->leftWall || x >= p->rightWall || y < 0)
    return 1;
  if (y > LJ_PF_HT)
    return 0;
  return p->b[y][x] > 1;
}

int isCollision(const LJField *p, int x, int y, int theta) {
  LJBlkSpec blocks[4];
  int piece = p->curPiece[0];

  expandPieceToBlocks(blocks, p, piece, x, y, theta);

  for (int blk = 0; blk < 4; ++blk) {
    if (blocks[blk].conn
        && isOccupied(p, blocks[blk].x, blocks[blk].y))
      return 1;
  }
  return 0;
}

const char *const optionsTspinNames[] = {
  "Off", "Immobile", "3-corner T", "3-corner T no kick"
};

/**
 * Determines whether the piece that just landed was a T-spin.
 * Must be called just BEFORE lockdown writes the blocks to the
 * playfield; otherwise TNT will break.
 */
static int isTspin(const LJField *p) {
  int blks = 0;
  int x = p->x;
  int y = p->hardDropY;

  switch (p->tSpinAlgo) {
  case LJTS_TNT:
    if (!isCollision(p, x, y + 1, p->theta)
        || !isCollision(p, x - 1, y, p->theta)
        || !isCollision(p, x + 1, y, p->theta)) {
      return 0;
    }    
    return p->isSpin;
  case LJTS_TDS_NO_KICK:
  
    // If t-spin involved wall kick, don't count it
    if (p->isSpin == 2) {
      return 0;
    }
    
    // otherwise fall through
  case LJTS_TDS:
    // 1. T tetromino
    if ((p->curPiece[0] & LJP_MASK) != LJP_T) {
      return 0;
    }

    // 2. Last move was spin
    if (!p->isSpin) {
      return 0;
    }

    // 3. At least three cells around the rotation center are full
    if (isOccupied(p, x, y + 1)) {
      ++blks;
    }
    if (isOccupied(p, x, y + 3)) {
      ++blks;
    }
    if (isOccupied(p, x + 2, y + 1)) {
      ++blks;
    }
    if (isOccupied(p, x + 2, y + 3)) {
      ++blks;
    }
    if (blks < 3) {
      return 0;
    }
    
    // 3. Last move was spin
    return p->isSpin;
  default:
    return 0;
  }
}

/**
 * Updates the current tetromino's hard drop Y location.
 * Call after the tetromino has been spawned, moved, or rotated.
 * @param p the playfield
 */
static void updHardDropY(LJField *p) {
  int x = p->x;
  int y = ljfixfloor(p->y);
  int theta = p->theta;

  while (!isCollision(p, x, y - 1, theta)) {
    --y;
  }
  p->hardDropY = y;
}


/**
 * Look for a TNT square in this position.
 * @param x column of left side, such that 0 <= x <= playfield width - 4
 * @param y row of bottom block, such that 0 <= y <= playfield height - 4
 * @param isMulti nonzero for multisquares; 0 for monosquares
 * @return nonzero if found; 0 if not found
 */
static int isSquareAt(LJField *p, int x, int y, int isMulti)
{
  int firstColor = p->b[y][x] & COLOR_MASK;

  // Check the frame to make sure it isn't connected to anything else
  for(int i = 0; i <= 3; i++)
  {
    /* don't allow squares within parts of squares */
    if((p->b[y + i][x] & COLOR_MASK) >= 0x80)
      return 0;
    /* the block doesn't connect on the left */
    if(p->b[y + i][x] & CONNECT_L)
      return 0;
    /* the block doesn't connect on the right */
    if(p->b[y + i][x + 3] & CONNECT_R)
      return 0;
    /* the block doesn't connect on the bottom */
    if(p->b[y][x + i] & CONNECT_D)
      return 0;
    /* the block doesn't connect on the top */
    if(p->b[y + 3][x + i] & CONNECT_U)
      return 0;
  }

  for(int ySub = 0; ySub < 4; ++ySub)
  {
    for(int xSub = 0; xSub <= 3; ++xSub)
    {
      int blkHere = p->b[y + ySub][x + xSub];

      /* the square contains no nonexistent blocks */
      if(!blkHere)
        return 0;
      /* the square contains no blocks of garbage or broken pieces */
      if((blkHere & COLOR_MASK) == 0x80)
        return 0;
      /* if looking for monosquares, disallow multisquares */
      if(isMulti == 0 && (blkHere & COLOR_MASK) != firstColor)
        return 0;
    }
  }
  return 1;
}

/**
 * Replaces the 4x4 blocks with a 4x4 square.
 * @param x column of left side, such that 0 <= x <= playfield width - 4
 * @param y row of bottom block, such that 0 <= y <= playfield height - 4
 * @param isMulti nonzero for multisquares; 0 for monosquares
 * @return the rows that were changed
 */
static LJBits markSquare(LJField *p, int x, int y, int isMulti)
{
  int baseBlk = (isMulti ? 0xA0 : 0xB0)
                | CONNECT_MASK;
  for(int i = 0; i < 4; ++i)
  {
    int c;

    if(i == 0)
      c = baseBlk & ~CONNECT_D;
    else if(i == 3)
      c = baseBlk & ~CONNECT_U;
    else
      c = baseBlk;

    p->b[y + i][x + 0] = c & ~CONNECT_L;
    p->b[y + i][x + 1] = c;
    p->b[y + i][x + 2] = c;
    p->b[y + i][x + 3] = c & ~CONNECT_R;
  }

  return 0x0F << y;
}


/**
 * Marks all 4x4 squares in the playfield.
 * In the case that a single tetromino forms multiple overlapping
 * squares, prefers gold over silver, high over low, left over right.
 * @param isMulti nonzero for multisquares; 0 for monosquares
 * @return the rows that were changed
 */
static LJBits findSquares(LJField *p, int isMulti) {
  LJBits changed = 0;

  for (int y = LJ_PF_HT - 4; y >= 0; --y) {
    for (int x = p->leftWall; x <= p->rightWall - 4; ++x) {
      int baseBlk = p->b[y][x];

      // If this block is filled in and not connected on the left or right
      // then do stuff to it
      if (baseBlk
          && !(baseBlk & (CONNECT_D | CONNECT_L))
          && isSquareAt(p, x, y, isMulti)) {
        changed |= markSquare(p, x, y, isMulti);
      }
    }
  }
  return changed;
}

const char *const optionsLockdownNames[] = {
  "Classic", "Entry reset", "Step reset", "Move reset"
};

/**
 * Locks the current tetromino in the playfield.
 * @param p the playfield
 * @return  the rows in which the tetromino was placed
 */
static LJBits lockPiece(LJField *p) {
  LJBits rows = 0;
  int xBase = p->x;
  int yBase = ljfixfloor(p->y);
  LJBlkSpec blocks[4];
  int piece = p->curPiece[0];
  expandPieceToBlocks(blocks, p, piece, xBase, yBase, p->theta);
  
  p->isSpin = isTspin(p);

  for (int blk = 0; blk < 4; ++blk) {
    int blkY = blocks[blk].y;
    int blkX = blocks[blk].x;
    int blkValue = blocks[blk].conn;

    if(blkValue && blkY >= 0 && blkY < LJ_PF_HT) {
      rows |= 1 << blkY;
      if (blkX >= p->leftWall && blkX < p->rightWall) {
        p->b[blkY][blkX] = blkValue;
      }
    }
  }
  p->sounds |= LJSND_LOCK;
  p->alreadyHeld = 0;
  p->nLinesThisPiece = 0;

  return rows;
}

void shuffleColumns(LJField *p) {
  unsigned int permu[LJ_PF_WID];

  for (int x = p->leftWall; x < p->rightWall; ++x) {
    permu[x] = x;
  }
  for (int x = p->rightWall - 1; x > p->rightWall + 1; --x) {
    int r = ljRand(p) % x;
    int t = permu[x];
    permu[x] = permu[r];
    permu[r] = t;
  }

  for (int y = 0; y < LJ_PF_HT; ++y) {
    unsigned int blk[10];
    
    // Copy blocks to temporary buffer, eliminating left and right connections
    for (int x = p->leftWall; x < p->rightWall; ++x) {
      blk[x] = p->b[y][permu[x]] & ~(CONNECT_L | CONNECT_R);
    }
    for (int x = p->leftWall; x < p->rightWall; ++x) {
      p->b[y][x] = blk[x];
    }
  }
}


static void rotateNewPiece(LJField *p) {
  int shape = p->curPiece[0] & LJP_MASK;
  int kickData = wkSpawnMove[p->rotationSystem][shape];
  p->x += WKX(kickData);
  p->y += ljitofix(WKY(kickData));
  p->theta = wkSpawnRotate[p->rotationSystem][shape];
}

/**
 * Spawns a tetromino onto the playfield.
 * @param p    the playfield
 * @param hold 0 for spawning from next; nonzero for swapping with hold
 * @return     the rows that were changed by banana effect
 */
static LJBits newPiece(LJField *p, int hold) {
  LJBits changed = 0;
  int ihs = p->state != LJS_FALLING
            && p->state != LJS_LANDED
            && hold;

  if (hold) {
    if (p->state == LJS_LANDED && p->lockReset == LJLOCK_SPAWN) {
      p->lockDelay = p->stateTime;
    }
  } else {
    p->upwardKicks = 0;
  }
  p->x = 4;
  p->dropDist = 0;
  if (!ihs) {
    p->state = LJS_FALLING;
    p->stateTime = 0;
  }
  p->isSpin = 0;
  p->y = ljitofix(p->ceiling - 2);
  
  /* Note: The gimmick sets the gravity */

  if (hold) {
    int temp = p->holdPiece;
    
    p->holdPiece = p->curPiece[ihs];
    p->curPiece[ihs] = temp;
    p->alreadyHeld = 1;
    p->sounds |= LJSND_HOLD;

    // If a negative number was swapped into the current piece,
    // then there was nothing in the hold space (e.g. at the
    // beginning of a round).  In this case, we'll need to fall
    // through and generate a new piece.
    if (temp >= 0) {
      rotateNewPiece(p);
      return changed;
    }
  }

  // Shift the next pieces down
  for (int i = 0; i < LJ_NEXT_PIECES; i++) {
    p->curPiece[i] = p->curPiece[i + 1];
  }
  p->sounds |= LJSND_SPAWN;

  p->curPiece[LJ_NEXT_PIECES] = randomize(p);
  ++p->nPieces;
  if (!p->canRotate) {
    p->theta = (ljRand(p) >> 12) & 0x03;
  } else {
    rotateNewPiece(p);
  }

  return changed;
}

void newGame(LJField *p) {

  // Clear playfield
  for (int y = 0; y < LJ_PF_HT; y++) {
    for(int x = 0; x < LJ_PF_WID; x++) {
      p->b[y][x] = 0;
    }
  }
  
  for (int y = 0; y < LJ_MAX_LINES_PER_PIECE; ++y) {
    p->nLineClears[y] = 0;
  }
  p->holdPiece = -1;  // sentinel for no hold piece
  // Generate pieces
  initRandomize(p);
  for(int i = 0; i < LJ_NEXT_PIECES; i++) {
    newPiece(p, 0);
  }
  p->clearedLines = 0;
  p->nPieces = 0;
  p->state = LJS_NEW_PIECE;
  p->stateTime = 1;
  p->garbage = 0;
  p->outGarbage = 0;
  p->score = 0;
  p->gameTime = 0;
  p->lines = 0;
  p->alreadyHeld = 0;
  p->chain = 0;
  p->garbageX = ljRand(p) % 10;
  p->theta = 0;
  p->nLinesThisPiece = 0;
  p->canRotate = 1;
  p->areTime = 0;
  p->garbageRandomness = 64;
  p->reloaded = 0;
  updHardDropY(p);
}

const char *const optionsDropScoringNames[LJDROP_N_STYLES] = {
  "None", "Continuous", "Drop", "Soft x1 Hard x2"
};


/**
 * Handles scoring for hard and soft drops.
 * @return 0 for no change or LJ_DIRTY_SCORE for change
 */
LJBits scoreDropRows(LJField *p, LJFixed gravity, LJFixed newY) {
  LJBits changed = 0;
  if (gravity > 0) {
    int fallDist = ljfixfloor(p->y) - ljfixfloor(newY);
      
    p->dropDist += fallDist;

    // Double scoring for hard drop
    if (p->dropScoreStyle == LJDROP_1S_2H
        && gravity >= ljitofix(p->ceiling)) {
      fallDist *= 2;
    }
    if (p->dropScoreStyle == LJDROP_1CELL
        || p->dropScoreStyle == LJDROP_1S_2H) {
      p->score += fallDist;
      changed |= LJ_DIRTY_SCORE;
    }

    // Handle scoring for continuous drop    
    if (p->dropScoreStyle == LJDROP_NES
        && newY <= ljitofix(p->hardDropY)) {
      p->score += p->dropDist;
      changed |= LJ_DIRTY_SCORE;
      p->dropDist = 0;
    }
  } else {
    p->dropDist = 0;
  }
  return changed;
}

/**
 * Handles gravity.
 * @param p         the playfield
 * @param gravity   amount of additional gravity applied by the player
 * @param otherKeys other LJI_* keys being pressed by the player
 *                  (specifically LJI_LOCK)
 */
static LJBits doPieceGravity(LJField *p, LJFixed gravity, LJBits otherKeys) {
  int changedRows = 0;

  LJFixed newY = p->y - gravity - p->gravity;

  // Check for landed
  if (newY <= ljitofix(p->hardDropY)) {
    newY = ljitofix(p->hardDropY);
    
    // Downward movement does not result in a T-spin
    if (ljfixfloor(newY) < ljfixfloor(p->y)) {
      p->isSpin = 0;
    }

    changedRows |= scoreDropRows(p, gravity, newY);
    p->y = newY;

    if (p->state == LJS_FALLING) {
      p->state = LJS_LANDED;
      p->stateTime = p->lockDelay;
      p->sounds |= LJSND_LAND;
    }
    if (p->stateTime > 0 && !(otherKeys & LJI_LOCK)) {
      // lock delay == 255 is a special case for don't lock at all
      if (p->setLockDelay < 128) {
        --p->stateTime;
      }
    } else {
      LJBits lockRows = lockPiece(p);
      p->state = LJS_LINES;
      changedRows |= lockRows | LJ_DIRTY_NEXT;
      if (!(lockRows & ((1 << p->ceiling) - 1))) {
        p->state = LJS_GAMEOVER;
      }
    }
  } else {
    changedRows |= scoreDropRows(p, gravity, newY);
    p->state = LJS_FALLING;

    // Downward movement does not result in a T-spin
    if (ljfixfloor(newY) < ljfixfloor(p->y)) {
      p->isSpin = 0;
    }
  }
  p->y = newY;
  return changedRows;
}

static int doRotate(LJField *p,
                    int newDir,
                    const WallKickTable *const pieceTable) {
  int baseX = p->x;
  int baseY = ljfixfloor(p->y);
  
  // allow specifying null tables for O
  if (!pieceTable) {
    if (!isCollision(p, baseX, baseY, newDir)) {
      p->theta = newDir;
      p->sounds |= LJSND_ROTATE;
      return 1;
    }
    return 0;
  }
  
  const unsigned char *const table = (*pieceTable)[newDir];
  int baseKickY = -1000;  // sentinel for uninitialized

  for (int kick = 0; kick < 5; kick++) {
    unsigned int kickData = table[kick];
    if (kickData == SKIP_IF) {
      unsigned int skipData = table[++kick];
      int skipX = WKX(skipData) + baseX;
      int skipY = WKY(skipData) + baseY;

      // Don't allow further rotation if the specified block is filled
      if (isOccupied(p, skipX, skipY)) {
        return 0;
      }
    } else {
      int kickX = WKX(kickData) + baseX;
      int kickY = WKY(kickData) + baseY;

      if (baseKickY == -1000) {
        baseKickY = kickY;
      }
      if ((kickY <= baseKickY || p->upwardKicks < p->maxUpwardKicks)
          && !isCollision(p, kickX, kickY, newDir)) {
        p->theta = newDir;
        p->x = kickX;
        if (kickY > baseKickY) {
          p->y = ljitofix(kickY);
          ++p->upwardKicks;
        } else if (kickY < baseKickY) {
          p->y = ljitofix(kickY) + 0xFFFF;
        } else {
          p->y = ljitofix(kickY) + (p->y & 0xFFFF);
        }
        p->sounds |= LJSND_ROTATE;
        return 1;
      }
    }
  }
  return 0;
}


/**
 * Tries to rotate the current piece 90 degrees counterclockwise,
 * using the counterclockwise wall kick tables.
 * @param p  the playfield
 */
static int doRotateLeft(LJField *p) {
  int newDir = (p->theta + 3) & 0x03;
  const WallKickTable *const pieceTable = wkTablesL[p->rotationSystem][p->curPiece[0] & LJP_MASK];
  return doRotate(p, newDir, pieceTable);
}

/**
 * Tries to rotate the current piece 90 degrees clockwise,
 * using the clockwise wall kick tables.
 * @param p  the playfield
 */
static int doRotateRight(LJField *p) {
  int newDir = (p->theta + 1) & 0x03;
  const WallKickTable *const pieceTable = wkTablesR[p->rotationSystem][p->curPiece[0] & LJP_MASK];
  return doRotate(p, newDir, pieceTable);
}

static LJBits checkLines(const LJField *p, LJBits checkRows) {
  LJBits foundLines = 0;
  for (int y = 0;
       y < LJ_PF_HT && checkRows != 0;
       ++y, checkRows >>= 1) {
    if (checkRows & 1) {
      const unsigned char *row = p->b[y];
      int found = 1;

      for (int x = p->leftWall; x < p->rightWall && found; ++x) {
        found = found && (row[x] != 0);
      }
      if (found) {
        foundLines |= 1 << y;
      }
    }
  }

  return foundLines;
}


const char *const optionsGravNames[] = {
  "Naive",
  "Sticky",
  "Sticky by color",
  "Cascade"
};

// FIXME for sticky or quadra gravity

static void fillCLoop(LJField *p, int x, int y, unsigned int src, unsigned int dst)
{
  int fillL, fillR, i;

  fillL = fillR = x;
  do {
    p->c[y][fillL] = dst;
    fillL--;
  } while ((fillL >= p->leftWall) && (p->c[y][fillL] == src));
  fillL++;

  do {
    p->c[y][fillR] = dst;
    fillR++;
  } while ((fillR < p->rightWall) && (p->c[y][fillR] == src));
  fillR--;

  for(i = fillL; i <= fillR; i++)
  {
    if(y > 0 && p->c[y - 1][i] == src) {
	  fillCLoop(p, i, y - 1, src, dst);
    }
    if(y < LJ_PF_HT - 1 && p->c[y + 1][i] == src) {
	  fillCLoop(p, i, y + 1, src, dst);
    }
  }
}


static void fillC(LJField *p, int x, int y, int dstC) {
  if (p->c[y][x] != dstC) {
    fillCLoop(p, x, y, p->c[y][x], dstC);
  }
}

/**
 * Locks the block regions that have landed.
 * @param p the playfield
 */
void lockLandedRegions(LJField *p) {
  // Look for regions that are on top of ground regions, where
  // "ground regions" are any block that is solid and whose region ID is 0.
  for (int landed = 1; landed != 0; ) {
    landed = 0;
    // If something hit the ground, erase its floating bit
    for (int y = 0; y < LJ_PF_HT; ++y) {
      for (int x = p->leftWall; x < p->rightWall; ++x) {
        // If there's a floating block here, and a not-floating block below,
        // erase this block group's floatiness
        if (p->c[y][x] &&
            (y == 0 || (!p->c[y - 1][x] && p->b[y - 1][x]))) {
          fillC(p, x, y, 0);
          p->sounds |= LJSND_LAND;
          landed = 1;
        }
      }
    }
  }
}

/**
 * Separates the playfield into regions that shall fall separately.
 * @param p the playfield
 * @param byColors Zero: Touching blocks form a region.
 *               Nonzero: Touching blocks of a single color form a region.
 */
static void stickyMark(LJField *p, int byColors) {
  for (unsigned int y = 0; y < LJ_PF_HT; ++y) {
    for (unsigned int x = p->leftWall; x < p->rightWall; ++x) {
      int blkHere = p->b[y][x] & COLOR_MASK;

      if (!byColors) {
        blkHere = blkHere ? 0x10 : 0;
      }
      p->c[y][x] = blkHere;
    }
  }
  
  if (byColors) {
    lockLandedRegions(p);
  } else {
    // mark the bottom row as landed
    for (unsigned int x = p->leftWall; x < p->rightWall; ++x) {
      if (p->c[0][x]) {
        fillC(p, x, 0, 0);
      }
    }
  }

  p->stateTime = 5;
}


/**
 * Sets the color of a piece to gray/garbage (0x80).
 * @param x column of a block in the piece
 * @param y row of a block in the piece
 * @param rgn the region ID
 */
static void cascadeMarkPiece(LJField *p, int x, int y, int rgn) {
  int blkHere = p->b[y][x];
  
  if (blkHere && !p->c[y][x]) {
    p->c[y][x] = rgn;
    if((blkHere & CONNECT_D) && y > 0)
      cascadeMarkPiece(p, x, y - 1, rgn);
    if((blkHere & CONNECT_U) && y < LJ_PF_HT - 1)
      cascadeMarkPiece(p, x, y + 1, rgn);
    if((blkHere & CONNECT_L) && x > p->leftWall)
      cascadeMarkPiece(p, x - 1, y, rgn);
    if((blkHere & CONNECT_R) && x < p->rightWall - 1 )
      cascadeMarkPiece(p, x + 1, y, rgn);
  }
}

static void cascadeMark(LJField *p) {
  int rgn = 0;

  for (unsigned int y = 0; y < LJ_PF_HT; ++y) {
    for (unsigned int x = p->leftWall; x < p->rightWall; ++x) {
      p->c[y][x] = 0;
    }
  }
  for (unsigned int y = 0; y < LJ_PF_HT; ++y) {
    for (unsigned int x = p->leftWall; x < p->rightWall; ++x) {
      cascadeMarkPiece(p, x, y, ++rgn);
    }
  }
  lockLandedRegions(p);
  p->stateTime = 5;
}

static void breakEverything(LJField *p) {
  for (unsigned int y = 0; y < LJ_PF_HT; ++y) {
    for (unsigned int x = p->leftWall; x < p->rightWall; ++x) {
      if (p->b[y][x]) {
        p->c[y][x] = x + 1;
        p->b[y][x] = 0x80;
      } else {
        p->c[y][x] = 0;
      }
    }
  }
  
  // fill bottom row
  for (unsigned int x = x = p->leftWall; x < p->rightWall; ++x) {
    if (p->c[0][x]) {
      fillC(p, x, 0, 0);
    }
  }
  p->stateTime = 5;
}

/**
 * Sets the color of a piece to gray/garbage (0x80).
 * @param x column of a block in the piece
 * @param y row of a block in the piece
 */
static LJBits breakPiece(LJField *p, int x, int y) {
  LJBits changed = 0;
  int blkHere = p->b[y][x];
  int colorHere = blkHere & COLOR_MASK;
  int connHere = blkHere & CONNECT_MASK;
  
  if (colorHere != 0x80) {
    p->b[y][x] = connHere | 0x80;
    changed |= 1 << y;
    if((blkHere & CONNECT_D) && y > 0)
      changed |= breakPiece(p, x, y - 1);
    if((blkHere & CONNECT_U) && y < LJ_PF_HT - 1)
      changed |= breakPiece(p, x, y + 1);
    if((blkHere & CONNECT_L) && x > p->leftWall)
      changed |= breakPiece(p, x - 1, y);
    if((blkHere & CONNECT_R) && x < p->rightWall - 1 )
      changed |= breakPiece(p, x + 1, y);
  }
  return changed;
}

/**
 * Removes blocks in cleared lines from the playfield and marks
 * remaining blocks for gravity.
 * @param the lines to be cleared
 * @return the rows that were changed
 */
static LJBits clearLines(LJField *p, LJBits foundLines) {
  LJBits changed = foundLines;

  p->clearedLines = foundLines;
  if (foundLines != 0) {
    p->sounds |= LJSND_LINE;
  }
  for (int y = 0;
       y < LJ_PF_HT && foundLines != 0;
       ++y, foundLines >>= 1) {
    if (foundLines & 1) {

      // In square mode, turn broken pieces (but not 4x4 squares)
      // into garbage blocks
      if (p->squareMode) {
        for (int x = p->leftWall; x < p->rightWall; ++x) {
          if (p->b[y][x] < 0x80) {
            changed |= breakPiece(p, x, y);
          } else if ((p->b[y][x] & (0xF0 | CONNECT_R)) == 0xA0) {
            p->score += 500;
            changed |= LJ_DIRTY_SCORE;
          } else if ((p->b[y][x] & (0xF0 | CONNECT_R)) == 0xB0) {
            p->score += 1000;
          }
        }
      }

      for (int x = p->leftWall; x < p->rightWall; ++x) {
        p->b[y][x] = 0;
      }

      // break connections up and down (like Tengen Tetyais)
      if (y > 0) {
        for (int x = p->leftWall; x < p->rightWall; ++x) {
          p->b[y - 1][x] &= ~CONNECT_U;
        }
        changed |= 1 << (y - 1);
      }
      if (y < LJ_PF_HT - 1) {
        for (int x = p->leftWall; x < p->rightWall; ++x) {
          p->b[y + 1][x] &= ~CONNECT_D;
        }
        changed |= 1 << (y + 1);
      }
    }
  }
  if (p->squareMode && p->isSpin) {
    breakEverything(p);
    changed |= (1 << LJ_PF_HT) - 1;
  } else if (p->clearGravity == LJGRAV_STICKY) {
    stickyMark(p, 0);
  } else if (p->clearGravity == LJGRAV_STICKY_BY_COLOR) {
    stickyMark(p, 1);
  } else if (p->clearGravity == LJGRAV_CASCADE) {
    cascadeMark(p);
  } else {
    p->stateTime = 0;
  }
  
  return changed;
}

static unsigned int stickyFallLines(LJField *p) {
  int minY = LJ_PF_HT;

  // Move floating stuff down by one block
  for (int y = 1; y < LJ_PF_HT; ++y) {
    for (int x = p->leftWall; x < p->rightWall; ++x) {
      int c = p->c[y][x];
      if (c) {
        p->c[y - 1][x] = c;
        p->c[y][x] = 0;
        p->b[y - 1][x] = p->b[y][x];
        p->b[y][x] = 0;

        if (minY > y) {
          minY = y;
        }
      }
    }
  }

  // If we're done, skip all the rest
  if (minY >= LJ_PF_HT) {
    return LJ_PF_HT;
  }

  lockLandedRegions(p);
  return minY - 1;
}

static unsigned int fallLines(LJField *p) {
  LJBits rowBits = p->tempRows;
  unsigned int lineRow = 0;
  
  if (p->clearGravity != LJGRAV_NAIVE
      || (p->squareMode && p->isSpin)) {
    return stickyFallLines(p);
  }

  if (rowBits == 0) {
    return LJ_PF_HT;
  }

  if ((rowBits & 0xFFFF) == 0) {
    rowBits >>= 16;
    lineRow += 16;
  }
  if ((rowBits & 0xFF) == 0) {
    rowBits >>= 8;
    lineRow += 8;
  }
  if ((rowBits & 0xF) == 0) {
    rowBits >>= 4;
    lineRow += 4;
  }
  if ((rowBits & 0x3) == 0) {
    rowBits >>= 2;
    lineRow += 2;
  }
  if ((rowBits & 0x1) == 0) {
    rowBits >>= 1;
    lineRow += 1;
  }
  p->tempRows = (p->tempRows & (-2 << lineRow)) >> 1;
  if (!(p->tempRows & (1 << lineRow))) {
    p->sounds |= LJSND_LAND;
  }

  // Move stuff down by 1 row
  for (int y = lineRow; y < LJ_PF_HT - 1; ++y) {
    unsigned char *row0 = p->b[y];
    const unsigned char *row1 = p->b[y + 1];
    for (int x = p->leftWall; x < p->rightWall; ++x) {
      row0[x] = row1[x];
    }
  }

  // Clear top row
  for (int x = p->leftWall; x < p->rightWall; ++x) {
    p->b[LJ_PF_HT - 1][x] = 0;
  }

  return lineRow;
}

unsigned int countOnes(LJBits b) {
  unsigned int ones = 0;

  while(b) {
    ++ones;
    b &= b - 1;
  }
  return ones;
}

static unsigned int addGarbage(LJField *p) {
  // Move stuff up by 1 row
  for (int y = LJ_PF_HT - 2; y >= 0; --y) {
    unsigned char *row1 = p->b[y + 1];
    const unsigned char *row0 = p->b[y];
    for (int x = p->leftWall; x < p->rightWall; ++x) {
      row1[x] = row0[x];
    }
  }

  // Garbage in bottom row
  for (int x = p->leftWall; x < p->rightWall; ++x) {
    p->b[0][x] = 0x80;
  }

  // Randomize location of garbage hole
  int r = (ljRand(p) >> 7) & 0xFF;
  int garbageX = (r <= p->garbageRandomness)
                 ? (ljRand(p) % (p->rightWall - p->leftWall)) + p->leftWall
                 : p->garbageX;
  p->b[0][garbageX] = 0;
  p->garbageX = garbageX;

  // Horizontally connect the blocks that make up garbage in bottom row
  for (int x = p->leftWall; x < p->rightWall - 1; ++x) {
    if (p->b[0][x] && p->b[0][x + 1]) {
      p->b[0][x] |= CONNECT_R;
      p->b[0][x + 1] |= CONNECT_L;
    }
  }

  // Vertically connect the blocks that make up garbage in bottom row
  for (int x = p->leftWall; x < p->rightWall; ++x) {
    if (p->b[0][x]
        && ((p->b[1][x] & COLOR_MASK) == 0x80)) {
      p->b[0][x] |= CONNECT_U;
      p->b[1][x] |= CONNECT_D;
    }
  }

  return (1 << LJ_PF_VIS_HT) - 1;
}

/**
 * Computes the score and outgoing garbage for lines
 * and adds them to the player's total.
 * @param p The playfield
 * @param lines Bit array where 1 means a line clear on this row.
 */
void addLinesScore(LJField *p, LJBits lines);

/**
 * Things to do just before launching a new piece.
 */
void prepareForNewPiece(LJField *p) {
  int nLines = p->nLinesThisPiece;

  // Add to number of clears of each number of lines.
  int idx;

  if (p->clearGravity == LJGRAV_NAIVE) {
    // In naive gravity, T-spin single, double, and triple counts
    // are stored in 5-row, 6-row, and 7-row slots.
    idx = (nLines > 4)
          ? 4
          : nLines;
            
    if (p->isSpin) {
      idx += 4;
    }
  } else {
    idx = (nLines > LJ_MAX_LINES_PER_PIECE)
          ? LJ_MAX_LINES_PER_PIECE
          : nLines;
  }
  
  if (nLines < 4 && !p->isSpin && p->garbageStyle == LJGARBAGE_HRDERBY) {
    p->garbage += nLines;
  }

  ljAssert(p,
           idx <= LJ_MAX_LINES_PER_PIECE,
           "Number of lines cleared with last piece out of bounds in prepareForNewPiece");
  if (idx > 0) {
    p->nLineClears[idx - 1] += 1;
  }
  
  p->state = LJS_NEW_PIECE;
  p->stateTime = p->areTime;
}

LJBits frame(LJField *p, const LJInput *in) {
  LJBits changedRows = 0;
  LJBits tempRows;
  int distance;
  int moved = 0;

  p->sounds = 0;

  // Make hold work at ANY time.
  if ((in->other & LJI_HOLD) && !p->alreadyHeld) {
    changedRows |= newPiece(p, 1) | LJ_DIRTY_NEXT;
    updHardDropY(p);
  }

  switch(p->state) {
  case LJS_NEW_PIECE:
    if (p->garbage > 0) { 
      changedRows |= addGarbage(p);
      --p->garbage;
      break;
    }
    
    // ARE
    if (p->stateTime > 0) {
      --p->stateTime;
    }
    if (p->stateTime > 0) {
      break;
    }

    changedRows |= newPiece(p, 0);
    updHardDropY(p);
    changedRows |= LJ_DIRTY_NEXT;
    if (isCollision(p, p->x, ljfixfloor(p->y), p->theta)) {
      p->state = LJS_GAMEOVER;
    }

    break;
      
  // the following executes for both falling and landed
  case LJS_FALLING:
  case LJS_LANDED:
    if (p->canRotate) {
      int oldX = p->x;
      int oldY = ljfixfloor(p->y);
      distance = in->rotation;
      for(; distance < 0; ++distance) {
        if (doRotateLeft(p)) {
          moved = 1;

          // isSpin == 1: twist in place
          // isSpin == 2: twist with kick
          // if (p->tSpinAlgo == LJTS_TDS_NO_KICK)
          // then only isSpin == 1 is worth points.
          if (p->x == oldX && ljfixfloor(p->y) == oldY) {
            p->isSpin = 1;
          } else {
            p->isSpin = 2;
          }
        } else {
          break;
        }
      }
      for(; distance > 0; --distance) {
        if (doRotateRight(p)) {
          moved = 1;
          if (p->x == oldX && ljfixfloor(p->y) == oldY) {
            p->isSpin = 1;
          } else {
            p->isSpin = 2;
          }
        } else {
          break;
        }
      }
    }

    distance = in->movement;
    for(; distance < 0; ++distance) {
      if (!isCollision(p, p->x - 1, ljfixfloor(p->y), p->theta)) {
        --p->x;
        p->sounds |= LJSND_SHIFT;
        moved = 1;
        p->isSpin = 0;
      }
    }
    for(; distance > 0; --distance) {
      if (!isCollision(p, p->x + 1, ljfixfloor(p->y), p->theta)) {
        ++p->x;
        p->sounds |= LJSND_SHIFT;
        moved = 1;
        p->isSpin = 0;
      }
    }
    if (moved) {
      if (p->state == LJS_LANDED) {

        // if tetromino can move down, go back to falling state;
        // otherwise, reset lock delay.
        if (!isCollision(p, p->x, ljfixfloor(p->y) - 1, p->theta)) {
          p->state = LJS_FALLING;
          if (p->lockReset == LJLOCK_SPAWN) {
            p->lockDelay = p->stateTime;
          }
          p->stateTime = 0;
        } else {
          p->state = LJS_LANDED;
          if (p->lockReset == LJLOCK_MOVE) {
            p->stateTime = p->lockDelay;
          }
        }
      }
      updHardDropY(p);
    }

    tempRows = doPieceGravity(p, ljitofix(in->gravity) >> 3, in->other);
    p->tempRows = tempRows;
    changedRows |= tempRows;

    // At this point, if the piece locked,
    // p->tempRows holds the rows in which the piece landed.
    break;

  case LJS_LINES:
    if (p->squareMode) {
      changedRows |= findSquares(p, 0);
      changedRows |= findSquares(p, 1);
    }

    // At this point, p->tempRows holds the rows in which
    // a line could possibly have been made.
    tempRows = p->tempRows;
    tempRows = checkLines(p, tempRows);
    p->tempRows = tempRows;
    // At this point, p->tempRows holds the rows in which
    // a line HAS been made.
    addLinesScore(p, tempRows);
    changedRows |= LJ_DIRTY_SCORE;

    // At this point, p->tempRows holds the rows in which a line
    // HAS been made.
    p->clearedLines = tempRows;
    if (!tempRows) {
      prepareForNewPiece(p);
      break;
    }

    changedRows |= clearLines(p, tempRows);

    p->state = LJS_LINES_FALLING;
    p->stateTime += p->lineDelay;
    break;

  case LJS_LINES_FALLING:
    if (p->stateTime > 0) {
      --p->stateTime;
    }
    if (p->stateTime > 0) {
      break;
    }
    moved = fallLines(p);
    if (moved >= LJ_PF_HT) {
      p->state = LJS_LINES;
      p->tempRows = (1 << LJ_PF_HT) - 1;
      p->stateTime = p->lineDelay;
    }
    changedRows |= (~0 << moved) & ((1 << LJ_PF_VIS_HT) - 1);
    break;

  default:
    break;

  }

  ++p->gameTime;
  return changedRows;
}


