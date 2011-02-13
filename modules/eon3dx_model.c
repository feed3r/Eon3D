/**************************************************************************
 * eon3dx_model.c -- Eon3D eXtension and companion tools                  *
 *                -- Model file loader.                                   *
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
#include <string.h>
#include <strings.h>


#include "eon3dx_model.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

/* yes, that's just an (ugly) workaround patch */
#ifdef RPLY_FOUND
#define HAVE_RPLY 1
#endif /* RPLY_FOUND */


#define EONx_EXT_SEP        '.'
#define EONx_EXT_PLY        ".ply"
#define EONx_MODEL_TAG      "EONMOD"
#define EONx_PLY_TAG        "EONPLY"


/* opaque. Can use PLY, 3DS or anything else. */
struct eonx_model_ {
    EONx_ModelStatus    Status;
};



EONx_Model *EONx_newModel()
{
    EONx_Model *model = EON_zalloc(sizeof(EONx_Model));
    if (model) {
        model->Status = EONx_MODEL_OK;
    }
    return model;
}

void EONx_delModel(EONx_Model *model)
{
    EON_free(model);
}

EONx_ModelStatus EONx_modelStatus(EONx_Model *model)
{
    EONx_ModelStatus status = EONx_MODEL_INVALID_HANDLE;
    if (model) {
        status = model->Status;
    }
    return status;
}

#define RETURN_NULL_IF_INVALID_PARAMS(MODEL, FILENAME) do { \
    if (!(MODEL)) { \
        return NULL; \
    } \
    if (!(FILENAME)) { \
        (MODEL)->Status = EONx_MODEL_INVALID_SOURCE; \
        return NULL; \
    } \
} while (0)



EON_Object *EONx_modelLoadFile(EONx_Model *model,
                               const char *fileName,
                               EON_Material *material)
{
    EON_Object *obj = NULL;
    const char *ext = NULL;

    RETURN_NULL_IF_INVALID_PARAMS(model, fileName);

    ext = strrchr(fileName, EONx_EXT_SEP);
    if (ext) {
        if (!strcasecmp(ext, EONx_EXT_PLY)) {
            obj = EONx_modelLoadFilePLY(model, fileName, material);
        } else {
            model->Status = EONx_MODEL_UNSUPPORTED;
        }
    }
    return obj;
}


#ifdef HAVE_RPLY

#include <rply.h>


enum {
    PLY_N = -1, /* meaning NULL */
    PLY_X =  0,
    PLY_Y =  1,
    PLY_Z =  2
};


typedef struct eonx_plycontext_ {
    EON_Object  *Obj;

    EON_UInt    Indexes[EON_DIMENSIONS];
    EON_Int     IndexNum;

    EON_Float   Coords[EON_DIMENSIONS];
    EON_Int     CoordsNum;

    EON_UInt    VertexesNum;
	EON_UInt    VertexIdx;
    EON_UInt    TrianglesNum;
    EON_UInt    FaceIdx;
} eonx_PLYContext;


static int loadVertex(p_ply_argument argument)
{
    eonx_PLYContext *ctx = NULL;
    long ax = PLY_N;

    ply_get_argument_user_data(argument, (void **)&ctx, &ax);
    ctx->Coords[ax] = ply_get_argument_value(argument);
    ctx->CoordsNum++;

    if (ctx->CoordsNum == EON_DIMENSIONS) {
        EON_Vertex *v = &(ctx->Obj->Vertexes[ctx->VertexIdx]);
        ctx->VertexIdx++;
        eon_PointSet(&(v->Coords), ctx->Coords[PLY_X],
                                   ctx->Coords[PLY_Y],
                                   ctx->Coords[PLY_Z]);
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
    ctx->Indexes[idx] = ply_get_argument_value(argument);

    if (ctx->IndexNum == EON_DIMENSIONS) {
        EON_Face *f = &(ctx->Obj->Faces[ctx->FaceIdx]);
        ctx->FaceIdx++;

        f->Vertexes[0] = &(ctx->Obj->Vertexes[ctx->Indexes[0]]);
        f->Vertexes[1] = &(ctx->Obj->Vertexes[ctx->Indexes[1]]);
        f->Vertexes[2] = &(ctx->Obj->Vertexes[ctx->Indexes[2]]);

        ctx->IndexNum = 0;
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
    
    EON_log(EONx_PLY_TAG, EON_LOG_DEBUG,
            "%s: %lix%li\n", fileName, V, T);
    ctx->VertexesNum = V;
    ctx->TrianglesNum = T;

    return;
}

static EON_Object *eonx_PLYRead(p_ply ply, const char *fileName)
{
    EON_Object *obj = NULL;
    int ret = 0;

    ret = ply_read_header(ply);
    if (!ret) {
        EON_log(EONx_PLY_TAG, EON_LOG_ERROR,
                "unable to parse the header of the model [%s]",
                fileName);
    } else {
        eonx_PLYContext ctx;
        memset(&ctx, 0, sizeof(ctx));

        eonx_PLYSetup(&ctx, ply, fileName);

        ctx.Obj = EON_newObject(V, T, material); 
        if (!ctx.Obj) {
            EON_log(EONx_PLY_TAG, EON_LOG_ERROR,
                    "unable to allocate the object");
        } else {
            ret = ply_read(ply);
            if (!ret) {
                EON_log(EONx_PLY_TAG, EON_LOG_ERROR,
                        "failed to parse the body of the model [%s]",
                        fileName);
                EON_delObject(ctx.Obj);
            } else {
                EON_objectCalcNormals(ctx.Obj);
                obj = Ctx.Obj;
            }
        }
    }
    return obj;
}


EON_Object *EONx_modelLoadFilePLY(EONx_Model *model,
                                  const char *fileName,
                                  EON_Material *material)
{
    EON_Object *obj = NULL;
    p_ply ply = NULL;

    RETURN_NULL_IF_INVALID_PARAMS(model, fileName);

    ply = ply_open(fileName, NULL);
    if (!ply) {
        EON_log(EONx_PLY_TAG, EON_LOG_ERROR,
                "unable to open the model [%s]", fileName);
    } else {
        obj = eonx_PLYRead(ply, fileName);
        ply_close(ply);
    } 
    return obj;
}

#else /* ! HAVE_RPLY */

EON_Object *EONx_modelLoadFilePLY(EONx_Model *model,
                                  const char *fileName)
{
    RETURN_NULL_IF_INVALID_PARAMS(model, fileName);
    
    model->Status = EONx_MODEL_UNSUPPORTED;

    return NULL;
}

#endif /* HAVE_RPLY */


/* vim: set ts=4 sw=4 et */
/* EOF */

