/**************************************************************************
 * eon3dx_console.h -- Eon3D eXtension and companion tools                *
 *                  -- Graphical console.                                 *
 * (C) 2010-2013 Francesco Romani <fromani at gmail dot com>              *
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

#include <eon3d.h>

/*
  The opaque console datatype.
  Can use SDL or anything else.
*/
typedef struct eonx_console_ EONx_Console;

typedef int (*EONx_KeyHandler)(int key, EONx_Console *con, void *userdata);


int EONx_ConsoleStartup(const char *title, const char *icon);

int EONx_ConsoleShutdown(void);

EONx_Console *EONx_ConsoleNew(EON_uInt sw, EON_uInt sh, EON_Float fov);

void *EONx_ConsoleDelete(EONx_Console *ctx);

EON_Cam *EONx_ConsoleGetCamera(EONx_Console *ctx);

int EONx_ConsoleBindEventKey(EONx_Console *ctx, int key,
                             EONx_KeyHandler handler, void *userdata);

int EONx_ConsoleNextEvent(EONx_Console *ctx);

int EONx_ConsoleSetPalette(EONx_Console *ctx,
                           const uint8_t *palette, int numcolors);

EON_Frame *EONx_ConsoleGetFrame(EONx_Console *ctx);

int EONx_ConsoleClearFrame(EONx_Console *ctx);

int EONx_ConsoleShowFrame(EONx_Console *ctx);

/* helpers */

const char *EONx_ConsoleMakeName(EONx_Console *ctx,
                                 char *buf, size_t len,
                                 const char *pfx, const char *ext);

int EONx_ConsoleSaveFrame(EONx_Console *ctx,
                          const char *filename, const char *filetype);

const char *EONx_ConsoleGetError(EONx_Console *ctx);

#endif /* EON3DX_CONSOLE_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

