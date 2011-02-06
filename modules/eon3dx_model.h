/**************************************************************************
 * eon3dx_model.h -- Eon3D eXtension and companion tools                  *
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

#ifndef EON3DX_MODEL_H
#define EON3DX_MODEL_H

#include <eon3d.h>

/** \file eon3dx_model.h
    \brief eon3d eXtension: the model file loader.
*/

/*************************************************************************
 * Model loader                                                          *
 *************************************************************************/

/** \enum model loader status (error) codes. */
typedef enum eonx_modelstatus_ {
    EONx_MODEL_OK             =  0, /**< all clean, no error            */
    EONx_MODEL_INVALID_HANDLE = -1, /**< invalid model reference        */
    EONx_MODEL_INVALID_SOURCE = -2, /**< invalid file name              */
    EONx_MODEL_UNSUPPORTED    = -3, /**< unsupported model format       */
    EONx_MODEL_MALFORMED      = -4  /**< malformed/incorrect model data */
} EONx_ModelStatus; 


/** \struct the opaque model datatype.
   Can carry PLY, 3DS or anything else.
*/
typedef struct eonx_model_ EONx_Model;


/** \fn allocate a new model.

    Allocate a new model handle.

    \return a new model handle if succesfull,
            NULL on error.

    \see EONx_delModel
*/
EONx_Model *EONx_newModel();


/** \fn release a model handle.

    Release a model handle obtained via EONx_newModel().

    \param model a valid EONx_Model handle to release.

    \see EONx_newModel
*/
void EONx_delModel(EONx_Model *model);


/** \fn get the last status code of this model.

    Get the status code associated to the last action performed
    on this model.

    \param model a valid EONx_Model handle.
    \return the status code associated to the last action.
*/
EONx_ModelStatus EONx_modelStatus(EONx_Model *model);


/** \fn load a generic model from a file.

    Load a model from a file, detecting the model format by
    the file extension and automatically selecting the
    right loader for the file.

    \param model a valid EONx_Model handle.
    \param filename the path of the file to load.
    \return an EON_Object representing the loaded model on success,
            NULL on error.

    \see EON_modelStatus
*/
EON_Object *EONx_modelLoadFile(EONx_Model *model, const char *fileName);

/** \fn load a PLY model from a file.

    Load a model from a PLY file. For a description of the
    PLY file format, see
    http://local.wasp.uwa.edu.au/~pbourke/dataformats/ply/

    \param model a valid EONx_Model handle.
    \param filename the path of the PLY file to load.
    \return an EON_Object representing the loaded model on success,
            NULL on error.

    \see EON_modelStatus
*/
EON_Object *EONx_modelLoadFilePLY(EONx_Model *model, const char *fileName);


#endif /* EON3DX_MODEL_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

