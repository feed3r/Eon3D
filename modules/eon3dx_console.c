/**************************************************************************
 * eon3dx_console.c -- Eon3D eXtension and companion tools                *
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

#include <stdlib.h>

#include "eon3dx_console.h"

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
    void        *_private;

    void      *(*NewConsole)(EON_Camera *camera);
    void       (*DelConsole)(void *console);
    EON_Status (*NextEvent)(void *console, void *event);
    EON_Status (*ShowFrame)(void *console, EON_Frame *frame);
    EON_Frame *(*GetFrame)(void *console, EON_Frame *frame);
    EON_Status (*Clear)(void *console);
};


/**************************************************************************
 * SDL backend Console implementation.                                    *
 **************************************************************************/

#ifdef HAVE_SDL

#define EONx_SDL_TAG    "EONSDL"

#include <SDL.h>

enum {
    EONx_SDL_BLANK_COLOR = 42
};

typedef struct eonx_SDLConsole_ {
    SDL_Surface *Surface;
    SDL_Rect    Rectangle;

    EON_Int     Width;
    EON_Int     Height;
} eonx_SDLConsole;


/*************************************************************************/

#define RETURN_IF_NULL_PTR(PTR) do { \
    if (!(PTR)) { \
        return NULL; \
    } \
} while (0);

#define RETURN_IF_NULL_PTR_MSG(PTR, TAG, MSG) do { \
    if (!(PTR)) { \
         EON_log((TAG), EON_LOG_ERROR, \
                 "%s failed: missing %s reference", (MSG), # PTR ); \
        return NULL; \
    } \
} while (0)        

#define FAIL_IF_NULL_PTR(PTR, TAG, MSG) do { \
    if (!(PTR)) { \
         EON_log((TAG), EON_LOG_ERROR, \
                 "%s failed: missing %s reference", (MSG), # PTR ); \
        return EON_ERROR; \
    } \
} while (0)        

/*************************************************************************/

static eonx_SDLConsole *eonx_initSDLVideoMode(eonx_SDLConsole *console)
{
    Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_HWACCEL|SDL_ASYNCBLIT;

    RETURN_IF_NULL_PTR(console);

    console->Surface = SDL_SetVideoMode(console->Width, console->Height,
                                        EON_RGB_BPP * 8, flags);
    if (!console->Surface) {
        EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                "cannot setup SDL Video Mode: %s", SDL_GetError());
        console = NULL;
    }

    return console;
}

static eonx_SDLConsole *eonx_initSDLSurface(eonx_SDLConsole *console)
{
    RETURN_IF_NULL_PTR(console);

    if (SDL_MUSTLOCK(console->Surface)) {
        if (SDL_LockSurface(console->Surface) < 0 ) {
            EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                    "cannot lock the SDL Surface: %s", SDL_GetError());
            console = NULL;
        }
    }

    return console;
}

static eonx_SDLConsole *eonx_finiSDLSurface(eonx_SDLConsole *console)
{
    RETURN_IF_NULL_PTR(console);

    if (SDL_MUSTLOCK(console->Surface)) {
        SDL_UnlockSurface(console->Surface);
    }
    return console;
}


/* Almost verbatim copy from www.libsdl.org "Introduction" section */
static EON_Status eonx_SDLDrawPixel(eonx_SDLConsole *console,
                                    int y, int x, Uint32 color)
{
    void *drawArea = console->Surface->pixels;
    Uint32 pitch = console->Surface->pitch;

    switch (console->Surface->format->BytesPerPixel) {
      case 1: { /* Assuming 8-bpp */
        Uint8 *bufp = (Uint8 *)drawArea + y * pitch + x;
        *bufp = color;
      }
      break;

      case 2: { /* Probably 15-bpp or 16-bpp */
        Uint16 *bufp = (Uint16 *)drawArea + y * pitch/2 + x;
        *bufp = color;
      }
      break;

      case 3: { /* Slow 24-bpp mode, usually not used */
        Uint8 *bufp = drawArea + y * pitch + x;
        SDL_PixelFormat *fmt = console->Surface->format;
        *(bufp + fmt->Rshift/8) = ((color & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
        *(bufp + fmt->Gshift/8) = ((color & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
        *(bufp + fmt->Bshift/8) = ((color & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
      }
      break;

      case 4: { /* Probably 32-bpp */
        Uint32 *bufp = drawArea + y * pitch/4 + x;
        *bufp = color;
      }
      break;
    }

    return EON_OK;
}

static EON_Status eonx_SDLPutPixel(EON_Frame *frame,
                                   EON_Int x, EON_Int y, EON_UInt32 color)
{
    if (!frame || !frame->_private) {
        EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                "put pixel failed: bad frame reference");
        return EON_ERROR;
    }
    eonx_SDLConsole *SDLCon = frame->_private;

    return eonx_SDLDrawPixel(SDLCon, y, x, color);
}

/*************************************************************************/

static EON_Status eonx_SDLConsoleClear(void *SDLCon_)
{
    eonx_SDLConsole *SDLCon = SDLCon_;
    Uint8 C = EONx_SDL_BLANK_COLOR; 
    
    FAIL_IF_NULL_PTR(SDLCon, EONx_SDL_TAG, "clear");
    
    Uint32 color = SDL_MapRGB(SDLCon->Surface->format, C, C, C);
    SDL_FillRect(SDLCon->Surface, NULL, color);

    return EON_OK;
}


static EON_Status eonx_SDLConsoleNextEvent(void *SDLCon_, void *ev)
{
    eonx_SDLConsole *SDLCon = SDLCon_;
    EON_Status ret = EON_OK; 
    SDL_Event event;

    if (SDL_WaitEvent(&event)) {
        if (event.type == SDL_QUIT) {
            EON_log(EONx_SDL_TAG, EON_LOG_INFO, "got quit event. Bye!");
            ret = EON_ERROR;
        }
    } else {
        EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                "error fetching event: %s", SDL_GetError());
        ret = EON_ERROR;
    }
    return ret;
}

static EON_Frame *eonx_SDLConsoleGetFrame(void *SDLCon_,
                                          EON_Frame *frame)
{
    eonx_SDLConsole *SDLCon = SDLCon_;

    RETURN_IF_NULL_PTR_MSG(SDLCon, EONx_SDL_TAG, "get frame");

    if (!frame) {
        frame = EON_zalloc(sizeof(EON_Frame));
    }
    if (frame) {
        frame->F.Width  = SDLCon->Width;
        frame->F.Height = SDLCon->Height;
        frame->Flags    = EON_FRAME_FLAG_DR;
        /* yes, this smells ugly */
        frame->Pixels   = SDLCon->Surface->pixels;
        frame->PutPixel = eonx_SDLPutPixel;
        frame->_private = SDLCon;
    }
    return frame;
}

static EON_Status eonx_SDLCopyFrame(eonx_SDLConsole *SDLCon,
                                    EON_Frame *frame)
{
}

static EON_Status eonx_SDLConsoleShow(void *SDLCon_,
                                      EON_Frame *frame)
{
    eonx_SDLConsole *SDLCon = SDLCon_;
    EON_Status ret = EON_OK;

    FAIL_IF_NULL_PTR(SDLCon, EONx_SDL_TAG, "show frame");
    FAIL_IF_NULL_PTR(frame,  EONx_SDL_TAG, "show frame");

    if (!(frame->Flags & EON_FRAME_FLAG_DR)) {
        ret = eonx_SDLCopyFrame(SDLCon, frame);
    }

    /* blit surface */

    return ret;
}


/*************************************************************************/


static eonx_SDLConsole *eonx_initSDLConsole(eonx_SDLConsole *console,
                                            EON_Camera *camera)
{
    console->Width  = camera->Screen.Width;
    console->Height = camera->Screen.Height;

    atexit(SDL_Quit); /* I'm not convinced about that */
    
    SDL_WM_SetCaption("EON3D SDL console", NULL);
   
    console = eonx_initSDLVideoMode(console);
    console = eonx_initSDLSurface(console);

    if (console) {
        EON_log(EONx_SDL_TAG, EON_LOG_INFO,
                "SDL console window: %ix%i",
                console->Width, console->Height);
    }
    return console;
}

static eonx_SDLConsole *eonx_finiSDLConsole(eonx_SDLConsole *console)
{
    if (console) {
        eonx_finiSDLSurface(console);
        /* 
         * surface obtained by SetVideoMode will be automagically
         * released by SDL_Quit
         */
         console = NULL;
    }
    return console;
}

static eonx_SDLConsole *eonx_SDLConsoleAlloc(EON_Camera *camera)
{
    eonx_SDLConsole *console = EON_zalloc(sizeof(eonx_SDLConsole));
    if (!console) {
        EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                "cannot allocate a Console Context");
    } else {
        void *SDLCon = eonx_initSDLConsole(console, camera);
        if (SDLCon) {
            console = SDLCon;
        } else {
            EON_free(console);
            console = NULL;
        }
    }
    return console;
}


static void *eonx_SDLConsoleNew(EON_Camera *camera)
{
    eonx_SDLConsole *SDLCon = NULL;
    int err = 0;

    RETURN_IF_NULL_PTR_MSG(camera, EONx_SDL_TAG, "SDL initialization");

    err = SDL_Init(SDL_INIT_VIDEO); /* XXX */
    if (err) {
        EON_log(EONx_SDL_TAG, EON_LOG_ERROR,
                "SDL initialization failed: %s", SDL_GetError());
    } else {
        SDLCon = eonx_SDLConsoleAlloc(camera);
    }

    if (SDLCon) {
        eonx_SDLConsoleClear(SDLCon);
    }
    return SDLCon;
}


static void eonx_SDLConsoleDel(void *console)
{
    console = eonx_finiSDLConsole(console);
    if (!console) {
        SDL_Quit();
    }
    return;
}


static void eonx_SDLConsoleRegister(EONx_Console *console)
{
    if (console) { /* XXX */
        console->NewConsole = eonx_SDLConsoleNew;
        console->DelConsole = eonx_SDLConsoleDel;
 
        console->NextEvent  = eonx_SDLConsoleNextEvent;
        console->ShowFrame  = eonx_SDLConsoleShow;
        console->GetFrame   = eonx_SDLConsoleGetFrame;
        console->Clear      = eonx_SDLConsoleClear;
    }
    return;
}


#endif /* HAVE_SDL */

/**************************************************************************
 * NULL backend Console implementation.                                   *
 **************************************************************************/

#define EONx_NULL_TAG    "EONCON"

static void *eonx_NullConsoleNew(EON_Camera *camera)
{
    return NULL; /* FIXME */ 
}


static void eonx_NullConsoleDel(void *console)
{
    return;
}

static EON_Status eonx_NullConsoleNextEvent(void *console, void *event)
{
    return EON_OK; /* never exits */
}

static EON_Status eonx_NullConsoleShow(void *console, EON_Frame *frame)
{
    return EON_OK;
}

static EON_Status eonx_NullConsoleClear(void *console)
{
    return EON_OK;
}

static EON_Frame *eonx_NullConsoleGetFrame(void *console,
                                           EON_Frame *frame)
{
    return frame;
}

static void eonx_NullConsoleRegister(EONx_Console *console)
{
    if (console) { /* XXX */
        console->NewConsole = eonx_NullConsoleNew;
        console->DelConsole = eonx_NullConsoleDel;

        console->NextEvent  = eonx_NullConsoleNextEvent;
        console->ShowFrame  = eonx_NullConsoleShow;
        console->GetFrame   = eonx_NullConsoleGetFrame;
        console->Clear      = eonx_NullConsoleClear;
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

        console->_private = console->NewConsole(camera);
    }
    return console;
}

void EONx_delConsole(EONx_Console *console)
{
    if (console && console->DelConsole) {
        console->DelConsole(console->_private);
    }
    return;
}

EON_Status EONx_consoleNextEvent(EONx_Console *console, void *event)
{
    EON_Status err = EON_ERROR;
    if (console && console->NextEvent) {
        err = console->NextEvent(console->_private, event);
    }
    return err;
}

EON_Status EONx_consoleShow(EONx_Console *console, EON_Frame *frame)
{
    EON_Status err = EON_ERROR;
    if (console && console->ShowFrame) {
        err = console->ShowFrame(console->_private, frame);
    }
    return err;
}

EON_Frame *EONx_consoleGetFrame(EONx_Console *console,
                                EON_Frame *frame)
{
    EON_Status err = EON_ERROR;
    if (console && console->GetFrame) {
        frame = console->GetFrame(console->_private, frame);
    }
    return frame;
}

EON_Status EONx_consoleClear(EONx_Console *console)
{
    EON_Status err = EON_ERROR;
    if (console && console->Clear) {
        err = console->Clear(console->_private);
    }
    return err;
}

/* vim: set ts=4 sw=4 et */
/* EOF */

