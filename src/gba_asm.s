@ fixed fastfmul(fixed x, fixed y)
@ Multiply two 16.16 fixed-point numbers.

.ARM
.ALIGN
.GLOBL  fastfmul

fastfmul:
  smull r1,r2,r0,r1
  mov r0,r2,LSL#16
  orr r0,r0,r1,LSR#16
  bx lr


@ int dv(int num, int den)
@ Divide two signed integers.

.THUMB
.THUMB_FUNC
.ALIGN
.GLOBL  dv

dv:
  cmp r1, #0
  beq 0f
  swi 6
  bx lr
0:
  ldr r0, =0x7fffffff
  bx lr


@ int fracmul(signed int x, signed int frac)
@ Multiply by a 0.32 fractional number between -0.5 and 0.5.
@ Used for fast division by a constant.

.ARM
.ALIGN
.GLOBL  fracmul

fracmul:
  smull r1,r2,r0,r1
  mov r0, r2
  bx lr


@ int fracumul(unsigned int x, unsigned int frac)
@ Multiply by a 0.32 fractional number between 0 and 1.
@ Used for fast division by a constant.

.ARM
.ALIGN
.GLOBL  fracumul

fracumul:
  umull r1,r2,r0,r1
  mov r0, r2
  bx lr


@ void gblz_unpack(const void *src, void *dst)
@ Unpack GB LZSS format data.

.THUMB
.THUMB_FUNC
.ALIGN
.GLOBL  _gblz_unpack

_gblz_unpack:
  swi 0x11  @ LZ77UnCompWRAM
  bx lr


@ void vsync(void)
@ Synchronize to vblank, assuming that the
@ installed ISR updates BIOS_IF.

.THUMB
.THUMB_FUNC
.ALIGN
.GLOBL  vsync

vsync:
  swi 0x05  @ VBlankIntrWait
  bx lr

