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

/*
  EONx_ConsoleStartup() initializes the console backend.
  Parameters:
    title: title of the graphical window when in foreground
    icon: title of the graphical window when iconized (WARNING!)
  Returns:
    0 on success, !0 on failure.
  Notes:
    Any call to console functions before this call will produce
    unexpected behaviour.
    Should be called once before to use any other console functions.
    Further calls are useless but harmless.
*/
int EONx_ConsoleStartup(const char *title, const char *icon);

/*
  EONx_ConsoleShutdown() finalizes and releases the console backend.
  Parameters:
    none
  Returns:
    0 on success, !0 on failure.
  Notes:
    Any call to console functions after this call will produce
    unexpected behaviour.
    Should be called once after use any other console functions.
    Further calls are useless but harmless.
*/
int EONx_ConsoleShutdown(void);

/*
  EONx_ConsoleNew() gets a new console instance. Depending on the
  actual implementation, a console can have multiple independent
  instance, multiple references to the same instance (aka the
  singleton pattern) or exactly one instance.
  Parameters:
    sw: screen width (pixels) of the console
    sh: screen height (pixels) of the console
    fov: field of vision (degrees) of the console (you usually want
         to use 90 here).
  Returns:
    NULL if a new console instance cannot be made, a valid pointer
    otherwise
  Notes:
    a return value of NULL does NOT necessarely means an error
    is occurred.
*/
EONx_Console *EONx_ConsoleNew(EON_uInt sw, EON_uInt sh, EON_Float fov);

/*
  EONx_ConsoleDelte() releases a console instance obtained through
  EONx_ConsoleNew.
  Parameters:
    ctx: a pointer to the console to be freed.
  Returns:
    always NULL (and never fails.)
*/
void *EONx_ConsoleDelete(EONx_Console *ctx);

/*
  EONx_ConsoleGetCamera() obtains a pointer to a Camera instance
  from a console instance. The returned pointer is a singleton.
  There is no need to free() or destroy in any way the returned
  pointer.
  Parameters:
    ctx: a pointer to a console instance.
  Returns:
    a pointer to a Camera bound to the given console instance.
*/
EON_Cam *EONx_ConsoleGetCamera(EONx_Console *ctx);

/*
  EONx_ConsoleBindEventKey() binds an handler to the event generated
  by a key hit. The handler will block the event loop and
  will run in the same thread running EONx_ConsoleNextEvent
  Parameters:
    ctx: console instance to bind the event/handler to.
    key: value of the key to be handled. Can either be an ASCII
         code or a SDL keymap value (XXX)
    handler: event handling callback.
    userdata: pointer to opaque data to be passed to the callback.
  Returns:
    0 on success, <0 on errors.
*/
int EONx_ConsoleBindEventKey(EONx_Console *ctx, int key,
                             EONx_KeyHandler handler, void *userdata);

/*
  EONx_ConsoleNextEvent() polls the console backend for the next
  event to be processed, NOT blocking the caller if no events
  are available. If an event is available, calls all handlers
  registered for it, or silently discards it if no handlers are
  available.
  Parameters:
    ctx: pointer to the console instance to use.
  Returns:
    if any handler has run, returns the value of the first handler
    registered which returned !0 (e.g. the error of ther first
    handler which run and failed).
    if no handler has run, returns 0 un success or !0 on error.
*/
int EONx_ConsoleNextEvent(EONx_Console *ctx);

int EONx_ConsoleSetPalette(EONx_Console *ctx,
                           const uint8_t *palette, int numcolors);

/*
  EONx_ConsoleGetCamera() obtains a pointer to a Camera instance
  from a console instance. The returned pointer is a singleton.
  There is no need to free() or destroy in any way the returned
  pointer.
  Parameters:
    ctx: a pointer to a console instance.
  Returns:
    a pointer to a Camera bound to the given console instance.
*/
EON_Frame *EONx_ConsoleGetFrame(EONx_Console *ctx);

/*
  EONx_ConsoleClearFrame() clears the current console frame.
  Parameters:
    ctx: a pointer to a console instance.
  Returns:
    0 un success, <0 on error.
*/
int EONx_ConsoleClearFrame(EONx_Console *ctx);

/*
  EONx_ConsoleShowFrame() paint the console with the current
  frame.
  Parameters:
    ctx: a pointer to a console instance.
  Returns:
    0 un success, <0 on error.
*/
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

