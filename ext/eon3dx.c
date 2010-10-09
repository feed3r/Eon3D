/**************************************************************************
 * eon3dx.c -- Eon3D eXtension and companion tools                        *
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

#include "eon3dx.h"

/* just stubs, yet. */


/*************************************************************************
 * Console                                                               *
 *************************************************************************/

struct eonx_console_ {
};

EONx_Console *EONx_newConsole(EON_Camera *camera)
{
    return NULL;
}

void EONX_delConsole(EONx_Console *console)
{
    return;
}

EON_Status EONx_consoleNextEvent(EONx_Console *console)
{
    return EON_ERROR;
}

EON_Status EONx_consoleShow(EONx_Console *console, EON_Frame *frame)
{
    return EON_ERROR;
}


/*************************************************************************
 * Error Handling                                                        *
 *************************************************************************/

EON_Status EONx_logError()
{
    return EON_ERROR;
}


/*************************************************************************
 * General Purpose functions                                             *
 *************************************************************************/

void EONx_exit()
{
    return;
}

/* vim: set ts=4 sw=4 et */
/* EOF */

