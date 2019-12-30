/*
   Axe clone
   Version 0.1
   Damian Yerrick (tepples@spamcop.net)
   http://www.pineight.com/ds/

   Subject to copyright license in README.txt

   Based on framebuffer_demo1 ARM9 Code 
   Chris Double (chris.double@double.co.nz)
   http://www.double.co.nz/nintendo_ds
*/

#include <nds.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////


#define TOUCH_CAL_X1 (*(vs16*)0x027FFCD8)
#define TOUCH_CAL_Y1 (*(vs16*)0x027FFCDA)
#define TOUCH_CAL_X2 (*(vs16*)0x027FFCDE)
#define TOUCH_CAL_Y2 (*(vs16*)0x027FFCE0)
#define SCREEN_WIDTH    256
#define SCREEN_HEIGHT   192
s32 TOUCH_WIDTH;
s32 TOUCH_HEIGHT;
s32 TOUCH_OFFSET_X;
s32 TOUCH_OFFSET_Y;


//////////////////////////////////////////////////////////////////////


/* Frequencies of 32 different notes on an E major pentatonic scale */
const unsigned int axeFreqs[32] =
{
  (int)SOUND_FREQ(  34.65*8),
  (int)SOUND_FREQ(  41.20*8),
  (int)SOUND_FREQ(  46.25*8),
  (int)SOUND_FREQ(  51.91*8),
  (int)SOUND_FREQ(  61.73*8),

  (int)SOUND_FREQ(  69.30*8),
  (int)SOUND_FREQ(  82.41*8),
  (int)SOUND_FREQ(  92.50*8),
  (int)SOUND_FREQ( 103.82*8),
  (int)SOUND_FREQ( 123.47*8), 

  (int)SOUND_FREQ( 138.591*8),
  (int)SOUND_FREQ( 164.841*8),
  (int)SOUND_FREQ( 184.997*8),
  (int)SOUND_FREQ( 207.652*8),
  (int)SOUND_FREQ( 246.942*8),

  (int)SOUND_FREQ( 277.183*8),
  (int)SOUND_FREQ( 329.628*8),
  (int)SOUND_FREQ( 369.994*8),
  (int)SOUND_FREQ( 415.305*8),
  (int)SOUND_FREQ( 493.883*8),

  (int)SOUND_FREQ( 554.37*8),
  (int)SOUND_FREQ( 659.25*8),
  (int)SOUND_FREQ( 739.99*8),
  (int)SOUND_FREQ( 830.61*8),
  (int)SOUND_FREQ( 987.77*8),

  (int)SOUND_FREQ(1108.73*8),
  (int)SOUND_FREQ(1318.51*8),
  (int)SOUND_FREQ(1479.98*8),
  (int)SOUND_FREQ(1661.22*8),
  (int)SOUND_FREQ(1975.33*8),

  (int)SOUND_FREQ(2217.46*8),
  (int)SOUND_FREQ(2637.02*8)
};

unsigned int heartbeat = 0;
int axePhase = 0;
int axeNextFreq = -1;
int axeLastPan = 0;
int axeLastDuty = 0;
int axeNextDuty = 0;

void axe(int touchXpx, int touchYpx) {
  int tempoCounter = heartbeat % 7;

  /*
   * Horizontal (X) = pitch and panning
   * Vertical (Y) = duty cycle
   */
  if(touchXpx > 255)
    touchXpx = 255;
  if(touchXpx > 0) {
    axeNextFreq = touchXpx >> 3;
    axeNextDuty = (touchYpx / 48) << 24;
  } else {
    axeNextFreq = -1;
  }

  // If it's time to start a note, do so
  if(tempoCounter == 0) {
    if(axeNextFreq >= 0) {
      axePhase = 7;
      axeLastDuty = axeNextDuty;
      axeLastPan = SOUND_PAN(4 * axeNextFreq + 2);
      SCHANNEL_TIMER(10) = axeFreqs[axeNextFreq];
    }
  }

  // Update the note
  if(axePhase >= 0) {
    int vol = (axePhase * axePhase * 3) >> 1;
    SCHANNEL_CR(10) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | axeLastPan
                   | axeLastDuty | vol;
    --axePhase;
  }
}

/*
 * Plays a repeating drum track and bass track.
 */
void runSound(void) {
  static const unsigned char bassNotes[4] = {5, 6, 4, 5};
  unsigned int tempoCounter = heartbeat % (56 * 16);
  unsigned int bassNote = axeFreqs[(int)bassNotes[tempoCounter / (56 * 4)]];

  tempoCounter %= 56;


  if(tempoCounter >= 28 && tempoCounter < 36) {
    // play snare
    int snarePhase = ((43 - tempoCounter) * (43 - tempoCounter)) >> 3;

    SCHANNEL_CR(14) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | SOUND_PAN(72)
                   | snarePhase;
    SCHANNEL_TIMER(14) = SOUND_FREQ(10923);
  } else {
    // play hat
    int hatPhase = tempoCounter;
    if(hatPhase >= 36 && hatPhase < 42)
      hatPhase -= 36;
    if(hatPhase >= 28)
      hatPhase -= 28;
    if(hatPhase >= 14)
      hatPhase -= 14;
    else
      hatPhase *= 3;
    if(hatPhase >= 14)
      hatPhase = 0;
    else
      hatPhase = 14 - hatPhase;
    hatPhase = (hatPhase * hatPhase) >> 4;

    SCHANNEL_CR(14) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | SOUND_PAN(64)
                   | hatPhase;
    SCHANNEL_TIMER(14) = SOUND_FREQ(32768);
  }

  // play kick and bass
  {
    int kickPhase = tempoCounter;
    if(kickPhase >= 28)
      kickPhase -= 28;
    if(kickPhase >= 14) {
      // Play bass
      int kickVol = (28 - kickPhase) * 4;
      SCHANNEL_CR(9) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | SOUND_PAN(64)
                     | 0x03000000 | kickVol;
      SCHANNEL_TIMER(9) = bassNote;
    } else {
      // Play kick drum
      int kickVol = (kickPhase >= 8) ? 0 : 10 * (8 - kickPhase);
      SCHANNEL_CR(9) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | SOUND_PAN(64)
                     | 0x03000000 | kickVol;
      SCHANNEL_TIMER(9) = SOUND_FREQ(660 * 8) * (kickPhase + 4);
    }
  }
}

int vcount;
touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	static int lastbut = -1;
	
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;

	if (!( (but ^ lastbut) & (1<<6))) {
 
		tempPos = touchReadXY();

		if ( tempPos.x == 0 || tempPos.y == 0 ) {
			but |= (1 <<6);
			lastbut = but;
		} else {
			x = tempPos.x;
			y = tempPos.y;
			xpx = tempPos.px;
			ypx = tempPos.py;
			z1 = tempPos.z1;
			z2 = tempPos.z2;
		}
		
	} else {
		lastbut = but;
		but |= (1 <<6);
	}

	if ( vcount == 80 ) {
		first = tempPos;
	} else {
		if (	abs( xpx - first.px) > 10 || abs( ypx - first.py) > 10 ||
				(but & ( 1<<6)) ) {

			but |= (1 <<6);
			lastbut = but;

		} else { 	
			IPC->mailBusy = 1;
			IPC->touchX			= x;
			IPC->touchY			= y;
			IPC->touchXpx		= xpx;
			IPC->touchYpx		= ypx;
			IPC->touchZ1		= z1;
			IPC->touchZ2		= z2;
			IPC->mailBusy = 0;
		}
	}
	IPC->buttons		= but;
	vcount ^= (80 ^ 130);
	SetYtrigger(vcount);

}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------

	u32 i;

	//sound code  :)
    runSound();
    axe((IPC->buttons & 0x40) ? -1 : IPC->touchXpx, IPC->touchYpx);
    ++heartbeat;
}

//---------------------------------------------------------------------------------
int main(int argc, char ** argv) {
//---------------------------------------------------------------------------------

	// Reset the clock if needed
	rtcReset();

	//enable sound
	powerON(POWER_SOUND);
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
	IPC->soundData = 0;

	irqInit();
	irqSet(IRQ_VBLANK, VblankHandler);
	SetYtrigger(80);
	vcount = 80;
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

	// Keep the ARM7 idle
	while (1) swiWaitForVBlank();
}

//////////////////////////////////////////////////////////////////////
