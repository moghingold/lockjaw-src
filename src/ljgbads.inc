/* DS/GBA frontend for LOCKJAW, an implementation of the Soviet Mind Game

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

void finishSprites();

void cls(void) {
  for (int y = 0; y < SCREEN_H; ++y) {
    for (int x = 0; x < SCREEN_W; ++x) {
      MAP[31][y][x] = ' ';
    }
  }
}

void drawFrame(LJView *v) {
  int left = v->field->leftWall;
  int right = v->field->rightWall;  
  cls();
  if (DS_PFTOP > 0) {
    for (int x = left - 1; x < right + 1; ++x) {
      MAP[31][DS_PFTOP - 1][DS_PFLEFT + x] = '_';
    }
  }
  if (DS_PFTOP + LJ_PF_VIS_HT < SCREEN_H) {
    for (int x = left; x < right; ++x) {
      MAP[31][DS_PFTOP + LJ_PF_VIS_HT][DS_PFLEFT + x] = '"';
    }
  }
  for (int i = 0; i < LJ_PF_VIS_HT; ++i) {
    MAP[31][i + DS_PFTOP][DS_PFLEFT + left - 1] = 0x8003;
    MAP[31][i + DS_PFTOP][DS_PFLEFT + right] = 0x8003;
  }
  textout("Score", 1, 6 + DS_PFTOP, 0);
  textout("Lines", 1, 8 + DS_PFTOP, 0);
}

static void drawBlock(struct LJPCView *unused, int x, int y, int b) {
  if (x >= 0 && x < LJ_PF_WID && y >= 0 && y < LJ_PF_VIS_HT) {
    int c;
    
    if (b == 0x100) {
      c = '-';
    } else if (b == 0x110) {
      c = '_';
    } else if (b >= 0xA0 && b < 0xC0) {
      c = (b & 0xF0) << 8 | (b & 0x0F) | 0x100;
    } else if (b >= 0x10) {
      c = (b & 0xF0) << 8 | (b & 0x0F) | 0x10;
    } else {
      c = ' ';
    }
    MAP[31][DS_PFTOP + LJ_PF_VIS_HT - 1 - y][DS_PFLEFT + x] = c;
  }
}

void updField(const LJView *const v, LJBits rows) {
  const LJField *const p = v->field;

  for (int y = 0;
       y < LJ_PF_VIS_HT && rows != 0;
       ++y, rows >>= 1) {
    int blankTile = 0;
    
    if (y == p->ceiling) {
      blankTile = 0x110;
    }
    if (hotlineRows[y] && v->field->scoreStyle == LJSCORE_HOTLINE) {
      blankTile = 0x100;
    }
    if (rows & 1) {
      for (int x = p->leftWall; x < p->rightWall; x++) {
        int b = v->hidePF ? 0 : p->b[y][x];
        drawBlock(v->plat, x, y, b ? b : blankTile);
      }
    }
  }
}

void blitField(LJView *v) {

}

int getTime() {
  return curTime;
}

#if !defined(DISP_VBLANK_IRQ)
#define DISP_VBLANK_IRQ LCDC_VBL
#endif
#if !defined(IRQ_HANDLER)
#define IRQ_HANDLER INT_VECTOR
#endif

void install_timer(void)
{

  // Turn off interrupts before doing anything
  REG_IME = 0;

  // Overwrite the ISR
  IRQ_HANDLER = isr;

  // Hook up the interrupt destination
  REG_IE = IRQ_VBLANK;

  // Acknowledge all pending interrupts
  REG_IF = ~0;

  // Set up an interrupt source
  REG_DISPSTAT = DISP_VBLANK_IRQ;

  // Turn interrupts back on
  REG_IME = 1;
}

void yieldCPU(void) {
  // we're not multitasking so we don't need this
  // on the GBA and DS, vsync() does all the waiting we need
}

static void upcvt_4bit(void *dst, const u8 *src, size_t len)
{
  u32 *out = dst;

  for(; len > 0; len--)
  {
    u32 dst_bits = 0;
    u32 src_bits = *src++;
    u32 x;

    for(x = 0; x < 8; x++)
    {
      dst_bits <<= 4;
      dst_bits |= src_bits & 1;
      src_bits >>= 1;
    }
    *out++ = dst_bits;
  }
}

extern const unsigned char text_chr[];
extern const unsigned int text_chr_size;
extern const unsigned char gbablk_chr[];
extern const unsigned int gbablk_chr_size;

static void loadOneConnection(void *in_dst, const void *in_src) {
  u16 *dst = in_dst;
  const u16 *src = in_src;
  for (unsigned int conn = 0; conn < 16; ++conn) {
    unsigned int topSegY = (conn & CONNECT_U) ? 32 : 0;
    unsigned int botSegY = (conn & CONNECT_D) ? 8 : 40;
    unsigned int leftSegX = (conn & CONNECT_L) ? 16 : 0;
    unsigned int rightSegX = (conn & CONNECT_R) ? 1 : 17;
    for (unsigned int i = 0; i < 8; i += 2) {
      *dst++ = src[leftSegX + topSegY + i];
      *dst++ = src[rightSegX + topSegY + i];
    }
    for (unsigned int i = 0; i < 8; i += 2) {
      *dst++ = src[leftSegX + botSegY + i];
      *dst++ = src[rightSegX + botSegY + i];
    }
  }
}

static void loadConnections(void) {
  loadOneConnection(PATRAM4(0, 16), gbablk_chr + 8*32);
  loadOneConnection(PATRAM4(0, 256), gbablk_chr + 12*32);
}

static void load_font(void) {
  upcvt_4bit(PATRAM4(0, 0), text_chr, text_chr_size);
  memcpy(PATRAM4(0, 0), gbablk_chr, 8*32);
  memcpy(SPR_VRAM(0), gbablk_chr, 8*32);
  loadConnections();
}

void textout(const char *str, int x, int y, int c) {
  u16 *dst = &(MAP[31][y][x]);
  int spacesLeft = SCREEN_W - x;

  c <<= 12;
  while (*str != 0 && spacesLeft > 0) {
    *dst++ = c | *(unsigned char *)str++;
    --spacesLeft;
  }
}

static const u16 srsColors[12] = {
  RGB5(2, 2, 2),
  RGB5(0, 3, 3),
  RGB5(0, 0, 3),
  RGB5(3, 2, 0),
  RGB5(3, 3, 0),
  RGB5(0, 3, 0),
  RGB5(2, 0, 3),
  RGB5(3, 0, 0),
  RGB5(2, 2, 2),
  RGB5(3, 0, 0),
  RGB5(2, 2, 2),
  RGB5(3, 2, 1),
};

static const u16 arsColors[12] = {
  RGB5(2, 2, 2),
  RGB5(3, 0, 0),
  RGB5(0, 0, 3),
  RGB5(3, 2, 0),
  RGB5(3, 3, 0),
  RGB5(2, 0, 3),
  RGB5(0, 3, 3),
  RGB5(0, 3, 0),
  RGB5(2, 2, 2),
  RGB5(3, 0, 0),
  RGB5(2, 2, 2),
  RGB5(3, 2, 1),
};

void setupPalette(const u16 *colors) {
  for (int i = 1; i < 12; ++i) {
    int c = colors[i];
    
    BG_PALETTE[i * 16 + 1] = RGB5(22,22,22) + 3 * c;
    BG_PALETTE[i * 16 + 2] = RGB5(13,13,13) + 6 * c;
    BG_PALETTE[i * 16 + 3] = RGB5( 4, 4, 4) + 9 * c;
    BG_PALETTE[i * 16 + 4] = RGB5( 4, 4, 4) + 7 * c;
    BG_PALETTE[i * 16 + 5] = RGB5( 4, 4, 4) + 5 * c;
    BG_PALETTE[i * 16 + 6] = RGB5( 4, 4, 4) + 3 * c;
  }
  memcpy(SPRITE_PALETTE, BG_PALETTE, 12 * 32);
}

// libnds style wrapper around libgba header
#ifndef DISPLAY_BG0_ACTIVE
#define DISPLAY_BG0_ACTIVE BG0_ON
#define DISPLAY_SPR_ACTIVE OBJ_ON
#define MODE_0_2D MODE_0
#define DISPLAY_SPR_1D_LAYOUT OBJ_1D_MAP
static inline void videoSetMode(int x) {
  REG_DISPCNT = x;
}

#endif

void coprNotice(void) {
  BG_PALETTE[0] = RGB5(31, 31, 31);
  videoSetMode(MODE_0_2D
               | DISPLAY_BG0_ACTIVE);
  cls();
  textout("LOCKJAW: The Reference",    2, 2, 0);
  textout(LJ_VERSION,                  2, 3, 0);

  textout("© 2007 Damian Yerrick",     2, 5, 0);
  textout("Not sponsored or endorsed", 2, 6, 0);
  textout("by Nintendo or The Tetris", 2, 7, 0);
  textout("Company.  Comes with",      2, 8, 0);
  textout("ABSOLUTELY NO WARRANTY.",   2, 9, 0);
  textout("This is free software, and",2,10, 0);
  textout("you are welcome to share",  2,11, 0);
  textout("it under the conditions",   2,12, 0);
  textout("described in GPL.txt.",     2,13, 0);

  {
    char line[36];
    siprintf(line, "spr dst = 0x%08x", (unsigned long int)SPR_VRAM(0));
    textout(line, 2,15, 0);
  }

  textout("Press Start",               2,SCREEN_H - 2, 0);

  while (!(REG_KEYINPUT & KEY_START)) {
    vsync();
  }
  while (REG_KEYINPUT & KEY_START) {
    vsync();
  }
}

LJBits readPad(void) {
  LJBits hwKeys = ~REG_KEYINPUT;
  LJBits out = 0;
  
  if (hwKeys & KEY_UP) {
    out |= VKEY_UP;
  }
  if (hwKeys & KEY_DOWN) {
    out |= VKEY_DOWN;
  }
  if (hwKeys & KEY_LEFT) {
    out |= VKEY_LEFT;
  }
  if (hwKeys & KEY_RIGHT) {
    out |= VKEY_RIGHT;
  }
  if (hwKeys & KEY_B) {
    out |= VKEY_ROTL;
  }
  if (hwKeys & KEY_A) {
    out |= VKEY_ROTR;
  }
  if (hwKeys & (KEY_L | KEY_R)) {
    out |= VKEY_HOLD;
  }

  // on the GBA, we need to add the console buttons in
  out |= (hwKeys & 0x3FF) << 16;
  return out;
}

void ljBeginDraw(LJView *v, int sync) {
  vsync();
  finishSprites();
}

void ljEndDraw(LJView *v) {

}

/* Replay stubs */
void replayRecord(struct LJReplay *r, LJBits keys, const LJInput *in) {

}

void replayClose(struct LJReplay *r) {

}

int getReplayFrame(struct LJReplay *r, LJInput *d) {
  return 0;
}

#define READY_GO_LINE 13

void startingAnimation(LJView *v) {
  drawFrame(v);
  finishSprites();
  videoSetMode(MODE_0_2D
               | DISPLAY_BG0_ACTIVE);

  textout("Ready",
          (LJ_PF_WID - 5) / 2 + DS_PFLEFT,
          DS_PFTOP + LJ_PF_VIS_HT - 1 - READY_GO_LINE,
          0);
  for (int i = 0; i < 30; ++i) {
    vsync();
    gba_poll_sound(v->plat);
  }
  v->backDirty = ~0;
  updField(v, ~0);
  videoSetMode(MODE_0_2D
               | DISPLAY_BG0_ACTIVE
               | DISPLAY_SPR_1D_LAYOUT 
               | DISPLAY_SPR_ACTIVE);
  drawScore(v);
  finishSprites();
  
  textout(" GO! ",
          (LJ_PF_WID - 5) / 2 + DS_PFLEFT,
          DS_PFTOP + LJ_PF_VIS_HT - 1 - READY_GO_LINE,
          0);
  v->backDirty |= 1 << READY_GO_LINE;
  for (int i = 0; i < 30; ++i) {
    vsync();
    gba_poll_sound(v->plat);
  }
}

int pauseGame(struct LJPCView *v) {
  LJBits lastKeys = ~0;
  int unpaused = 0;
  
  // hide playfield
  for (int y = DS_PFTOP; y < DS_PFTOP + LJ_PF_VIS_HT; ++y) {
    for (int x = DS_PFLEFT;
         x < DS_PFLEFT + LJ_PF_WID;
         ++x) {
      MAP[31][y][x] = ' ';
    }
  }
  textout("Paused", 2 + DS_PFLEFT, 8 + DS_PFTOP, 0);

  while (!unpaused || (lastKeys & KEY_START)) {
    int keys = ~REG_KEYINPUT;
    if (keys & ~lastKeys & KEY_START) {
      unpaused = 1;
    }
    finishSprites();
    vsync();
    gba_poll_sound(v);
    lastKeys = keys;
  }
  return 0;
}

int ljHandleConsoleButtons(LJView *v) {
  LJBits keys = ~REG_KEYINPUT;
  
  if (keys & KEY_START) {
    pauseGame(v->plat);
    drawFrame(v);
    v->backDirty = ~0;
  }
  return 0;
}

LJBits drawPiece(LJView *const v, void *const b,
                 int piece, int x, int y, int theta,
                 int color, int w, int h);

void drawFallingPiece(LJView *v) {
  LJBits bits = 0;
  const LJField *const p = v->field;
  int piece = p->curPiece[0];
  int y = ljfixfloor(p->y);
  const int w = 8;
  const int h = 8;
  int drawnY = v->smoothGravity ? ljfixfloor(h * p->y) : h * y;
  const int color = (p->state == LJS_LANDED)
                    ? -128 - ((p->stateTime + 1) * 128 / (p->lockDelay + 1))
                    : pieceColors[piece];

  bits = drawPiece(v, NULL, piece,
                   w * (p->x + DS_PFLEFT),
                   h * (LJ_PF_VIS_HT + DS_PFTOP) - drawnY,
                   p->theta,
                   color, w, h);
  bits = (y >= 0) ? bits << y : bits >> -y;
  bits &= (1 << LJ_PF_VIS_HT) - 1;

  v->backDirty |= bits | (bits << 1);
  v->frontDirty |= bits | (bits << 1);
}

#define SHADOW_BLOCK 0x00

void drawShadow(LJView *v) {
  LJBits bits = 0;
  const LJField *const p = v->field;
  int piece = p->curPiece[0];
  int y = p->hardDropY;
  const int w = 8;
  const int h = 8;

  bits = drawPiece(v, NULL, piece,
                   w * (p->x + DS_PFLEFT),
                   h * (LJ_PF_VIS_HT + DS_PFTOP - y),
                   p->theta,
                   SHADOW_BLOCK, w, h);
  bits = (y >= 0) ? bits << y : bits >> -y;
  bits &= (1 << LJ_PF_VIS_HT) - 1;

  v->backDirty |= bits;
  v->frontDirty |= bits;
}

void drawNextPieces(LJView *v) {
  int holdPieceColor = v->field->alreadyHeld
                       ? 0x80
                       : pieceColors[v->field->holdPiece];

  // Draw hold piece
  drawPiece(v, NULL,
            v->field->holdPiece,
            (DS_PFLEFT - 5) * 8, (DS_PFTOP + 5) * 8, 4,
            holdPieceColor, 8, 8);

  // Draw next pieces
  int y = 32 + 8 * DS_PFTOP;
  int x = (DS_PFLEFT + LJ_PF_WID) * 8;
  for(int i = 1; i <= v->nextPieces; ++i) {
    int piece = v->field->curPiece[i];

    if (!v->hideNext) {
      drawPiece(v, NULL,
                piece, x, y, 4,
                pieceColors[piece], 8, 8);
    }
    y += 20;
  }
  v->frontDirty &= (1 << LJ_PF_VIS_HT) - 1;
}

void drawScore(LJView *v) {
  char txt[16];
  
  siprintf(txt, "%8u", v->field->score);
  textout(txt, 0, 7 + DS_PFTOP, 0);
  siprintf(txt, "%8u", v->field->lines);
  textout(txt, 0, 9 + DS_PFTOP, 0);
  
  if (v->field->speedLevel > 0) {
    textout("Level:", 1, SCREEN_H - 3, 0);
    siprintf(txt, "%9u", v->field->speedLevel);
    textout(txt, 0, SCREEN_H - 2, 0);
  }
  
  {
    int seconds = v->field->gameTime / 60;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    siprintf(txt, "%6d:%02d", minutes, seconds);
    textout(txt, 0, SCREEN_H - 1, 0);
  }
  drawNextPieces(v);
}

