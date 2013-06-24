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
** Spline Interpolation (spline.c)
******************************************************************************/

/**
 * Spline type.
 * @see EON_Spline*().
 */
typedef struct _EON_Spline {
    EON_Float *keys;    //!< Key data, keyWidth*numKeys
    EON_sInt keyWidth;  //!< Number of floats per key
    EON_sInt numKeys;   //!< Number of keys
    EON_Float cont;     //!< Continuity. Should be -1.0 -> 1.0
    EON_Float bias;     //!< Bias. -1.0 -> 1.0
    EON_Float tens;     //!< Tension. -1.0 -> 1.0
} EON_Spline;


/**
 * initializes a spline.
 * Call this once, or when you change any of the settings
 * @param s the spline to be initialized.
 */
void EON_SplineInit(EON_Spline *s);

/**
 * gets a point on the spline.
 * @param[in] s a spline.
 * @param[in] frame time into spline. 0.0 is start, 1.0 is second key point, etc.
 * @param[out] out a pointer to an array of s->keyWidth floats that will be filled in.
*/
void EON_SplineGetPoint(EON_Spline *s, EON_Float frame, EON_Float *out);


/******************************************************************************
** Object Primitives Code (make.c)
******************************************************************************/

/**
 * makes a plane centered at the origin facing up the y axis.
 * @param w width of the plane (along the x axis).
 * @param d depth of the plane (along the z axis)
 * @param res resolution of plplane, i.e. subdivisions.
 * @param m material to use.
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakePlane(EON_Float w, EON_Float d, EON_uInt res, EON_Mat *m);

/**
 * makes a box centered at the origin.
 * @param w width of the box (x axis).
 * @param d depth of the box (z axis).
 * @param h height of the box (y axis).
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakeBox(EON_Float w, EON_Float d, EON_Float h, EON_Mat *m);

/**
 * makes a cone centered at the origin.
 * @param r radius of the cone (x-z axis).
 * @param h height of the cone (y axis).
 * @param div division of cone (>=3).
 * @param cap close the big end? (boolean value)
 * @param m material to use.
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakeCone(EON_Float r, EON_Float h, EON_uInt div, EON_Bool cap, EON_Mat *m);

/**
 * makes a cylinder centered at the origin.
 * @param r radius of the cylinder (x-z axis).
 * @param h height of the cylinder (y axis).
 * @param divr division of of cylinder (around the circle) (>=3).
 * @param captop: close the top? (boolean value)
 * @param capbottom close the bottom? (boolean value)
 * @param m material to use.
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakeCylinder(EON_Float r, EON_Float h, EON_uInt divr, EON_Bool captop,
                          EON_Bool capbottom, EON_Mat *m);

/**
 * makes a sphere centered at the origin.
 * @param r radius of the sphere.
 * @param divr division of the sphere (around the y axis) (>=3).
 * @param divh division of the sphere (around the x,z axis) (>=3).
 * @param m material to use.
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakeSphere(EON_Float r, EON_uInt divr, EON_uInt divh, EON_Mat *m);

/**
 * makes a torus centered at the origin.
 * @param r1 inner radius of the torus.
 * @param r2 outer radius of the torus.
 * @param divrot division of the torus (around the y axis) (>=3).
 * @param divrad division of the radius of the torus (x>=3).
 * @param m material to use.
 * @return a pointer to object created or NULL on error.
 */
EON_Obj *EON_MakeTorus(EON_Float r1, EON_Float r2, EON_uInt divrot,
                       EON_uInt divrad, EON_Mat *m);


/******************************************************************************
** 8xX  Bitmapped Text (text.c)
******************************************************************************/

typedef struct _EON_Font {
    const EON_Byte *Face; //!< the actual font value
    EON_Color Color;      //!< color of the font
    EON_uInt Height;      //!< height of the font (px)
} EON_Font;

/**
 * gets the default builtin EON font.
 * There is no need to free() or destroy in any way the returned
 * pointer.
 * @return a pointer to a Font object.
 */
EON_Font *EON_TextDefaultFont();

/**
 * frees all memory associated with a given font.
 * @param font the font to be released.
 */
void EON_FontDelete(EON_Font *font);

/**
 * puts a character to a given frame through a camera.
 * @param font the Font object to be used.
 * @param cam the camera. If the camera has a zBuffer, it will be used.
 * @param frame the frame on which to render.
 * @param x the x screen position of the left of the text.
 * @param y the y screen position of the top of the text.
 * @param z the depth of the text (used when cam->zBuffer is set).
 * @param c the character to put. Special characters such as '\n' aren't handled.
 */
void EON_TextPutChar(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                     EON_sInt x, EON_sInt y, EON_Float z,
                     char c);

/**
 * puts an array of characters to a frame through a camera.
 * @param font the Font object to be used.
 * @param cam The camera. If the camera has a zBuffer, it will be used.
 * @param frame the frame on which to render.
 * @param x the x screen position of the left of the text.
 * @param y the y screen position of the top of the text.
 * @param z the depth of the text (used when cam->zBuffer is set).
 * @param string the characters to put. '\n' and '\t' are handled as one would expect.
 */
void EON_TextPutStr(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                    EON_sInt x, EON_sInt y, EON_Float z,
                    const char *string);

/**
 * printf() for graphics.
 * @see EON_TextPutStr
 * @param font the Font object to be used.
 * @param cam The camera. If the camera has a zBuffer, it will be used.
 * @param frame the frame on which to render.
 * @param x the x screen position of the left of the text.
 * @param y the y screen position of the top of the text.
 * @param z the depth of the text.
 * @param format the characters to put, with printf() formatting codes.
 *        '\n' and '\t' are handled as one would expect
 * @param ... any additional parameters specified by format.
 */
void EON_TextPrintf(EON_Font *font, EON_Cam *cam, EON_Frame *frame,
                    EON_sInt x, EON_sInt y, EON_Float z,
                    const char *format, ...);

/*************************************************************************/

#endif /* ! EON3D_AUX_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

