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

#include "pcjoy.h"
#include <allegro.h>
#include <string.h>

extern const FONT *aver32, *aver16;
extern int bgColor, fgColor, hiliteColor;

static volatile int lastScancodePressed = -1;
static void (*oldKeyListener)(int scancode);
volatile int wantsClose = 0;
void ezPlaySample(const char *filename, int vol);

static void ljpcKeyListener(int scancode) {
  if (!(scancode & 0x80)) {
    lastScancodePressed = scancode;
  }
  if (oldKeyListener) {
    oldKeyListener(scancode);
  }
} END_OF_FUNCTION(ljpcKeyListener);


/**
 * Presses the escape key when the user clicks the close box.
 */
static void setWantsClose() {
  wantsClose = 1;
}

static int withJoystick = 0;

#define N_VKEYS 14

static const struct pkeyMapping defaultKeymappings[N_VKEYS] = {
  {-1, -1, KEY_UP},
  {-1, -1, KEY_DOWN},
  {-1, -1, KEY_LEFT},
  {-1, -1, KEY_RIGHT},
  {-1, -1, KEY_Z},
  {-1, -1, KEY_X},
  {-1, -1, KEY_S},
  {-1, -1, KEY_SPACE},
  {-1, -1, KEY_C},
  {-1, -1, KEY_W},
  {-1, -1, KEY_Q},
  {-1, -1, KEY_E},
  {-1, -1, KEY_ENTER},
  {-1, -1, KEY_D}
};

static struct pkeyMapping keymappings[N_VKEYS];

void getPkeyName(char *dst, int j, int s, int a) {
  if (j < 0) {
    usprintf(dst, "Key %d (%s)", a, scancode_to_name(a));
  } else if (s < 0) {
    usprintf(dst,
             "Joy %d button %s",
             j,
             joy[j].button[a].name);
  } else if (a < 0) {
    usprintf(dst,
             "Joy %d stick %s axis %s -",
             j,
             joy[j].stick[s].name,
             joy[j].stick[s].axis[~a].name);
  } else {
    usprintf(dst,
             "Joy %d stick %s axis %s +",
             j,
             joy[j].stick[s].name,
             joy[j].stick[s].axis[a].name);
  }
}

static int getPkeyState(int j, int s, int a) {
  int k;
  if (j < 0) {
    k = key[a];
  } else if (s < 0) {
    k = joy[j].button[a].b;
  } else if (a < 0) {
    k = joy[j].stick[s].axis[~a].d1;
  } else {
    k = joy[j].stick[s].axis[a].d2;
  }
  return k;
}

const char *const vkeyNames[] = {
  "Up",
  "Down",
  "Left",
  "Right",
  "Rotate Left",
  "Rotate Right",
  "Hold",
  "Item",
  "Alt. Rotate Left",
  "Rotate Left Twice",
  "Far Left",
  "Far Right",
  "Alt. Firm Drop",
  "Alt. Hold",
  "Macro G",
  "Macro H"
};

static int getVkeyState(int vkey) {
  int j = keymappings[vkey].joy;
  int s = keymappings[vkey].stick;
  int a = keymappings[vkey].axis;

  return getPkeyState(j, s, a);
}

LJBits readPad(void) {
  int keys = 0;
  poll_joystick();

  for (int i = 0;
       i < N_VKEYS;
       ++i) {
    if (getVkeyState(i)) {
      keys |= 1 << i;
    }
  }
  return keys;
}

LJBits menuReadPad(void) {
  if (key[KEY_ENTER]) {
    return VKEY_ROTR;
  } else if (key[KEY_ESC]) {
    return VKEY_ROTL;
  } else if (key[KEY_UP]) {
    return VKEY_UP;
  } else if (key[KEY_DOWN]) {
    return VKEY_DOWN;
  } else if (key[KEY_LEFT]) {
    return VKEY_LEFT;
  } else if (key[KEY_RIGHT]) {
    return VKEY_RIGHT;
  } else {
    return readPad();
  }
}

// These contain the states of ALL buttons on ALL
// joysticks 
static LJBits lastConfigButtons[8];
static LJBits lastConfigStickAxis[8];

static int newButton(int *outJ, int *outS, int *outA) {
  poll_joystick();
  int found = 0;

  if (lastScancodePressed >= 0) {
    *outJ = -1;
    *outS = -1;
    *outA = lastScancodePressed;
    lastScancodePressed = -1;
    return 1;
  }

  for (int j = 0;
       j < num_joysticks;
       ++j) {
    LJBits cur = 0;

    for (int b = 0; b < joy[j].num_buttons; ++b) {
      if (joy[j].button[b].b) {
        if (!(lastConfigButtons[j] & (1 << b)) && !found) {
          *outJ = j;
          *outS = -1;
          *outA = b;
          found = 1;
        }
        cur |= 1 << b;
      }
    }
    lastConfigButtons[j] = cur;
  }
  if (found) {
    return 1;
  }

  for (int j = 0;
       j < num_joysticks;
       ++j) {
    LJBits cur = 0;
    LJBits mask = 1;

    for (int s = 0; s < joy[j].num_sticks; ++s) {
      for (int a = 0; a < joy[j].stick[s].num_axis; ++a) {
        if (joy[j].stick[s].axis[a].d1) {
          if (!(lastConfigStickAxis[j] & mask) && !found) {
            *outJ = j;
            *outS = s;
            *outA = ~a;
            found = 1;
          }
          cur |= mask;
        }
        mask <<= 1;
        if (joy[j].stick[s].axis[a].d2) {
          if (!(lastConfigStickAxis[j] & mask) && !found) {
            *outJ = j;
            *outS = s;
            *outA = a;
            found = 1;
          }
          cur |= mask;
        }
        mask <<= 1;
      }
    }
    lastConfigStickAxis[j] = cur;
  }
  return found;
}

static void clearNewButton(void) {
  int j, s, a;
  while (newButton(&j, &s, &a));
}

void loadKeys(const char *filename) {
  PACKFILE *fp = pack_fopen(filename, "r");

  memcpy(keymappings, defaultKeymappings, sizeof(keymappings));
  if (fp) {
    for (unsigned int vkey = 0;
         vkey < N_VKEYS && !pack_feof(fp);
         ++vkey) {
      int j = pack_getc(fp);
      int s = pack_getc(fp);
      int a = pack_getc(fp);

      keymappings[vkey].joy = j;
      keymappings[vkey].stick = s;
      keymappings[vkey].axis = a;
    }
    pack_fclose(fp);
  }
}

void saveKeys(const char *filename) {
  PACKFILE *fp = pack_fopen(filename, "w");

  if (fp) {
    for (unsigned int vkey = 0;
         vkey < N_VKEYS && !pack_feof(fp);
         ++vkey) {
      pack_putc(keymappings[vkey].joy, fp);
      pack_putc(keymappings[vkey].stick, fp);
      pack_putc(keymappings[vkey].axis, fp);
    }
    pack_fclose(fp);
  }
}

#define VKEY_ROWHT 24
#define VKEY_TOP 120

void drawVkeyRow(int vkey, int hilite) {
  char name[256];
  int y = VKEY_TOP + vkey * VKEY_ROWHT;
  
  rectfill(screen,
           16, y, 479, y + VKEY_ROWHT - 1,
           hilite ? hiliteColor : bgColor);
  if (hilite) {
    rect(screen,
         16, y, 479, y + VKEY_ROWHT - 1,
         0);
  }
  getPkeyName(name,
              keymappings[vkey].joy,
              keymappings[vkey].stick,
              keymappings[vkey].axis);
  textout_ex(screen, aver16, vkeyNames[vkey], 24, y + 4, fgColor, -1);
  textout_ex(screen, aver16, name, 240, y + 4, fgColor, -1);
}

void configureKeys(void) {
  clear_to_color(screen, bgColor);
  textout_ex(screen, aver32, "LOCKJAW > Game Keys", 16, 32, fgColor, -1);
  textout_ex(screen, aver16,
             "Press the key that you want to use for the highlighted action, or Esc to return.",
             40, 80, fgColor, -1);

  // Draw each vkey's name
  for (int vkey = 0; vkey < N_VKEYS; ++vkey) {
    drawVkeyRow(vkey, vkey == 0);
  };

  clearNewButton();
  for (int vkey = 0; vkey < N_VKEYS && !wantsClose; ) {
    int j, s, a;

    if (keypressed()) {
      int scancode;
      ureadkey(&scancode);
      if (scancode == KEY_ESC) {
        vkey = N_VKEYS;
      }
    }
    if (vkey < N_VKEYS && newButton(&j, &s, &a)) {
      if (j >= 0 || s > 0 || a != KEY_ESC) {
        keymappings[vkey].joy = j;
        keymappings[vkey].stick = s;
        keymappings[vkey].axis = a;
        ezPlaySample("nextS_wav", 128);
      }

      // Redraw old vkey
      drawVkeyRow(vkey, 0);
      rest(250);

      // Draw new vkey
      ++vkey;
      if (vkey < N_VKEYS) {
        drawVkeyRow(vkey, 1);
      }
      
      // Ignore all other buttons pressed at the same time
      clearNewButton();
    }
    rest(30);
  }
  saveKeys("lj-keys.005");
}


void initKeys(void) {
  LOCK_FUNCTION(ljpcKeyListener);
  LOCK_VARIABLE(lastScancodePressed);
  LOCK_VARIABLE(oldKeyListener);
  LOCK_FUNCTION(setWantsClose);
  LOCK_VARIABLE(wantsClose);

  oldKeyListener = keyboard_lowlevel_callback;
  keyboard_lowlevel_callback = ljpcKeyListener;
  withJoystick = !install_joystick(JOY_TYPE_AUTODETECT);
  loadKeys("lj-keys.005");
  set_close_button_callback(setWantsClose);
}


