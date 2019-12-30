/* PC frontend for LOCKJAW, an implementation of the Soviet Mind Game

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
#include <stdio.h>

#define USING_DEBRIEF_PAGE 1

// On newlib, use a variant of sprintf that doesn't handle floating
// point formats. On other C libraries, use standard sprintf.
#define siprintf sprintf

void buildDebriefPage(char *dst, const LJView *v) {

#if USING_DEBRIEF_PAGE
  const time_t finishTimeUNIX = time(NULL);
  const struct tm *finishTime = localtime(&finishTimeUNIX);
  char finishTimeStr[64];
  strftime(finishTimeStr, sizeof(finishTimeStr), "%Y-%m-%d at %H:%M", finishTime);

  const LJField *p = v->field;
  unsigned int gameSeconds = p->gameTime / 60U;
  unsigned long int tpmCenti = p->gameTime
                               ? 360000ULL * p->nPieces / p->gameTime
                               : 0;
  unsigned long int gpmCenti = p->gameTime
                               ? 360000ULL * p->outGarbage / p->gameTime
                               : 0;
  unsigned long int kptCenti = p->nPieces
                               ? 100U * v->control->presses / p->nPieces
                               : 666;
  
  int pos = 0;
  
  unsigned int nBlocksLeft = 0;
  for (int y = 0; y < LJ_PF_HT; ++y) {
    for (int x = 0; x < LJ_PF_WID; ++x) {
      if (p->b[y][x]) {
        ++nBlocksLeft;
      }
    }
  }

  pos += siprintf(dst + pos,
                   "\n\n%s %s",
                   v->control->countdown <= 0 ? "Cleared" : "Stopped",
                   gimmickNames[p->gimmick]);
  if (p->speedCurve == LJSPD_RHYTHM
      || p->speedCurve == LJSPD_TGM
      || p->speedCurve == LJSPD_DEATH) {
    pos += siprintf(dst + pos,
                    " at level %d",
                     p->speedLevel);
  }
  pos += siprintf(dst + pos,
                   "\non %s%s\n",
                   finishTimeStr,
                   p->reloaded ? " from saved state" : "");
  pos += siprintf(dst + pos,
                   "(rot=%s, lock=",
                   optionsRotNames[(size_t)p->rotationSystem]);
  if (p->setLockDelay >= 128) {
    pos += siprintf(dst + pos, "Never");
  } else {
    if (p->setLockDelay > 0) {
      pos += siprintf(dst + pos,
                       "%d ms ",
                       p->setLockDelay * 50 / 3);
    }
    pos += siprintf(dst + pos,
                     "%s",
                     optionsLockdownNames[(size_t)p->lockReset]);
  }
  pos += siprintf(dst + pos,
                   ", rnd=%s, spin=%s, speed=%s)\n",
                   optionsRandNames[(size_t)p->randomizer],
                   optionsTspinNames[(size_t)p->tSpinAlgo],
                   optionsSpeedCurveNames[(size_t)p->speedCurve]);
  pos += siprintf(dst + pos,
                   "(grav=%s, drop=%s, are=%d ms, das=%d ms %s, shadow=%s, next=%d pieces)\n",
                   optionsGravNames[(size_t)p->clearGravity],
                   optionsDropScoringNames[(size_t)p->dropScoreStyle],
                   v->field->areStyle * 50 / 3,
                   v->control->dasDelay * 50 / 3,
                   optionsDASNames[v->control->dasSpeed],
                   optionsShadowNames[v->hideShadow],
                   v->nextPieces);

  pos += siprintf(dst + pos,
                   "\nPlayed for %u:%02u.%02u\n",
                   gameSeconds / 60U,
                   gameSeconds % 60U,
                   (p->gameTime - gameSeconds * 60) * 5U / 3U);
  pos += siprintf(dst + pos,
                   "Played %d tetrominoes (%u.%02u per minute)\n",
                   p->nPieces,
                   (unsigned int) (tpmCenti / 100),
                   (unsigned int) (tpmCenti % 100));
  pos += siprintf(dst + pos,
                   "Pressed %u keys (%u.%02u per tetromino)\n\n",
                   v->control->presses,
                   (unsigned int) (kptCenti / 100),
                   (unsigned int) (kptCenti % 100));

  pos += siprintf(dst + pos,
                   "Made %u lines\n",
                   p->lines);
  if (p->clearGravity == LJGRAV_NAIVE) {
    pos += siprintf(dst + pos,
                     "(single: %u; double: %u; triple: %u; home run: %u; ",
                     p->nLineClears[0],
                     p->nLineClears[1],
                     p->nLineClears[2],
                     p->nLineClears[3]);
    pos += siprintf(dst + pos,
                     "T single: %u; T double: %u; T triple: %u)\n",
                     p->nLineClears[4],
                     p->nLineClears[5],
                     p->nLineClears[6]);
  } else {
    pos += siprintf(dst + pos,
                     "(single: %u; double: %u; triple: %u; quad: %u; ",
                     p->nLineClears[0],
                     p->nLineClears[1],
                     p->nLineClears[2],
                     p->nLineClears[3]);
    pos += siprintf(dst + pos,
                     "x5: %u; x6: %u; x7: %u; x8+: %u)\n",
                     p->nLineClears[4],
                     p->nLineClears[5],
                     p->nLineClears[6],
                     p->nLineClears[7]);
  }
  pos += siprintf(dst + pos,
                   "Sent %u garbage (%u.%02u per minute)\n",
                   p->outGarbage,
                   (unsigned int) (gpmCenti / 100),
                   (unsigned int) (gpmCenti % 100));
  pos += siprintf(dst + pos,
                   "Left %d blocks behind\n\n",
                   nBlocksLeft);

  pos += siprintf(dst + pos,
                   "%s score: %d\n",
                   optionsScoringNames[p->scoreStyle], p->score);
      
      /*
      tod stats not in lj

        points/line, 40 lines score, silver squares, gold squares
        
        */

#else
  sprintf(dst, "\n\nDebrief disabled. Score: %u\n", v->field->score);
#endif
}


static void debriefDrawPage(const char *page) {
  int y = 40;
  char line[256];
  int linePos = 0;

  
  acquire_screen();
  clear_to_color(screen, bgColor);
  textout_centre_ex(screen, aver32, "GAME OVER", 400, 40, fgColor, -1);

  while (*page) {
    int c = *page++;

    // Break at newline and at end of text
    if (c == '\n' || c == 0) {

      // Draw parenthetical lines in smaller font
      const FONT *f = (line[0] == '(') ? aver16 : aver32;
      int lineH = text_height(f);

      // Terminate the line of text and print it
      line[linePos] = 0;
      textout_ex(screen, f, line, 40, y, fgColor, -1);

      linePos = 0;  // Carriage return
      y += (line[0] != 0)
           ? lineH * 6 / 5
           : 20;  // Line feed
    } else {
      line[linePos++] = c;
    }
  }
  textout_centre_ex(screen, aver32, "Press Rotate", 400, 552, fgColor, -1);
  release_screen();
}

/**
 * Reports the player's performance.
 */
void debrief(const LJView *v) {
  char page[5000];
  
  buildDebriefPage(page, v);
  FILE *logFile = fopen("lj-scores.txt", "at");
  fputs(page, logFile);
  fclose(logFile);

  LJBits lastKeys = ~0;
  redrawWholeScreen = 1;
  clear_keybuf();

  while (!wantsClose) {
    if (redrawWholeScreen) {
      redrawWholeScreen = 0;
      debriefDrawPage(page);
    }
    LJBits keys = menuReadPad();
    LJBits newKeys = keys & ~lastKeys;

    while (keypressed()) {
      int scancode;
      ureadkey(&scancode);

      if (scancode == KEY_PRTSCR) {
        saveScreen();
      }
    }


    if (newKeys & (VKEY_ROTR | VKEY_ROTL)) {
      break;
    } else {
      rest(30);
      lastKeys = keys;
    }
  }
}
