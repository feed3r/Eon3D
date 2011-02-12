/**************************************************************************
 * eon3dx_console.h -- Eon3D eXtension and companion tools                *
 *                  -- Graphical console.                                 *
 * (C) 2010-2011 Francesco Romani <fromani at gmail dot com>              *
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
 **************************************************************************/

#ifndef EON3DX_CONSOLE_H
#define EON3DX_CONSOLE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <eon3d.h>

/** \file eon3dx_console.h
    \brief eon3d eXtension: the graphical console.
*/

/*************************************************************************
 * Console                                                               *
 *************************************************************************/

/** \struct the opaque console datatype.
   Can use SDL or anything else.
*/
typedef struct eonx_console_ EONx_Console;


/** \fn allocates a new console.

    Allocates a new console and attaches it to a given EON_Camera.
    A console represents a viewpoint, so it has to have an attached camera.

    \param camera a EON_Camera representing the console viewpoint.
    \return a new console handle on success,
            NULL on error.

    \see EONx_delConsole
*/
EONx_Console *EONx_newConsole(EON_Camera *camera);


/** \fn release a console handle.

    Release a console handle obtained via EONx_newConsole.

    \param console a valid EONx_Console handle to release.

    \see EONx_newConsole
*/
void EONx_delConsole(EONx_Console *console);

/* TODO */
EON_Status EONx_consoleNextEvent(EONx_Console *console, void *event);

/** \fn shows a frame into the console.

    Shows a given frame into a console.

    \param console a valid EONx_Console handle.
    \param frame the frame to display.
    \return EON_OK on success,
            EON_ERROR otherwise.

    \see EONx_consoleGetFrame
*/
EON_Status EONx_consoleShow(EONx_Console *console, EON_Frame *frame);

/** \fn blanks the console.

    Blanks the given console.

    \param console a valid EONx_Console handle.
    \return EON_OK on success,
            EON_ERROR otherwise.
*/
EON_Status EONx_consoleClear(EONx_Console *console);


/** \fn Gets a frame handle optimized for the given console.

    Any EON_Frame can safely displayed to any console, in the worst case
    involving some memory area copies. This function allow the caller to
    obtain the most optimized frame handle to be used with the given
    console. Using the obtained frame, the display can be done in the most
    efficient manner.

    \param console a valid EONx_Console handle.
    \param frame a EON_Frame handle to setup. If NULL, a new EON_Frame
           will be allocated. The newly allocated frame can be safely
           disposed as usual.
    \return an optimized frame on success,
            NULL on error.

    \see EONx_consoleShow
*/
EON_Frame *EONx_consoleGetFrame(EONx_Console *console, EON_Frame *frame);



#endif /* EON3DX_CONSOLE_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

