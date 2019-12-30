/* PC joystick code

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

#ifndef PCJOY_H
#define PCJOY_H

#include "ljtypes.h"
#include "ljcontrol.h"


/* Physical key definitions

m.joy < 0:
  key[m.axis]
m.joy >= 0, m.stick < 0:
  joy[m.joy].button[m.axis].b
m.joy >= 0, m.stick >= 0, m.axis < 0:
  joy[m.joy].stick[m.stick].axis[~m.axis].d1
m.joy >= 0, m.stick >= 0, m.axis >= 0:
  joy[m.joy].stick[m.stick].axis[m.axis].d2
*/

struct pkeyMapping {
  signed char joy;
  signed char stick;
  signed char axis;
};

extern volatile int wantsClose;

void initKeys(void);
/**
 * Reads the physical keys to produce a set of pressed virtual keys.
 */
LJBits readPad(void);
/**
 * Reads the physical keys to produce a set of pressed virtual keys,
 * hardcoding keyboard Up, Down, Left, Right, Esc, and Enter
 * to the appropriate vkeys.
 */
LJBits menuReadPad(void);
void getPkeyName(char *dst, int j, int s, int a);
extern const char *const vkeyNames[];
void configureKeys(void);

#endif
