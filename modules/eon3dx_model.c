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
                               const char *fileName)
{
    EON_Object *obj = NULL;
    const char *ext = NULL;

    RETURN_NULL_IF_INVALID_PARAMS(model, fileName);

    ext = strrchr(fileName, EONx_EXT_SEP);
    if (ext) {
        if (!strcasecmp(ext, EONx_EXT_PLY)) {
            obj = EONx_modelLoadFilePLY(model, fileName);
        } else {
            model->Status = EONx_MODEL_UNSUPPORTED;
        }
    }
    return obj;
}


#ifdef HAVE_RPLY

EON_Object *EONx_modelLoadFilePLY(EONx_Model *model,
                                  const char *fileName)
{
    RETURN_NULL_IF_INVALID_PARAMS(model, fileName);
    
    return NULL;
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

