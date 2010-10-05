/**************************************************************************
 * eon3dx.h -- Eon3D eXtension and companion toolsrenderer.               *
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
void            EONX_delConsole(EONx_Console *console);

EON_Status      EONx_consoleNextEvent(EONx_Console *console);

EON_Status      EONx_consoleShow(EONx_Console *console, EON_Frame *frame);


/*************************************************************************
 * Error Handling                                                        *
 *************************************************************************/

EON_Status EONx_logError(); // XXX?!?!


/*************************************************************************
 * General Purpose functions                                             *
 *************************************************************************/

void EONx_exit(); // XXX

/* vim: set ts=4 sw=4 et */
/* EOF */

