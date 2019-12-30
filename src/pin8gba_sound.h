/*
 * pin8gba_sound.h
 * Header file for GBA sound registers
 */


/* Copyright 2001-2006 Damian Yerrick

(insert zlib license here)

*/


/* Why the funny register names?

The register names do not match the names in the official GBA
documentation.  I choose the names that seem most logical to me.
In addition, I do slick macro tricks with the VRAM addresses for
maps and the like and with registers that control timers, DMA,
and backgrounds.  The gba.h that comes with wintermute's libgba
incorporates some but not all of these tricks.  In addition,
the names in gba.h for the sound registers aren't descriptive
at all.

*/

#ifndef PIN8GBA_SOUND_H
#ifdef __cplusplus
extern "C" {
#endif
#define PIN8GBA_SOUND_H


#define DMGSNDCTRL         (*(volatile u16 *)0x04000080)
#define DMGSNDCTRL_LVOL(x) (x)
#define DMGSNDCTRL_RVOL(x) ((x) << 4)
#define DMGSNDCTRL_LSQR1   0x0100
#define DMGSNDCTRL_LSQR2   0x0200
#define DMGSNDCTRL_LTRI    0x0400
#define DMGSNDCTRL_LNOISE  0x0800
#define DMGSNDCTRL_RSQR1   0x1000
#define DMGSNDCTRL_RSQR2   0x2000
#define DMGSNDCTRL_RTRI    0x4000
#define DMGSNDCTRL_RNOISE  0x8000

#define DSOUNDCTRL           (*(volatile u16 *)0x04000082)
#define DSOUNDCTRL_DMG25     0x0000
#define DSOUNDCTRL_DMG50     0x0001
#define DSOUNDCTRL_DMG100    0x0002
#define DSOUNDCTRL_A50       0x0000
#define DSOUNDCTRL_A100      0x0004
#define DSOUNDCTRL_B50       0x0000
#define DSOUNDCTRL_B100      0x0008
#define DSOUNDCTRL_AR        0x0100
#define DSOUNDCTRL_AL        0x0200
#define DSOUNDCTRL_ATIMER(x) ((x) << 10)
#define DSOUNDCTRL_ARESET    0x0400
#define DSOUNDCTRL_BR        0x1000
#define DSOUNDCTRL_BL        0x2000
#define DSOUNDCTRL_BTIMER(x) ((x) << 14)
#define DSOUNDCTRL_BRESET    0x8000

#define SNDSTAT        (*(volatile u16*)0x04000084)
#define SNDSTAT_SQR1   0x0001
#define SNDSTAT_SQR2   0x0002
#define SNDSTAT_TRI    0x0004
#define SNDSTAT_NOISE  0x0008
#define SNDSTAT_ENABLE 0x0080

#define SNDBIAS      (*(volatile u16 *)0x04000088)
#define SETSNDRES(x) SNDBIAS = (SNDBIAS & 0x3fff) | (x << 14)

#define DSOUND_FIFOA (*(volatile u32 *)0x040000a0)
#define DSOUND_FIFOB (*(volatile u32 *)0x040000a4)


#define SQR1SWEEP   (*(volatile u16 *)0x04000060)
#define SQR1SWEEP_OFF 0x0008

#define SQR1CTRL    (*(volatile u16 *)0x04000062)
#define SQR2CTRL    (*(volatile u16 *)0x04000068)
#define NOISECTRL   (*(volatile u16 *)0x04000078)
#define SQR_DUTY(n) ((n) << 6)
#define SQR_VOL(n)  ((n) << 12)

#define SQR1FREQ      (*(volatile u16 *)0x04000064)
#define SQR2FREQ      (*(volatile u16 *)0x0400006c)
#define TRIFREQ       (*(volatile u16 *)0x04000074)
#define FREQ_HOLD  0x0000
#define FREQ_TIMED 0x4000
#define FREQ_RESET 0x8000

#define NOISEFREQ       (*(volatile u16 *)0x0400007c)
#define NOISEFREQ_127   0x0008
#define NOISEFREQ_OCT(x) ((x) << 4)

#define TRICTRL         (*(volatile u16 *)0x04000070)
#define TRICTRL_2X32    0x0000
#define TRICTRL_1X64    0x0020
#define TRICTRL_BANK(x) ((x) << 6)
#define TRICTRL_ENABLE  0x0080

#define TRILENVOL        (*(volatile u16 *)0x04000072)
#define TRILENVOL_LEN(x) (256 - (x))
#define TRILENVOL_MUTE   0x0000
#define TRILENVOL_25     0x6000
#define TRILENVOL_50     0x4000
#define TRILENVOL_75     0x8000
#define TRILENVOL_100    0x2000

#define TRIWAVERAM ((volatile u32 *)0x04000090)


#ifdef __cplusplus
}
#endif
#endif
