/* PC option screen for LOCKJAW, an implementation of the Soviet Mind Game

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

#include "ljpc.h"
#include "ctype.h"
#include <stdio.h>

static const char ljIniName[] = "lj.ini";

static const char *const optionsBoolNames[] = {
  "Off", "On"
};

#define N_KICK_LIMITS 7
static const char *const optionsKickLimitNames[N_KICK_LIMITS] = {
  "Off", "1", "2", "3", "4", "5", "Unlimited"
};

extern const char *const optionsZangiNames[];

static const char *const optionsNextNames[] = {
  "Hide", "1 piece", "2 pieces", "3 pieces", "4 pieces",
  "5 pieces", "6 pieces", "7 pieces", "8 pieces"
};

static const char *const optionsWindowedNames[] = {
  "Full screen", "Window"
};

const char *const optionsShadowNames[] = {
  "Fainter color", "Faint color", "Color",
  "Monochrome", "None", "No falling piece either"
};

#define N_LOCK_DELAY_LENGTHS 19
// Both entry delay and lock delay use this length table
static const unsigned char lockDelayLengths[N_LOCK_DELAY_LENGTHS] = {
  0, 3, 6, 9,
  12, 15, 18, 21,
  24, 27, 30, 36,
  42, 48, 54, 60,
  90, 120, 128
};

const char *const optionsLockDelayNames[N_LOCK_DELAY_LENGTHS] = {
  "Set by speed curve", "3/60 s (50 ms)", "6/60 s (100 ms)", "9/60 s (150 ms)",
  "12/60 s (200 ms)", "15/60 s (250 ms)", "18/60 s (300 ms)", "21/60 s (350 ms)",
  "24/60 s (400 ms)", "27/60 s (450 ms)", "30/60 s (500 ms)", "36/60 s (600 ms)",
  "42/60 s (700 ms)", "48/60 s (800 ms)", "54/60 s (900 ms)", "60/60 s (1.0 s)",
  "90/60 s (1.5 s)", "120/60 s (2.0 s)", "Never"
};

#define LJARE_N_STYLES 18
const char *const optionsARENames[] = {
  "Off", "50 ms", "100 ms", "150 ms",
  "200 ms", "250 ms", "300 ms", "350 ms",
  "400 ms", "450 ms", "500 ms", "600 ms",
  "700 ms", "800 ms", "900 ms", "1 s",
  "1.5 s", "2 s"
};

const char *const optionsWidthNames[] = {
  "4", "5", "6", "7", "8", "9", "10", "11", "12"
};

const char *const optionsHeightNames[] = {
  "8", "9", "10", "11", "12", "13", "14",
  "15", "16", "17", "18", "19", "20"
};

typedef struct OptionsLine {
  const char *name;
  const char *const *valueNames;
  int maxValue;
} OptionsLine;

/**
 * Human-readable name for each element of union LJPrefs,
 * human-readable names for each possible value, and the number of
 * distinct such values.  Order must be exactly the same as in
 * struct LJPrefsNamed (lj.h).
 */
static const OptionsLine optionsMenu[OPTIONS_MENU_LEN] = {
  { "Well width",             optionsWidthNames, LJ_PF_WID - 4 + 1 },
  { "Well height",            optionsHeightNames, LJ_PF_VIS_HT - 8 + 1 },
  { "Speed curve",            optionsSpeedCurveNames, LJSPD_N_CURVES },
  { "Max entry delay",        optionsARENames, LJARE_N_STYLES },
  { "Randomizer",             optionsRandNames, LJRAND_N_RANDS },

  { "Rotation system",        optionsRotNames, N_ROTATION_SYSTEMS },
  { "Floor kicks",            optionsKickLimitNames, N_KICK_LIMITS },
  { "Lockdown",               optionsLockdownNames, LJLOCK_N_STYLES },
  { "Lock delay",             optionsLockDelayNames, N_LOCK_DELAY_LENGTHS },

  { "Line clear delay",       optionsLockDelayNames, N_LOCK_DELAY_LENGTHS - 1 },
  { "Clear gravity",          optionsGravNames, LJGRAV_N_ALGOS },
  { "4x4 squares",            optionsBoolNames, 2 },
  { "Scoring",                optionsScoringNames, LJSCORE_N_STYLES },
  { "Drop scoring",           optionsDropScoringNames, LJDROP_N_STYLES },
  { "T-spin detection",       optionsTspinNames, LJTS_N_ALGOS },
  { "Garbage",                optionsGarbageNames, LJGARBAGE_N_STYLES },

  { "Sideways speed",         optionsDASNames, 10 },
  { "Max sideways delay",     optionsDASDelayNames, 18 },
  { "Initial sideways motion",optionsBoolNames, 2 },
  { "Initial rotation",       optionsBoolNames, 2 },
  { "Allow diagonal motion",  optionsBoolNames, 2 },

  { "Soft drop speed",        optionsSoftSpeedNames, 3 },
  { "Soft drop",              optionsZangiNames, LJZANGI_N_STYLES },
  { "Hard drop",              optionsZangiNames, LJZANGI_N_STYLES },
  { "Next pieces",            optionsNextNames, LJ_NEXT_PIECES + 1 },
  { "Next above shadow",      optionsNextNames, LJ_NEXT_PIECES + 1 },
  { "Shadow",                 optionsShadowNames, 6 },
  
  { "Hide blocks in well",    optionsBoolNames, 2 },
  { "Smooth gravity",         optionsBoolNames, 2 },
  { "Trails",                 optionsBoolNames, 2 },
  { "Playfield position",     optionsSideNames, 3 },
  { "Next piece position",    optionsNextStyleNames, LJNEXT_N_STYLES },
  { "Pause on task switch",   optionsBoolNames, 2 },
  { "Record all games",       optionsBoolNames, 2 },
  { "Display (after restart)",optionsWindowedNames, 2 }
};


/*
   Semantics:
   If option name is set to value,
   then gray out option name2 and draw its value as reason.
*/

struct DisabledOption {
  unsigned char name;
  unsigned char value;
  unsigned char name2;
  char reason[45];
};

#define N_DISABLED_OPTIONS 8
const struct DisabledOption disabledOptions[N_DISABLED_OPTIONS] = {
  {  LJPREFS_LOCK_RESET,     LJLOCK_NOW,
     LJPREFS_SOFT_DROP_LOCK, "Lockdown is immediate" },
  {  LJPREFS_LOCK_RESET,     LJLOCK_NOW,
     LJPREFS_HARD_DROP_LOCK, "Lockdown is immediate" },
  {  LJPREFS_LOCK_RESET,     LJLOCK_NOW,
     LJPREFS_LOCK_DELAY,     "Lockdown is immediate" },
  {  LJPREFS_LOCK_DELAY,     N_LOCK_DELAY_LENGTHS - 1,
     LJPREFS_LOCK_RESET,     "Lockdown is manual" },
  {  LJPREFS_SPEED_CURVE,    LJSPD_DEATH,
     LJPREFS_SOFT_DROP_SPEED,"Death: pieces fall instantly" },
  {  LJPREFS_SPEED_CURVE,    LJSPD_RHYTHM,
     LJPREFS_SOFT_DROP_SPEED,"Rhythm: pieces fall instantly" },
  {  LJPREFS_SPEED_CURVE,    LJSPD_DEATH,
     LJPREFS_SMOOTH_GRAVITY, "Death: pieces fall instantly" },
  {  LJPREFS_SPEED_CURVE,    LJSPD_RHYTHM,
     LJPREFS_SMOOTH_GRAVITY, "Rhythm: pieces fall instantly" }
};

static const char *isDisabledOption(const union LJPrefs *prefs, int y) {
  for (int i = 0; i < N_DISABLED_OPTIONS; ++i) {
    if (y == disabledOptions[i].name2) {
      int name = disabledOptions[i].name;
      int value = disabledOptions[i].value;

      if (prefs->number[name] == value) {
        return disabledOptions[i].reason;
      }
    }
  }
  return NULL;
}

static int getOptionValueByName(int row, const char *value) {
  for (int x = 0; x < optionsMenu[row].maxValue; ++x) {
    if (!ustrcmp(value, optionsMenu[row].valueNames[x])) {
      return x;
    }
  }
  return -1;
}

static void setOptionValueByName(union LJPrefs *prefs,
                                 int row, const char *value) {
  int num = getOptionValueByName(row, value);
      
  if (num >= 0) {
    prefs->number[row] = num;
  }
}

static void setOptionValueByBoolean(union LJPrefs *prefs,
                                 int row, const char *value) {
  int num = atoi(value);

  if (num >= 0) {
    prefs->number[row] = num ? 1 : 0;
  }
}

/* parse_ini_line *******************
*/
int parse_ini_line(const char *in, char *key, char *var, char *val)
{
  int c;
  char *kstart = key;

  /* Skip whitespace before key */
  while(*in && isspace(*in))
    in++;

  /* Parse key */
  if(*in == '[')  /* if we have a new key, load it */
  {
    in++;

    /* Skip whitespace before actual key */
    while(*in && isspace(*in))
      in++;

    for(c = *in++;
        c != 0 && c != ']' && c != '\n' && c != '\r';
        c = *in++)
    {
      if(!isspace(c))
        *key++ = c;
    }
    *key = 0;
    /* Strip whitespace after key */
    do {
      *key-- = 0;
    } while(key >= kstart && isspace(*key));
  }

  /* Skip whitespace before variable */
  while(*in && isspace(*in))
    in++;
  if(*in == 0)  /* if there is no variable, don't do anything */
    return 0;

  for(c = *in++;
      c != 0 && c != '=' && c != '\n' && c != '\r';
      c = *in++)
  {
    if(!isspace(c))
    {
      *var++ = c;
    }
  }
  *var = 0;

  /* Skip whitespace before value */
  while(*in && isspace(*in))
    in++;

  /* Get value */
  kstart = val;
  for(c = *in++;
      c != 0 && c != '\n' && c != '\r';
      c = *in++)
  {
    *val++ = c;
  }
  /* Strip whitespace after value */
  do {
    *val-- = 0;
  } while(val >= kstart && isspace(*val));

  return 0;
}

int loadOptions(union LJPrefs *prefs) {
  FILE *fp = fopen(ljIniName, "rt");
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

    if(!ustrcmp ("Width", var)) {
      setOptionValueByName(prefs, LJPREFS_WIDTH, val);
    }
    else if(!ustrcmp ("Height", var)) {
      setOptionValueByName(prefs, LJPREFS_HEIGHT, val);
    }
    else if(!ustrcmp ("SpeedCurve", var)) {
      setOptionValueByName(prefs, LJPREFS_SPEED_CURVE, val);
    }
    else if(!ustrcmp ("ARE", var)) {
      setOptionValueByName(prefs, LJPREFS_ARE_STYLE, val);
    }
    else if(!ustrcmp ("Randomizer", var)) {
      setOptionValueByName(prefs, LJPREFS_RANDOMIZER, val);
    }
    else if(!ustrcmp ("Rotation", var)) {
      setOptionValueByName(prefs, LJPREFS_ROTATION_SYSTEM, val);
    }
    else if(!ustrcmp ("FloorKick", var)) {
      setOptionValueByName(prefs, LJPREFS_KICK_LIMIT, val);
    }
    else if(!ustrcmp ("Lockdown", var)) {
      setOptionValueByName(prefs, LJPREFS_LOCK_RESET, val);
    }
    else if(!ustrcmp ("LockDelay", var)) {
      setOptionValueByName(prefs, LJPREFS_LOCK_DELAY, val);
    }
    else if(!ustrcmp ("LineDelay", var)) {
      setOptionValueByName(prefs, LJPREFS_LINE_DELAY, val);
    }
    else if(!ustrcmp ("TSpin", var)) {
      setOptionValueByName(prefs, LJPREFS_T_SPIN_ALGO, val);
    }
    else if(!ustrcmp ("ClearGravity", var)) {
      setOptionValueByName(prefs, LJPREFS_CLEAR_GRAVITY, val);
    }
    else if(!ustrcmp ("Square", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_SQUARE_MODE, val);
    }
    else if(!ustrcmp ("Scoring", var)) {
      setOptionValueByName(prefs, LJPREFS_SCORE_STYLE, val);
    }
    else if(!ustrcmp ("DropScoring", var)) {
      setOptionValueByName(prefs, LJPREFS_DROP_SCORE_STYLE, val);
    }
    else if(!ustrcmp ("Garbage", var)) {
      setOptionValueByName(prefs, LJPREFS_GARBAGE, val);
    }
    else if(!ustrcmp ("DASDelay", var)) {
      int num = atoi(val);
      
      if (num >= 1 && num <= 18) {
        prefs->number[LJPREFS_DAS_DELAY] = num;
      }
    }
    else if(!ustrcmp ("DAS", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < 10) {
        prefs->number[LJPREFS_DAS_SPEED] = num;
      }
    }
    else if(!ustrcmp ("InitialDAS", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_INITIAL_DAS, val);
    }
    else if(!ustrcmp ("IRS", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_IRS, val);
    }
    else if(!ustrcmp ("8Way", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_ALLOW_DIAGONALS, val);
    }
    else if(!ustrcmp ("SoftDrop", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < LJZANGI_N_STYLES) {
        prefs->number[LJPREFS_SOFT_DROP_LOCK] = num;
      }
    }
    else if(!ustrcmp ("SoftSpeed", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < 3) {
        prefs->number[LJPREFS_SOFT_DROP_SPEED] = num;
      }
    }
    else if(!ustrcmp ("HardDrop", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < LJZANGI_N_STYLES) {
        prefs->number[LJPREFS_HARD_DROP_LOCK] = num;
      }
    }
    else if(!ustrcmp ("NextPieces", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < LJ_NEXT_PIECES + 1) {
        prefs->number[LJPREFS_NEXT_PIECES] = num;
      }
    }
    else if(!ustrcmp ("NextAbove", var)) {
      int num = atoi(val);
      
      if (num >= 0 && num < LJ_NEXT_PIECES + 1) {
        prefs->number[LJPREFS_NEXT_ABOVE] = num;
      }
    }
    else if(!ustrcmp ("Shadow", var)) {
      setOptionValueByName(prefs, LJPREFS_HIDE_SHADOW, val);
    }
    else if(!ustrcmp ("HidePF", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_HIDE_PF, val);
    }
    else if(!ustrcmp ("SmoothGravity", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_SMOOTH_GRAVITY, val);
    }
    else if(!ustrcmp ("Trails", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_TRAILS, val);
    }
    else if(!ustrcmp ("Side", var)) {
      setOptionValueByName(prefs, LJPREFS_SIDE, val);
    }
    else if(!ustrcmp ("NextPos", var)) {
      setOptionValueByName(prefs, LJPREFS_NEXT_POS, val);
    }
    else if(!ustrcmp ("AutoPause", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_AUTO_PAUSE, val);
    }
    else if(!ustrcmp ("AutoRecord", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_AUTO_RECORD, val);
    }
    else if(!ustrcmp ("Windowed", var)) {
      setOptionValueByBoolean(prefs, LJPREFS_WINDOWED, val);
    }
    else if(!ustrcmp ("Skin", var)) {
      ustrzcpy(skinName, sizeof(skinName) - 1, val);
    }
  }
  fclose(fp);

  if (prefs->number[LJPREFS_DAS_DELAY] < prefs->number[LJPREFS_DAS_SPEED]) {
    prefs->number[LJPREFS_DAS_DELAY] = prefs->number[LJPREFS_DAS_SPEED];
  }

  return 0;
}

void saveOptions(const union LJPrefs *prefs) {
  FILE *out = fopen(ljIniName, "wt");
  
  if (out) {
    fprintf(out, "Width=%s\n",
                 optionsWidthNames[prefs->number[LJPREFS_WIDTH]]);
    fprintf(out, "Height=%s\n",
                 optionsHeightNames[prefs->number[LJPREFS_HEIGHT]]);
    fprintf(out, "Rotation=%s\n",
                 optionsRotNames[prefs->number[LJPREFS_ROTATION_SYSTEM]]);
    fprintf(out, "FloorKick=%s\n", 
                 optionsKickLimitNames[prefs->number[LJPREFS_KICK_LIMIT]]);
    fprintf(out, "Lockdown=%s\n",
                 optionsLockdownNames[prefs->number[LJPREFS_LOCK_RESET]]);
    fprintf(out, "LockDelay=%s\n",
                 optionsLockDelayNames[prefs->number[LJPREFS_LOCK_DELAY]]);
    fprintf(out, "LineDelay=%s\n",
                 optionsLockDelayNames[prefs->number[LJPREFS_LINE_DELAY]]);
    fprintf(out, "Randomizer=%s\n",
                 optionsRandNames[prefs->number[LJPREFS_RANDOMIZER]]);
    fprintf(out, "ClearGravity=%s\n",
                 optionsGravNames[prefs->number[LJPREFS_CLEAR_GRAVITY]]);
    fprintf(out, "SpeedCurve=%s\n",
                 optionsSpeedCurveNames[prefs->number[LJPREFS_SPEED_CURVE]]);
    fprintf(out, "TSpin=%s\n",
                 optionsTspinNames[prefs->number[LJPREFS_T_SPIN_ALGO]]);
    fprintf(out, "Square=%d\n",
                 prefs->number[LJPREFS_SQUARE_MODE]);
    fprintf(out, "ARE=%s\n",
                 optionsARENames[prefs->number[LJPREFS_ARE_STYLE]]);
    fprintf(out, "Scoring=%s\n",
                 optionsScoringNames[prefs->number[LJPREFS_SCORE_STYLE]]);
    fprintf(out, "DropScoring=%s\n",
                 optionsDropScoringNames[prefs->number[LJPREFS_DROP_SCORE_STYLE]]);
    fprintf(out, "Garbage=%s\n",
                 optionsGarbageNames[prefs->number[LJPREFS_GARBAGE]]);
    fprintf(out, "DAS=%d\n",
                 prefs->number[LJPREFS_DAS_SPEED]);
    fprintf(out, "DASDelay=%d\n",
                 prefs->number[LJPREFS_DAS_DELAY]);
    fprintf(out, "InitialDAS=%d\n",
                 prefs->number[LJPREFS_INITIAL_DAS]);
    fprintf(out, "IRS=%d\n",
                 prefs->number[LJPREFS_IRS]);
    fprintf(out, "8Way=%d\n",
                 prefs->number[LJPREFS_ALLOW_DIAGONALS]);
    fprintf(out, "SoftSpeed=%d\n",
                 prefs->number[LJPREFS_SOFT_DROP_SPEED]);
    fprintf(out, "SoftDrop=%d\n",
                 prefs->number[LJPREFS_SOFT_DROP_LOCK]);
    fprintf(out, "HardDrop=%d\n",
                 prefs->number[LJPREFS_HARD_DROP_LOCK]);
    fprintf(out, "NextPieces=%d\n",
                 prefs->number[LJPREFS_NEXT_PIECES]);
    fprintf(out, "NextAbove=%d\n",
                 prefs->number[LJPREFS_NEXT_ABOVE]);
    fprintf(out, "Shadow=%s\n",
                 optionsShadowNames[prefs->number[LJPREFS_HIDE_SHADOW]]);
    fprintf(out, "HidePF=%d\n",
                 prefs->number[LJPREFS_HIDE_PF]);
    fprintf(out, "SmoothGravity=%d\n",
                 prefs->number[LJPREFS_SMOOTH_GRAVITY]);
    fprintf(out, "Trails=%d\n",
                 prefs->number[LJPREFS_TRAILS]);
    fprintf(out, "NextPos=%s\n",
                 optionsNextStyleNames[prefs->number[LJPREFS_NEXT_POS]]);
    fprintf(out, "Side=%s\n",
                 optionsSideNames[prefs->number[LJPREFS_SIDE]]);
    fprintf(out, "AutoPause=%d\n",
                 prefs->number[LJPREFS_AUTO_PAUSE]);
    fprintf(out, "AutoRecord=%d\n",
                 prefs->number[LJPREFS_AUTO_RECORD]);
    fprintf(out, "Windowed=%d\n",
                 prefs->number[LJPREFS_WINDOWED]);
    fprintf(out, "Skin=%s\n",
                 skinName);
    fclose(out);
  }
}

void unpackOptions(LJView *v, const union LJPrefs *prefs) {
  {
    int width = prefs->number[LJPREFS_WIDTH] + 4;
    v->field->leftWall = (LJ_PF_WID - width) / 2;
    v->field->rightWall = v->field->leftWall + width;
  }
  v->field->ceiling   = prefs->number[LJPREFS_HEIGHT] + 8;
  v->field->rotationSystem   = prefs->number[LJPREFS_ROTATION_SYSTEM];
  v->field->maxUpwardKicks   = prefs->number[LJPREFS_KICK_LIMIT];
  if (v->field->maxUpwardKicks >= N_KICK_LIMITS - 1) {
    v->field->maxUpwardKicks = 255;
  }
  v->field->lockReset        = prefs->number[LJPREFS_LOCK_RESET];
  v->field->setLockDelay     = lockDelayLengths[prefs->number[LJPREFS_LOCK_DELAY]];
  v->field->setLineDelay     = lockDelayLengths[prefs->number[LJPREFS_LINE_DELAY]];
  v->field->randomizer       = prefs->number[LJPREFS_RANDOMIZER];
  v->field->tSpinAlgo        = prefs->number[LJPREFS_T_SPIN_ALGO];
  v->field->speedCurve       = prefs->number[LJPREFS_SPEED_CURVE];
  v->field->clearGravity     = prefs->number[LJPREFS_CLEAR_GRAVITY];
  v->field->squareMode       = prefs->number[LJPREFS_SQUARE_MODE];
  v->field->areStyle         = lockDelayLengths[prefs->number[LJPREFS_ARE_STYLE]];
  v->field->scoreStyle       = prefs->number[LJPREFS_SCORE_STYLE];
  v->field->dropScoreStyle   = prefs->number[LJPREFS_DROP_SCORE_STYLE];
  v->field->garbageStyle     = prefs->number[LJPREFS_GARBAGE];
  v->control->dasSpeed       = prefs->number[LJPREFS_DAS_SPEED];
  v->control->dasDelay       = prefs->number[LJPREFS_DAS_DELAY] + 1;
  if (v->control->dasDelay < v->control->dasSpeed) {
    v->control->dasDelay = v->control->dasSpeed;
  }
  v->control->initialDAS     = prefs->number[LJPREFS_INITIAL_DAS];
  v->control->initialRotate  = prefs->number[LJPREFS_IRS];
  v->control->allowDiagonals = prefs->number[LJPREFS_ALLOW_DIAGONALS];
  v->control->softDropSpeed  = prefs->number[LJPREFS_SOFT_DROP_SPEED];
  v->control->softDropLock   = prefs->number[LJPREFS_SOFT_DROP_LOCK];
  v->control->hardDropLock   = prefs->number[LJPREFS_HARD_DROP_LOCK];
  v->nextPieces              = prefs->number[LJPREFS_NEXT_PIECES];
  v->plat->nextAbove         = prefs->number[LJPREFS_NEXT_ABOVE];
  v->hideShadow              = prefs->number[LJPREFS_HIDE_SHADOW];
  v->hidePF                  = prefs->number[LJPREFS_HIDE_PF];
  v->smoothGravity           = prefs->number[LJPREFS_SMOOTH_GRAVITY];
  v->showTrails              = prefs->number[LJPREFS_TRAILS];
  v->plat->nextPos           = prefs->number[LJPREFS_NEXT_POS];
  autoPause                  = prefs->number[LJPREFS_AUTO_PAUSE];
}

/********************************************************************

Allegro based option drawing code

********************************************************************/

#define OPTIONS_TOP 100
#define OPTIONS_ROW_HT 24
#define OPTIONS_ROW_LEFT 80
#define OPTIONS_ROW_MID 280
#define OPTIONS_ROW_RIGHT 480
#define OPTIONS_MENU_VIS 15

static void optionsDrawScrollbar(int scroll) {
  int scrollboxTop = OPTIONS_TOP + OPTIONS_ROW_HT
                   + scroll
                     * OPTIONS_ROW_HT * (OPTIONS_MENU_VIS - 2)
                     / OPTIONS_MENU_LEN;
  int scrollboxBot = OPTIONS_TOP - 1 + OPTIONS_ROW_HT
                   + (scroll + OPTIONS_MENU_VIS)
                      * OPTIONS_ROW_HT * (OPTIONS_MENU_VIS - 2)
                      / OPTIONS_MENU_LEN;
  int lightGray = makecol(204, 204, 204);
  int darkGray = makecol(102, 102, 102);
  
  // bg
  rectfill(screen,
           OPTIONS_ROW_RIGHT,
           OPTIONS_TOP,
           OPTIONS_ROW_RIGHT + OPTIONS_ROW_HT - 1,
           OPTIONS_TOP - 1 + OPTIONS_ROW_HT * OPTIONS_MENU_VIS,
           lightGray);
  // lower arrow
  rect(screen,
       OPTIONS_ROW_RIGHT,
       OPTIONS_TOP + OPTIONS_ROW_HT * (OPTIONS_MENU_VIS - 1),
       OPTIONS_ROW_RIGHT + OPTIONS_ROW_HT - 1,
       OPTIONS_TOP - 1 + OPTIONS_ROW_HT * OPTIONS_MENU_VIS,
       0);
  // upper arrow
  rect(screen,
       OPTIONS_ROW_RIGHT,
       OPTIONS_TOP,
       OPTIONS_ROW_RIGHT + OPTIONS_ROW_HT - 1,
       OPTIONS_TOP - 1 + OPTIONS_ROW_HT,
       0);
  // thumb box
  rectfill(screen,
           OPTIONS_ROW_RIGHT, scrollboxTop,
           OPTIONS_ROW_RIGHT + OPTIONS_ROW_HT - 1, scrollboxBot,
           darkGray);
}

/**
 * @param y number of option to draw
 * @param hilite bitfield: 1=focused, 0=not
 */
static void optionsDrawRow(const OptionsLine *dlg,
                           const union LJPrefs *prefs,
                           int y, int value, int hilite, int scroll) {
  unsigned int ht = text_height(aver16);
  int buttonY = OPTIONS_TOP + OPTIONS_ROW_HT * (y - scroll);
  int bgcolor = bgColor;
  const char *valueOverride = isDisabledOption(prefs, y);
  int textcolor = fgColor;
  
  if (valueOverride) {
    hilite |= 2;
    textcolor = makecol(128, 128, 128);
  }
  
  if (hilite == 3) {
    bgcolor = makecol(204, 204, 204);
  } else if (hilite == 1) {
    bgcolor = hiliteColor;
  }
  
  acquire_screen();
  rectfill(screen,
           OPTIONS_ROW_LEFT, buttonY,
           OPTIONS_ROW_RIGHT - 1, buttonY + OPTIONS_ROW_HT - 1,
           bgcolor);
  textout_ex(screen, aver16, dlg[y].name,
             OPTIONS_ROW_LEFT + 8, buttonY + (OPTIONS_ROW_HT - ht) / 2,
             textcolor, bgcolor);
  if (value >= 0 && value < dlg[y].maxValue) {
    textout_ex(screen, aver16,
               valueOverride ? valueOverride : dlg[y].valueNames[value],
               OPTIONS_ROW_MID, buttonY + (OPTIONS_ROW_HT - ht) / 2,
               textcolor, bgcolor);
  }
  
  // For an enabled selected item, draw the frame
  if (hilite == 1) {
    rect(screen,
         OPTIONS_ROW_LEFT, buttonY,
         OPTIONS_ROW_RIGHT - 1, buttonY + OPTIONS_ROW_HT - 1,
         fgColor);
  }
  
  release_screen();
}

void options(union LJPrefs *prefs)
{
  LJBits lastKeys = ~0;
  int done = 0;
  int y = 0;
  int scroll = 0;
  int redrawOptionsBox = 1;
  int redrawThisRow = 1;
  int lastTime = getTime();
  int dasDir = 0;
  int dasCounter = 0;

  clear_keybuf();
  redrawWholeScreen = 1;

  while (!done) {
    if (redrawWholeScreen) {
      redrawWholeScreen = 0;
      redrawOptionsBox = 1;
      acquire_screen();
      clear_to_color(screen, bgColor);
      textout_ex(screen, aver32, "LOCKJAW > Options", 16, 32, fgColor, -1);
      textout_ex(screen, aver32, "Up/Down: select; Left/Right change; Rotate: exit", 16, 522, fgColor, -1);
      textout_ex(screen, aver32, "(Saved to lj.ini)", 16, 552, fgColor, -1);
      release_screen();
    }
    
    if (redrawOptionsBox) {
      redrawOptionsBox = 0;
      redrawThisRow = 0;
      vsync();
      acquire_screen();
      optionsDrawScrollbar(scroll);
      for (int i = scroll; i < scroll + OPTIONS_MENU_VIS; ++i) {
        if (i != y) {
          optionsDrawRow(optionsMenu, prefs, i, prefs->number[i], 0, scroll);
        }
      }
      optionsDrawRow(optionsMenu, prefs, y, prefs->number[y], 1, scroll);
      release_screen();
    } else if (redrawThisRow) {
      redrawThisRow = 0;
      optionsDrawRow(optionsMenu, prefs, y, prefs->number[y], 1, scroll);
    }

    LJBits keys = menuReadPad();
    int lastY = y;
    LJBits newKeys = keys & ~lastKeys;
    while (keypressed()) {
      int scancode;
      ureadkey(&scancode);

      if (scancode == KEY_TAB) {
        keys |= VKEY_DOWN;
      } else if (scancode == KEY_PRTSCR) {
        saveScreen();
      }
    }
    
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
      ezPlaySample("shift_wav", 128);
    }
    if (newKeys & VKEY_DOWN) {
      dasDir = VKEY_DOWN;
      ++y;
      if (y >= OPTIONS_MENU_LEN) {
        y = 0;
      }
      ezPlaySample("shift_wav", 128);
    }

    if (!isDisabledOption(prefs, y)) {    
      if (newKeys & VKEY_RIGHT) {
        dasDir = VKEY_RIGHT;
        ++prefs->number[y];
        if (prefs->number[y] >= optionsMenu[y].maxValue) {
          prefs->number[y] = 0;
        }
      
        // need to redraw the whole box because options may have
        // become enabled or disabled
        redrawOptionsBox = 1;
        ezPlaySample("rotate_wav", 96);
      }
    
      if (newKeys & VKEY_LEFT) {
        dasDir = VKEY_LEFT;
        --prefs->number[y];
        if (prefs->number[y] < 0) {
          prefs->number[y] = optionsMenu[y].maxValue - 1;
        }
      
        redrawOptionsBox = 1;
        ezPlaySample("rotate_wav", 96);
      }
    }
    
    if (newKeys & (VKEY_ROTL | VKEY_ROTR)) {
      ezPlaySample("line_wav", 128);
      done = 1;
    }

    if (lastY != y) {
      int lastScroll = scroll;
      
      if (scroll > y - OPTIONS_MENU_VIS / 2 + 1) {
        scroll = y - OPTIONS_MENU_VIS / 2 + 1;
      } else if (scroll < y - OPTIONS_MENU_VIS / 2) {
        scroll = y - OPTIONS_MENU_VIS / 2;
      }
      if (scroll < 0) {
        scroll = 0;
      } else if (scroll > OPTIONS_MENU_LEN - OPTIONS_MENU_VIS) {
        scroll = OPTIONS_MENU_LEN - OPTIONS_MENU_VIS;
      }
      if (lastScroll == scroll) {
        vsync();
        optionsDrawRow(optionsMenu, prefs, lastY, prefs->number[lastY], 0, scroll);
        redrawThisRow = 1;
      } else {
        redrawOptionsBox = 1;
      }
    }

    while (lastTime > getTime()) {
      rest(5);
    }
    lastKeys = keys;
    ++lastTime;
  }
  saveOptions(prefs);
}

