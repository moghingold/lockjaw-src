/* Wall kick tables for LOCKJAW, an implementation of the Soviet Mind Game

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


/* SRS *************************************************************/

// These wall kicks are for counterclockwise rotation TO a
// given orientation.
// Based on http://www.the-shell.net/img/srs_study.html

static const WallKickTable jlstzSRSKicksL = {
  { WK( 0, 0),WK( 1, 0),WK( 1,-1),WK( 0, 2),WK( 1, 2) },  // R->U
  { WK( 0, 0),WK(-1, 0),WK(-1, 1),WK( 0,-2),WK(-1,-2) },  // D->R
  { WK( 0, 0),WK(-1, 0),WK(-1,-1),WK( 0, 2),WK(-1, 2) },  // L->D
  { WK( 0, 0),WK( 1, 0),WK( 1, 1),WK( 0,-2),WK( 1,-2) }   // U->L
};

static const WallKickTable iSRSKicksL = {
  { WK( 0, 0),WK(-1, 0),WK( 2, 0),WK(-1,-2),WK( 2, 1) },  // R->U
  { WK( 0, 0),WK( 1, 0),WK(-2, 0),WK( 1,-1),WK(-2, 1) },  // D->R
  { WK( 0, 0),WK( 1, 0),WK(-2, 0),WK(-2,-1),WK( 1, 2) },  // L->D
  { WK( 0, 0),WK(-1, 0),WK( 2, 0),WK( 2,-1),WK(-1, 2) }   // U->L
};

static const WallKickTable jlstzSRSKicksR = {
  { WK( 0, 0),WK(-1, 0),WK(-1,-1),WK( 0, 2),WK(-1, 2) },  // L->U
  { WK( 0, 0),WK(-1, 0),WK(-1, 1),WK( 0,-2),WK(-1,-2) },  // U->R
  { WK( 0, 0),WK( 1, 0),WK( 1,-1),WK( 0, 2),WK( 1, 2) },  // R->D
  { WK( 0, 0),WK( 1, 0),WK( 1, 1),WK( 0,-2),WK( 1,-2) },  // D->L
};

static const WallKickTable iSRSKicksR = {
  { WK( 0, 0),WK( 1, 0),WK(-2, 0),WK( 1,-2),WK(-2, 1) },  // L->U
  { WK( 0, 0),WK( 1, 0),WK(-2, 0),WK(-2,-1),WK( 1, 2) },  // U->R
  { WK( 0, 0),WK(-1, 0),WK( 2, 0),WK( 2,-1),WK(-1, 2) },  // R->D
  { WK( 0, 0),WK(-1, 0),WK( 2, 0),WK(-1,-1),WK( 2, 1) }   // D->L
};

/* Arika ******************************************************/

// In ARS, kicks to position Up go down one, and kicks from position Up go up one
// each default kick should be followed by Right, then Left
// Remember: horizontal kicks are reversed in R tables,
// but SKIP_IF positions are not

// I does not kick
static const WallKickTable iTGM2KicksL = {
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // R->U
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // D->R
  { WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1) },  // L->D
  { WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0) }   // U->L
};

static const WallKickTable iTGM2KicksR = {
  { WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0) },  // L->U
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // U->R
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },  // R->D
  { WK( 1,-1),WK( 1,-1),WK( 1,-1),WK( 1,-1),WK( 1,-1) }   // D->L
};

static const WallKickTable jltSegaKicksL = {
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // R->U
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // D->R
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) }   // U->L
};

static const WallKickTable jltSegaKicksR = {
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // L->U
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },   // U->R
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // R->D
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) }  // D->L
};

// J and L do not kick if rotating from D and the square below bottom is filled
static const WallKickTable jlARSKicksL = {
  { WK( 0,-1),WK( 1,-1),WK(-1,-1),WK( 0,-1),WK( 0,-1) },  // R->U
  { SKIP_IF,WK( 1, 1)  ,WK( 0, 0),WK( 1, 0),WK(-1, 0) },  // D->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 1, 1),WK(-1, 1) }   // U->L
};

static const WallKickTable jlARSKicksR = {
  { WK( 0,-1),WK(-1,-1),WK( 1,-1),WK( 0,-1),WK( 0,-1) },  // L->U
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 1, 1),WK(-1, 1) },  // U->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { SKIP_IF,WK( 1, 1)  ,WK( 0, 0),WK( 1, 0),WK(-1, 0) },  // D->L
};

// S rounds to left
static const WallKickTable sARSKicksL = {
  { WK( 1,-1),WK( 2,-1),WK( 0,-1),WK( 1,-1),WK( 1,-1) },  // R->U
  { WK(-1, 0),WK( 0, 0),WK(-2, 0),WK(-1, 0),WK(-1, 0) },  // D->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { WK( 0, 1),WK( 1, 1),WK(-1, 1),WK( 0, 1),WK( 0, 1) }   // U->L
};

static const WallKickTable sARSKicksR = {
  { WK( 0,-1),WK( 1,-1),WK(-1,-1),WK( 0,-1),WK( 0,-1) },  // L->U
  { WK(-1, 1),WK( 0, 1),WK(-2, 1),WK(-1, 1),WK(-1, 1) },  // U->R
  { WK( 1, 0),WK( 2, 0),WK( 0, 0),WK( 1, 0),WK( 1, 0) },  //R->D
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) }   // D->L
};

// Z rounds to right
static const WallKickTable zARSKicksL = {
  { WK( 0,-1),WK( 1,-1),WK(-1,-1),WK( 0,-1),WK( 0,-1) },  // R->U
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // D->R
  { WK(-1, 0),WK( 0, 0),WK(-2, 0),WK(-1, 0),WK(-1, 0) },  // L->D
  { WK( 1, 1),WK( 2, 1),WK( 0, 1),WK( 1, 1),WK( 1, 1) }   // U->L
};

static const WallKickTable zARSKicksR = {
  { WK(-1,-1),WK( 0,-1),WK(-2,-1),WK(-1,-1),WK(-1,-1) },  // L->U
  { WK( 0, 1),WK( 1, 1),WK(-1, 1),WK( 0, 1),WK( 0, 1) },  // U->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // R->D
  { WK( 1, 0),WK( 2, 0),WK( 0, 0),WK( 1, 0),WK( 1, 0) }   // D->L
};


/* Sega (TGM3) *****************************************/

// I when rotating to U or D can kick up by 1 or 2
// T when rotating to U can kick up by 1

static const WallKickTable iTGM3KicksL = {
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // R->U
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0, 0),WK( 0, 1) },  // D->R
  { WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1) },  // L->D
  { WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 1),WK( 1, 2) }   // U->L
};

static const WallKickTable iTGM3KicksR = {
  { WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0) },  // L->U
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 1),WK( 0, 2) },  // U->R
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },  // R->D
  { WK( 1,-1),WK( 1,-1),WK( 1,-1),WK( 1, 0),WK( 1, 1) }   // D->L
};

static const WallKickTable tTGM3KicksL = {
  { WK( 0,-1),WK( 1,-1),WK(-1,-1),WK( 0, 0),WK( 0,-1) },  // R->U
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // D->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { SKIP_IF,WK( 1, 4)  ,WK( 0, 1),WK( 1, 1),WK(-1, 1) }   // U->L
};

static const WallKickTable tTGM3KicksR = {
  { WK( 0,-1),WK( 1,-1),WK(-1,-1),WK( 0, 0),WK( 0,-1) },  // L->U
  { SKIP_IF,WK( 1, 4)  ,WK( 0, 1),WK( 1, 1),WK(-1, 1) },  // U->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) },  // R->D
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 0),WK( 0, 0) }   // D->L
};


/* Tengen **********************************************************/

// All pieces are started with their left side in column 5 and flat side up.
// All 3-wide pieces stick to the left side of the bounding box.
// I sticks to the top when left and right and occupies the second column
// when up and down.
// No wall kick.
static const WallKickTable iTengenKicksL = {
  { WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1) },  // R->U
  { WK(-1,-2),WK(-1,-2),WK(-1,-2),WK(-1,-2),WK(-1,-2) },  // D->R
  { WK( 0, 2),WK( 0, 2),WK( 0, 2),WK( 0, 2),WK( 0, 2) },  // L->D
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) }   // U->L
};
static const WallKickTable iTengenKicksR = {
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0,-1) },  // L->U
  { WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1) },  // U->R
  { WK( 1, 2),WK( 1, 2),WK( 1, 2),WK( 1, 2),WK( 1, 2) },  // R->D
  { WK( 0,-2),WK( 0,-2),WK( 0,-2),WK( 0,-2),WK( 0,-2) }   // D->L
};

static const WallKickTable i3TengenKicks = {
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0,-1) },  // L->U
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // U->R
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },  // R->D
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) }   // D->L
};


static const WallKickTable jlstzTengenKicksL = {
  { WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0) },  // R->U
  { WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1) },  // D->R
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },  // L->D
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) }   // U->L
};
static const WallKickTable jlstzTengenKicksR = {
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // L->U
  { WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0) },  // U->R
  { WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1) },  // R->D
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) }   // D->L
};

/* NES **********************************************/

// No wall kick
// 3-wide pieces start out one block to the right
// I, S and Z round to the right and use positions R and D

static const WallKickTable roundRKicksL = {
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // R->U
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // D->R
  { WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0) },  // L->D
  { WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1),WK( 1, 1) }   // U->L
};

static const WallKickTable roundRKicksR = {
  { WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1),WK(-1,-1) },  // L->U
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) },  // U->R
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // R->D
  { WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0) }   // D->L
};

/* GB **********************************************/

// No wall kick
// I, S and Z round to the right and use positions D and L

// S rounds to left
static const WallKickTable roundLKicksL = {
  { WK( 1,-1),WK( 1,-1),WK( 1,-1),WK( 1,-1),WK( 1,-1) },  // R->U
  { WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0),WK(-1, 0) },  // D->R
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) },  // L->D
  { WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1),WK( 0, 1) }   // U->L
};

static const WallKickTable roundLKicksR = {
  { WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1),WK( 0,-1) },  // L->U
  { WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1),WK(-1, 1) },  // U->R
  { WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0),WK( 1, 0) },  //R->D
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0) }   // D->L
};


/* LJ: The Overdose (GBA) ******************************/
static const WallKickTable todKicks = {
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 1),WK( 0, 0) },  // R->U
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 1),WK( 0, 0) },  // D->R
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 1),WK( 0, 0) },  // L->D
  { WK( 0, 0),WK( 1, 0),WK(-1, 0),WK( 0, 1),WK( 0, 0) }   // U->L
};

const char *const optionsRotNames[N_ROTATION_SYSTEMS] = {
  "SRS", "Sega 1988", "Arika", "Tengen", "NES", "Game Boy", "TOD M4"
};

// I, J, L, O, S, T, Z, I2, I3, L3

const WallKickTable *const wkTablesL[N_ROTATION_SYSTEMS][10] = {
  { &iSRSKicksL, &jlstzSRSKicksL, &jlstzSRSKicksL, NULL, &jlstzSRSKicksL, &jlstzSRSKicksL, &jlstzSRSKicksL,
    NULL, &jlstzSRSKicksL, &jlstzSRSKicksL },
  { &iTGM2KicksL, &jltSegaKicksL, &jltSegaKicksL, NULL, &roundLKicksL, &jltSegaKicksL, &roundRKicksL,
    &roundLKicksL, NULL, NULL },
  { &iTGM3KicksL, &jlARSKicksL, &jlARSKicksL, NULL, &sARSKicksL, &tTGM3KicksL, &zARSKicksL,
    &sARSKicksL, NULL, NULL },
  { &iTengenKicksL, &jlstzTengenKicksL, &jlstzTengenKicksL, NULL, &jlstzTengenKicksL, &jlstzTengenKicksL, &jlstzTengenKicksL,
    &jlstzTengenKicksL, &i3TengenKicks, NULL },
  { &roundRKicksL, NULL, NULL, NULL, &roundRKicksL, NULL, &roundRKicksL,
    &roundRKicksL, NULL, NULL },
  { &roundLKicksL, NULL, NULL, NULL, &roundLKicksL, NULL, &roundLKicksL,
    &roundLKicksL, NULL, NULL },
  { &todKicks, &todKicks, &todKicks, NULL, &todKicks, &todKicks, &todKicks,
    &todKicks, &todKicks, &todKicks }  
};

const WallKickTable *const wkTablesR[N_ROTATION_SYSTEMS][10] = {
  { &iSRSKicksR, &jlstzSRSKicksR, &jlstzSRSKicksR, NULL, &jlstzSRSKicksR, &jlstzSRSKicksR, &jlstzSRSKicksR,
    NULL, &jlstzSRSKicksR, &jlstzSRSKicksR },
  { &iTGM2KicksR, &jltSegaKicksR, &jltSegaKicksR, NULL, &roundLKicksR, &jltSegaKicksR, &roundRKicksR,
    &roundLKicksR, NULL, NULL },
  { &iTGM3KicksR, &jlARSKicksR, &jlARSKicksR, NULL, &sARSKicksR, &tTGM3KicksR, &zARSKicksR,
    &sARSKicksR, NULL, NULL },
  { &iTengenKicksR, &jlstzTengenKicksR, &jlstzTengenKicksR, NULL, &jlstzTengenKicksR, &jlstzTengenKicksR, &jlstzTengenKicksR,
    &jlstzTengenKicksR, &i3TengenKicks, NULL },
  { &roundRKicksR, NULL, NULL, NULL, &roundRKicksR, NULL, &roundRKicksR,
    &roundRKicksR, NULL, NULL },
  { &roundLKicksR, NULL, NULL, NULL, &roundLKicksR, NULL, &roundLKicksR,
    &roundLKicksR, NULL, NULL },
  { &todKicks, &todKicks, &todKicks, NULL, &todKicks, &todKicks, &todKicks,
    &todKicks, &todKicks, &todKicks }
};

const unsigned char wkSpawnRotate[N_ROTATION_SYSTEMS][10] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 2, 2, 0, 0, 2, 0, 0, 0, 2 },
  { 0, 2, 2, 0, 0, 2, 0, 0, 0, 2 },
  { 0, 2, 2, 0, 0, 2, 0, 0, 0, 2 },
  { 0, 2, 2, 0, 0, 2, 0, 0, 0, 2 },
  { 0, 2, 2, 0, 0, 2, 0, 0, 0, 2 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

/**
 * Displacement of each piece's entry in each rotation system relative to SRS entry.
 */
const unsigned char wkSpawnMove[N_ROTATION_SYSTEMS][10] = {
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) },
  { WK( 0, 0),WK( 0, 1),WK( 0, 1),WK( 0, 0),WK( 0, 0),WK( 0, 1),WK( 0, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) },
  { WK( 0, 0),WK( 0, 1),WK( 0, 1),WK( 0, 0),WK( 0, 0),WK( 0, 1),WK( 0, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) },
  { WK( 1, 0),WK( 1, 1),WK( 1, 1),WK( 0, 0),WK( 1, 0),WK( 1, 1),WK( 1, 0),
    WK( 0, 0),WK( 1, 0),WK( 0, 0) },
  { WK( 0, 0),WK( 1, 1),WK( 1, 1),WK( 0, 0),WK( 1, 0),WK( 1, 1),WK( 1, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) },
  { WK( 0, 0),WK( 0, 1),WK( 0, 1),WK( 0, 0),WK( 0, 0),WK( 0, 1),WK( 0, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) },
  { WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),WK( 0, 0),
    WK( 0, 0),WK( 0, 0),WK( 0, 0) }
};

/**
 * Displacement of next piece in each rotation system relative to piece's entry.
 */
const unsigned char wkNextMove[N_ROTATION_SYSTEMS] = {
  WK( 0, 0),
  WK( 0, 0),
  WK( 0, 0),
  WK(-1, 0),
  WK( 0, 0),
  WK( 0, 0),
  WK( 0, 0)
};

/**
 * Color scheme for this rotation system (0: SRS; 1: Sega)
 * Use colorset 0 (SRS) if all wkSpawnRotate elements are 0 (that is,
 * (flat side down) and all default kicks are WK(0, 0).
 * Otherwise, your rotation system has what Eddie Rogers has called
 * a topknot, and you should use Sega colors.
 */
const unsigned char wkColorScheme[N_ROTATION_SYSTEMS] = {
  0, 1, 1, 1, 1, 1, 0
};
