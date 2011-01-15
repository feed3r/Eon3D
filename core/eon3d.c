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
 * Meaning: all good stuff is credited to the plush authors.              *
 * All the bugs,misdesigns and pessimizations are credited to me. ;)      *
 *                                                                        *
 **************************************************************************/



#include "eon3d.h"

enum {
    EON_MAX_LOG_PREFIX_LEN = 32
};


/**************************************************************************
 * The unavoidable forward declarations.                                  *
 **************************************************************************/



/**************************************************************************
 * Memory manipulation                                                    *
 **************************************************************************/

void *EON_malloc(size_t size)
{
    return malloc(size);
}

void *EON_zalloc(size_t size)
{
    return calloc(1, size);
}

void *EON_free(void *ptr)
{
    free(ptr);
}

/*************************************************************************
 * Error Handling                                                        *
 *************************************************************************/

#define EON_TAG "EON3D"

typedef struct {
    void            *UserData;
    int             MinLevel;
    EON_logHandler  Log;
} EON_LogContext;

/* BIG FAT FIXME */
static EON_LogContext EON_LogCtx = {
    .UserData = NULL, /* watch out below on EON_startup() */
    .MinLevel = EON_LOG_DEBUG,
    .Log      = EON_logDefaultHandler
};

/* ``it's just a %#@ decoy'' */
void EON_log(const char *where, int level, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    EON_vlog(where, level, fmt, ap);

    va_end(ap);
    return;
}

/* here's the real workhorse */
void EON_vlog(const char *where, int level, const char *fmt, va_list ap)
{
    if (!where) {
        where = EON_TAG;
    }
    if (EON_LogCtx.Log) {
        EON_LogCtx.Log(EON_LogCtx.UserData, where, level, fmt, ap);
    }
    return;
}

void EON_logSetHandler(EON_logHandler logHandler, void *userData)
{
    if (logHandler) {
        /* we always must have a valid log handler */
        EON_LogCtx.Log = logHandler;
    }
    if (EON_LogCtx.Log != EON_logDefaultHandler) {
        /* can't overwrite just the default userdata to avoid
           strange things to happen */
        EON_LogCtx.UserData = userData;
    }
    return;
}

EON_logHandler EON_logGetHandler(void)
{
    return EON_LogCtx.Log;
}

void *EON_logGetUserData(void)
{
    return EON_LogCtx.UserData;
}

static const char *eon_logPrefix(int level)
{
    /* FIXME: to be kept in sync with EON_LogLevel */
    static const char *prefixNames[] = {
        "CRI",
        "ERR",
        "WRN",
        "INF",
        "DBG",
        "UNK"  /* EON_LOG_LAST - you should'nt see this */
    };

    level = EON_Clamp(level, EON_LOG_CRITICAL, EON_LOG_LAST);
    return prefixNames[level];
}

/* FIXME: too much stack? */
void EON_logDefaultHandler(void *userData,
                           const char *where, int level,
                           const char* fmt, va_list ap)
{
    char buf[EON_MAX_LOG_LINE_LEN] = { '\0' };
    const char *prefix = eon_logPrefix(level);
    FILE *sink = userData;

    vsnprintf(buf, sizeof(buf), fmt, ap);

    fprintf(sink, "[%s] %s %s\n", prefix, where, buf);
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

static void eon_matrix4x4Rotation(EON_Float matrix[],
                                  EON_Byte m, EON_Float Deg)
{
    EON_Byte m1, m2;
    EON_Double d = Deg * EON_PI / 180.0;
    EON_Double c = cos(d), s = sin(d);

    memset(matrix, 0, sizeof(EON_Float)* 4 * 4);
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
     memset(m, 0, sizeof(EON_Float) * 4 * 4);
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
    EON_Float tmp[4 * 4];
    EON_UInt i;
    memcpy(tmp, dest, sizeof(EON_Float) * 4 * 4);

    for (i = 0; i < (4 * 4); i += 4) {
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
    EON_Material *m = EON_zalloc(sizeof(EON_Material));
    if (m) { /* FIXME magic numbers */
        m->EnvScaling = 1.0f;
        m->TexScaling = 1.0f;
        eon_RGBset(&m->Ambient,  0,   0,   0);
        eon_RGBset(&m->Diffuse,  128, 128, 128);
        eon_RGBset(&m->Specular, 128, 128, 128);
        m->Shininess = 4;
        m->NumGradients = 32;
        m->FadeDist = 1000.0;
        m->ZBufferable = EON_TRUE;
    }
    return m;
}

void EON_delMaterial(EON_Material *m)
{
    if (m) {
        if (m->_reMapTable) {
            EON_free(m->_reMapTable);
        }
        if (m->_requestedColors) {
            EON_free(m->_requestedColors);
        }
        if (m->_addTable) {
            EON_free(m->_addTable);
        }
        free(m);
    }
}

static void EON_freePalette(EON_Material *m)
{
    /* single solid color special case */
    if (m && m->_requestedColors != &m->_solidColor) {
        EON_free(m->_requestedColors);
    }
    return;
}

static void eon_makeSinglePalette(EON_Material *m)
{
    EON_freePalette(m);
    m->NumGradients = 1; /* enforce */
    m->_colorsUsed = m->NumGradients;

    m->_solidColor = m->Ambient;
    m->_requestedColors = &m->_solidColor;
}

static void eon_makePhongPalette(EON_Material *m)
{
    EON_Double a = EON_PI/2.0, da = 0.0;
    EON_UInt i;

    EON_freePalette(m);
    m->_colorsUsed = m->NumGradients;
    m->_requestedColors = EON_malloc(m->_colorsUsed * sizeof(EON_RGB));

    if (m->NumGradients > 1)
        da = -EON_PI/((m->NumGradients-1)<<1);

    for (i = 0; i < m->NumGradients; i++) {
        EON_RGB *pal = &(m->_requestedColors[i]);
        EON_Double ca = 1, cb = 0.0;
        EON_Int c;

        if (m->NumGradients > 1) {
            ca = cos((EON_Double) a);
            a += da;
        }

        cb = pow((EON_Double) ca, (EON_Double)m->Shininess);

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
        EON_free(obj->Vertices);
    }
    if (obj->Faces) {
        EON_free(obj->Faces);
    }
    EON_free(obj);
    return NULL;
}

EON_Object *eon_objectAllocItems(EON_Object *obj, void **p,
                                 size_t size, EON_UInt32 num)
{
    if (obj && size && num) {
        *p = EON_zalloc(size * num);
        if (!(*p)) {
            obj = eon_delObjectData(obj);
        }
    }
    return obj;
}

EON_Object *EON_delObject(EON_Object *obj)
{
    if (obj) {
        EON_UInt i;
        for (i = 0; i < EON_MAX_CHILDREN; i++) {
            EON_delObject(obj->Children[i]);
        }
        eon_delObjectData(obj);
        obj = NULL;
    }
    return obj;
}


EON_Object *EON_newObject(EON_UInt32 vertices, EON_UInt32 faces)
{
    EON_Object *object = EON_zalloc(sizeof(EON_Object));
    if (object) {
        object->GenMatrix    = EON_TRUE;
        object->BackfaceCull = EON_TRUE;
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


static int EON_objectCalcNormals(EON_Object *object)
{
    EON_UInt32 i;

    if (!object) {
        return EON_ERROR;
    }

    for (i = 0; i < object->NumFaces; i++) {
        EON_Face *f = &(object->Faces[i]);
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

    for (i = 0; i < object->NumVertices; i++) {
        EON_Vertex *v = &(object->Vertices[i]);
        eon_normalizeVector((EON_Vector *)&(v->Norm));
    }

    for (i = 0; i < EON_MAX_CHILDREN; i++) {
        EON_objectCalcNormals(object->Children[i]);
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

            f->MappingU[0] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);
            f->MappingV[0] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);
            f->MappingU[1] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);
            f->MappingV[1] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);
            f->MappingU[2] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);
            f->MappingV[2] = (EON_Int32) ((EON_Double)*mm++ * 65535.0);

            f->Material = m;

            f++;
        }

        EON_objectCalcNormals(o);
    }
    return o;
}

EON_Status EON_objectCenter(EON_Object *object)
{
    return EON_ERROR;
}

/*************************************************************************/
/* Lights                                                                */
/*************************************************************************/

EON_Light *EON_newLight(EON_LightMode mode,
                        EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity,
                        EON_Float halfDist)
{
    EON_Light *light = EON_zalloc(sizeof(EON_Light));
    if (light) {
        EON_Float m[4 * 4], m2[4 * 4];

        light->Type = mode;
        light->Intensity = intensity;
        light->HalfDistSquared = halfDist * halfDist;

        if (mode == EON_LIGHT_VECTOR) {
            eon_matrix4x4Rotation(m,  1, x);
            eon_matrix4x4Rotation(m2, 2, y);
            eon_matrix4x4Multiply(m,     m2);
            eon_matrix4x4Rotation(m2, 3, z);
            eon_matrix4x4Multiply(m,     m2);
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
        EON_free(light);
    }
    return;
}

/*************************************************************************/
/* Frames                                                                */
/*************************************************************************/

static EON_Status eon_FramePutPixel(EON_Frame *frame,
                                    EON_Int x, EON_Int y, EON_UInt32 color)
{
    /* TODO */
    return EON_ERROR;
}


static size_t eon_FrameRGBSize(EON_Int width, EON_Int height)
{
    return (EON_RGB_BPP * width * height);
}

EON_Frame *EON_newFrame(EON_Int width, EON_Int height)
{
    size_t size = eon_FrameRGBSize(width, height);
    EON_Byte *data = EON_zalloc(sizeof(EON_Frame) + size);
    EON_Frame *frame = NULL;
    if (data) {
        /* FIXME: Pixels alignment */
        frame = (EON_Frame *)data;
        frame->Pixels   = data + sizeof(EON_Frame);
        frame->F.Width  = width;
        frame->F.Height = height;
        frame->Flags    = EON_FRAME_FLAG_NONE;
        frame->PutPixel = eon_FramePutPixel;
        frame->_private = NULL;
    }
    return frame;
}

void EON_delFrame(EON_Frame *frame)
{
    EON_free(frame);
}


void EON_frameClean(EON_Frame *frame)
{
    if (frame && frame->Pixels) {
        size_t size = eon_FrameRGBSize(frame->F.Width, frame->F.Height);
        memset(frame->Pixels, EON_RGB_BLACK, size);
    }
    return;
}

EON_Status	EON_framePutPixel(EON_Frame *frame,
                              EON_Int x, EON_Int y, EON_UInt32 color)
{
    if (!frame || !frame->PutPixel) {
        /* FIXME: log */
        return EON_ERROR;
    }

    return frame->PutPixel(frame, x, y, color);
}


/*************************************************************************/
/* Camera                                                                */
/*************************************************************************/

EON_Camera *EON_newCamera(EON_Int width, EON_Int height,
                          EON_Float aspectRatio,
                          EON_Float fieldOfView)
{
    EON_Camera *cam = EON_zalloc(sizeof(EON_Camera));
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
        EON_free(camera);
    }
    return;
}

/*************************************************************************/
/* Rendering                                                             */
/*************************************************************************/

enum {
    EON_TRIANGLES_START = 1024,
    EON_LIGHTS_START    = 32
};

typedef struct {
    EON_Face    *Face;
    EON_Float   ZD;
} eon_faceInfo;

typedef struct {
  EON_Light *Light;
  EON_Point Pos;
} eon_lightInfo;

/*************************************************************************/
/* Rendering utilities: generic resizable array object                   */
/*************************************************************************/

typedef struct {
    void        *D;

    EON_UInt32  ItemSize;

    EON_UInt32  Size;
    EON_UInt32  Used;
} eon_array;

static EON_Status eon_arrayAlloc(eon_array *array,
                                 EON_UInt32 size, EON_UInt32 itemSize)
{
    return EON_ERROR;
}

static EON_Status eon_arrayFree(eon_array *array)
{
    return EON_ERROR;
}

static EON_Status eon_arrayResize(eon_array *array, EON_UInt32 size)
{
    return EON_ERROR;
}

static EON_Status eon_arrayAppend(eon_array *array, const void *item)
{
    return EON_ERROR;
}

static EON_Status eon_arrayReset(eon_array *array)
{
    EON_Status err = EON_ERROR;
    if (array) {
        array->Used = 0;
        err = EON_OK;
    }
    return err;
}

/*************************************************************************/
/* Rendering utilities: clipping support                                 */
/*************************************************************************/

typedef enum eon_clipdirection_ {
    EON_CLIP_BACK   = 0,
    EON_CLIP_LEFT   = 1,
    EON_CLIP_RIGHT  = 2,
    EON_CLIP_TOP    = 3,
    EON_CLIP_BOTTOM = 4
} eon_clipDirection;

/* FIXME 8 vs 4x2 */
enum {
    EON_NUM_CLIP_PLANES = 5, /* see clipDirection above */
};

typedef struct eon_clipinfo_ {
    EON_Vertex  newVertices[8];

    EON_Double  Shades[8];
    EON_Double  MappingU[8];
    EON_Double  MappingV[8];
    EON_Double  EMappingU[8];
    EON_Double  EMappingV[8];
} eon_clipInfo;


typedef struct eon_clipcontext_ {
    eon_clipInfo    ClipInfo[2];
    EON_Double      ClipPlanes[EON_NUM_CLIP_PLANES][4];
    EON_Camera      *Cam;
    EON_Int32       CX; /* XXX */
    EON_Int32       CY; /* XXX */
    EON_Double      Fov;
    EON_Double      AdjAsp;
} eon_clipContext;

static void eon_findNormal(EON_Double x2, EON_Double x3,EON_Double y2, EON_Double y3,
                           EON_Double zv, EON_Double *res)
{
    res[0] = zv * (y2 - y3);
    res[1] = zv * (x3 - x2);
    res[2] = x2 * y3 - y2 * x3;
}


static EON_Double eon_clipCalcDot(eon_clipInfo *CI,
                                  EON_UInt vIdx, EON_Double *plane)
{
    EON_Double dot = 0.0;
    dot = CI[0].newVertices[vIdx].Formed.X * plane[0] +
          CI[0].newVertices[vIdx].Formed.Y * plane[1] +
          CI[0].newVertices[vIdx].Formed.Z * plane[2];
    return dot;
}

typedef struct eon_clipplanedata {
    EON_UInt   In;
    EON_Double Dot;
} eon_clipPlaneData;

static void eon_clipCalcPlaneData(eon_clipPlaneData *PD,
                                  eon_clipInfo *CI,
                                  EON_UInt vIdx, EON_Double *plane)
{
    PD->Dot = eon_clipCalcDot(CI, vIdx, plane);
    PD->In  = (PD->Dot >= plane[3]);
    return;
}

static void eon_clipCopyInfo(eon_clipInfo *CI, EON_UInt inV, EON_UInt outV)
{
    CI[1].Shades[outV]      = CI[0].Shades[inV];
    CI[1].MappingU[outV]    = CI[0].MappingU[inV];
    CI[1].MappingV[outV]    = CI[0].MappingV[inV];
    CI[1].EMappingU[outV]   = CI[0].EMappingU[inV];
    CI[1].EMappingV[outV]   = CI[0].EMappingV[inV];
    CI[1].newVertices[outV] = CI[0].newVertices[inV];
    return;
}

/* yes, this is one of the ugliest names I ever gave to a function */
static EON_Float eon_clipCalcScaled(EON_Float aX, EON_Float bX, EON_Double scale)
{
    EON_Float f = (EON_Float) (aX + (bX - aX) * scale);
}

static EON_Point *eon_clipGetVertexFormed(eon_clipInfo *CI, int j, int vi)
{
    return &(CI[j].newVertices[vi].Formed);
}

/* FIXME: massive rephrasing, and I'm not that good.
 * Many bugs are lurking here.
 */
 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_UInt eon_clipToPlane(eon_clipContext *clip,
                                EON_UInt numVerts, EON_Double *plane)
{
    eon_clipPlaneData cur, next;
    eon_clipInfo *CI = NULL;
    EON_UInt i = 0;
    EON_UInt iV = 0; /* In  Vertex */
    EON_UInt oV = 0; /* Out Vertex */

    CI = clip->ClipInfo;

    eon_clipCalcPlaneData(&cur, CI, 0, plane);

    for (i = 0 ; i < numVerts; i++) {
        EON_UInt nV = (i + 1) % numVerts; /* Next Vertex */
        if (cur.In) {
            eon_clipCopyInfo(CI, iV, oV);
            oV++;
        }
        eon_clipCalcPlaneData(&next, CI, nV, plane);
        if (cur.In != next.In) {
            EON_Double scale = (plane[3] - cur.Dot) / (next.Dot - cur.Dot);

            /* Points corresponding to Vertices */
            EON_Point *iP = eon_clipGetVertexFormed(CI, 0, iV);
            EON_Point *nP = eon_clipGetVertexFormed(CI, 0, nV);
            EON_Point *oP = eon_clipGetVertexFormed(CI, 1, oV); /* careful! */

            oP->X = eon_clipCalcScaled(iP->X, nP->X, scale);
            oP->Y = eon_clipCalcScaled(iP->Y, nP->Y, scale);
            oP->Z = eon_clipCalcScaled(iP->Z, nP->Z, scale);

            CI[1].Shades[oV]    = eon_clipCalcScaled(CI[0].Shades[iV],    CI[0].Shades[nV],    scale);
            CI[1].MappingU[oV]  = eon_clipCalcScaled(CI[0].MappingU[iV],  CI[0].MappingU[nV],  scale);
            CI[1].MappingV[oV]  = eon_clipCalcScaled(CI[0].MappingV[iV],  CI[0].MappingV[nV],  scale);
            CI[1].EMappingU[oV] = eon_clipCalcScaled(CI[0].EMappingU[iV], CI[0].EMappingU[nV], scale);
            CI[1].EMappingV[oV] = eon_clipCalcScaled(CI[0].EMappingV[iV], CI[0].EMappingV[nV], scale);

            outV++;
        }
        cur = next;
        inV++;
    }
    return outV;
}


static void eon_clipSetFrustumPlane(eon_clipContext *clip,
                                    eon_clipDirection dir,
                                    EON_Int ref, EON_Int limit)
{
    /* FIXME: ok, those here really smell.
     * Can't yet figure out a better way.
     */
    static const EON_Int nVector[EON_NUM_CLIP_PLANES][4] = {
        {    0,    0,    0,    0 },
        { -100, -100,  100, -100 },
        {  100,  100, -100,  100 },
        {  100, -100,  100,  100 },
        { -100,  100, -100, -100 }
    };
    static const EON_Double orient[EON_NUM_CLIP_PLANES] = {
        0.0, 1.0, -1.0, -1.0, 1.0
    };
    static const EON_Double fovFact[EON_NUM_CLIP_PLANES] = {
        0.0, -100.0, 100.0, 100.0, -100.0
    };

    EON_Double *clipPlane = clip->ClipPlanes[dir];
    const int *nVec = nVector[dir];

    clipPlane[3] = 0.00000001; /* FIXME */
    if (ref == limit) {
        clipPlane[0] = orient[dir];
    } else {
        eon_findNormal(nVec[0], nVec[1], nVec[2], nVec[3],
                       clip->Fov * fovFact[dir] / (ref - limit),
                       clipPlane);
    }
    if (ref > limit) {
        clipPlane[0] = -clipPlane[0];
        clipPlane[1] = -clipPlane[1];
        clipPlane[2] = -clipPlane[2];
    }

    return;
}

static void eon_clipInit(eon_clipContext *clip)
{
    if (clip) {
        memset(clip, 0, sizeof(eon_clipContext));
    }
    return;
}

static void eon_clipSetFrustum(eon_clipContext *clip, EON_Camera *cam)
{
    if (!clip || !cam) {
        return; /* FIXME */
    }

    clip->AdjAsp = 1.0 / cam->AspectRatio;
    clip->Fov = EON_Clamp(cam->Fov, 1.0, 179.0);
    clip->CX = cam->Center.Width  << 20;
    clip->CY = cam->Center.Height << 20;
    clip->Cam = cam;
    memset(clip->ClipPlanes, 0, sizeof(clip->ClipPlanes));

    clip->ClipPlanes[EON_CLIP_BACK][2] = -1.0;
    clip->ClipPlanes[EON_CLIP_BACK][3] = -cam->ClipBack;

    eon_clipSetFrustumPlane(clip, EON_CLIP_LEFT,
                            cam->Clip.Left, cam->Center.Width);
    eon_clipSetFrustumPlane(clip, EON_CLIP_RIGHT,
                            cam->Clip.Right, cam->Center.Width);
    eon_clipSetFrustumPlane(clip, EON_CLIP_TOP,
                            cam->Clip.Top, cam->Center.Height);
    eon_clipSetFrustumPlane(clip, EON_CLIP_BOTTOM,
                            cam->Clip.Bottom, cam->Center.Height);

    return;
}

static void eon_clipCopyFaceInfo(eon_clipContext *clip,
                                 EON_Face *face)
{
    EON_UInt a;

    for (a = 0; a < 3; a++) {
        clip->ClipInfo[0].newVertices[a] = *(face->Vertices[a]);
        clip->ClipInfo[0].Shades[a]      =   face->Shades[a];
        clip->ClipInfo[0].MappingU[a]    =   face->MappingU[a];
        clip->ClipInfo[0].MappingV[a]    =   face->MappingV[a];
        clip->ClipInfo[0].EMappingU[a]   =   face->EnvMappingU[a];
        clip->ClipInfo[0].EMappingV[a]   =   face->EnvMappingV[a];
    }

    return;
}

// XXX: maybe misleading name?
static EON_UInt eon_clipCountVertices(eon_clipContext *clip,
                                      EON_Uint numVerts)
{
    EON_UInt a = (clip->ClipPlanes[0][3] < 0.0 ? 0 : 1);

    while (a < NUM_CLIP_PLANES && numVerts > 2) {
        numVerts = eon_clipToPlane(clip, numVerts, clip->ClipPlanes[a]);
        memcpy(&clip->ClipInfo[0], &clip->ClipInfo[1],
               sizeof(clip->ClipInfo[0]));
        a++;
    }

    return numVerts;
}

// FIXME: explain the `2'
static void eon_clipDoRenderFace(eon_clipContext *clip,
                                 EON_Renderer *rend, EON_Face *face)
{
    EON_Face newface;
    EON_UInt a, k;

    memcpy(&newface, face, sizeof(EON_Face));
    for (k = 2; k < numVerts; k ++) {
        newface.FlatShade = EON_CLAMP(face->FlatShade, 0, 1);
        for (a = 0; a < 3; a ++) {
            eon_clipInfo *CI = &(clip->ClipInfo[0]);
            EON_UInt w = (a == 0) ?0 :(a + (k - 2));
            EON_Double XFov, YFov, ZFov;

            newface.Vertices[a]  =            CI->newVertices[w];
            newface.Shades[a]    = (EON_Float)CI->Shades[w];
            newface.MappingU[a]  = (EON_Int32)CI->MappingU[w];
            newface.MappingV[a]  = (EON_Int32)CI->MappingV[w];
            newface.EMappingU[a] = (EON_Int32)CI->EMappingU[w];
            newface.EMappingV[a] = (EON_Int32)CI->EMappingV[w];
            newface.ScrZ[a]      = 1.0f / (newface.Vertices[a]->Formed.Z);

            ZFov = clip->Fov * newface.ScrZ[a];
            XFov = ZFov * newface.Vertices[a]->Formed.X;
            YFov = ZFov * newface.Vertices[a]->Formed.Y;

            // XXX
            newface.ScrX[a] = clip->CX + ((EON_Int32)((XFov *                (float) (1<<20))));
            newface.ScrX[a] = clip->CY - ((EON_Int32)((YFov * clip->AdjAsp * (float) (1<<20))));
        }
        newface.Material->_renderFace(rend, &newface);
        rend->TriStats[3]++; /* WTF?! */
    }
    rend->TriStats[2]++; /* WTF?! */

    return;
}

// FIXME: explain the `2'
void eon_clipRenderFace(eon_clipContext *clip,
                        EON_Renderer *rend, EON_Face *face)
{
    EON_UInt numVerts = 3;

    eon_clipCopyFaceInfo(clip, face);

    numVerts = eon_clipCountVertices(clip, 3);

    if (numVerts > 2) {
        eon_clipDoRenderFace(clip, rend, face);
    }
    return;
}

EON_Int eon_clipIsNeeded(eon_clipContext *clip, EON_Face *face)
{
    EON_Rectangle *center = &(clip->Cam->Center);
    EON_Area *clip = &(clip->Cam->Clip);
    EON_Float clipBack = clip->Cam->ClipBack;
    EON_Double f  = clip->Fov * clip->AdjAsp;
    EON_Int needed = 0;
    
    EON_Double dr = (Clip->Right  - Center->Width);
    EON_Double dl = (Clip->Left   - Center->Width);
    EON_Double db = (Clip->Bottom - Center->Height);
    EON_Double dt = (Clip->Top    - Center->Height);
    
    EON_Float XFov[3] = { 0.0, 0.0, 0.0 };
    EON_Float YFov[3] = { 0.0, 0.0, 0.0 };
    EON_Float VZ[3]   = { 0.0, 0.0, 0.0 };
    
    XFov[0] = face->Vertices[0]->Formed.X * clip->Fov;
    XFov[1] = face->Vertices[1]->Formed.X * clip->Fov;
    XFov[2] = face->Vertices[2]->Formed.X * clip->Fov;

    YFov[0] = face->Vertices[0]->Formed.Y * f;
    YFov[1] = face->Vertices[1]->Formed.Y * f;
    YFov[2] = face->Vertices[2]->Formed.Y * f;

    VZ[0]   = face->Vertices[0]->Formed.Z;
    VZ[1]   = face->Vertices[1]->Formed.Z;
    VZ[2]   = face->Vertices[2]->Formed.Z;

    needed = ((clipBack <= 0.0 || VZ[0] <= clipBack || VZ[1] <= clipBack || VZ[2] <= clipBack)
           && (VZ[0]   >= 0          || VZ[1]   >= 0          || VZ[2]   >= 0         ) 
           && (XFov[0] <= dr * VZ[0] || XFov[1] <= dr * VZ[1] || XFov[2] <= dr * VZ[2])
           && (XFov[0] <= dl * VZ[0] || XFov[1] <= dl * VZ[1] || XFov[2] <= dl * VZ[2])
           && (YFov[0] <= db * VZ[0] || YFov[1] <= db * VZ[2] || YFov[2] <= db * VZ[2])
           && (YFov[0] <= dt * VZ[0] || YFov[1] <= dt * VZ[2] || YFov[2] <= dt * VZ[2]));

    return needed;
}



/*************************************************************************/
/* Rendering core                                                        */
/*************************************************************************/

struct eon_renderer_ {
    EON_Camera      *Camera;
    EON_ZBuffer     *ZBuffer;

    eon_array       Faces;
    eon_array       Lights;

    EON_Float       CMatrix[4 * 4];
    EON_UInt32      TriStats[4]; /* FIXME: magic number */

    eon_clipContext Clip;
};

static void *eon_rendererDestroy(EON_Renderer *rend)
{
    EON_delRenderer(rend);
    return NULL;
}

EON_Renderer *eon_rendererAllocArray(EON_Renderer *rend,
                                     eon_array *array,
                                     EON_UInt32 size, EON_UInt32 itemSize)
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
    EON_Renderer *rend = EON_zalloc(sizeof(EON_Renderer));
    if (rend) {
        rend = eon_rendererAllocArray(rend, &(rend->Faces),
                                      EON_TRIANGLES_START,
                                      sizeof(eon_faceInfo));
        rend = eon_rendererAllocArray(rend, &(rend->Lights),
                                      EON_LIGHTS_START,
                                      sizeof(eon_lightInfo));
    }
    return rend;
}

void EON_delRenderer(EON_Renderer *rend)
{
    if (rend) {
        eon_arrayFree(&(rend->Faces));
        eon_arrayFree(&(rend->Lights));
        EON_free(rend);
    }
    return;
}

EON_Status EON_rendererSetup(EON_Renderer *rend,
                             EON_Camera *camera)
{
    EON_Float tempMatrix[4 * 4];
    EON_Status ret = EON_OK;

    if (!rend && !camera) {
        return EON_ERROR;
    }

    memset(rend->TriStats, 0, sizeof(rend->TriStats));

    rend->Camera = 0;

    eon_arrayReset(&(rend->Faces));
    eon_arrayReset(&(rend->Lights));

    eon_matrix4x4Rotation(rend->CMatrix, 2, -camera->Pan);
    eon_matrix4x4Rotation(tempMatrix,    1, -camera->Pitch);
    eon_matrix4x4Multiply(rend->CMatrix,    tempMatrix);
    eon_matrix4x4Rotation(tempMatrix,    3, -camera->Roll);
    eon_matrix4x4Multiply(rend->CMatrix,    tempMatrix);

    eon_clipInit(&(rend->Clip));
    eon_clipSetFrustum(&(rend->Clip), camera);

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


/*************************************************************************
 * Initialization and Finalization                                       *
 *************************************************************************/

void EON_startup()
{
    EON_LogCtx.UserData = stderr;
    return;
}


void EON_shutdown()
{
    /* nothing interesting in here */
    return;
}


/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

