/**************************************************************************
 * eon3dx_reader.h -- Eon3D eXtension and companion tools                 *
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
 * Includes code from plush, licensed under the following terms           *
 * Copyright (C) 1996-1998, Justin Frankel                                *
 * Copyright (C) 1998-2000, Nullsoft, Inc.                                *
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

#ifndef EON3DX_READER_H
#define EON3DX_READER_H

#include <eon3d.h>

/**
 * reads an object in the PLY format.
 * @param filename filename of the file containg the object description.
 * @param mat material to apply to the loaded object.
 * @return a pointer to the loaded object. NULL on error.
 */
EON_Obj *EONx_ReadPLYObj(const char *filename, EON_Mat *mat);

/**
 * reads a 8bpp PCX texture.
 * The PCX must be a 8bpp zSoft version 5 PCX. The texture's palette will
 * be optimized, and the texture might be scaled up so that it's dimensions
 * will be a nice power of two.
 * @param filename filename of texture to read
 * @return pointer to the read texture. NULL on error.
 */
EON_Texture *EONx_ReadPCXTex(const char *filename);

#endif /* EON3DX_READER_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

