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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* yes, that's just an (ugly) workaround patch */
#ifdef SDL_FOUND
#define HAVE_SDL 1
#endif /* SDL_FOUND */


/**************************************************************************
 * The unavoidable forward declarations.                                  *
 **************************************************************************/

struct eonx_console_ {
    void        *private;

    void      *(*newConsole)(EON_Camera *camera);
    void       (*delConsole)(void *console);
    EON_Status (*nextEvent)(void *console);
    EON_Status (*showFrame)(void *console, EON_Frame *frame);
};


/**************************************************************************
 * SDL backend Console implementation.                                    *
 **************************************************************************/

#ifdef HAVE_SDL

#define EONX_SDL_TAG    "EONSDL"

#include <SDL.h>

typedef struct eonx_SDLconsole_ {
    SDL_Surface *Surface;
    SDL_Overlay *Overlay;
    SDL_Rect    Rectangle;

    EON_Int     Width;
    EON_Int     Height;
} eonx_SDLConsole;


static eonx_SDLConsole *eonx_initSDLConsole(eonx_SDLConsole *console,
                                            EON_Camera *camera)
{
    console->Width  = camera->Screen.Width;
    console->Height = camera->Screen.Height;

    SDL_WM_SetCaption("EON3D SDL console", NULL);
    
    console->Surface = SDL_SetVideoMode(console->Width, console->Height,
                                        32, SDL_SWSURFACE);
    if (!console->Surface) {
        EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                "cannot setup SDL Video Mode: %s", SDL_GetError());
        console = NULL;
    } else {
        console->Overlay = SDL_CreateYUVOverlay(console->Width,
                                                console->Height,
                                                SDL_YV12_OVERLAY,
                                                console->Surface);
        if (!console->Overlay) {
            EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                    "cannot setup SDL YUV overlay: %s",
                    SDL_GetError());
            console = NULL;
        }
    }
    if (console) {
        EON_log(EONX_SDL_TAG, EON_LOG_INFO,
                "SDL console window: %ix%i YV12 overlay",
                console->Width, console->Height);
    }
    return console;
}

static eonx_SDLConsole *eonx_finiSDLConsole(eonx_SDLConsole *console)
{
    if (console) {
        if (console->Overlay) {
            SDL_FreeYUVOverlay(console->Overlay);
            console->Overlay = NULL;
        }

        /* 
         * surface obtained by SetVideoMode will be automagically
         * released by SDL_Quit
         */
         console = NULL;
    }
    return console;
}

static void *eonx_newSDLConsole(EON_Camera *camera)
{
    eonx_SDLConsole *console = NULL;
    int err = 0;

    if (!camera) {
        EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                "SDL initialization failed: missing camera reference");
        return NULL;
    }

    err = SDL_Init(SDL_INIT_VIDEO); /* XXX */
    if (err) {
        EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                "SDL initialization failed: %s", SDL_GetError());
    } else {
        console = EON_zalloc(sizeof(eonx_SDLConsole));
        if (!console) {
            EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                    "cannot allocate a Console Context");
        } else {
            console = eonx_initSDLConsole(console, camera);
        }
    }
    return console;
}


static void eonx_delSDLConsole(void *console)
{
    console = eonx_finiSDLConsole(console);
    if (!console) {
        SDL_Quit();
    }
    return;
}

static EON_Status eonx_SDLConsoleNextEvent(void *console)
{
    EON_Status ret = EON_OK; 
    SDL_Event event;

    if (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            EON_log(EONX_SDL_TAG, EON_LOG_INFO, "got quit event. Bye!");
            ret = EON_ERROR;
        }
    } else {
        EON_log(EONX_SDL_TAG, EON_LOG_ERROR,
                "error fetching event: %s", SDL_GetError());
        ret = EON_ERROR;
    }
    return ret;
}

static EON_Status eonx_SDLConsoleShow(void *console, EON_Frame *frame)
{
    /* TODO */
    return EON_ERROR;
}

static void eonx_SDLConsoleRegister(EONx_Console *console)
{
    if (console) { /* XXX */
        console->newConsole = eonx_newSDLConsole;
        console->delConsole = eonx_delSDLConsole;
        console->nextEvent  = eonx_SDLConsoleNextEvent;
        console->showFrame  = eonx_SDLConsoleShow;
    }
    return;
}


#endif /* HAVE_SDL */

/**************************************************************************
 * NULL backend Console implementation.                                   *
 **************************************************************************/

#define EONX_NULL_TAG    "EONCON"

static void *eonx_newNullConsole(EON_Camera *camera)
{
    
    return NULL; /* FIXME */ 
}


static void eonx_delNullConsole(void *console)
{
    return;
}

static EON_Status eonx_NullConsoleNextEvent(void *console)
{
    return EON_OK; /* never exits */
}

static EON_Status eonx_NullConsoleShow(void *console, EON_Frame *frame)
{
    return EON_OK;
}


static void eonx_NullConsoleRegister(EONx_Console *console)
{
    if (console) { /* XXX */
        console->newConsole = eonx_newNullConsole;
        console->delConsole = eonx_delNullConsole;
        console->nextEvent  = eonx_NullConsoleNextEvent;
        console->showFrame  = eonx_NullConsoleShow;
    }
    return;
}


/*************************************************************************
 * Console (frontend)                                                    *
 *************************************************************************/

EONx_Console *EONx_newConsole(EON_Camera *camera)
{
    EONx_Console *console = EON_zalloc(sizeof(EONx_Console));
    if (console) {
        eonx_NullConsoleRegister(console);
#ifdef HAVE_SDL
        eonx_SDLConsoleRegister(console);
#endif /* HAVE_SDL */

        console->private = console->newConsole(camera);
    }
    return console;
}

void EONx_delConsole(EONx_Console *console)
{
    if (console) { /* XXX */
        console->delConsole(console->private);
    }
    return;
}

EON_Status EONx_consoleNextEvent(EONx_Console *console)
{
    EON_Status err = EON_ERROR;
    if (console) { /* XXX */
        err = console->nextEvent(console->private);
    }
    return err;
}

EON_Status EONx_consoleShow(EONx_Console *console, EON_Frame *frame)
{
    EON_Status err = EON_ERROR;
    if (console) { /* XXX */
        err = console->showFrame(console->private, frame);
    }
    return err;
}


/*************************************************************************
 * Initialization and Finalization                                       *
 *************************************************************************/

void EONx_setup()
{
    EON_startup();
}

void EONx_exit()
{
    EON_shutdown();
}

/* vim: set ts=4 sw=4 et */
/* EOF */

