/**************************************************************************
 * eon3dx_console.c -- Eon3D eXtension and companion tools                *
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

#include <time.h>

#include "stringkit.h"
#include "arraykit.h"

#include "eon3dx_console.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* yes, that's just an (ugly) workaround patch */
#ifdef SDL_FOUND
#define HAVE_SDL 1
#endif /* SDL_FOUND */

/* ditto */
#ifdef PNG_FOUND
#define HAVE_PNG 1
#endif /* PNG_FOUND */

/**************************************************************************
 * SDL backend Console implementation.                                    *
 **************************************************************************/

#ifdef HAVE_SDL

#define EONx_SDL_TAG    "EONSDL"

#include <SDL.h>

#include "savepng.h"


// FIXME BPP ambiguitiy (depth/BPP/bpp)

/* for future usage */
enum {
    EONx_CONSOLE_FLAG_NONE = 0,
};

enum {
    TRUECOLOR_BPP   = 4,
    TRUECOLOR_DEPTH = 32 // XXX
};

typedef struct eonx_event_ {
    EONx_KeyHandler handler;
    void *userdata;
    int key;
} EONx_Event;

struct eonx_console_ {
    EON_Cam *cam;
    EON_Frame *fb;
    SDL_Surface *screen;
    SDL_Surface *frame;
    CX_VArray *keyhandlers;
};


int EONx_ConsoleStartup(const char *title, const char *icon)
{
    int err = SDL_Init(SDL_INIT_VIDEO);
    if (!err) {
        SDL_WM_SetCaption(title, icon);
        atexit(SDL_Quit);
    }
    return err;
}


int EONx_ConsoleShutdown(void)
{
    return 0;
}

int EONx_ConsoleSetPalette(EONx_Console *ctx, const uint8_t *palette, int numcolors)
{
    EON_CamSetPalette(ctx->cam, palette, numcolors);
    return 0;
}

static int EONx_ConsoleInitSurfaces(EONx_Console *ctx)
{
    int err = -1;
    Uint32 flags = SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_HWACCEL|SDL_ASYNCBLIT;
    ctx->screen = SDL_SetVideoMode(ctx->fb->Width, ctx->fb->Height,
                                   TRUECOLOR_DEPTH, flags);
    /*
     * always truecolor on screen, even if internally we use a palette.
     * SDL blitting will take care of the conversion.
     */
    if (ctx->screen) {
        ctx->frame = SDL_CreateRGBSurfaceFrom(ctx->fb->Data,
                                              ctx->fb->Width, ctx->fb->Height,
                                              TRUECOLOR_DEPTH,
                                              ctx->fb->Width * TRUECOLOR_BPP,
                                              0, 0, 0, 0);
        if (ctx->frame) {
            err = 0;
        }
    }
    return err;
}


static void *EONx_ConsoleCleanup(EONx_Console *ctx)
{
    if (ctx->frame) {
        SDL_FreeSurface(ctx->frame);
        ctx->frame = NULL;
    }
    if (ctx->cam) {
        EON_CamDelete(ctx->cam);
        ctx->cam = NULL;
    }
    if (ctx->fb) {
        EON_FrameDelete(ctx->fb);
        ctx->fb = NULL;
    }
    free(ctx);
    return NULL;
}

EONx_Console *eon_ConsoleCreate(EON_uInt sw, EON_uInt sh,
                                 EON_Float ar, EON_Float fov,
                                 EON_uInt32 flags)
{
    int err = -1;
    EONx_Console *ctx = calloc(1, sizeof(*ctx));
    if (ctx) {
        ctx->fb = EON_FrameCreate(sw, sh, TRUECOLOR_BPP);
        if (ctx->fb) {
            ctx->cam = EON_CamCreate(sw, sh, ar, fov);
            if (ctx->cam) {
                err = EONx_ConsoleInitSurfaces(ctx);
            }
            ctx->keyhandlers = CX_varray_new(8, sizeof(EONx_Event)); //XXX
        }
    }
    if (err) {
        ctx = EONx_ConsoleCleanup(ctx);
    }
    return ctx;
}

EONx_Console *EONx_ConsoleNew(EON_uInt sw, EON_uInt sh, EON_Float fov)
{
    return eon_ConsoleCreate(sw, sh, 1.0, fov,
                              EONx_CONSOLE_FLAG_NONE);
}

void *EONx_ConsoleDelete(EONx_Console *ctx)
{
    EONx_ConsoleCleanup(ctx);
    return NULL;
}


int EONx_ConsoleClearFrame(EONx_Console *ctx)
{
    int err = -1;
    if (ctx) {
        EON_FrameClear(ctx->fb);
        err = 0;
    }
    return err;
}

EON_Frame* EONx_ConsoleGetFrame(EONx_Console *ctx)
{
    EON_Frame *frame = NULL;
    if (ctx) {
        frame = ctx->fb;
    }
    return frame;
}


int EONx_ConsoleShowFrame(EONx_Console *ctx)
{
    int err = -1;
    if (ctx) {
        err = SDL_BlitSurface(ctx->frame, NULL, ctx->screen, NULL);
        err = SDL_Flip(ctx->screen);
    }
    return err;
}


EON_Cam *EONx_ConsoleGetCamera(EONx_Console *ctx)
{
    EON_Cam *cam = NULL;
    if (ctx) {
        cam = ctx->cam;
    }
    return cam;
}

int EONx_ConsoleBindEventKey(EONx_Console *ctx, int key,
                             EONx_KeyHandler handler, void *userdata)
{
    int err = 0;
    if (ctx) {
        EONx_Event ev = {
            .key = key,
            .handler = handler,
            .userdata = userdata
        };
        err = CX_varray_append(ctx->keyhandlers, &ev);
    }
    return err;
}

int eonx_ConsoleHandleKeyEvent(EONx_Console *ctx, int key)
{
    int err = 0, i = 0, found = 0;
    int n = CX_varray_length(ctx->keyhandlers);
    for (i = 0; !found && i < n; i++) {
        EONx_Event *ev = CX_varray_get_ref(ctx->keyhandlers, i);
        if (key == ev->key) {
            err = ev->handler(key, ctx, ev->userdata);
            found = 1;
        }
    }
    return err;
}

int EONx_ConsoleNextEvent(EONx_Console *ctx)
{
    int err = 0;
    SDL_Event event;

    while (!err && SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                err = -1;
                break;
            case SDL_KEYDOWN:
                err = eonx_ConsoleHandleKeyEvent(ctx,
                                                 event.key.keysym.sym);
                break;
            default:
                err = 0;
                break;
        }
    }

    return err;
}


const char *EONx_ConsoleGetError(EONx_Console *ctx)
{
    return SDL_GetError();
}

int EONx_ConsoleSaveFrame(EONx_Console *ctx,
                          const char *filename, const char *filetype)
{
    int ret = -1;
    if (ctx && filename) {
        if (!filetype || !strcasecmp(filetype, "bmp")) {
            ret = SDL_SaveBMP(ctx->frame, filename);
        }
#ifdef HAVE_PNG
        else if (filetype && !strcasecmp(filetype, "png")) {
            SDL_Surface *shot = SDL_PNGFormatAlpha(ctx->frame);
            ret = SDL_SavePNG(shot, filename);
            SDL_FreeSurface(shot);
        }
#endif
    }
    return ret;
}

#else /* ! HAVE_SDL */


struct eonx_console_ {
    void *useless;
};

int EONx_ConsoleStartup(void)
{
    return 0;
}

int EONx_ConsoleShutdown(void)
{
    return 0;
}

EONx_Console *eon_ConsoleCreate(EON_uInt sw, EON_uInt sh,
                                 EON_Float ar, EON_Float fov,
                                 EON_uInt flags)
{
    return NULL;
}

EONx_Console *EONx_ConsoleNew(EON_uInt sw, EON_uInt sh, EON_Float fov)
{
    return NULL;
}

void *EONx_ConsoleDelete(EONx_Console *ctx)
{
    return NULL;
}

EON_Cam *EONx_ConsoleGetCamera(EONx_Console *ctx)
{
    return NULL;
}

int EONx_ConsoleNextEvent(EONx_Console *ctx)
{
    return 0;
}

int EONx_ConsoleSetPalette(EONx_Console *ctx, const uint8_t *palette, int numcolors)
{
    return 0;
}

const EON_Frame* EONx_ConsoleGetFrame(EONx_Console *ctx)
{
    return NULL;
}

int EONx_ConsoleClearFrame(EONx_Console *ctx)
{
    return 0;
}

int EONx_ConsoleShowFrame(EONx_Console *ctx)
{
    return 0;
}

const char *EONx_ConsoleGetError(EONx_Console *ctx)
{
    return NULL;
}

int EONx_ConsoleSaveFrame(EONx_Console *ctx,
                          const char *filename, const char *filetype)
{
    return -1;
}

#endif /* HAVE_SDL */

/* some code is indipendent from SDL */

const char *EONx_ConsoleMakeName(EONx_Console *ctx,
                                 char *buf, size_t len,
                                 const char *pfx, const char *ext)
{
    char timebuf[256] = { '\0' }; // XXX
    time_t now = time(0);
    struct tm res;
    localtime_r(&now, &res);
    strftime(timebuf, sizeof(timebuf), "%F_%T", &res);
    snprintf(buf, len, "%s_%s.%s", pfx, timebuf, ext);
    return buf;
}

/* vim: set ts=4 sw=4 et */
/* EOF */

