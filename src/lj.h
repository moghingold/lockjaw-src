/* Header file for the engine of LOCKJAW, an implementation of the Soviet Mind Game

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

#ifndef LJ_H
#define LJ_H

#include <stdlib.h>
#include "ljtypes.h"

enum {
  CONNECT_U    = 0x01,
  CONNECT_R    = 0x02,
  CONNECT_D    = 0x04,
  CONNECT_L    = 0x08,
  CONNECT_UD   = CONNECT_U | CONNECT_D,
  CONNECT_UL   = CONNECT_U | CONNECT_L,
  CONNECT_DL   = CONNECT_D | CONNECT_L,
  CONNECT_UR   = CONNECT_U | CONNECT_R,
  CONNECT_DR   = CONNECT_D | CONNECT_R,
  CONNECT_LR   = CONNECT_L | CONNECT_R,
  CONNECT_UDL  = CONNECT_UD | CONNECT_L,
  CONNECT_UDR  = CONNECT_UD | CONNECT_R,
  CONNECT_ULR  = CONNECT_UL | CONNECT_R,
  CONNECT_DLR  = CONNECT_DL | CONNECT_R,
  CONNECT_UDLR = CONNECT_UD | CONNECT_LR,
  CONNECT_MASK = CONNECT_UDLR,
  COLOR_MASK   = 0xF0
};

typedef unsigned char LJBlock;

typedef enum LJState {
  LJS_INACTIVE,
  LJS_NEW_PIECE,  /* delay is ARE */
  LJS_FALLING,  /* delay is fall delay; NOT IMPLEMENTED */
  LJS_LANDED,  /* delay is lock delay */
  LJS_LINES,  /* delay is line delay */
  LJS_LINES_FALLING, /* delay is fall delay per line; NOT IMPLEMENTED */
  LJS_GAMEOVER  /* game over animation */
} LJState;

// for other
enum {
  LJI_HOLD = 0x01,
  LJI_LOCK = 0x02
};

// for dirty bits
enum {
  LJ_DIRTY_NEXT = 0x01000000,
  LJ_DIRTY_SCORE = 0x02000000
};

// for gimmick (game mode)
enum {
  LJGM_ATYPE,          // marathon
  LJGM_BTYPE,          // 40 lines
  LJGM_ULTRA,          // 180 seconds
  LJGM_ITEMS,          // no rotation + no next + fast drop + garbage + banana
  LJGM_BABY,           // 300 keypresses
  LJGM_N_GIMMICKS
};

enum {
  LJP_I = 0,
  LJP_J,
  LJP_L,
  LJP_O,
  LJP_S,
  LJP_T,
  LJP_Z,
  LJP_GARBAGE,
  LJP_BOMB,
  LJP_MASK = 0x0F
};

enum {
  LJSND_ROTATE    = 0x0001,
  LJSND_SHIFT     = 0x0002,
  LJSND_DROP      = 0x0004,
  LJSND_LAND      = 0x0008,
  LJSND_LOCK      = 0x0010,
  LJSND_LINE      = 0x0020,  // a line was scored
  LJSND_SETB2B    = 0x0040,  // this line is b2b worthy (tetris or t-spin)
  LJSND_B2B       = 0x0080,  // this line AND last line were b2b worthy
  LJSND_SPAWN     = 0x0100,  // next piece was moved up
  LJSND_HOLD      = 0x0200,  // hold piece was activated
  LJSND_IRS       = 0x0400,  // initial rotation: spawn last frame and rotation this frame
  LJSND_COUNTDOWN = 0x0800,  // countdown value changed
  LJSND_SECTIONUP = 0x1000,  // section increased
};

enum {
  LJRAND_7BAG,
  LJRAND_14BAG,
  LJRAND_HIST_INF,
  LJRAND_HIST_6,
  LJRAND_7,
  LJRAND_2,
  LJRAND_6BAG,
  LJRAND_10BAG,
  LJRAND_N_RANDS
};

enum {
  LJTS_OFF  = 0,
  LJTS_TNT,
  LJTS_TDS,
  LJTS_TDS_NO_KICK,
  LJTS_N_ALGOS
};

enum {
  LJSPD_ZERO = 0,
  LJSPD_RHYTHMZERO,
  LJSPD_EXP,
  LJSPD_RHYTHM,
  LJSPD_TGM,
  LJSPD_DEATH,
  LJSPD_NES,
  LJSPD_GB,
  LJSPD_N_CURVES
};

enum {
  LJLOCK_NOW = 0,
  LJLOCK_SPAWN,
  LJLOCK_STEP,
  LJLOCK_MOVE,
  LJLOCK_N_STYLES
};

enum {
  LJGRAV_NAIVE = 0,
  LJGRAV_STICKY,
  LJGRAV_STICKY_BY_COLOR,
  LJGRAV_CASCADE,
  LJGRAV_N_ALGOS
};

enum {
  LJSCORE_LJ,
  LJSCORE_TNT64,
  LJSCORE_HOTLINE,
  LJSCORE_TDS,
  LJSCORE_NES,
  LJSCORE_LJ_NERF,
  LJSCORE_N_STYLES,
  LJSCORE_WORLDS,
  LJSCORE_TGM1,
  LJSCORE_IPOD
};

enum {
  LJDROP_NOSCORE,
  LJDROP_NES,
  LJDROP_1CELL,
  LJDROP_1S_2H,
  LJDROP_N_STYLES
};

enum {
  LJGARBAGE_NONE,
  LJGARBAGE_1,
  LJGARBAGE_2,
  LJGARBAGE_3,
  LJGARBAGE_4,
  LJGARBAGE_HRDERBY,
  LJGARBAGE_DRILL,
  LJGARBAGE_N_STYLES
};

// Guideline says 10 wide, but we want to support tetrinet mode
#define LJ_PF_WID ((size_t)12)
#define LJ_SPAWN_X ((LJ_PF_WID - 3) / 2)
#define LJ_PF_HT ((size_t)24)
#define LJ_PF_VIS_HT ((size_t)20)
#define LJ_NEXT_PIECES 8
#define LJ_MAX_LINES_PER_PIECE 8

typedef struct LJBlkSpec
{
  signed char x, y, conn, reserved;
} LJBlkSpec;

typedef struct LJInput {
  signed char rotation, movement;
  unsigned char gravity;  /* 5.3 format */
  unsigned char other;
} LJInput;

typedef struct LJField
{
  /* Game state */
  LJBlock b[LJ_PF_HT][LJ_PF_WID];
  LJBlock c[LJ_PF_HT][LJ_PF_WID];
  LJBits clearedLines;
  LJBits sounds;
  LJBits tempRows;
  unsigned char curPiece[1 + LJ_NEXT_PIECES];
  signed char permuPiece[14], permuPhase;
  unsigned short nLineClears[LJ_MAX_LINES_PER_PIECE];  // [n-1] = # of n-line clears
  LJFixed y;
  LJState state;
  signed char stateTime;
  unsigned char theta;
  signed char x;
  signed char hardDropY;
  char alreadyHeld;
  char isSpin;
  char nLinesThisPiece;
  char canRotate;
  unsigned char upwardKicks;
  
  /* Persist from piece to piece */
  int score, lines;
  unsigned int gameTime;  // number of frames
  signed short holdPiece;
  char chain;
  signed char garbage;
  unsigned char dropDist;
  unsigned char garbageX;
  unsigned short nPieces, outGarbage;

  /* Determined by gimmick */
  signed char gimmick;
  signed int speedLevel;  // -1: unused and don't display
  signed int bpmCounter;
  unsigned int speedupCounter;
  unsigned int goalCount;
  unsigned int seed;
  LJFixed gravity;
  unsigned char speedCurve;
  unsigned char goalType;
  unsigned char areTime;
  unsigned char areStyle;
  unsigned char lockReset;  // lockdown reset rule type
  unsigned char lockDelay;  // lock delay when a piece lands
  unsigned char setLockDelay;  // Overridden lock delay (255 = never)
  unsigned char lineDelay;  // delay after a line
  unsigned char setLineDelay;  // Overridden line delay
  unsigned char garbageStyle;
  unsigned char ceiling;
  unsigned char leftWall, rightWall;
  unsigned char randomizer;  // 0: generate 7 at once; 1: pick one of front 3 and move to back; 2: memoryless
  unsigned char rotationSystem;
  unsigned char garbageRandomness;  // 64: change garbageX in 1/4 of rows; 255: change all
  unsigned char tSpinAlgo;  // 0: off, 1: TNT, 2: TDS
  unsigned char clearGravity;
  unsigned char squareMode;  // 0: off; 1: TNT
  unsigned char scoreStyle;
  unsigned char dropScoreStyle;
  unsigned char maxUpwardKicks;
  unsigned char reloaded;  // 0: played through; 1: reloaded from saved state
} LJField;

/**
 * Names of the supported rotation systems (wktables.c).
 */
#define N_ROTATION_SYSTEMS 7
extern const char *const optionsRotNames[N_ROTATION_SYSTEMS];
extern const unsigned char wkColorScheme[N_ROTATION_SYSTEMS];

extern const char pieceColors[10];

#ifdef LJ_INTERNAL
  // Used for defining wall kick tables
  #define WK(x, y) (((x) & 0x0F) | ((y) & 0x0F) << 4)
  #define WKX(wk) ((((wk) & 0x0F) ^ 8) - 8)
  #define WKY(wk) WKX((wk) >> 4)
  // SKIP_IF means that the next position is a block offset from the piece's
  // lower left corner, not a kick offset, and if it is filled, stop trying more
  // positions.
  #define SKIP_IF 0x80
  typedef unsigned char WallKickTable[4][5];
  extern const WallKickTable *const wkTablesL[N_ROTATION_SYSTEMS][10];
  extern const WallKickTable *const wkTablesR[N_ROTATION_SYSTEMS][10];
  extern const unsigned char wkSpawnMove[N_ROTATION_SYSTEMS][10];
  extern const unsigned char wkNextMove[N_ROTATION_SYSTEMS];
  extern const unsigned char wkSpawnRotate[N_ROTATION_SYSTEMS][10];
#endif

/**
 * Expands a tetromino to the blocks that make it up.
 * @param out   an array of length 4
 * @param p the field into which the tetromino will be spawned
 *        (used for default spawn orientation)
 * @param piece a piece number
 * @param xBase x coordinate of base position of the tetromino
 * @param yBase y coordinate of base position of the tetromino
 * @param theta the orientation of the tetromino
 *        (0-3: use this; 4: use default spawn rotation)
 */
void expandPieceToBlocks(LJBlkSpec out[],
                         const LJField *p,
                         int piece, int xBase, int yBase, int theta);

/**
 * Names of the supported lockdown systems.
 */
extern const char *const optionsLockdownNames[];

/**
 * Tests whether a particular block is occupied.
 * @param p the playfield
 * @param x the column
 * @param y the row (0 = bottom)
 * @return  zero iff the space is open
 */
int isOccupied(const LJField *p, int x, int y);

/**
 * Tests whether a particular tetromino would overlap one or more
 * blocks, a side wall, or the floor
 * @param p the playfield
 * @param x the column of the left side of the piece's bounding 4x4
 * @param y the row of the bottom of the piece's bounding 4x4 (0 = bottom)
 * @return  zero iff the space is open
 */
int isCollision(const LJField *p, int x, int y, int theta);

/**
 * Blanks a playfield and prepares it for a new game.
 * @param p    the playfield
 */
void newGame(LJField *p);

/**
 * Runs one frame of S.M.G.
 * @param p   the playfield
 * @param in  the player's input
 * @return the rows that were modified
 */
LJBits frame(LJField *p, const LJInput *in);

/**
 * Applies gimmicks to a new game.
 * @param p   the playfield
 * @param in  the player's input
 */
void initGimmicks(LJField *p);

/**
 * Runs gimmicks for one frame of S.M.G.
 * @param p   the playfield
 * @param c   the control settings
 * @return the rows that were modified
 */
struct LJControl;
LJBits gimmicks(LJField *p, struct LJControl *c);

/**
 * Sets up the randomizer.
 * @return the number of this piece
 */
void initRandomize(LJField *p);

/**
 * Chooses a pseudo-random piece.
 * @param the field on which the piece will be generated
 * @return the number of this piece
 *   (0=i, 1=j, 2=l, 3=o, 4=s, 5=t, 6=z)
 */
unsigned int randomize(LJField *p);

/**
 * Names of all the gimmicks.
 */
extern const char *const gimmickNames[LJGM_N_GIMMICKS];

/**
 * Names of all the randomizers.
 */
extern const char *const optionsRandNames[];

/**
 * Names of the T-spin detection algorithms.
 */
extern const char *const optionsTspinNames[LJTS_N_ALGOS];

/**
 * Names of the line falling algorithms.
 */
extern const char *const optionsGravNames[];

/**
 * Names of the speed curves.
 */
extern const char *const optionsSpeedCurveNames[];

/**
 * Names of the entry delay settings.
 */
extern const char *const optionsARENames[];

/**
 * Names of the scoring methods.
 */
const char *const optionsScoringNames[LJSCORE_N_STYLES];
const char *const optionsDropScoringNames[LJDROP_N_STYLES];

/**
 * Counts the number of 1 bits in a bitfield.
 * @param p   the bitfield
 * @return    the number of bits in p with a value of 1.
 */
unsigned int countOnes(LJBits b);

/**
 * Shuffles the columns of blocks in the playfield.
 * @param p   the playfield
 */
void shuffleColumns(LJField *p);

/**
 * Random number generator.  Use this for all random events
 * that affect the game state.
 * @param field
 * @return uniformly distributed number in 0 to 0x7FFF
 */
unsigned int ljRand(LJField *p);

/**
 * List of lines used for hotline scoring.
 * A line clear completed at row hotlineRows[i] is worth 100*(i + 1).
 */
const char hotlineRows[LJ_PF_HT];

/**
 * Names of the supported garbage generators.
 */
extern const char *const optionsGarbageNames[];

#endif
