/**************************************************************************
 * eon3dx_reader.c -- Eon3D eXtension and companion tools                 *
 *                 -- Models/Mesh/Texture readers.                        *
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

#include "eon3dx_reader.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* yes, that's just an (ugly) workaround patch */
#ifdef RPLY_FOUND
#define HAVE_RPLY 1
#endif /* RPLY_FOUND */


#ifdef HAVE_RPLY

#include <rply.h>


enum {
    PLY_N = -1, /* meaning NULL */
    PLY_X =  0,
    PLY_Y =  1,
    PLY_Z =  2
};


typedef struct eonx_plycontext_ {
    EON_Obj  *Obj;

    EON_uInt    Indexes[3];
    EON_sInt    IndexNum;

    EON_Float   Coords[3];
    EON_sInt    CoordsNum;

    EON_uInt    VerticesNum;
	EON_uInt    VertexIdx;
    EON_uInt    FacesNum;
    EON_uInt    FaceIdx;
} eonx_PLYContext;


static int loadVertex(p_ply_argument argument)
{
    eonx_PLYContext *ctx = NULL;
    long ax = PLY_N;

    ply_get_argument_user_data(argument, (void **)&ctx, &ax);
    ctx->Coords[ax] = ply_get_argument_value(argument);
    ctx->CoordsNum++;

    if (ctx->CoordsNum == 3) {
        EON_Vertex *v = &(ctx->Obj->Vertices[ctx->VertexIdx]);
        ctx->VertexIdx++;
        /* FIXME: cleanup */
        v->x = ctx->Coords[PLY_X];
        v->y = ctx->Coords[PLY_Y];
        v->z = ctx->Coords[PLY_Z];
        ctx->CoordsNum = 0;
    }
    return 1;
}

static int loadFace(p_ply_argument argument)
{
    long length = 0, idx = 0;
    eonx_PLYContext *ctx = NULL;

    ply_get_argument_user_data(argument, (void **)&ctx, NULL);
    ply_get_argument_property(argument, NULL, &length, &idx);

    if (idx >= 0 && idx <= 2) {
        int N = ply_get_argument_value(argument);
        EON_Vertex *v = &(ctx->Obj->Vertices[N]);
        EON_Face *f = &(ctx->Obj->Faces[ctx->FaceIdx]);
        f->Vertices[idx] = v;
        if (idx == 2) {
            ctx->FaceIdx++;
        }
    }
    return 1;
}

static void eonx_PLYSetup(eonx_PLYContext *ctx, p_ply ply,
                          const char *fileName)
{
    long V = 0, T = 0;

    V = ply_set_read_cb(ply, "vertex", "x", loadVertex, ctx, PLY_X);
        ply_set_read_cb(ply, "vertex", "y", loadVertex, ctx, PLY_Y);
        ply_set_read_cb(ply, "vertex", "z", loadVertex, ctx, PLY_Z);
    T = ply_set_read_cb(ply, "face", "vertex_indices",
                        loadFace, ctx, 0);

    ctx->VerticesNum = V;
    ctx->FacesNum = T;

    return;
}

static EON_Obj *eonx_PLYRead(p_ply ply, const char *filename, EON_Mat *mat)
{
    eonx_PLYContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    
    ply_read_header(ply);

    eonx_PLYSetup(&ctx, ply, filename);

    ctx.Obj = EON_ObjCreate(ctx.VerticesNum, ctx.FacesNum);
    if (ctx.Obj) {
        ply_read(ply);
        EON_ObjSetMat(ctx.Obj, mat, 1);
        EON_ObjCalcNormals(ctx.Obj);
    }
    return ctx.Obj;
}


EON_Obj *EONx_ReadPLYObj(const char *filename, EON_Mat *mat)
{
    EON_Obj *obj = NULL;
    p_ply ply = ply_open(filename, NULL, 0, NULL);
    if (ply) {
        obj = eonx_PLYRead(ply, filename, mat);
        ply_close(ply);
    } 
    return obj;
}

#else /* ! HAVE_RPLY */

EON_Obj *EONx_ReadPLYObj(const char *filename, EON_Mat *mat)
{
    return NULL;
}

#endif /* HAVE_RPLY */

/******************************************************************************
taken from
Plush Version 1.2
read_pcx.c
PCX Texture Reader
Copyright (c) 1996-2000, Justin Frankel
******************************************************************************/

static EON_uInt eon_HiBit(EON_uInt16 x)
{
    EON_uInt i = 16, mask = 1<<15;
    while (mask) {
        if (x & mask)
            return i;
        mask >>= 1;
        i--;
    }
    return 0;
}

static EON_uInt eon_OptimizeImage(EON_uChar *pal,
                                  EON_uChar *data, EON_uInt32 len)
{
    EON_uChar colors[256], *dd = data;
    EON_uChar remap[256];
    EON_sInt32 lastused, firstunused;
    EON_uInt32 x;
    memset(colors,0,256);
    for (x = 0; x < len; x ++)
        colors[(EON_uInt) *dd++] = 1;
    lastused = -1;
    for (x = 0; x < 256; x ++)
        remap[x] = (EON_uChar)x;
    lastused = 255;
    firstunused = 0;
    for (;;) {
        while (firstunused < 256 && colors[firstunused])
            firstunused++;
        if (firstunused > 255)
            break;
        while (lastused >= 0 && !colors[lastused])
            lastused--;
        if (lastused < 0)
            break;
	    if (lastused <= firstunused)
            break;
        pal[firstunused*3  ] = pal[lastused*3  ];
        pal[firstunused*3+1] = pal[lastused*3+1];
        pal[firstunused*3+2] = pal[lastused*3+2];
        colors[lastused] = 0;
        colors[firstunused] = 1;
	    remap[lastused] = (EON_uChar) firstunused;
    }
    x = len;
    while (x--) {
        EON_uChar value = remap[(EON_uInt) *data];
        *data++ = value;
    }
    return (lastused+1);
}

static EON_sInt eon_ReadPCX(char *filename, EON_uInt16 *width, EON_uInt16 *height,
                            EON_uChar **pal, EON_uChar **data)
{
    EON_uInt16 sx, sy, ex, ey;
    FILE *fp = fopen(filename,"rb");
    EON_uChar *data2;
    if (!fp)
        return -1;
    fgetc(fp);
    if (fgetc(fp) != 5) {
        fclose(fp);
        return -2;
    }
    if (fgetc(fp) != 1) {
        fclose(fp);
        return -2;
    }
    if (fgetc(fp) != 8) {
        fclose(fp);
        return -3;
    }
    sx  = fgetc(fp);
    sx |= fgetc(fp)<<8;
    sy  = fgetc(fp);
    sy |= fgetc(fp)<<8;
    ex  = fgetc(fp);
    ex |= fgetc(fp)<<8;
    ey  = fgetc(fp);
    ey |= fgetc(fp)<<8;
    *width = ex - sx + 1;
    *height = ey - sy + 1;
    fseek(fp,128,SEEK_SET);
    if (feof(fp)) {
        fclose(fp);
        return -4;
    }
    *data = malloc((*width) * (*height));
    if (!*data) {
        fclose(fp);
        return -128;
    }
    sx = *height;
    data2 = *data;
    do {
        int xpos = 0;
        do {
            char c = fgetc(fp);
            if ((c & 192) == 192) {
                char oc = fgetc(fp);
                c &= ~192;
                do {
                  *(data2++) = oc;
                  xpos++;
                } while (--c && xpos < *width);
            } else {
                *(data2++) = c;
                xpos++;
            }
        } while (xpos < *width);
    } while (--sx);
    if (feof(fp)) {
        fclose(fp);
        free(*data);
        return -5;
    }
    fseek(fp,-769,SEEK_END);
    if (fgetc(fp) != 12) {
        fclose(fp);
        free(*data);
        return -6;
    }
    *pal = malloc(3 * 256);
    if (!*pal) {
        fclose(fp);
        free(*data);
        return -7;
    }
    fread(*pal,3,256,fp);
    fclose(fp);
    return 0;
}

static void eon_RescaleImage(EON_uChar *in, EON_uChar *out, EON_uInt inx,
                             EON_uInt iny, EON_uInt outx, EON_uInt outy)
{
    EON_uInt x;
    EON_uInt32 X, dX, dY, Y;
    dX = (inx<<16) / outx;
    dY = (iny<<16) / outy;
    Y = 0;
    do {
        EON_uChar *ptr = in + inx*(Y>>16);
        X = 0;
        Y += dY;
        x = outx;
        do {
            *out++ = ptr[X>>16];
            X += dX;
        } while (--x);
    } while (--outy);
}


EON_Texture *EON_ReadPCXTex(char *fn, EON_Bool rescale, EON_Bool optimize)
{
    EON_uChar *data, *pal;
    EON_uInt16 x, y;
    EON_Texture *t = NULL;
    if (eon_ReadPCX(fn,&x,&y,&pal,&data) < 0)
        return NULL;
    t = malloc(sizeof(EON_Texture));
    if (!t)
        return NULL;
    t->Width = eon_HiBit(x);
    t->Height = eon_HiBit(y);
    if (rescale && (1 << t->Width != x || 1 << t->Height != y)) {
        EON_uChar *newdata = NULL;
        EON_uChar nx = t->Width;
        EON_uChar ny = t->Height;
        if ((1 << t->Width) != x)
            nx++;
        if ((1 << t->Height) != y)
            ny++;
        newdata = malloc((1<<nx)*(1<<ny));
        if (!newdata) {
            EON_TexDelete(t);
            return NULL;
        }
        eon_RescaleImage(data,newdata,x,y,1<<nx,1<<ny);
        free(data);
        data = newdata;
        t->Width = nx;
        t->Height = ny;
        x = 1<<nx;
        y = 1<<ny;
    }
    t->iWidth = x;
    t->iHeight = y;
    t->uScale = (EON_Float) (1<<t->Width);
    t->vScale = (EON_Float) (1<<t->Height);
    if (optimize)
        t->NumColors = eon_OptimizeImage(pal, data,x*y);
    else
        t->NumColors = 256;
    t->Data = data;
    t->PaletteData = pal;
    return t;
}


/* vim: set ts=4 sw=4 et */
/* EOF */

