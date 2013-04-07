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

#include "eon3dx_console.h"


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* yes, that's just an (ugly) workaround patch */
#ifdef SDL_FOUND
#define HAVE_SDL 1
#endif /* SDL_FOUND */


/**************************************************************************
 * SDL backend Console implementation.                                    *
 **************************************************************************/

#ifdef HAVE_SDL

#define EONx_SDL_TAG    "EONSDL"

#include <SDL.h>

// FIXME BPP ambiguitiy (depth/BPP/bpp)

enum {
    TRUECOLOR_BPP   = 4,
    TRUECOLOR_DEPTH = 32 // XXX
};

struct eonx_console_ {
    EON_Cam *cam;
    uint8_t *fb;
    int width;
    int height;
    int bpp;
    EON_ZBuffer *zb;
    SDL_Surface *screen;
    SDL_Surface *frame;
};


static inline int EONx_ConsoleFrameSize(EONx_Console *ctx)
{
    return ctx->width * ctx->height * ctx->bpp;
}

static inline int EONx_ConsoleZBufferSize(EONx_Console *ctx)
{
    return ctx->width * ctx->height * sizeof(EON_ZBuffer);
}

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
    ctx->screen = SDL_SetVideoMode(ctx->width, ctx->height,
                                   TRUECOLOR_DEPTH, flags);
    /*
     * always truecolor on screen, even if internally we use a palette.
     * SDL blitting will take care of the conversion.
     */
    if (ctx->screen) {
        ctx->frame = SDL_CreateRGBSurfaceFrom(ctx->fb,
                                              ctx->width, ctx->height,
                                              ctx->bpp,
                                              ctx->width * TRUECOLOR_BPP,
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
        free(ctx->fb);
        ctx->fb = NULL;
    }
    if (ctx->zb) {
        free(ctx->zb);
        ctx->zb = NULL;
    }
    free(ctx);
    return NULL;
}

EONx_Console *EONx_ConsoleNew(EON_uInt sw, EON_uInt sh, EON_Float fov)
{
    return EONx_ConsoleCreate(sw, sh, 1.0, fov,
                              EONx_CONSOLE_FLAG_ZBUFFER);
}

EONx_Console *EONx_ConsoleCreate(EON_uInt sw, EON_uInt sh,
                                 EON_Float ar, EON_Float fov,
                                 EON_uInt32 flags)
{
    int err = -1;
    EONx_Console *ctx = calloc(1, sizeof(*ctx));
    if (ctx) {
        ctx->width = sw;
        ctx->height = sh;
        ctx->bpp = TRUECOLOR_DEPTH; // XXX
        if (flags & EONx_CONSOLE_FLAG_ZBUFFER) {
            ctx->zb = malloc(EONx_ConsoleZBufferSize(ctx));
        }
        ctx->fb = malloc(EONx_ConsoleFrameSize(ctx));
        if (ctx->fb) {
            ctx->cam = EON_CamCreate(sw, sh, ar, fov,
                                     ctx->fb, ctx->zb);
            if (ctx->cam) {
                err = EONx_ConsoleInitSurfaces(ctx);
            }

        }
    }
    if (err) {
        ctx = EONx_ConsoleCleanup(ctx);
    }
    return ctx;
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
        // clear framebuffer for next frame
        memset(ctx->fb, 0, EONx_ConsoleFrameSize(ctx));
        if (ctx->zb) {
            memset(ctx->zb, 0, EONx_ConsoleZBufferSize(ctx));
        }
        err = 0;
    }
    return err;
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


int EONx_ConsoleWaitKey(EONx_Console *ctx)
{
    int ret = 0;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                ret = -1;
                break;
            default:
                ret = 0;
                break;
        }
    }

    return ret;
}


const char *EONx_ConsoleGetError(EONx_Console *ctx)
{
    return SDL_GetError();
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

EONx_Console *EONx_ConsoleCreate(EON_uInt sw, EON_uInt sh,
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

int EONx_ConsoleWaitKey(EONx_Console *ctx)
{
    return 0;
}

int EONx_ConsoleSetPalette(EONx_Console *ctx, const uint8_t *palette, int numcolors)
{
    return 0;
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

#endif /* HAVE_SDL */

/* vim: set ts=4 sw=4 et */
/* EOF */

