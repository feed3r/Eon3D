/**************************************************************************
 * eon3dx.h -- Eon3D eXtension and companion tools                        *
 * (C) 2010 Francesco Romani <fromani at gmail dot com>                   *
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

#ifndef EON3DX_H
#define EON3DX_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <eon3d.h>


/*************************************************************************
 * Console                                                               *
 *************************************************************************/

/* opaque. Can use SDL or anything else. */
typedef struct eonx_console_ EONx_Console;

EONx_Console    *EONx_newConsole(EON_Camera *camera);
void            EONx_delConsole(EONx_Console *console);

EON_Status      EONx_consoleNextEvent(EONx_Console *console,
                                      void *event); /* TODO */

EON_Status      EONx_consoleShow(EONx_Console *console, EON_Frame *frame);

EON_Status      EONx_consoleClear(EONx_Console *console);

EON_Frame      *EONx_consoleGetFrame(EONx_Console *console,
                                     EON_Frame *frame);



#endif /* EON3DX_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

