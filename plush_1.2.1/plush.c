/******************************************************************************
Plush Version 1.2
plush.c
Misc code and data
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

#include "plush.h"

pl_uChar plText_DefaultFont[256*16] = {
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

/* Can't find another place to put this... */
void plTexDelete(pl_Texture *t) {
  if (t) {
    if (t->Data) free(t->Data);
    if (t->PaletteData) free(t->PaletteData);
    free(t);
  }
}
