/**************************************************************************
 * eon3dx.c -- Eon3D is a simplistic 3D software renderer.                *
 * (C) 2010-2013 Francesco Romani <fromani at gmail dot com>              *
 *                                                                        *
 * derived from                                                           *
 *                                                                        *
 * PLUSH 3D VERSION 1.2                                                   *
 * Copyright (c) 1996-2000 Justin Frankel <justin at nullsoft dot com>    *
 * Copyright (c) 1998-2000 Nullsoft, Inc.                                 *
 * http://www.nullsoft.com/plush                                          *
 *                                                                        *
 * This software is provided 'as-is', without any express or implied      *
 * warranty.  In no event will the authors be held liable for any damages *
 * arising from the use of this software.                                 *
 *                                                                        *
 * Permission is granted to anyone to use this software for any purpose,  *
 * including commercial applications, and to alter it and redistribute it *
 * freely, subject to the following restrictions:                         *
 *                                                                        *
 * 1. The origin of this software must not be misrepresented; you must    *
 *    not claim that you wrote the original software. If you use this     *
 *    software in a product, an acknowledgment in the product             *
 *    documentation would be appreciated but is not required.             *
 * 2. Altered source versions must be plainly marked as such, and must    *
 *    not be misrepresented as being the original software.               *
 * 3. This notice may not be removed or altered from any source           *
 *    distribution.                                                       *
 *                                                                        *
 * Meaning: all good stuff is credited to the plush author(s).            *
 * All the bugs, misdesigns and pessimizations are credited to me. ;)     *
 *                                                                        *
 **************************************************************************/

#include "eon3dx.h"


// plush.c
//

EON_uChar EON_Text_DefaultFont[256*16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 60, 66, 129, 231, 165, 153, 129, 153, 66, 60, 0, 0, 0, 0,
  0, 0, 60, 126, 255, 153, 219, 231, 255, 231, 126, 60, 0, 0, 0, 0,
  0, 0, 0, 0, 102, 255, 255, 255, 255, 126, 60, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 60, 126, 255, 126, 60, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 24, 60, 60, 90, 255, 255, 90, 24, 60, 0, 0, 0, 0,
  0, 0, 0, 24, 60, 126, 255, 255, 255, 90, 24, 60, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 24, 60, 60, 24, 0, 0, 0, 0, 0, 0,
  255,255,255, 255, 255, 255, 231, 195, 195, 231, 255, 255, 255, 255, 255, 255,
  0, 0, 0, 0, 0, 60, 102, 66, 66, 102, 60, 0, 0, 0, 0, 0,
  255,255,255, 255, 255, 195, 153, 189, 189, 153, 195, 255, 255, 255, 255, 255,
  0, 0, 15, 7, 13, 24, 62, 99, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 126, 195, 195, 195, 126, 24, 24, 30, 120, 24, 0, 0, 0, 0,
  0, 0, 8, 12, 14, 11, 8, 8, 8, 120, 248, 112, 0, 0, 0, 0,
  0, 16, 24, 28, 22, 26, 22, 18, 114, 242, 98, 14, 30, 12, 0, 0,
  0, 0, 24, 24, 219, 126, 60, 255, 60, 126, 219, 24, 24, 0, 0, 0,
  0, 0, 96, 112, 120, 124, 126, 126, 124, 120, 112, 96, 0, 0, 0, 0,
  0, 0, 6, 14, 30, 62, 126, 126, 62, 30, 14, 6, 0, 0, 0, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 56, 254, 124, 56, 16, 0, 0,
  0, 0, 102, 102, 102, 102, 102, 102, 102, 0, 102, 102, 0, 0, 0, 0,
  0, 0, 63, 123, 219, 219, 219, 127, 59, 27, 27, 27, 0, 0, 0, 0,
  0, 31, 48, 120, 220, 206, 231, 115, 59, 30, 12, 24, 48, 224, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 126, 126, 126, 126, 0, 0, 0, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 254, 124, 56, 16, 0, 254, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 56, 56, 56, 56, 56, 0, 0,
  0, 0, 56, 56, 56, 56, 56, 56, 56, 56, 254, 124, 56, 16, 0, 0,
  0, 0, 0, 0, 0, 8, 12, 254, 255, 254, 12, 8, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 16, 48, 127, 255, 127, 48, 16, 0, 0, 0, 0,
  0, 0, 204, 102, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 36, 102, 255, 255, 102, 36, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 60, 60, 126, 126, 255, 255, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 255, 255, 126, 126, 60, 60, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 24, 24, 24, 24, 24, 24, 24, 0, 24, 24, 0, 0, 0, 0,
  0, 0, 51, 102, 204, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 51, 51, 255, 102, 102, 102, 102, 255, 204, 204, 0, 0, 0, 0,
  0, 24, 126, 219, 216, 120, 28, 30, 27, 219, 219, 126, 24, 0, 0, 0,
  0, 0, 96, 209, 179, 102, 12, 24, 54, 109, 203, 6, 0, 0, 0, 0,
  0, 0, 28, 54, 102, 60, 56, 108, 199, 198, 110, 59, 0, 0, 0, 0,
  0, 0, 12, 24, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 12, 24, 48, 48, 96, 96, 96, 96, 48, 48, 24, 12, 0, 0, 0,
  0, 48, 24, 12, 12, 6, 6, 6, 6, 12, 12, 24, 48, 0, 0, 0,
  0, 0, 102, 60, 255, 60, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 24, 48, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0,
  0, 0, 6, 4, 12, 8, 24, 16, 48, 32, 96, 64, 192, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 207, 219, 243, 198, 124, 0, 0, 0, 0,
  0, 0, 12, 28, 60, 108, 12, 12, 12, 12, 12, 12, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 3, 6, 12, 24, 48, 99, 255, 0, 0, 0, 0,
  0, 0, 255, 198, 12, 24, 62, 3, 3, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 6, 14, 30, 54, 102, 199, 222, 246, 6, 6, 0, 0, 0, 0,
  0, 0, 31, 240, 192, 220, 246, 3, 3, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 192, 220, 246, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 255, 195, 6, 12, 12, 24, 24, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 108, 62, 99, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 198, 222, 118, 6, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 48, 0, 0, 0, 0,
  0, 0, 6, 12, 24, 48, 96, 96, 48, 24, 12, 6, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 96, 48, 24, 12, 6, 6, 12, 24, 48, 96, 0, 0, 0, 0,
  0, 0, 62, 99, 198, 12, 24, 48, 48, 0, 48, 48, 0, 0, 0, 0,
  0, 0, 30, 51, 111, 219, 219, 219, 222, 216, 198, 220, 112, 0, 0, 0,
  0, 0, 24, 24, 60, 36, 102, 110, 122, 227, 195, 195, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 198, 220, 247, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 30, 51, 96, 192, 192, 192, 192, 192, 99, 62, 0, 0, 0, 0,
  0, 0, 252, 198, 195, 195, 195, 195, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 30, 240, 192, 192, 220, 240, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 30, 240, 192, 192, 220, 240, 192, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 62, 99, 192, 192, 192, 207, 195, 195, 102, 60, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 207, 251, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 28, 56, 24, 24, 24, 24, 24, 24, 28, 56, 0, 0, 0, 0,
  0, 0, 3, 3, 3, 3, 3, 3, 195, 195, 99, 62, 0, 0, 0, 0,
  0, 0, 195, 198, 220, 240, 224, 240, 216, 204, 198, 195, 0, 0, 0, 0,
  0, 0, 192, 192, 192, 192, 192, 192, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 195, 195, 231, 239, 251, 211, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 227, 243, 211, 219, 207, 199, 195, 195, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 220, 240, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 195, 243, 222, 204, 124, 6, 3, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 252, 216, 204, 198, 195, 0, 0, 0, 0,
  0, 0, 126, 195, 192, 112, 28, 6, 3, 195, 195, 126, 0, 0, 0, 0,
  0, 0, 15, 248, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 195, 195, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 102, 102, 124, 56, 48, 48, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 219, 219, 219, 255, 231, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 60, 102, 102, 195, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 31, 246, 4, 12, 24, 16, 48, 32, 111, 248, 0, 0, 0, 0,
  0, 62, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 62, 0, 0, 0,
  0, 0, 192, 64, 96, 32, 48, 16, 24, 8, 12, 4, 6, 0, 0, 0,
  0, 124, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 124, 0, 0, 0,
  0, 24, 60, 102, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
  0, 0, 48, 24, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 192, 192, 192, 220, 246, 195, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 30, 51, 96, 192, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 3, 3, 3, 31, 115, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 30, 51, 48, 48, 60, 240, 48, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 63, 99, 195, 199, 207, 219, 115, 3, 195, 126, 0,
  0, 0, 192, 192, 192, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 12, 12, 0, 12, 12, 12, 12, 12, 12, 12, 204, 204, 120, 0,
  0, 0, 192, 192, 192, 198, 204, 216, 248, 236, 198, 195, 0, 0, 0, 0,
  0, 0, 56, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 230, 219, 219, 219, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 227, 195, 198, 252, 192, 192, 192, 0,
  0, 0, 0, 0, 0, 115, 219, 207, 199, 195, 99, 63, 3, 3, 3, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 224, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 126, 195, 112, 30, 3, 195, 126, 0, 0, 0, 0,
  0, 0, 16, 48, 48, 60, 240, 48, 48, 54, 60, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 102, 108, 56, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 219, 219, 255, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 102, 60, 24, 60, 102, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 195, 195, 102, 62, 12, 216, 112, 0,
  0, 0, 0, 0, 0, 255, 6, 12, 24, 48, 96, 255, 0, 0, 0, 0,
  0, 14, 24, 24, 24, 24, 112, 112, 24, 24, 24, 24, 14, 0, 0, 0,
  0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0,
  0, 112, 24, 24, 24, 24, 14, 14, 24, 24, 24, 24, 112, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 118, 220, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 16, 56, 108, 198, 198, 198, 254, 0, 0, 0, 0, 0,
  0, 0, 30, 51, 96, 192, 192, 192, 192, 192, 99, 62, 12, 24, 240, 0,
  0, 0, 102, 102, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  12, 30, 51, 96, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 54, 54, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 48, 24, 12, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  28, 54, 54, 28, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 30, 51, 96, 192, 192, 195, 126, 12, 24, 240, 0,
  24, 60, 102, 192, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  102, 102, 24, 24, 60, 36, 102, 102, 126, 195, 195, 195, 0, 0, 0, 0,
  24, 36, 36, 24, 60, 36, 102, 102, 126, 195, 195, 195, 0, 0, 0, 0,
  24, 48, 96, 30, 240, 192, 222, 240, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 126, 219, 59, 126, 220, 219, 110, 0, 0, 0, 0,
  0, 0, 63, 62, 60, 108, 111, 110, 124, 204, 207, 206, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 195, 195, 195, 195, 195, 102, 62, 12, 216, 112, 0,
  102, 0, 62, 99, 195, 195, 195, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  102, 0, 195, 195, 195, 195, 195, 195, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 8, 8, 30, 59, 104, 200, 200, 203, 126, 8, 8, 0, 0,
  0, 0, 60, 102, 96, 248, 96, 248, 96, 96, 99, 126, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 24, 126, 24, 126, 24, 24, 0, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 220, 248, 204, 222, 204, 15, 14, 4, 0,
  0, 0, 30, 51, 48, 48, 60, 240, 48, 48, 48, 48, 224, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 12, 24, 48, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 3, 118, 220, 0, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  3, 118, 220, 0, 195, 227, 243, 211, 219, 207, 199, 195, 0, 0, 0, 0,
  0, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 255, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 255, 0, 0, 0, 0,
  0, 0, 12, 12, 0, 12, 12, 24, 48, 99, 198, 124, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 254, 192, 192, 192, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 254, 6, 6, 6, 0, 0, 0, 0, 0, 0,
  0, 96, 225, 99, 102, 108, 24, 48, 96, 206, 155, 6, 13, 31, 0, 0,
  0, 96, 225, 99, 102, 108, 24, 48, 102, 206, 151, 62, 6, 6, 0, 0,
  0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 3, 54, 108, 216, 216, 108, 54, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 108, 54, 27, 27, 54, 108, 192, 0, 0, 0, 0,
  130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16,
  0, 149, 0, 169, 0, 149, 0, 169, 0, 149, 0, 169, 0, 149, 0, 169,
  146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 248, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 248, 248, 248, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 60, 60, 252, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 0, 0, 252, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 248, 248, 248, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 252, 252, 252, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 252, 252, 252, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 252, 252, 252, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 60, 60, 252, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 248, 248, 248, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 248, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 31, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 24, 24, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 31, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 24, 24, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 31, 31, 31, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 60, 60, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 63, 63, 63, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 63, 63, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 63, 63, 63, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 255, 255, 255, 60, 60, 60, 60, 60, 60, 60,
  24, 24, 24, 24, 24, 24, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 60, 60, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 63, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 31, 31, 31, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 31, 31, 31, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 255, 60, 60, 60, 60, 60, 60, 60,
  24, 24, 24, 24, 24, 24, 255, 255, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 248, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 31, 24, 24, 24, 24, 24, 24, 24,
  255, 255, 255,255,255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255,
  240, 240, 240, 240,240,240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 118, 204, 204, 204, 222, 115, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 194, 204, 194, 195, 195, 206, 216, 192, 192, 0, 0,
  0, 0, 31, 243, 195, 192, 192, 192, 192, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 0, 0, 3, 126, 230, 102, 102, 102, 102, 68, 0, 0, 0, 0,
  0, 0, 31, 240, 96, 48, 24, 48, 96, 192, 223, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 127, 240, 216, 216, 216, 216, 112, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 99, 99, 99, 99, 103, 111, 123, 96, 96, 192, 0,
  0, 0, 0, 0, 111, 184, 48, 48, 48, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 24, 24, 126, 219, 219, 219, 219, 126, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 60, 102, 195, 195, 255, 195, 195, 102, 60, 0, 0, 0, 0,
  0, 0, 60, 102, 195, 195, 195, 195, 102, 36, 165, 231, 0, 0, 0, 0,
  0, 7, 28, 48, 24, 12, 62, 102, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 110, 219, 219, 219, 118, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 2, 4, 124, 206, 214, 230, 124, 64, 128, 0, 0, 0, 0,
  0, 0, 30, 48, 96, 96, 126, 96, 96, 96, 48, 30, 0, 0, 0, 0,
  0, 0, 0, 126, 195, 195, 195, 195, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 48, 24, 12, 6, 12, 24, 48, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 12, 24, 48, 96, 48, 24, 12, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 14, 27, 27, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 216, 216, 112, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 255, 0, 24, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 118, 220, 0, 118, 220, 0, 0, 0, 0, 0, 0,
  0, 0, 60, 102, 102, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 3, 2, 6, 4, 12, 8, 216, 80, 112, 32, 0, 0, 0, 0,
  0, 0, 220, 246, 230, 198, 198, 198, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 120, 204, 24, 48, 100, 252, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 126, 126, 126, 126, 126, 126, 126, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// text.c
//

static EON_uChar font_height = 16;

static EON_uChar *current_font = EON_Text_DefaultFont;

void EON_TextSetFont(EON_uChar *font, EON_uChar height)
{
  current_font = font;
  font_height = height;
}

void EON_TextPutChar(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                   EON_uChar color, EON_uChar c)
{
  EON_uChar *font = current_font + (c*font_height);
  EON_sInt offset = x+(y*cam->ScreenWidth);
  EON_ZBuffer zz = (EON_ZBuffer) (1.0/z);
  EON_sInt xx = x, a;
  EON_uChar len = font_height;
  EON_uChar ch;
  EON_uChar *outmem;
  EON_ZBuffer *zbuffer;
  if (y+font_height < cam->ClipTop || y >= cam->ClipBottom) return;
  if (y < cam->ClipTop) {
    font += (cam->ClipTop-y);
    offset += (cam->ClipTop-y)*cam->ScreenWidth;
    len -= (cam->ClipTop-y);
    y = cam->ClipTop;
  }
  if (y+font_height >= cam->ClipBottom) {
    len = cam->ClipBottom-y;
  }
  if (len > 0) {
    if (cam->zBuffer && z != 0.0) do {
      outmem = cam->frameBuffer + offset;
      zbuffer = cam->zBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft)
          if (ch & a)
            if (zz > *zbuffer) {
              *zbuffer = zz;
              *outmem = color;
            }
        zbuffer++;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
    else do {
      outmem = cam->frameBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft) if (ch & a) *outmem = color;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
  }
}

void EON_TextPutStr(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                    EON_uChar color, EON_sChar *string)
{
  EON_sInt xx = x;
  while (*string) {
    switch (*string) {
      case '\n': y += font_height; xx = x; break;
      case ' ': xx += 8; break;
      case '\r': break;
      case '\t': xx += 8*5; break;
      default:
        EON_TextPutChar(cam,xx,y,z,color,(EON_uChar) *string);
        xx += 8;
      break;
    }
    string++;
  }
}

void EON_TextPrintf(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                    EON_uChar color, EON_sChar *format, ...)
{
  va_list arglist;
  EON_sChar str[256];
  va_start(arglist, format);
  vsprintf((char *)str, (char *) format,arglist);
  va_end(arglist);
  EON_TextPutStr(cam,x,y,z,color,str);
}

// make.c
//

EON_Obj *EON_MakeTorus(EON_Float r1, EON_Float r2, EON_uInt divrot, EON_uInt divrad,
                    EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  double ravg, rt, a, da, al, dal;
  EON_sInt32 U,V,dU,dV;
  if (divrot < 3) divrot = 3;
  if (divrad < 3) divrad = 3;
  ravg = (r1+r2)*0.5;
  rt = (r2-r1)*0.5;
  o = EON_ObjCreate(divrad*divrot,divrad*divrot*2);
  if (!o) return 0;
  v = o->Vertices;
  a = 0.0;
  da = 2*EON_PI/divrot;
  for (y = 0; y < divrot; y ++) {
    al = 0.0;
    dal = 2*EON_PI/divrad;
    for (x = 0; x < divrad; x ++) {
      v->x = (EON_Float) (cos((double) a)*(ravg + cos((double) al)*rt));
      v->z = (EON_Float) (sin((double) a)*(ravg + cos((double) al)*rt));
      v->y = (EON_Float) (sin((double) al)*rt);
      v++;
      al += dal;
    }
    a += da;
  }
  v = o->Vertices;
  f = o->Faces;
  dV = 65535/divrad;
  dU = 65535/divrot;
  U = 0;
  for (y = 0; y < divrot; y ++) {
    V = -32768;
    for (x = 0; x < divrad; x ++) {
      f->Vertices[0] = v+x+y*divrad;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V;
      f->Material = m;
      f++;
      f->Vertices[0] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[0] = U+dU;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+(x+1==divrad?0:x+1)+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m;
      f++;
      V += dV;
    }
    U += dU;
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeSphere(EON_Float r, EON_uInt divr, EON_uInt divh, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  double a, da, yp, ya, yda, yf;
  EON_sInt32 U,V,dU,dV;
  if (divh < 3) divh = 3;
  if (divr < 3) divr = 3;
  o = EON_ObjCreate(2+(divh-2)*(divr),2*divr+(divh-3)*divr*2);
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0.0; v->y = r; v++;
  v->x = v->z = 0.0; v->y = -r; v++;
  ya = 0.0;
  yda = EON_PI/(divh-1);
  da = (EON_PI*2.0)/divr;
  for (y = 0; y < divh - 2; y ++) {
    ya += yda;
    yp = cos((double) ya)*r;
    yf = sin((double) ya)*r;
    a = 0.0;
    for (x = 0; x < divr; x ++) {
      v->y = (EON_Float) yp;
      v->x = (EON_Float) (cos((double) a)*yf);
      v->z = (EON_Float) (sin((double) a)*yf);
      v++;
      a += da;
    }
  }
  f = o->Faces;
  v = o->Vertices + 2;
  a = 0.0;
  U = 0;
  dU = 65535/divr;
  dV = V = 65535/divh;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = v + (x+1==divr ? 0 : x+1);
    f->Vertices[2] = v + x;
    f->MappingU[0] = U;
    f->MappingV[0] = 0;
    f->MappingU[1] = U+dU;
    f->MappingV[1] = V;
    f->MappingU[2] = U;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  da = 1.0/(divr+1);
  v = o->Vertices + 2;
  for (x = 0; x < (divh-3); x ++) {
    U = 0;
    for (y = 0; y < divr; y ++) {
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+divr+(y+1==divr?0:y+1);
      f->Vertices[2] = v+y+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V+dV;
      f->MappingU[2] = U;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+(y+1==divr?0:y+1);
      f->Vertices[2] = v+(y+1==divr?0:y+1)+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V;
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      U += dU;
    }
    V += dV;
    v += divr;
  }
  v = o->Vertices + o->NumVertices - divr;
  U = 0;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices + 1;
    f->Vertices[1] = v + x;
    f->Vertices[2] = v + (x+1==divr ? 0 : x+1);
    f->MappingU[0] = U;
    f->MappingV[0] = 65535;
    f->MappingU[1] = U;
    f->MappingV[1] = V;
    f->MappingU[2] = U+dU;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeCylinder(EON_Float r, EON_Float h, EON_uInt divr, EON_Bool captop,
                          EON_Bool capbottom, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v, *topverts, *bottomverts, *topcapvert=0, *bottomcapvert=0;
  EON_Face *f;
  EON_uInt32 i;
  double a, da;
  if (divr < 3) divr = 3;
  o = EON_ObjCreate(divr*2+((divr==3)?0:(captop?1:0)+(capbottom?1:0)),
                  divr*2+(divr==3 ? (captop ? 1 : 0) + (capbottom ? 1 : 0) :
                  (captop ? divr : 0) + (capbottom ? divr : 0)));
  if (!o) return 0;
  a = 0.0;
  da = (2.0*EON_PI)/divr;
  v = o->Vertices;
  topverts = v;
  for (i = 0; i < divr; i ++) {
    v->y = h/2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float)(r*sin(a));
    v->xformedx = (EON_Float) (32768.0 + (32768.0*cos((double) a))); // temp
    v->xformedy = (EON_Float) (32768.0 + (32768.0*sin((double) a))); // use xf
    v++;
    a += da;
  }
  bottomverts = v;
  a = 0.0;
  for (i = 0; i < divr; i ++) {
    v->y = -h/2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float) (r*sin(a));
    v->xformedx = (EON_Float) (32768.0 + (32768.0*cos((double) a)));
    v->xformedy = (EON_Float) (32768.0 + (32768.0*sin((double) a)));
    v++; a += da;
  }
  if (captop && divr != 3) {
    topcapvert = v;
    v->y = h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  if (capbottom && divr != 3) {
    bottomcapvert = v;
    v->y = -h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  f = o->Faces;
  for (i = 0; i < divr; i ++) {
    f->Vertices[0] = bottomverts + i;
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[0] = f->MappingV[2] = 65535; f->MappingV[1] = 0;
    f->MappingU[0] = f->MappingU[1] = (i<<16)/divr;
    f->MappingU[2] = ((i+1)<<16)/divr;
    f->Material = m; f++;
    f->Vertices[0] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = topverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[1] = f->MappingV[2] = 0; f->MappingV[0] = 65535;
    f->MappingU[0] = f->MappingU[2] = ((i+1)<<16)/divr;
    f->MappingU[1] = (i<<16)/divr;
    f->Material = m; f++;
  }
  if (captop) {
    if (divr == 3) {
      f->Vertices[0] = topverts + 0;
      f->Vertices[1] = topverts + 2;
      f->Vertices[2] = topverts + 1;
      f->MappingU[0] = (EON_sInt32) topverts[0].xformedx;
      f->MappingV[0] = (EON_sInt32) topverts[0].xformedy;
      f->MappingU[1] = (EON_sInt32) topverts[1].xformedx;
      f->MappingV[1] = (EON_sInt32) topverts[1].xformedy;
      f->MappingU[2] = (EON_sInt32) topverts[2].xformedx;
      f->MappingV[2] = (EON_sInt32) topverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = topverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[1] = topverts + i;
        f->Vertices[2] = topcapvert;
        f->MappingU[0] = (EON_sInt32) topverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[0] = (EON_sInt32) topverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[1] = (EON_sInt32) topverts[i].xformedx;
        f->MappingV[1] = (EON_sInt32) topverts[i].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  if (capbottom) {
    if (divr == 3) {
      f->Vertices[0] = bottomverts + 0;
      f->Vertices[1] = bottomverts + 1;
      f->Vertices[2] = bottomverts + 2;
      f->MappingU[0] = (EON_sInt32) bottomverts[0].xformedx;
      f->MappingV[0] = (EON_sInt32) bottomverts[0].xformedy;
      f->MappingU[1] = (EON_sInt32) bottomverts[1].xformedx;
      f->MappingV[1] = (EON_sInt32) bottomverts[1].xformedy;
      f->MappingU[2] = (EON_sInt32) bottomverts[2].xformedx;
      f->MappingV[2] = (EON_sInt32) bottomverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = bottomverts + i;
        f->Vertices[1] = bottomverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[2] = bottomcapvert;
        f->MappingU[0] = (EON_sInt32) bottomverts[i].xformedx;
        f->MappingV[0] = (EON_sInt32) bottomverts[i].xformedy;
        f->MappingU[1] = (EON_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[1] = (EON_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeCone(EON_Float r, EON_Float h, EON_uInt div,
                      EON_Bool cap, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt32 i;
  double a, da;
  if (div < 3) div = 3;
  o = EON_ObjCreate(div + (div == 3 ? 1 : (cap ? 2 : 1)),
                  div + (div == 3 ? 1 : (cap ? div : 0)));
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0; v->y = h/2;
  v->xformedx = 1<<15;
  v->xformedy = 1<<15;
  v++;
  a = 0.0;
  da = (2.0*EON_PI)/div;
  for (i = 1; i <= div; i ++) {
    v->y = h/-2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float) (r*sin((double) a));
    v->xformedx = (EON_Float) (32768.0 + (cos((double) a)*32768.0));
    v->xformedy = (EON_Float) (32768.0 + (sin((double) a)*32768.0));
    a += da;
    v++;
  }
  if (cap && div != 3) {
    v->y = h / -2.0f;
    v->x = v->z = 0.0f;
    v->xformedx = (EON_Float) (1<<15);
    v->xformedy = (EON_Float) (1<<15);
    v++;
  }
  f = o->Faces;
  for (i = 1; i <= div; i ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = o->Vertices + (i == div ? 1 : i + 1);
    f->Vertices[2] = o->Vertices + i;
    f->MappingU[0] = (EON_sInt32) o->Vertices[0].xformedx;
    f->MappingV[0] = (EON_sInt32) o->Vertices[0].xformedy;
    f->MappingU[1] = (EON_sInt32) o->Vertices[(i==div?1:i+1)].xformedx;
    f->MappingV[1] = (EON_sInt32) o->Vertices[(i==div?1:i+1)].xformedy;
    f->MappingU[2] = (EON_sInt32) o->Vertices[i].xformedx;
    f->MappingV[2] = (EON_sInt32) o->Vertices[i].xformedy;
    f->Material = m;
    f++;
  }
  if (cap) {
    if (div == 3) {
      f->Vertices[0] = o->Vertices + 1;
      f->Vertices[1] = o->Vertices + 2;
      f->Vertices[2] = o->Vertices + 3;
      f->MappingU[0] = (EON_sInt32) o->Vertices[1].xformedx;
      f->MappingV[0] = (EON_sInt32) o->Vertices[1].xformedy;
      f->MappingU[1] = (EON_sInt32) o->Vertices[2].xformedx;
      f->MappingV[1] = (EON_sInt32) o->Vertices[2].xformedy;
      f->MappingU[2] = (EON_sInt32) o->Vertices[3].xformedx;
      f->MappingV[2] = (EON_sInt32) o->Vertices[3].xformedy;
      f->Material = m;
      f++;
    } else {
      for (i = 1; i <= div; i ++) {
        f->Vertices[0] = o->Vertices + div + 1;
        f->Vertices[1] = o->Vertices + i;
        f->Vertices[2] = o->Vertices + (i==div ? 1 : i+1);
        f->MappingU[0] = (EON_sInt32) o->Vertices[div+1].xformedx;
        f->MappingV[0] = (EON_sInt32) o->Vertices[div+1].xformedy;
        f->MappingU[1] = (EON_sInt32) o->Vertices[i].xformedx;
        f->MappingV[1] = (EON_sInt32) o->Vertices[i].xformedy;
        f->MappingU[2] = (EON_sInt32) o->Vertices[i==div?1:i+1].xformedx;
        f->MappingV[2] = (EON_sInt32) o->Vertices[i==div?1:i+1].xformedy;
        f->Material = m;
        f++;
      }
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

static EON_uChar verts[6*6] = {
  0,4,1, 1,4,5, 0,1,2, 3,2,1, 2,3,6, 3,7,6,
  6,7,4, 4,7,5, 1,7,3, 7,1,5, 2,6,0, 4,0,6
};
static EON_uChar map[24*2*3] = {
  1,0, 1,1, 0,0, 0,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 1,1, 0,1, 1,0,
  0,0, 1,0, 0,1, 1,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 0,1, 1,0, 1,1,
  1,0, 0,1, 0,0, 0,1, 1,0, 1,1,
  1,0, 1,1, 0,0, 0,1, 0,0, 1,1
};


EON_Obj *EON_MakeBox(EON_Float w, EON_Float d, EON_Float h, EON_Mat *m)
{
  EON_uChar *mm = map;
  EON_uChar *vv = verts;
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x;
  o = EON_ObjCreate(8,12);
  if (!o) return 0;
  v = o->Vertices;
  v->x = -w/2; v->y = h/2; v->z = d/2; v++;
  v->x = w/2; v->y = h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = -d/2; v++;
  f = o->Faces;
  for (x = 0; x < 12; x ++) {
    f->Vertices[0] = o->Vertices + *vv++;
    f->Vertices[1] = o->Vertices + *vv++;
    f->Vertices[2] = o->Vertices + *vv++;
    f->MappingU[0] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[0] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[1] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[1] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[2] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[2] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->Material = m;
    f++;
  }

  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakePlane(EON_Float w, EON_Float d, EON_uInt res, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  o = EON_ObjCreate((res+1)*(res+1),res*res*2);
  if (!o) return 0;
  v = o->Vertices;
  for (y = 0; y <= res; y ++) {
    for (x = 0; x <= res; x ++) {
      v->y = 0;
      v->x = ((x*w)/res) - w/2;
      v->z = ((y*d)/res) - d/2;
      v++;
    }
  }
  f = o->Faces;
  for (y = 0; y < res; y ++) {
    for (x = 0; x < res; x ++) {
      f->Vertices[0] = o->Vertices + x+(y*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = (y<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[1] = (x<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
      f->Vertices[0] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = ((y+1)<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+1+((y+1)*(res+1));
      f->MappingU[1] = ((x+1)<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

