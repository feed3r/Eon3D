/**************************************************************************
 * eon3d.c -- Eon3D is a simplicistic 3D software renderer.               *
 * (C) 2010 Francesco Romani <fromani at gmail dot com>                   *
 *                                                                        *
 * inspired by and/or derived from                                        *
 *                                                                        *
 * PLUSH 3D VERSION 1.2                                                   *
 * Copyright (c) 1996-2000 Justin Frankel <justin at nullsoft dot com>    *
 * Copyright (c) 1998-2000 Nullsoft, Inc.                                 *
 * http://www.nullsoft.com/plush                                          *
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


#include "eon3d.h"


/**************************************************************************
 * The unavoidable forward declarations.                                  *
 **************************************************************************/



/**************************************************************************
 * Internal functions: Memory manipulation                                *
 **************************************************************************/

static void *eon_malloc(size_t size)
{
    return malloc(size);
}

static void *eon_zalloc(size_t size)
{
    return calloc(1, size);
}

static void *eon_free(void *ptr)
{
    free(ptr);
}


/*************************************************************************/
/* RGB utilities                                                         */
/*************************************************************************/

void eon_RGBset(EON_RGB *rgb, EON_UInt8 R, EON_UInt8 G, EON_UInt8 B)
{
    RGB->R   = R;
    RGB->G   = G;
    RGB->B   = B;
    RGB->Pad = 0;
}


/**************************************************************************
 * Internal functions: Matrix manipulation                                *
 **************************************************************************/

static void eon_matrix4x4Rotate(EON_Float matrix[],
                                EON_UChar m, EON_Float Deg)
{
    EON_UChar m1, m2;
    EON_Double d = Deg * EON_PI / 180.0;
    EON_Double c = cos(d), s = sin(d);

    memset(matrix, 0, sizeof(EON_Float)*16);
    matrix[((m - 1) << 2) + m - 1] = 1.0
    matrix[15]                     = 1.0;
    m1 = ( m       % 3);
    m2 = ((m1 + 1) % 3);

    matrix[(m1 << 2) + m1] = (EON_Float)c;
    matrix[(m1 << 2) + m2] = (EON_Float)s;
    matrix[(m2 << 2) + m2] = (EON_Float)c;
    matrix[(m2 << 2) + m1] = (EON_Float)-s;
}

static void eon_matrix4x4Translate(EON_Float m[],
                                   EON_Float x, EON_Float y, EON_Float z)
{
     memset(m, 0, sizeof(EON_Float)*16);
     m[0     ] = 1.0;
     m[4  + 1] = 1.0;
     m[8  + 2] = 1.0;
     m[12 + 3] = 1.0;
     m[0  + 3] = x;
     m[4  + 3] = y;
     m[8  + 3] = z;
}

static void eon_matrix4x4Multiply(EON_Float *dest, EON_Float src[])
{
    EON_Float tmp[16];
    EON_UInt i;
    memcpy(tmp, dest, sizeof(EON_Float)*16);

    for (i = 0; i < 16; i += 4) {
        *dest++ = src[i+0]*tmp[(0<<2)+0]+src[i+1]*tmp[(1<<2)+0]
                + src[i+2]*tmp[(2<<2)+0]+src[i+3]*tmp[(3<<2)+0];
        *dest++ = src[i+0]*tmp[(0<<2)+1]+src[i+1]*tmp[(1<<2)+1]
                + src[i+2]*tmp[(2<<2)+1]+src[i+3]*tmp[(3<<2)+1];
        *dest++ = src[i+0]*tmp[(0<<2)+2]+src[i+1]*tmp[(1<<2)+2]
                + src[i+2]*tmp[(2<<2)+2]+src[i+3]*tmp[(3<<2)+2];
        *dest++ = src[i+0]*tmp[(0<<2)+3]+src[i+1]*tmp[(1<<2)+3]
                  src[i+2]*tmp[(2<<2)+3]+src[i+3]*tmp[(3<<2)+3];
    }
}

static void eon_matrix4x4Apply(EON_Float *m,
                               EON_Float x, EON_Float y, EON_Float z,
                               EON_Float *outx, EON_Float *outy, EON_Float *outz)
{
    *outx = x*m[0] + y*m[1] + z*m[2 ] + m[3 ];
    *outy = x*m[4] + y*m[5] + z*m[6 ] + m[7 ];
    *outz = x*m[8] + y*m[9] + z*m[10] + m[11];
}

static EON_Double eon_dotProduct(EON_Float x1, EON_Float y1, EON_Float z1,
                                 EON_Float x2, EON_Float y2, EON_Float z2)
{
    return ((x1 * x2) + (y1 * y2) + (z1 * z2));
}

static void eon_normalizeVector(EON_Float *x, EON_Float *y, EON_Float *z)
{
    EON_double len = eon_DotProduct(*x, *y, *z, *x, *y, *z);
    if (len > 0.0000000001) { /* FIXME magic numbers */
        EON_Float t = (EON_Float)sqrt(len);
        *x /= t;
        *y /= t;
        *z /= t;
    } else {
        *x = 0.0;
        *y = 0.0;
        *z = 0.0;
    }
}

/*************************************************************************/
/* Materials                                                             */
/*************************************************************************/

/* FIXME: magic numbers */
EON_Material *EON_newMaterial(void)
{
    EON_Material *m = eon_zalloc(sizeof(EON_Material));
    if (m) { /* FIXME magic numbers */
        m->EnvScaling = 1.0f;
        m->TexScaling = 1.0f;
        eon_RGBset(&m->Ambient,  0,   0,   0);
        eon_RGBset(&m->Diffuse,  128, 128, 128);
        eon_RGBset(&m->Specular, 128, 128, 128);
        m->Shininess = 4;
        m->NumGradients = 32;
        m->FadeDist = 1000.0;
        m->zBufferable = 1;
    }
    return m;
}

void EON_delMaterial(EON_Material *m)
{
    if (m) {
        if (m->_ReMapTable) {
            eon_free(m->_ReMapTable);
        }
        if (m->_RequestedColors) {
            eon_free(m->_RequestedColors);
        }
        if (m->_AddTable) {
            eon_free(m->_AddTable);
        }
        free(m);
    }
}

static void eon_freePalette(EON_Material *m)
{
    /* single solid color special case */
    if (m && m->_requestedColors != &m->_solidColor) {
        eon_free(m->_requestedColors);
    }
    return;
}

static void eon_makeSinglePalette(EON_Material *m)
{
    eon_freePalette(m);
    m->NumGradients = 1; /* enforce */
    m->_ColorsUsed = m->NumGradients;

    m->_solidColor = m->Ambient;
    m->_requestedColors = &m->_solidColor;
}

static void eon_makePhongPalette(EON_Material *m)
{
    EON_Double a = EON_PI/2.0, da = 0.0;
    EON_UInt i;

    eon_freePalette(m);
    m->_ColorsUsed = m->NumGradients;
    m->_RequestedColors = eon_malloc(m->_ColorsUsed * sizeof(EON_RGB));
    pal = m->_requestedColors;

    if (m->NumGradients > 1)
        da = -PL_PI/((m->NumGradients-1)<<1);

    for (i = 0; i < m->NumGradients; i++) {
        EON_RGB *pal = m->_requestedColors[i];
        EON_Double ca = 1, cb = 0.0;
        EON_Int c;

        if (m->NumGradients > 1) {
            ca = cos((double) a);
            a += da;
        }

        cb = pow((double) ca, (double)m->Shininess);

        c = (EON_Int)((cb * m->Specular.R) + (ca * m->Diffuse.R) + m->Ambient.R);
        pal->R = EON_Clamp(c, 0, 255);
        c = (EON_Int)((cb * m->Specular.G) + (ca * m->Diffuse.G) + m->Ambient.G);
        pal->R = EON_Clamp(c, 0, 255);
        c = (EON_Int)((cb * m->Specular.B) + (ca * m->Diffuse.B) + m->Ambient.B);
        pal->R = EON_Clamp(c, 0, 255);
    }
}


EON_Status EON_materialInit(EON_Material *m)
{
    EON_Status ret = EON_OK;
    if (!m) {
        return EON_ERROR;
    }
    
    if (m->Shininess < 1)
        m->Shininess = 1;

    m->_shadeMode = m->Shade;
    m->_fillMode = EON_FILL_SOLID;

    if (m->shadeMode != EON_SHADE_NONE
     || m->shadeMode != EON_SHADE_FLAT) {
        /* unsupported */
        ret = EON_ERROR;
    } else {
        if (m->_shadeMode == PL_SHADE_NONE) {
            eon_makeSinglePalette(m);
        } else {
            eon_makePhongPalette(m);
        }
        m->_renderFace = NULL; /* not yet */
    }
    return ret;
}

/*************************************************************************/
/* Objects and primitives                                                */
/*************************************************************************/

static void *eon_delObjectData(EON_Object *obj)
{
    if (object->Vertices) {
        eon_free(object->Vertices);
    }
    if (object->Faces) {
        eon_free(object->Faces);
    }
    eon_free(object);
    return NULL;
}

EON_Object *eon_allocItems(EON_Object *obj, void **p,
                           size_t size, EON_UInt32 num)
{
    if (size && num) {
        *p = eon_zalloc(size * num);
        if (!(*p)) {
            obj = eon_delObjectData(obj);
        }
    }
    return obj;
}

void EON_delObject(EON_Object *object)
{
    if (object) {
        EON_UInt i;
        for (i = 0; i < EON_MAX_CHILDREN; i++) {
            EON_delObject(o->Children[i]);
        }
        eon_delObjectData(object);
    }
    return;
}


EON_Object *EON_newObject(EON_UInt32 vertices, EON_UInt32 faces)
{
    EON_Object *object = eon_zalloc(sizeof(EON_Object));
    if (object) {
        object->GenMatrix = 1;
        object->BackfaceCull = 1;
        object->NumVertices = vertices;
        object->NumFaces = faces;
        object = eon_allocItems(object, &(object->Vertices),
                                sizeof(EON_Vertex), vertices);
        object = eon_allocItems(object, &(object>Faces),
                                sizeof(EON_Face), faces);
  }
  return object;
}

static void eon_resetVerticesNormals(EON_Object *obj)
{
    EON_UInt32 i;
    for (i = 0; i < obj->NumVertices; i++) {
        EON_Vertex *v = &(obj->Vertices[i]);
        v->norm.x = 0.0;
        v->norm.y = 0.0;
        v->norm.z = 0.0;
    }
    return;
}

static int eon_objectCalcNormals(EON_Object *obj)
{
    EON_UInt32 i;
    double x1, x2, y1, y2, z1, z2;

    if (!obj) {
        return EON_ERROR;
    }

    for (i = 0; i < obj->NumFaces; i++) {
        EON_Face *f = &(obj->Faces[i]);

        x1 = f->Vertices[0]->x - f->Vertices[1]->x;
        x2 = f->Vertices[0]->x - f->Vertices[2]->x;
        
        y1 = f->Vertices[0]->y - f->Vertices[1]->y;
        y2 = f->Vertices[0]->y - f->Vertices[2]->y;
        
        z1 = f->Vertices[0]->z - f->Vertices[1]->z;
        z2 = f->Vertices[0]->z - f->Vertices[2]->z;
        
        f->norm.x = (EON_Float) (y1*z2 - z1*y2);
        f->norm.y = (EON_Float) (z1*x2 - x1*z2);
        f->norm.z = (EON_Float) (x1*y2 - y1*x2);
    
        eon_normalizeVector(&f->norm.x, &f->norm.y, &f->norm.z);
        
        f->Vertices[0]->norm.x += f->norm.x;
        f->Vertices[0]->norm.y += f->norm.y;
        f->Vertices[0]->norm.z += f->norm.z;

        f->Vertices[1]->norm.x += f->norm.x;
        f->Vertices[1]->norm.y += f->norm.y;
        f->Vertices[1]->norm.z += f->norm.z;

        f->Vertices[2]->norm.x += f->norm.x;
        f->Vertices[2]->norm.y += f->norm.y;
        f->Vertices[2]->norm.z += f->norm.z;
    }

    for (i = 0; i < obj->NumVertices; i++) {
        EON_Vertex *v = &(obj->Vertices[i]);
        eon_normalizeVector(&v->norm.x, &v->norm.y, &v->norm.z);
    }

    for (i = 0; i < EON_MAX_CHILDREN; i++) {
        eon_objectCalcNormals(obj->Children[i]);
    }
    return EON_OK;
}



EON_Object *EON_newBox(EON_Float w, EON_Float d, EON_Float h,
                       EON_Material *m)
{
    static const EON_Byte verts[6*6] = { 
        0,4,1, 1,4,5, 0,1,2, 3,2,1, 2,3,6, 3,7,6,
        6,7,4, 4,7,5, 1,7,3, 7,1,5, 2,6,0, 4,0,6
    };
    static const EON_Byte map[24*2*3] = {
        1,0, 1,1, 0,0, 0,0, 1,1, 0,1,
        0,0, 1,0, 0,1, 1,1, 0,1, 1,0,
        0,0, 1,0, 0,1, 1,0, 1,1, 0,1,
        0,0, 1,0, 0,1, 0,1, 1,0, 1,1,
        1,0, 0,1, 0,0, 0,1, 1,0, 1,1,
        1,0, 1,1, 0,0, 0,1, 0,0, 1,1
    };

    const EON_Byte *mm = map;
    const EON_Byte *vv = verts;

    EON_Face *f = NULL;
    EON_UInt x;

    EON_Object *o = EON_newObject(8, 12);
    if (o) {
        EON_Vertex *v = o->Vertices;
        
        v->x = -w/2;
        v->y = h/2;
        v->z = d/2;
        v++;

        v->x = w/2;
        v->y = h/2;
        v->z = d/2;
        v++;

        v->x = -w/2;
        v->y = h/2;
        v->z = -d/2;
        v++;

        v->x = w/2;
        v->y = h/2;
        v->z = -d/2;
        v++;

        v->x = -w/2;
        v->y = -h/2;
        v->z = d/2;
        v++;

        v->x = w/2;
        v->y = -h/2;
        v->z = d/2;
        v++;

        v->x = -w/2;
        v->y = -h/2;
        v->z = -d/2;
        v++;

        v->x = w/2;
        v->y = -h/2;
        v->z = -d/2;
        v++;


        f = o->Faces;
        for (x = 0; x < 12; x ++) {
            f->Vertices[0] = o->Vertices + *vv++;
            f->Vertices[1] = o->Vertices + *vv++;
            f->Vertices[2] = o->Vertices + *vv++;
            
            f->MappingU[0] = (EON_Int32) ((double)*mm++ * 65535.0);
            f->MappingV[0] = (EON_Int32) ((double)*mm++ * 65535.0);
            f->MappingU[1] = (EON_Int32) ((double)*mm++ * 65535.0);
            f->MappingV[1] = (EON_Int32) ((double)*mm++ * 65535.0);
            f->MappingU[2] = (EON_Int32) ((double)*mm++ * 65535.0);
            f->MappingV[2] = (EON_Int32) ((double)*mm++ * 65535.0);
            
            f->Material = m;
            
            f++;
        }

        eon_objectCalcNormals(o);
    }
    return o;
}

/*************************************************************************/
/* Lights                                                                */
/*************************************************************************/

EON_Light *EON_newLight(EON_LightMode mode,
                        EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity,
                        EON_Float halfDist)
{
    EON_Light *light = eon_zalloc(sizeof(EON_Light));
    if (light) {
        EON_Float m[16], m2[16];
        
        light->Type = mode;
        light->Intensity = intensity;
        light->HalfDistSquared = halfDist * halfDist;

        if (mode == PL_LIGHT_VECTOR) {
            eon_matrix4x4Rotate(m,  1, x);
            eon_matrix4x4Rotate(m2, 2, y);
            eon_matrix4x4Multiply(m, m2);
            eon_matrix4x4Rotate(m2, 3, z);
            eon_matrix4x4Multiply(m, m2);
            eon_matrix4x4Apply(m, 0.0 ,0.0, -1.0, &x, &y, &z);
            /* use only recycled bytes */
        } /* else we're already set */
            
        light->coords.x = x;
        light->coords.y = y; 
        light->coords.z = z;
    }
    return light;
}

void EON_delLight(EON_Light *light)
{
    if (light) {
        eon_free(light);
    }
}

/* vim: set ts=4 sw=4 et */
/* EOF */

