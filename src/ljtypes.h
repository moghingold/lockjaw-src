/* Basic data types for LOCKJAW, an implementation of the Soviet Mind Game

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

#ifndef LJTYPES_H
#define LJTYPES_H

/**
 * A 16.16 signed number. Used commonly for Y piece positions.
 */
typedef signed int LJFixed;

static inline signed int ljfixfloor(LJFixed f) __attribute__((const)); 

static inline signed int ljfixfloor(LJFixed f) {
  return f >> 16;
}

static inline LJFixed ljitofix(signed int f) __attribute__((const)); 

static inline LJFixed ljitofix(signed int f) {
  return f << 16;
}

/*
 * In most cases, macros are deprecated in favor of static inline functions
 * because the latter allow the compiler to perform more type checks.
 * However, the C language forbids calling a function in an inline
 * constructor, even if it is a static inline function with no side effects.
 * For example, GCC gives the misleading error message
 * "error: initializer element is not constant".
 * Therefore, I have to provide a second implementation of ljitofix()
 * as a macro for use in global variable initializers.
 */
#define LJITOFIX(f) ((LJFixed)((f) << 16))


typedef unsigned int LJBits;

#endif

