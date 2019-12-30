#include "ljgba.h"
#define BIOS_IF (*(volatile unsigned short *)0x03FFFFF8)

volatile int curTime = 0;

void isr(void) {
  unsigned int interrupts = REG_IF;

  if (interrupts & IRQ_VBLANK) {
    ++curTime;
  }

  BIOS_IF |= interrupts;
  REG_IF = interrupts;
}
