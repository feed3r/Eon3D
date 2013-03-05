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


EON_Obj *EON_ReadPLYObj(const char *filename, EON_Mat *mat)
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

EON_Obj *EON_ReadPLYObj(const char *filename, EON_Mat *mat)
{
    return NULL;
}

#endif /* HAVE_RPLY */


/* vim: set ts=4 sw=4 et */
/* EOF */

