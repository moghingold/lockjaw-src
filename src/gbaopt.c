/*

options for gba port of lockjaw

*/

#include "lj.h"
#include "ljcontrol.h"

#ifdef ARM9
#include "ljds.h"
#define SCREEN_H 24
#else
#include "ljgba.h"
#define SCREEN_H 20
#endif

#include <stdio.h>
#include <string.h>

typedef struct OptionsLine {
  const char *name;
  const char *const *valueNames;
  unsigned char minValue;
  unsigned char nValues;  
  unsigned char startValue;
  unsigned char style;
} OptionsLine;

typedef struct OptionsPage {
  unsigned int start;
  const char *name;
} OptionsPage;

const char *const optionsBoolNames[2] = {
  "Off", "On"
};

#define N_KICK_LIMITS 7
static const char *const optionsKickLimitNames[N_KICK_LIMITS] = {
  "Off", "1", "2", "3", "4", "5", "Unlimited"
};

const char *const optionsShadowNames[] = {
  "Color", "Monochrome", "None", "No falling piece either"
};

// Overrides
const char *const optionsLockDelayNames[] = {
  "Set by speed curve", "No lock"
};

// Overrides
const char *const optionsLineDelayNames[] = {
  "Set by speed curve", "Max"
};

/*
 * OPTSTYLE_DEFAULT and no valueNames:
 *   Render value as a number.
 * OPTSTYLE_DEFAULT and valueNames:
 *   Render value as valueNames[value - minValue]
 * OPTSTYLE_FRAMES and no valueNames:
 *   Render value as a number and as a number times 50/3.
 * OPTSTYLE_FRAMES and no valueNames:
 *   Similar, except override value == minValue with valueNames[0]
 *   and value == minValue + nValues - 1 with valueNames[1].
 */
enum {
  OPTSTYLE_DEFAULT,  // names, or numbers
  OPTSTYLE_FRAMES
};


extern const char *const optionsZangiNames[];
extern const char *const gimmickNames[];

enum {
  OPTIONS_GIMMICK,

  OPTIONS_WIDTH,
  OPTIONS_HEIGHT,
  OPTIONS_SPEED_CURVE,
  OPTIONS_ENTRY_DELAY,
  OPTIONS_RANDOMIZER,

  OPTIONS_ROTATION_SYSTEM,
  OPTIONS_FLOOR_KICKS,
  OPTIONS_LOCKDOWN,
  OPTIONS_LOCK_DELAY,

  OPTIONS_LINE_DELAY,
  OPTIONS_CLEAR_GRAVITY,
  OPTIONS_4X4_SQUARES,
  OPTIONS_SCORING,
  OPTIONS_T_SPIN,
  OPTIONS_DROP_SCORING,
  OPTIONS_GARBAGE,

  OPTIONS_SIDEWAYS_DELAY,
  OPTIONS_SIDEWAYS_SPEED,
  OPTIONS_INITIAL_SIDEWAYS,
  OPTIONS_IRS,
  OPTIONS_DIAGONAL_MOTION,
  OPTIONS_SOFT_DROP_SPEED,
  OPTIONS_SOFT_DROP,
  OPTIONS_HARD_DROP,

  OPTIONS_SHADOW,
  OPTIONS_HIDE_PF,
  OPTIONS_NEXT_PIECES,
  OPTIONS_SMOOTH_GRAVITY,

  OPTIONS_MENU_LEN
};

const OptionsLine optionsMenu[OPTIONS_MENU_LEN] = {
  { "Gimmick",         gimmickNames, 0, LJGM_N_GIMMICKS, 0 },
  
  { "Well width",      NULL, 4, LJ_PF_WID - 4 + 1, 10 },
  { "Well height",     NULL, 8, LJ_PF_VIS_HT - 8 + 1, LJ_PF_VIS_HT },
  { "Speed curve",     optionsSpeedCurveNames, 0, LJSPD_N_CURVES, LJSPD_EXP },
  { "Max entry delay", NULL, 0, 121, 0, OPTSTYLE_FRAMES },
  { "Randomizer",      optionsRandNames, 0, LJRAND_N_RANDS, LJRAND_7BAG },

  { "Rotation system", optionsRotNames, 0, N_ROTATION_SYSTEMS, 0 },
  { "Floor kicks",     optionsKickLimitNames, 0, N_KICK_LIMITS, N_KICK_LIMITS - 1 },
  { "Lockdown",        optionsLockdownNames, 0, LJLOCK_N_STYLES, LJLOCK_MOVE },
  { "Lock delay",      optionsLockDelayNames, 0, 129, 0, OPTSTYLE_FRAMES },
  { "Clear gravity",   optionsGravNames, 0, LJGRAV_N_ALGOS, 0 },

  { "Line clear delay",optionsBoolNames, 0, 128, 0 },
  { "4x4 squares",     optionsBoolNames, 0, 2, 0 },
  { "Scoring",         optionsScoringNames, 0, LJSCORE_N_STYLES, 0 },
  { "T-spin detection",optionsTspinNames, 0, LJTS_N_ALGOS, LJTS_TDS },
  { "Drop scoring",    optionsDropScoringNames, 0, LJDROP_N_STYLES, 0 },
  { "Garbage",         optionsGarbageNames, 0, LJGARBAGE_N_STYLES, 0 },

  { "Max sideways delay",NULL, 1, 18, 10, OPTSTYLE_FRAMES },
  { "Sideways speed",  optionsDASNames, 0, 10, 1 },
  { "Initial sideways",optionsBoolNames, 0, 2, 1 },
  { "Initial rotation",optionsBoolNames, 0, 2, 1 },
  { "Diagonal motion", optionsBoolNames, 0, 2, 0 },
  { "Soft drop speed", optionsSoftSpeedNames, 0, 3, 0 },
  { "Soft drop",       optionsZangiNames, 0, 3, 0 },
  { "Hard drop",       optionsZangiNames, 0, 3, 1 },
  
  { "Shadow",          optionsShadowNames, LJSHADOW_COLORED, LJSHADOW_N_STYLES - 2, LJSHADOW_COLORED },
  { "Hide blocks in well", optionsBoolNames, 0, 2, 0 },
  { "Next pieces",     NULL, 0, LJ_NEXT_PIECES + 1, 3 },
  { "Smooth gravity",  optionsBoolNames, 0, 2, 1 },
};

#define N_OPTIONS_PAGES 7
const OptionsPage optionsPages[N_OPTIONS_PAGES + 1] = {
  {OPTIONS_GIMMICK, "Game"},
  {OPTIONS_WIDTH, "Rules: Well"},
  {OPTIONS_ROTATION_SYSTEM, "Rules: Movement"},
  {OPTIONS_LINE_DELAY, "Rules: Line clear"},
  {OPTIONS_SIDEWAYS_DELAY, "Control: Movement"},
  {OPTIONS_SOFT_DROP_SPEED, "Control: Drop"},
  {OPTIONS_SHADOW, "Display"},
  {OPTIONS_MENU_LEN, NULL}
};

unsigned char prefs[OPTIONS_MENU_LEN];

int isDisabledOption(unsigned char *prefs, int y) {
  return 0;
}

void unpackOptions(LJView *v, unsigned char *prefs) {
  v->field->gimmick = prefs[OPTIONS_GIMMICK];

  {
    int width = prefs[OPTIONS_WIDTH];
    v->field->leftWall = (LJ_PF_WID - width) / 2;
    v->field->rightWall = v->field->leftWall + width;
  }
  v->field->ceiling = prefs[OPTIONS_HEIGHT];
  v->field->speedCurve = prefs[OPTIONS_SPEED_CURVE];
  v->field->areStyle = prefs[OPTIONS_ENTRY_DELAY];
  v->field->randomizer = prefs[OPTIONS_RANDOMIZER];

  v->field->rotationSystem = prefs[OPTIONS_ROTATION_SYSTEM];
  v->field->maxUpwardKicks = prefs[OPTIONS_FLOOR_KICKS] == N_KICK_LIMITS - 1
                             ? 128
                             : prefs[OPTIONS_FLOOR_KICKS];
  v->field->lockReset = prefs[OPTIONS_LOCKDOWN];
  v->field->setLockDelay = prefs[OPTIONS_LOCK_DELAY];

  v->field->setLineDelay = prefs[OPTIONS_LINE_DELAY];
  v->field->tSpinAlgo = prefs[OPTIONS_T_SPIN];
  v->field->clearGravity = prefs[OPTIONS_CLEAR_GRAVITY];
  v->field->squareMode = prefs[OPTIONS_4X4_SQUARES];
  v->field->scoreStyle = prefs[OPTIONS_SCORING];
  v->field->dropScoreStyle = prefs[OPTIONS_DROP_SCORING];
  v->field->garbageStyle = prefs[OPTIONS_GARBAGE];

  v->control->dasDelay = prefs[OPTIONS_SIDEWAYS_DELAY];
  v->control->dasSpeed = prefs[OPTIONS_SIDEWAYS_SPEED];
  v->control->initialDAS   = prefs[OPTIONS_INITIAL_SIDEWAYS];
  v->control->initialRotate = prefs[OPTIONS_IRS];
  v->control->allowDiagonals = prefs[OPTIONS_DIAGONAL_MOTION];
  v->control->softDropSpeed  = prefs[OPTIONS_SOFT_DROP_SPEED];
  v->control->softDropLock = prefs[OPTIONS_SOFT_DROP];
  v->control->hardDropLock = prefs[OPTIONS_HARD_DROP];

  v->hideShadow = prefs[OPTIONS_SHADOW];
  v->hidePF = prefs[OPTIONS_HIDE_PF];
  v->nextPieces = prefs[OPTIONS_NEXT_PIECES];
  v->smoothGravity = prefs[OPTIONS_SMOOTH_GRAVITY];
}

void initOptions(void) {
  for (int i = 0; i < OPTIONS_MENU_LEN; ++i) {
    prefs[i] = optionsMenu[i].startValue;
  }
}


/**
 * @param dst 32-byte buffer
 */
void getOptionsValueStr(char *dst, int line, int value) {
  switch (optionsMenu[line].style) {
  case OPTSTYLE_DEFAULT:
    if (optionsMenu[line].valueNames) {
      strncpy(dst,
              optionsMenu[line].valueNames[value - optionsMenu[line].minValue],
              31);
      dst[31] = 0;
    } else {
      siprintf(dst, "%d", value);
    }
    break;
    
  case OPTSTYLE_FRAMES:
    if (optionsMenu[line].valueNames
        && value == optionsMenu[line].minValue) {
        
      // override first with name 0
      strncpy(dst,
              optionsMenu[line].valueNames[0],
              31);
      dst[31] = 0;
    } else if (optionsMenu[line].valueNames
                && value == optionsMenu[line].minValue
                            + optionsMenu[line].nValues - 1) {

      // override second with name 1
      strncpy(dst,
              optionsMenu[line].valueNames[1],
              31);
      dst[31] = 0;
    } else {
      if (value >= 60) {
        int ds = value / 6;
        int s = ds / 10;
        ds -= s * 10;
        siprintf(dst, "%d/60 s (%d.%d s)", value, s, ds);
      } else {
        int ms = value * 50 / 3;
        siprintf(dst, "%d/60 s (%d ms)", value, ms);
      }
    } break;
  }
  
}


void drawOptionsLine(int y, int line, int value, int hilite) {
  char txt[32];

  // Clear previous value
  for (int x = 0; x < 30; ++x) {
    MAP[31][y * 2 + 5][x] = ' ';
  }
  
  // Draw label
  textout(optionsMenu[line].name, 3, y * 2 + 4, 0);

  // Format and draw value
  getOptionsValueStr(txt, line, value);
  textout(txt, 6, y * 2 + 5, 0);

  // Draw cursor
  MAP[31][y * 2 + 4][2] = hilite ? '>' : ' ';
}

void options(LJView *v) {
  int page = 0, y = 0;
  int redraw = 2;
  int done = 0;
  LJBits lastKeys = ~0;
  int dasDir = 0;
  int dasCounter = 0;

  while (!done) {
    if (redraw) {
      if (redraw == 2) {
        cls();
        textout("LOCKJAW > Options", 0, 1, 0);
        MAP[31][2][0] = page + '1';
        MAP[31][2][1] = '/';
        MAP[31][2][2] = N_OPTIONS_PAGES + '0';
        MAP[31][2][3] = ':';
        textout(optionsPages[page].name, 5, 2, 0);
        for (int i = optionsPages[page].start;
             i < optionsPages[page + 1].start; ++i) {
          drawOptionsLine(i - optionsPages[page].start,
                          i, prefs[i], 0);
        }
        redraw = 1;
        textout("\201\200: change page; \206\207: select",
                1, SCREEN_H - 2, 0);
        textout("\205\204: adjust; Start: play",
                1, SCREEN_H - 1, 0);
      }
      if (redraw == 1) {
        drawOptionsLine(y - optionsPages[page].start,
                        y, prefs[y], 1);
        redraw = 0;
      }
    }
    
    LJBits keys = readPad();

    int lastY = y;
    LJBits newKeys = keys & ~lastKeys;

    // Handle DAS within options (fixed at 250 ms 30 Hz)
    if (keys & dasDir) {
      ++dasCounter;
      if (dasCounter >= 15) {
        dasCounter -= 2;
        newKeys |= dasDir;
      }
    } else {
      dasCounter = 0;
    }
    
    if (newKeys & VKEY_UP) {
      dasDir = VKEY_UP;
      if (y <= 0) {
        y = OPTIONS_MENU_LEN - 1;
      } else {
        --y;
      }
    }
    if (newKeys & VKEY_DOWN) {
      dasDir = VKEY_DOWN;
      ++y;
      if (y >= OPTIONS_MENU_LEN) {
        y = 0;
      }
    }

    if (!isDisabledOption(prefs, y)) {    
      if (newKeys & VKEY_RIGHT) {
        int num = prefs[y] + 1;

        if (num >= optionsMenu[y].minValue + optionsMenu[y].nValues) {
          prefs[y] = optionsMenu[y].minValue;
        } else {
          prefs[y] = num;
        }

        gba_play_sound(v->plat, 1);
        // need to redraw the whole box because options may have
        // become enabled or disabled
        redraw = 2;
        dasDir = VKEY_RIGHT;
      }

      if (newKeys & VKEY_LEFT) {
        int num = prefs[y] - 1;

        if (num < optionsMenu[y].minValue) {
          prefs[y] = optionsMenu[y].minValue + optionsMenu[y].nValues - 1;
        } else {
          prefs[y] = num;
        }

        gba_play_sound(v->plat, 1);
        dasDir = VKEY_LEFT;
        redraw = 2;
      }
    }

    // Rotate left: Go back a page    
    if (newKeys & VKEY_ROTL) {
      if (page > 0) {
        y = optionsPages[page - 1].start;
      } else {
        y = 0;
      }
    }

    // Rotate right: Go forward a page or,
    // if on last page, finish
    if (newKeys & VKEY_ROTR) {
      if (page + 1 >= N_OPTIONS_PAGES) {
        done = 1;
      } else {
        y = optionsPages[page + 1].start;
      }
    }
    
    // Start: finish
    if (newKeys & (KEY_START << 16)) {
      done = 1;
    }

    vsync();
    gba_poll_sound(v->plat);
    
    if (lastY != y) {
      gba_play_sound(v->plat, 0);

      // calculate which page the cursor has moved to
      int lastPage = page;
      while (y < optionsPages[page].start) {
        --page;
      }
      while (y >= optionsPages[page + 1].start) {
        ++page;
      }
      
      if (lastPage == page) {
        drawOptionsLine(lastY - optionsPages[page].start,
                       lastY, prefs[lastY], 0);
        if (redraw < 1) {
          redraw = 1;
        }
      } else {
        gba_play_sound(v->plat, 5);  // hold == turn page sound
        redraw = 2;  // redraw the whole screen
      }
    }
    lastKeys = keys;
  }
  gba_play_sound(v->plat, 4);
  unpackOptions(v, prefs);
}
