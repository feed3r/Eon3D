/**************************************************************************
 * eon3d.c -- Eon3D is a simplistic 3D software renderer.                 *
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
    rgb->R   = R;
    rgb->G   = G;
    rgb->B   = B;
    rgb->Pad = 0;
}


/**************************************************************************
 * Internal functions: Matrix manipulation                                *
 **************************************************************************/

static void eon_matrix4x4Rotate(EON_Float matrix[],
                                EON_Byte m, EON_Float Deg)
{
    EON_Byte m1, m2;
    EON_Double d = Deg * EON_PI / 180.0;
    EON_Double c = cos(d), s = sin(d);

    memset(matrix, 0, sizeof(EON_Float)*16);
    matrix[((m - 1) << 2) + m - 1] = 1.0;
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
                + src[i+2]*tmp[(2<<2)+3]+src[i+3]*tmp[(3<<2)+3];
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

static EON_Double eon_dotProduct(EON_Vector *V1, EON_Vector *V2)
{
    return ((V1->X * V2->X) + (V1->Y * V2->Y) + (V1->Z * V2->Z));
}

static void eon_normalizeVector(EON_Vector *V)
{
    EON_Double len = eon_dotProduct(V, V);
    if (len > 0.0000000001) { /* FIXME magic numbers */
        EON_Float t = (EON_Float)sqrt(len);
        V->X /= t;
        V->Y /= t;
        V->Z /= t;
    } else {
        V->X = 0.0;
        V->Y = 0.0;
        V->Z = 0.0;
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
        m->ZBufferable = EON_True;
    }
    return m;
}

void EON_delMaterial(EON_Material *m)
{
    if (m) {
        if (m->_reMapTable) {
            eon_free(m->_reMapTable);
        }
        if (m->_requestedColors) {
            eon_free(m->_requestedColors);
        }
        if (m->_addTable) {
            eon_free(m->_addTable);
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
    m->_colorsUsed = m->NumGradients;

    m->_solidColor = m->Ambient;
    m->_requestedColors = &m->_solidColor;
}

static void eon_makePhongPalette(EON_Material *m)
{
    EON_Double a = EON_PI/2.0, da = 0.0;
    EON_UInt i;

    eon_freePalette(m);
    m->_colorsUsed = m->NumGradients;
    m->_requestedColors = eon_malloc(m->_colorsUsed * sizeof(EON_RGB));

    if (m->NumGradients > 1)
        da = -EON_PI/((m->NumGradients-1)<<1);

    for (i = 0; i < m->NumGradients; i++) {
        EON_RGB *pal = &(m->_requestedColors[i]);
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

    if (m->Shade != EON_SHADE_NONE
     || m->Shade != EON_SHADE_FLAT) {
        /* unsupported */
        ret = EON_ERROR;
    } else {
        if (m->Shade == EON_SHADE_NONE) {
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
    if (obj->Vertices) {
        eon_free(obj->Vertices);
    }
    if (obj->Faces) {
        eon_free(obj->Faces);
    }
    eon_free(obj);
    return NULL;
}

EON_Object *eon_objectAllocItems(EON_Object *obj, void **p,
                           size_t size, EON_UInt32 num)
{
    if (obj && size && num) {
        *p = eon_zalloc(size * num);
        if (!(*p)) {
            obj = eon_delObjectData(obj);
        }
    }
    return obj;
}

void EON_delObject(EON_Object *obj)
{
    if (obj) {
        EON_UInt i;
        for (i = 0; i < EON_MAX_CHILDREN; i++) {
            EON_delObject(obj->Children[i]);
        }
        eon_delObjectData(obj);
    }
    return;
}


EON_Object *EON_newObject(EON_UInt32 vertices, EON_UInt32 faces)
{
    EON_Object *object = eon_zalloc(sizeof(EON_Object));
    if (object) {
        object->GenMatrix    = EON_True;
        object->BackfaceCull = EON_True;
        object->NumVertices  = vertices;
        object->NumFaces     = faces;
        object = eon_objectAllocItems(object, (void**)&(object->Vertices),
                                      sizeof(EON_Vertex), vertices);
        object = eon_objectAllocItems(object, (void**)&(object->Faces),
                                      sizeof(EON_Face), faces);
  }
  return object;
}

static void eon_resetVerticesNormals(EON_Object *obj)
{
    EON_UInt32 i;
    for (i = 0; i < obj->NumVertices; i++) {
        EON_Vertex *v = &(obj->Vertices[i]);
        v->Norm.X = 0.0;
        v->Norm.Y = 0.0;
        v->Norm.Z = 0.0;
    }
    return;
}

typedef struct eon_pointx_ {
    EON_Double X;
    EON_Double Y;
    EON_Double Z;
} EON_PointX;

static void eon_PointSet(EON_Point *P, EON_Float X, EON_Float Y, EON_Float Z)
{
    P->X = X;
    P->Y = Y;
    P->Z = Z;
}

static void eon_PointRAdd(EON_Point *P, const EON_Point *OP)
{
    P->X += OP->X;
    P->Y += OP->Y;
    P->Z += OP->Z;
}

static void eon_PointDiff(const EON_Point *P1, const EON_Point *P2,
                          EON_PointX *D)
{
    D->X = P1->X - P2->X;
    D->Y = P1->Y - P2->Y;
    D->Z = P1->Z - P2->Z;
}


static int eon_objectCalcNormals(EON_Object *obj)
{
    EON_UInt32 i;

    if (!obj) {
        return EON_ERROR;
    }

    for (i = 0; i < obj->NumFaces; i++) {
        EON_Face *f = &(obj->Faces[i]);
        EON_PointX d1, d2;

        eon_PointDiff(&(f->Vertices[0]->Coords),
                      &(f->Vertices[1]->Coords), &d1);
        eon_PointDiff(&(f->Vertices[0]->Coords),
                      &(f->Vertices[2]->Coords), &d2);
        
        f->Norm.X = (EON_Float) (d1.Y * d2.Z - d1.Z * d2.Y);
        f->Norm.Y = (EON_Float) (d1.Z * d2.X - d1.X * d2.Z);
        f->Norm.Z = (EON_Float) (d1.X * d2.Y - d1.Y * d2.X);
    
        eon_normalizeVector((EON_Vector *)&(f->Norm));
        
        eon_PointRAdd(&(f->Vertices[0]->Norm), &(f->Norm));
        eon_PointRAdd(&(f->Vertices[1]->Norm), &(f->Norm));
        eon_PointRAdd(&(f->Vertices[2]->Norm), &(f->Norm));
    }

    for (i = 0; i < obj->NumVertices; i++) {
        EON_Vertex *v = &(obj->Vertices[i]);
        eon_normalizeVector((EON_Vector *)&(v->Norm));
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

    EON_Object *o = EON_newObject(8, 12);
    if (o) {
        EON_Vertex *v = o->Vertices;
        EON_Face *f = o->Faces;
        EON_UInt x;

        w /= 2;
        h /= 2;
        d /= 2;

        eon_PointSet(&(v[0].Coords), -w,  h,  d);
        eon_PointSet(&(v[1].Coords),  w,  h,  d);
        eon_PointSet(&(v[2].Coords), -w,  h, -d);
        eon_PointSet(&(v[3].Coords),  w,  h, -d);
        eon_PointSet(&(v[4].Coords), -w, -h,  d);
        eon_PointSet(&(v[5].Coords),  w, -h,  d);
        eon_PointSet(&(v[6].Coords), -w, -h, -d);
        eon_PointSet(&(v[7].Coords),  w, -h, -d);

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

        if (mode == EON_LIGHT_VECTOR) {
            eon_matrix4x4Rotate(m,  1, x);
            eon_matrix4x4Rotate(m2, 2, y);
            eon_matrix4x4Multiply(m, m2);
            eon_matrix4x4Rotate(m2, 3, z);
            eon_matrix4x4Multiply(m, m2);
            eon_matrix4x4Apply(m, 0.0 ,0.0, -1.0, &x, &y, &z);
            /* use only recycled bytes */
        } /* else we're already set */
            
        light->Coords.X = x;
        light->Coords.Y = y; 
        light->Coords.Z = z;
    }
    return light;
}

void EON_delLight(EON_Light *light)
{
    if (light) {
        eon_free(light);
    }
    return;
}

/*************************************************************************/
/* Frames                                                                */
/*************************************************************************/

enum {
    EON_RGB_BPP   = 3,    /* Bytes Per Pixel */
    EON_RGB_BLACK = 0xFF  /* per component (XXX) */
};

static size_t eon_FrameRGBSize(EON_Int width, EON_Int height)
{
    return (EON_RGB_BPP * width * height);
}

EON_Frame *EON_newFrame(EON_Int width, EON_Int height)
{
    size_t size = eon_FrameRGBSize(width, height);
    EON_Byte *data = eon_malloc(sizeof(EON_Frame) + size);
    EON_Frame *frame = NULL;
    if (data) {
        /* FIXME: Pixels alignment */
        frame = (EON_Frame *)data;
        frame->Pixels   = data + sizeof(EON_Frame);
        frame->F.Width  = width;
        frame->F.Height = height;
    }
    return frame;
}

void EON_delFrame(EON_Frame *frame)
{
    eon_free(frame);
}


void EON_frameClean(EON_Frame *frame)
{
    if (frame && frame->Pixels) {
        size_t size = eon_FrameRGBSize(frame->F.Width, frame->F.Height);
        memset(frame->Pixels, EON_RGB_BLACK, size);
    }
    return;
}

/*************************************************************************/
/* Camera                                                                */
/*************************************************************************/

EON_Camera *EON_newCamera(EON_Int width, EON_Int height,
                          EON_Float aspectRatio,
                          EON_Float fieldOfView)
{
    EON_Camera *cam = eon_zalloc(sizeof(EON_Camera));
    if (cam) {
        cam->Fov            = fieldOfView;
        cam->AspectRatio    = aspectRatio;
        cam->Sort           = 1; /* FIXME: if ZBuffer Sort=0 */
        cam->ClipBack       = 8.0e30f;
        cam->Screen.Width   = width;
        cam->Screen.Height  = height;
        cam->Clip.Right     = width;
        cam->Clip.Bottom    = height;
        cam->Center.Width   = width / 2;
        cam->Center.Height  = height / 2;
    }
    return cam;
}

void EON_delCamera(EON_Camera *camera)
{
    if (camera) {
        eon_free(camera);
    }
    return;
}

/*************************************************************************/
/* Rendering                                                             */
/*************************************************************************/

enum {
    EON_TRIANGLES_START = 1024,
    EON_LIGHTS_START    = 32
}

typedef struct {
    EON_Face    *Face;
    EON_Float   ZD;
} eon_faceInfo;

typedef struct {
  EON_Light *Light;
  EON_Point Pos;
} eon_lightInfo;

typedef struct {
    void        *D;

    EON_Uint32  itemSize;

    EON_Uint32  size;
    EON_Uint32  used;
} eon_array;

static EON_Status eon_arrayAlloc(eon_array *array,
                                 EON_Uint32 size, EON_Uint32 itemSize)
{
    return EON_Error;
}

static EON_Status eon_arrayFree(eon_array *array)
{
    return EON_Error;
}

static EON_Status eon_arrayResize(eon_array *array, EON_Uint32 size)
{
    return EON_Error;
}

static EON_Status eon_arrayAppend(eon_array *array, const void *item)
{
    return EON_Error;
}

static EON_Status eon_arrayReset(eon_array *array)
{
    EON_Status err = EON_ERROR;
    if (array) {
        array->used = 0;
        err = EON_OK;
    }
    return array;
}


struct eon_renderer_ {
    EON_Camera  *Camera;
    EON_ZBuffer *ZBuffer;

    eon_array   Faces;
    eon_array   Lights;

    EON_Float   CMatrix[16];
    EON_Uint32  TriStats;
};

static void *eon_rendererDestroy(EON_Renderer *rend)
{
    EON_delRenderer(rend);
    return NULL;
}

EON_Object *eon_rendererAllocArray(EON_Renderer *rend,
                                   EON_Array *array,
                                   EON_Uint32 size, EON_Uint32 itemSize)
{
    if (rend && size && itemSize) {
        EON_Status err = eon_arrayAlloc(array, size, itemSize);
        if (err) {
            rend = eon_rendererDestroy(rend);
        }
    }
    return rend;
}



EON_Renderer *EON_newRenderer(void)
{
    EON_Renderer *rend = eon_zalloc(sizeof(EON_Renderer));
    if (rend) {
        rend = eon_rendederAllocArray(rend, &(rend->Faces),
                                      EON_TRIANGLES_START,
                                      sizeof(eon_FaceInfo));
        rend = eon_rendederAllocArray(rend, &(rend->Lights),
                                      EON_LIGHTS_START,
                                      sizeof(eon_LightInfo));
    }
    return rend;
}

void EON_delRenderer(EON_Renderer *rend)
{
    if (rend) {
        eon_arrayFree(&(rend->Faces));
        eon_arrayFree(&(rend->Lights));
        eon_free(rend);
    }
    return;
}

EON_Status EON_rendererSetup(EON_Renderer *rend,
                             EON_Camera *camera)
{
    EON_Float tempMatrix[16];
    EON_Status ret = EON_OK;

    if (!rend && !camera) {
        return EON_ERROR;
    }

    memset(rend->TriStats, 0, sizeof(rend->TriStats));

    rend->Camera = 0;

    eon_arrayReset(&(rend->Faces));
    eon_arrayReset(&(rend->Lights));

    eon_matrix4x4Rotate(rend->CMatrix,   2, -Camera->Pan);
    eon_matrix4x4Rotate(tempMatrix,      1, -Camera->Pitch);
    eon_matrix4x4Multiply(rend->CMatrix,    tempMatrix);
    eon_matrix4x4Rotate(tempMatrix,      3, -Camera->Roll);
    eon_matrix4x4Multiply(rend->CMatrix,    tempMatrix);
  
    /* TODO: frustum */

    return ret;
}

EON_Status EON_rendererAddLight(EON_Renderer *rend,
                                EON_Light *light)
{
    EON_Status ret = EON_OK;
    if (!rend && !light) {
        return EON_ERROR;
    }
    return EON_ERROR;
}

EON_Status EON_rendererAddObject(EON_Renderer *rend,
                                 EON_Object *object)
{
    EON_Status ret = EON_OK;
    if (!rend && !object) {
        return EON_ERROR;
    }
    return EON_ERROR;
}

EON_Status EON_rendererProcess(EON_Renderer *rend,
                               EON_Frame *frame)
{
    EON_Status ret = EON_OK;
    if (!rend && !frame) {
        return EON_ERROR;
    }
    return EON_ERROR;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

