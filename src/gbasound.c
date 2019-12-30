#include "ljgba.h"
#include "pin8gba_sound.h"
extern const unsigned short note_freqs[];

static const u16 moveEffect[] = {
  0, 3,
  0x516D, 0x0000, 0x0100
};

static const u16 rotateEffect[] = {
  1, 7,
  0x5161, 0x0000, 0x5166, 0x0000, 0x516b, 0x0000, 0x0100
};

static const u16 landEffect[] = {
  1, 6,
  0x8188, 0x7185, 0x6182, 0x5181, 0x3180, 0x0000
};

static const u16 lockEffect[] = {
  3, 1,
  0x2138
};

static const u16 lineEffect[] = {
  0, 12,
  0xB1A4, 0xA1A6, 0x91A9, 0x81A4, 0x71A7, 0x61A9,
  0x61A4, 0x51A6, 0x41A9, 0x31A4, 0x21A7, 0x11A9
};

static const u16 holdEffect[] = {
  3, 8,
  0x3030, 0x0000, 0x4032, 0x0000, 0x3130, 0x0000, 0x202c, 0x2128
};

static const u16 irsRotateEffect[] = {
  1, 12,
  0x5161, 0x51B1, 0x5166, 0x51B6, 0x516B, 0x51BB, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0100
};

static const u16 homerEffect[] = {
  0, 22,
  0xB1A4, 0xA1A6, 0x91A9, 0x81A4, 0x71A7, 0x61A9,
  0x61A4, 0x51A6, 0x41A9, 0x31A4,
  0xB1A7, 0xA1A9, 0x91AC, 0x81A7, 0x71AA, 0x61AC,
  0x51A7, 0x41A9, 0x31AC, 0x21A7, 0x11AA, 0x01AC
};

static const u16 streakEffect[] = {
  0, 30,
  0xB1A4, 0xA1A6, 0x91A9, 0x81A4, 0x71A7, 0x61A9,
  0x61A4, 0x51A6, 0x41A9, 0x31A4,
  0xB1A7, 0xA1A9, 0x91AC, 0x81A7, 0x71AA, 0x61AC,
  0x51A7, 0x41A9, 0x31AC, 0x21A7,
  0xB1AA, 0xA1AC, 0x91AF, 0x71AA, 0x61AD, 0x51AF,
  0x31AA, 0x21AC, 0x11AF, 0x01AA
};

static const u16 sectionEffect[] = {
  1, 30,
  0xB1B0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0xB1B4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0xB1B7, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0xB1BC, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0xB1B0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x01B0
};

static const u16 *const effects[] = {
  moveEffect, rotateEffect, landEffect, lockEffect, lineEffect,
  holdEffect, irsRotateEffect, homerEffect, streakEffect, sectionEffect
};

void play_note(unsigned int ch, unsigned int note, unsigned int instrument) {
  switch (ch) {
  case 0:
    SQR1CTRL = instrument;
    SQR1FREQ = note_freqs[note] | FREQ_RESET;
    break;
  case 1:
    SQR2CTRL = instrument;
    SQR2FREQ = note_freqs[note] | FREQ_RESET;
    break;
  case 3:
    NOISECTRL = instrument;
    {
      int octave = (note & 0x3E) << 2;
      int pitch = note & 0x03;
      NOISEFREQ = (octave | pitch) ^ (0xF7 | FREQ_RESET);
    }
    break;
  }
}

void gba_poll_sound(struct LJPCView *v) {
  for (int i = 0; i < 4; ++i) {
    if (v->sndLeft[i]) {
      unsigned int data = *v->sndData[i];
      if (data) {
        play_note(i, data & 0x003F, data & 0xFFC0);
      }
      ++v->sndData[i];
      --v->sndLeft[i];
    }
  }
}

void gba_play_sound(struct LJPCView *v, int effect) {
  const u16 *data = effects[effect];
  int ch = data[0];
  int len = data[1];
  
  // square waves can be played on either channel 0 or 1;
  // switch if this channel is occupied and the other is free
  if (ch < 2 && v->sndLeft[ch] && !v->sndLeft[1 - ch]) {
    ch = 1 - ch;
  }
  v->sndData[ch] = data + 2;
  v->sndLeft[ch] = len;
}

/**
 * Sets the sound bias to mid and resolution to 8-bit.
 * Setting bias is needed because some launchers (such as pogoshell)
 * set it to a state that mutes the tone generators.
 */
static void set_bias(void)
{
  asm volatile("mov r2, #2; lsl r2, #8; swi 0x19" ::: "r0", "r1", "r2", "r3");
  SETSNDRES(1);
}

void install_sound(struct LJPCView *v) {
  SNDSTAT = SNDSTAT_ENABLE;
  DMGSNDCTRL = DMGSNDCTRL_LVOL(7) | DMGSNDCTRL_RVOL(7)
             | DMGSNDCTRL_LSQR1 | DMGSNDCTRL_RSQR1
             | DMGSNDCTRL_LSQR2 | DMGSNDCTRL_RSQR2
             | DMGSNDCTRL_LTRI | DMGSNDCTRL_RTRI
             | DMGSNDCTRL_LNOISE | DMGSNDCTRL_RNOISE;
  DSOUNDCTRL = DSOUNDCTRL_DMG100;
  set_bias();
  SQR1SWEEP = SQR1SWEEP_OFF;

  play_note(0, 0, SQR_DUTY(1) | SQR_VOL(8) | 0x0100);
  v->sndLeft[0] = 0;
  v->sndLeft[1] = 0;
  v->sndLeft[2] = 0;
  v->sndLeft[3] = 0;
}

