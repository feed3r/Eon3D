/**************************************************************************
 * eon3dx.h -- Eon3D is a simplistic 3D software renderer.                *
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

#ifndef EON3D_AUX_H
#define EON3D_AUX_H

#include <eon3d.h>


/******************************************************************************
** Object Primitives Code (make.c)
******************************************************************************/

/*
  EON_MakePlane() makes a EON_ane centered at the origin facing up the y axis.
  Parameters:
    w: width of the EON_ane (along the x axis)
    d: depth of the EON_ane (along the z axis)
    res: resolution of EON_plane, i.e. subdivisions
    m: material to use
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakePlane(EON_Float w, EON_Float d, EON_uInt res, EON_Mat *m);

/*
  EON_MakeBox() makes a box centered at the origin
  Parameters:
    w: width of the box (x axis)
    d: depth of the box (z axis)
    h: height of the box (y axis)
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakeBox(EON_Float w, EON_Float d, EON_Float h, EON_Mat *m);

/*
  EON_MakeCone() makes a cone centered at the origin
  Parameters:
    r: radius of the cone (x-z axis)
    h: height of the cone (y axis)
    div: division of cone (>=3)
    cap: close the big end?
    m: material to use
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakeCone(EON_Float r, EON_Float h, EON_uInt div, EON_Bool cap, EON_Mat *m);

/*
  EON_MakeCylinder() makes a cylinder centered at the origin
  Parameters:
    r: radius of the cylinder (x-z axis)
    h: height of the cylinder (y axis)
    divr: division of of cylinder (around the circle) (>=3)
    captop: close the top
    capbottom: close the bottom
    m: material to use
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakeCylinder(EON_Float r, EON_Float h, EON_uInt divr, EON_Bool captop,
                          EON_Bool capbottom, EON_Mat *m);

/*
  EON_MakeSphere() makes a sphere centered at the origin.
  Parameters:
    r: radius of the sphere
    divr: division of the sphere (around the y axis) (>=3)
    divh: division of the sphere (around the x,z axis) (>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakeSphere(EON_Float r, EON_uInt divr, EON_uInt divh, EON_Mat *m);

/*
  EON_MakeTorus() makes a torus centered at the origin
  Parameters:
    r1: inner radius of the torus
    r2: outer radius of the torus
    divrot: division of the torus (around the y axis) (>=3)
    divrad: division of the radius of the torus (x>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
EON_Obj *EON_MakeTorus(EON_Float r1, EON_Float r2, EON_uInt divrot,
                       EON_uInt divrad, EON_Mat *m);


/******************************************************************************
** 8xX  Bitmapped Text (text.c)
******************************************************************************/

typedef struct _EON_Font {
    const EON_Byte *Face;
    EON_Color Color;
    EON_uInt Height;
} EON_Font;

/*
  EON_TextDefaultFont() gets the default builtin EON font.
    Parameters:
      nothing.
    Returns:
      a pointer to a Font object to be initialized.
*/
EON_Font *EON_TextDefaultFont();

/*
  EON_FontDelete() frees all memory associated with "font"
*/
void EON_FontDelete(EON_Font *font);

/*
  EON_TextPutChar() puts a character to a camera
  Parameters:
    font: the Font object to be used.
    cam: the camera. If the camera has a zBuffer, it will be used.
    frame: the frame on which to render.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    c: the character to put. Special characters such as '\n' aren't handled.
  Returns:
    nothing
*/
void EON_TextPutChar(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                     EON_sInt x, EON_sInt y, EON_Float z,
                     char c);

/*
  EON_TextPutString() puts an array of characters to a camera
  Parameters:
    font: the Font object to be used.
    cam: The camera. If the camera has a zBuffer, it will be used.
    frame: the frame on which to render.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    string:
      the characters to put. '\n' and '\t' are handled as one would expect
  Returns:
    nothing
*/
void EON_TextPutStr(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                    EON_sInt x, EON_sInt y, EON_Float z,
                    const char *string);

/*
  EON_TextPrintf() is printf() for graphics
  Parameters:
    font: the Font object to be used.
    cam: The camera. If the camera has a zBuffer, it will be used.
    frame: the frame on which to render.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text
    format:
      the characters to put, with printf() formatting codes.
      '\n' and '\t' are handled as one would expect
    ...: any additional parameters specified by format
  Returns:
    nothing
*/
void EON_TextPrintf(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                    EON_sInt x, EON_sInt y, EON_Float z,
                    const char *format, ...);

/*************************************************************************/

#endif /* ! EON3D_AUX_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

