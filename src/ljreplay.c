/* Replay functionality for LOCKJAW, an implementation of the Soviet Mind Game

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
#include "ljreplay.h"
#include <stdio.h>

#define FORMAT_VERSION 20070323

struct LJReplayFrame {
  char length;
  char reserved;
  unsigned short keys;
  LJInput x;
};

struct LJReplay {
  FILE *file;
};

static void fput32(unsigned int src, FILE *dst) {
  fputc(src >> 24, dst);
  fputc(src >> 16, dst);
  fputc(src >> 8, dst);
  fputc(src, dst);
}

static unsigned int fget32(FILE *src) {
  int c0 = fgetc(src) & 0xFF;
  c0 = (c0 << 8) | (fgetc(src) & 0xFF);
  c0 = (c0 << 8) | (fgetc(src) & 0xFF);
  c0 = (c0 << 8) | (fgetc(src) & 0xFF);
  return c0;
}
static void fput16(unsigned int src, FILE *dst) {
  fputc(src >> 8, dst);
  fputc(src, dst);
}

static unsigned int fget16(FILE *src) {
  int c0 = fgetc(src) & 0xFF;
  c0 = (c0 << 8) | (fgetc(src) & 0xFF);
  return c0;
}

static void LJField_serialize(const LJField *p, FILE *fp) {
  fput32(FORMAT_VERSION, fp);

  for (int y = 0; y < LJ_PF_HT; ++y) {
    for (int x = 0; x < LJ_PF_WID; ++x) {
      fputc(p->b[y][x], fp);
    }
  }
  for (int y = 0; y < LJ_PF_HT; ++y) {
    for (int x = 0; x < LJ_PF_WID; ++x) {
      fputc(p->c[y][x], fp);
    }
  }
  fput32(p->clearedLines, fp);
  fput32(p->sounds, fp);
  fput32(p->tempRows, fp);
  for (int y = 0; y < 1 + LJ_NEXT_PIECES; ++y) {
    fputc(p->curPiece[y], fp);
  }
  for (int y = 0; y < 14; ++y) {
    fputc(p->permuPiece[y], fp);
  }
  fputc(p->permuPhase, fp);
  for (int y = 0; y < LJ_MAX_LINES_PER_PIECE; ++y) {
    fput16(p->nLineClears[y], fp);
  }
  fput32(p->y, fp);
  fputc(p->state, fp);
  fputc(p->stateTime, fp);
  fputc(p->theta, fp);
  fputc(p->x, fp);
  fputc(p->hardDropY, fp);
  fputc(p->alreadyHeld, fp);
  fputc(p->isSpin, fp);
  fputc(p->nLinesThisPiece, fp);
  fputc(p->canRotate, fp);
  
  fput32(p->score, fp);
  fput32(p->lines, fp);
  fput32(p->gameTime, fp);
  fput16(p->holdPiece, fp);
  fputc(p->chain, fp);
  fputc(p->garbage, fp);
  fputc(p->garbageX, fp);
  fput16(p->nPieces, fp);
  fput16(p->outGarbage, fp);
  
  fputc(p->gimmick, fp);
  fput32(p->speedLevel, fp);
  fput32(p->bpmCounter, fp);
  fput32(p->speedupCounter, fp);
  fput32(p->goalCount, fp);
  fput32(p->seed, fp);
  fput32(p->gravity, fp);
  fputc(p->speedCurve, fp);
  fputc(p->goalType, fp);
  fputc(p->areTime, fp);
  fputc(p->areStyle, fp);
  fputc(p->lockReset, fp);
  fputc(p->lockDelay, fp);
  fputc(p->lineDelay, fp);
  fputc(p->ceiling, fp);
  fputc(p->leftWall, fp);
  fputc(p->rightWall, fp);
  fputc(p->randomizer, fp);
  fputc(p->rotationSystem, fp);
  fputc(p->garbageRandomness, fp);
  fputc(p->tSpinAlgo, fp);
  fputc(p->clearGravity, fp);
  fputc(p->squareMode, fp);
  fputc(p->scoreStyle, fp);
  fputc(p->setLockDelay, fp);
  fputc(p->upwardKicks, fp);
  fputc(p->maxUpwardKicks, fp);
  fputc(p->setLineDelay, fp);
  fputc(p->garbageStyle, fp);
}

static int LJField_deserialize(LJField *p, FILE *fp) {
  if (fget32(fp) != FORMAT_VERSION) {
    return -1;
  }

  for (int y = 0; y < LJ_PF_HT; ++y) {
    for (int x = 0; x < LJ_PF_WID; ++x) {
      p->b[y][x] = fgetc(fp);
    }
  }
  for (int y = 0; y < LJ_PF_HT; ++y) {
    for (int x = 0; x < LJ_PF_WID; ++x) {
      p->c[y][x] = fgetc(fp);
    }
  }
  p->clearedLines = fget32(fp);
  p->sounds = fget32(fp);
  p->tempRows = fget32(fp);
  for (int y = 0; y < 1 + LJ_NEXT_PIECES; ++y) {
    p->curPiece[y] = fgetc(fp);
  }
  for (int y = 0; y < 14; ++y) {
    p->permuPiece[y] = fgetc(fp);
  }
  p->permuPhase = fgetc(fp);
  for (int y = 0; y < LJ_MAX_LINES_PER_PIECE; ++y) {
    p->nLineClears[y] = fget16(fp);
  }
  p->y = fget32(fp);
  p->state = fgetc(fp);
  p->stateTime = fgetc(fp);
  p->theta = fgetc(fp);
  p->x = fgetc(fp);
  p->hardDropY = fgetc(fp);
  p->alreadyHeld = fgetc(fp);
  p->isSpin = fgetc(fp);
  p->nLinesThisPiece = fgetc(fp);
  p->canRotate = fgetc(fp);
  
  p->score = fget32(fp);
  p->lines = fget32(fp);
  p->gameTime = fget32(fp);
  p->holdPiece = fget16(fp);
  p->chain = fgetc(fp);
  p->garbage = fgetc(fp);
  p->garbageX = fgetc(fp);
  p->nPieces = fget16(fp);
  p->outGarbage = fget16(fp);
  
  p->gimmick = fgetc(fp);
  p->speedLevel = fget32(fp);
  p->bpmCounter = fget32(fp);
  p->speedupCounter = fget32(fp);
  p->goalCount = fget32(fp);
  p->seed = fget32(fp);
  p->gravity = fget32(fp);
  p->speedCurve = fgetc(fp);
  p->goalType = fgetc(fp);
  p->areTime = fgetc(fp);
  p->areStyle = fgetc(fp);
  p->lockReset = fgetc(fp);
  p->lockDelay = fgetc(fp);
  p->lineDelay = fgetc(fp);
  p->ceiling = fgetc(fp);
  p->leftWall = fgetc(fp);
  p->rightWall = fgetc(fp);
  p->randomizer = fgetc(fp);
  p->rotationSystem = fgetc(fp);
  p->garbageRandomness = fgetc(fp);
  p->tSpinAlgo = fgetc(fp);
  p->clearGravity = fgetc(fp);
  p->squareMode = fgetc(fp);
  p->scoreStyle = fgetc(fp);
  p->setLockDelay = fgetc(fp);
  p->upwardKicks = fgetc(fp);
  p->maxUpwardKicks = fgetc(fp);
  p->setLineDelay = fgetc(fp);
  p->garbageStyle = fgetc(fp);
  return 0;
}


LJReplay *newReplay(const char *filename, LJField *p) {
  LJReplay *r = malloc(sizeof(struct LJReplay));
  
  if (!r) {
    return NULL;
  }
  r->file = fopen(filename, "wb");
  if (!r->file) {
    free(r);
    return NULL;
  }
  LJField_serialize(p, r->file);
  return r;
}

void replayRecord(LJReplay *r, LJBits keys, const LJInput *in) {
  fputc(0, r->file);
  fputc(0, r->file);
  fputc(keys >> 8, r->file);
  fputc(keys, r->file);
  fputc(in->rotation, r->file);
  fputc(in->movement, r->file);
  fputc(in->gravity, r->file);
  fputc(in->other, r->file);
}

LJReplay *openReplay(const char *filename, LJField *p) {
  LJReplay *r = malloc(sizeof(struct LJReplay));
  
  if (!r) {
    return NULL;
  }
  r->file = fopen(filename, "rb");
  if (!r->file) {
    free(r);
    return NULL;
  }

  /* This deserialization is still NOT robust. */
  if (LJField_deserialize(p, r->file) < 0) {
    fclose(r->file);
    free(r);
    return 0;
  }
  return r;
}

int getReplayFrame(LJReplay *r, LJInput *d) {
  fgetc(r->file);
  fgetc(r->file);
  int keys = fgetc(r->file);
  
  if (keys == EOF) {
    return LJREPLAY_EOF;
  }
  keys = (keys << 8 & 0xFF) | (fgetc(r->file) & 0xFF);
  d->rotation = fgetc(r->file);
  d->movement = fgetc(r->file);
  d->gravity = fgetc(r->file);
  d->other = fgetc(r->file);
  return keys;
}

void replayClose(LJReplay *r) {
  if (r) {
    if (r->file) {
      fclose(r->file);
    }
    free(r);
  }
}
