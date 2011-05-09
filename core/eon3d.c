/**************************************************************************
 * eon3d.c -- Eon3D is a simplistic 3D software renderer.                 *
 * (C) 2010-2011 Francesco Romani <fromani at gmail dot com>              *
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


#include "memorykit.h"
#include "arraykit.h"

#include "eon3d.h"

enum {
    EON_MAX_LOG_PREFIX_LEN = 32
};


/* `almost' zero in floating point */
#define EON_ZEROF   (0.0000000001)

/*************************************************************************/
/* Rendering utilities: generic resizable array object                   */
/*************************************************************************/

typedef struct {
    void       *Data;

    EON_Int32   ItemSize;

    EON_Int32   Size;
    EON_Int32   Length;
} eon_array;

enum {
    EON_ARRAY_ITEM_SIZE_MIN = 1,
};

#define EON_ARRAY_CHECK_REF(VA) do { \
    if (!(VA)) { \
        return EON_ERROR; \
    } \
} while (0)

static int eon_arrayAdjustPosition(eon_array *array, EON_Int32 position)
{
    int32_t pos = position;
    if (pos < 0) {
        pos = array->Length - pos;
    }
    if (pos < 0) {
        pos = 0;
    }
    if (pos >= array->Length) {
        pos = array->Length;
    }
    return position;
}

static EON_Status eon_arrayGrow(eon_array *array)
{
    EON_Status ret = EON_OK;
    int memSize = array->Size * array->ItemSize;
    void *newData = realloc(array->Data, memSize * 2);
    if (newData) {
        memset(newData + memSize, 0, array->Size);
    } else {
        newData = calloc(1, memSize);
        if (newData) {
            memcpy(newData, array->Data, array->Size);
            free(array->Data);
        }
    }
    
    if (newData) {
        array->Data = newData;
        array->Size *= 2;
    } else {
        ret = EON_ERROR;
    }
    return ret;
}

static int eon_arrayPreparePut(eon_array *array, EON_Int32 position)
{
    EON_Status ret = EON_OK;
    position = eon_arrayAdjustPosition(array, position);
    if (position >= array->Size) {
        ret = eon_arrayGrow(array);
    }
    return ret;
}

static EON_Int32 eon_arrayAdjustItemSize(EON_Int32 itemSize)
{
    if (itemSize <= 0) {
        itemSize = sizeof(void*);
    }
    if (itemSize <= EON_ARRAY_ITEM_SIZE_MIN) {
        itemSize  = EON_ARRAY_ITEM_SIZE_MIN;
    }

    return itemSize;
}

EON_PRIVATE
EON_Status eon_arrayAlloc(eon_array *array,
                          EON_Int32 size, EON_Int32 itemSize)
{
    EON_Status ret = EON_OK;
    itemSize = eon_arrayAdjustItemSize(itemSize);

    EON_ARRAY_CHECK_REF(array);

    if (!array || !size) {
        ret = EON_ERROR;
    } else {
        array->Data = calloc(size, itemSize);
        if (!array->Data) {
            ret = EON_ERROR;
        } else {
            array->Size = size;
            array->ItemSize = itemSize;
        }
    }

    return ret;
}

EON_PRIVATE
EON_Status eon_arrayFree(eon_array *array)
{
    EON_ARRAY_CHECK_REF(array);

    if (array->Data) {
        free(array->Data);
        array->Data = NULL;
    }
 
    return EON_ERROR;
}

#ifdef EON_DEBUG

EON_PRIVATE
eon_array *eon_arrayNew(EON_Int32 size, EON_Int32 itemSize)
{
    eon_array *array = calloc(1, sizeof(eon_array));
    if (array) {
        EON_Status ret = eon_arrayAlloc(array, size, itemSize);
        if (ret != EON_OK) {
            free(array);
            array = NULL;
        }
    }
    return array;
}

#endif /* EON_DEBUG */

static void *eon_arrayItemPtr(eon_array *array, EON_Int32 position)
{
    void *p = (EON_Byte *)array->Data + (position * array->ItemSize);
    return p;
}

EON_PRIVATE
EON_Status eon_arrayInsert(eon_array *array, EON_Int32 position, const void *element)
{
    EON_Status ret = EON_ERROR;

    EON_ARRAY_CHECK_REF(array);

    ret = eon_arrayPreparePut(array, position);
    if (ret == EON_OK) {
        void *ptr = eon_arrayItemPtr(array, position);
        memcpy(ptr, element, array->ItemSize);
        array->Length++;
    }
    return ret;
}

EON_PRIVATE
EON_Status eon_arrayAppend(eon_array *array, const void *element)
{
    EON_ARRAY_CHECK_REF(array);
    return eon_arrayInsert(array, array->Length, element);
}

EON_PRIVATE
EON_Status eon_arrayReset(eon_array *array)
{
    EON_ARRAY_CHECK_REF(array);

    array->Length = 0;
    return EON_OK;
}

EON_PRIVATE
EON_Status eon_arrayLength(eon_array *array, EON_Int32 *len)
{
    EON_ARRAY_CHECK_REF(array);
    if (len) {
        *len = array->Length;
    }
    return EON_OK;
}

EON_PRIVATE
void *eon_arrayGet(eon_array *array, EON_Int32 index)
{
    void *ptr = NULL;
    if (array && index >= 0 && index < array->Length) {
        ptr = eon_arrayItemPtr(array, index);
    }
    return ptr;
}

#ifdef EON_FUTURE_ARRAY
EON_PRIVATE
void *eon_arrayLast(eon_array *array)
{
    void *ptr = NULL;
    if (array && array->Length > 0) {
        ptr = eon_arrayItemPtr(array, array->Length - 1);
    }
    return ptr;
}
#endif /* EON_FUTURE_ARRAY */

#undef EON_ARRAY_CHECK_REF



/**************************************************************************
 * The unavoidable forward declarations.                                  *
 **************************************************************************/

enum {
    EON_NUM_CLIP_PLANES = 5, /* see clipDirection above */
};

/* can't find a better nome */
typedef enum eon_tri_stat_ {
    EON_TRI_STAT_INITIAL = 0, /* initial triangles                  */
    EON_TRI_STAT_CULLED,      /* triangles after culling            */
    EON_TRI_STAT_CLIPPED,     /* final polygons after real clipping */
    EON_TRI_STAT_TESSELLED,   /* final triangles after tessellation */
    EON_TRI_STAT_NUM          /* this MUST be the last              */
} EON_TriStat;

/* FIXME 8 vs 4x2 et al */
typedef struct eon_clipinfo_ {
    EON_Vertex  newVertexes[8];

    EON_Double  Shades[8];
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


struct eon_renderer_ {
    EON_Camera      *Camera;        /**< camera reference */
    EON_ZBuffer     *ZBuffer;       /**< WRITEME          */

    eon_array       Faces;          /**< faces to render  */
    eon_array       Lights;         /**< lights to render */

    EON_Float       CMatrix[4 * 4]; /**< viewpoint transformation matrix */
    EON_UInt32      TriStats[EON_TRI_STAT_NUM];
    eon_clipContext Clip;           /**< clip information */
};

/**************************************************************************
 * The unavoidable forward declarations (/2: functions).                  *
 **************************************************************************/

static EON_Status eon_rendererProcessObject(EON_Renderer *rend,
                                            EON_Object *object,
                                            EON_Float *bmatrix,
                                            EON_Float *bnmatrix);

static int eon_renderFaceNull(EON_Renderer *renderer,
                              EON_Face *face, EON_Frame *frame);

static int eon_renderFaceVertexes(EON_Renderer *renderer,
                                  EON_Face *face, EON_Frame *frame);


static int eon_processFaceNull(EON_Face *face, EON_Renderer *rend);

static int eon_processFaceFlatLightining(EON_Face *face,
                                         EON_Renderer *rend);

static int eon_processFaceFillEnvironment(EON_Face *face,
                                          EON_Renderer *rend);

static int eon_processFaceGouradShading(EON_Face *face,
                                        EON_Renderer *rend);


/*************************************************************************
 * Error Handling                                                        *
 *************************************************************************/

#define EON_TAG "EON3D"

typedef struct {
    void            *UserData;
    int             MinLevel;
    EON_LogHandler  Log;
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

    EON_LogCtx.Log(EON_LogCtx.UserData, where, level, fmt, ap);

    return;
}

void EON_logSetHandler(EON_LogHandler LogHandler, void *userData)
{
    if (LogHandler) {
        /* we always must have a valid log handler */
        EON_LogCtx.Log = LogHandler;
    }
    if (EON_LogCtx.Log != EON_logDefaultHandler) {
        /* can't overwrite just the default userdata to avoid
           strange things to happen */
        EON_LogCtx.UserData = userData;
    }
    return;
}

EON_LogHandler EON_logGetHandler(void)
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

void EON_RGBSet(EON_RGB *rgb, EON_UInt8 R, EON_UInt8 G, EON_UInt8 B)
{
    rgb->R = R;
    rgb->G = G;
    rgb->B = B;
    rgb->A = 0;
}

EON_UInt32 EON_RGBPack(const EON_RGB *RGB)
{
    EON_UInt32 color = (((EON_UInt32)RGB->R) << 24)
                     | (((EON_UInt32)RGB->G) << 16)
                     | (((EON_UInt32)RGB->B) <<  8)
                     | (((EON_UInt32)RGB->A)      );
    return color;
}

void EON_RGBUnpack(EON_RGB *RGB, EON_UInt32 color)
{
    RGB->R = (color >> 24) & 0xFF;
    RGB->G = (color >> 16) & 0xFF;
    RGB->B = (color >>  8) & 0xFF;
    RGB->A = (color      ) & 0xFF;
    return;
}

/*************************************************************************/
/* Vector3 operations (internal use only)                                */
/*************************************************************************/
/* the definition of the following operation set, and the design decision
   to use Vector3s in lieu of Point3s, came from
   [3DMath] `3D Math Primer for Graphics and Game Development'
   (F. Dunn, I. Parberry), WordWare.
*/

/* TODO? zero, not */

EON_PRIVATE 
EON_Bool eon_floatAreEquals(EON_Float A, EON_Float B)
{
    return (fabs(A - B) < EON_ZEROF);
}

EON_PRIVATE
void eon_Vector3Set(EON_Vector3 *V, EON_Float X, EON_Float Y, EON_Float Z)
{
    V->X = X;
    V->Y = Y;
    V->Z = Z;
}

#ifdef EON_FUTURE_VECTOR3
EON_PRIVATE
int eon_Vector3IsEqual(const EON_Vector3 *A, const EON_Vector3 *B)
{
    return (eon_floatAreEquals(A->X, B->X)
         && eon_floatAreEquals(A->Y, B->Y)
         && eon_floatAreEquals(A->Z, B->Z));
}
#endif /* EON_FUTURE_VECTOR3 */

EON_PRIVATE
void eon_Vector3RAdd(EON_Vector3 *D, const EON_Vector3 *V)
{
    D->X += V->X;
    D->Y += V->Y;
    D->Z += V->Z;
}

#ifdef EON_FUTURE_VECTOR3
EON_PRIVATE
void eon_Vector3Add(const EON_Vector3 *A, const EON_Vector3 *B,
                    EON_Vector3 *D)
{
    D->X = A->X + B->X;
    D->Y = A->Y + B->Y;
    D->Z = A->Z + B->Z;
}
#endif /* EON_FUTURE_VECTOR3 */

EON_PRIVATE
void eon_Vector3Diff(const EON_Vector3 *A, const EON_Vector3 *B,
                     EON_Vector3 *D)
{
    D->X = A->X - B->X;
    D->Y = A->Y - B->Y;
    D->Z = A->Z - B->Z;
}

EON_PRIVATE
EON_Float eon_Vector3DotProduct(const EON_Vector3 *A, const EON_Vector3 *B)
{
    return (A->X * B->X + A->Y * B->Y + A->Z * B->Z);
}

#define EON_VECTOR3KMULT(V, K) do { \
    (V)->X *= (K); \
    (V)->Y *= (K); \
    (V)->Z *= (K); \
} while (0)

#ifdef EON_FUTURE_VECTOR3
/* scalar multipliction */
EON_PRIVATE
void eon_Vector3Mul(EON_Vector3 *V, EON_Float k)
{
    EON_VECTOR3KMULT(V, k);
}
#endif /* EON_FUTURE_VECTOR3 */

/* scalar division */
EON_PRIVATE
void eon_Vector3Div(EON_Vector3 *V, EON_Float k)
{
    /* FIXME: optimization?*/
    EON_Float kp = 1/k;
    EON_VECTOR3KMULT(V, kp);
}

#undef EON_VECTOR3KMULT

EON_PRIVATE
void eon_Vector3Normalize(EON_Vector3 *V)
{
    EON_Float len2 = eon_Vector3DotProduct(V, V);
    if (len2 > EON_ZEROF) {
        EON_Float t = (EON_Float)sqrt(len2);
        eon_Vector3Div(V, t);
    } else {
        eon_Vector3Set(V, 0.0, 0.0, 0.0);
    }
    return;
}

#ifdef EON_FUTURE_VECTOR3
EON_PRIVATE
EON_Float eon_Vector3Magnitude(const EON_Vector3 *V)
{
    return (EON_Float)sqrt(eon_Vector3DotProduct(V, V));
}

EON_PRIVATE
void eon_Vector3CrossProduct(const EON_Vector3 *A, const EON_Vector3 *B,
                             EON_Vector3 *D)
{
    D->X = A->Y * B->Z - A->Z * B->Y;
    D->Y = A->Z * B->X - A->X * B->Z;
    D->Z = A->X * B->Y - A->Y * B->X;
}

EON_PRIVATE
EON_Float eon_Vector3Distance(const EON_Vector3 *A, const EON_Vector3 *B)
{
    EON_Vector3 D;
    eon_Vector3Diff(A, B, &D);
    return eon_Vector3Magnitude(&D);
}
#endif /* EON_FUTURE_VECTOR3 */


/**************************************************************************
 * Internal functions: Matrix manipulation                                *
 **************************************************************************/
/* NOTE: eon3D uses column vectors                                        */

EON_PRIVATE
EON_Float *eon_matrix4x4MakeRotation(EON_Float *matrix,
                                     EON_Byte ax, EON_Float deg)
{
    EON_Byte m1, m2;
    EON_Double d = deg * EON_PI / 180.0;
    EON_Double c = cos(d), s = sin(d);

    memset(matrix, 0, sizeof(EON_Float)* 4 * 4);
    matrix[((ax - 1) << 2) + ax - 1] = 1.0;
    matrix[15]                       = 1.0;
    m1 = ( ax      % 3);
    m2 = ((m1 + 1) % 3);

    matrix[(m1 << 2) + m1] = (EON_Float)+c;
    matrix[(m1 << 2) + m2] = (EON_Float)+s;
    matrix[(m2 << 2) + m2] = (EON_Float)+c;
    matrix[(m2 << 2) + m1] = (EON_Float)-s;

    return matrix;
}

EON_PRIVATE
EON_Float *eon_matrix4x4MakeTranslation(EON_Float *matrix,
                                        EON_Float x,
                                        EON_Float y,
                                        EON_Float z)
{
     memset(matrix, 0, sizeof(EON_Float) * 4 * 4);
     matrix[0     ] = 1.0;
     matrix[4  + 1] = 1.0;
     matrix[8  + 2] = 1.0;
     matrix[12 + 3] = 1.0;
     matrix[0  + 3] = x;
     matrix[4  + 3] = y;
     matrix[8  + 3] = z;
     return matrix;
}

EON_PRIVATE
EON_Float *eon_matrix4x4Multiply(EON_Float *dst, const EON_Float *src)
{
    EON_UInt i;
    EON_Float tmp[4 * 4];
    memcpy(tmp, dst, sizeof(EON_Float) * 4 * 4);

    for (i = 0; i < (4 * 4); i += 4) {
        *dst++ = src[i+0]*tmp[(0<<2)+0]+src[i+1]*tmp[(1<<2)+0]
               + src[i+2]*tmp[(2<<2)+0]+src[i+3]*tmp[(3<<2)+0];
        *dst++ = src[i+0]*tmp[(0<<2)+1]+src[i+1]*tmp[(1<<2)+1]
               + src[i+2]*tmp[(2<<2)+1]+src[i+3]*tmp[(3<<2)+1];
        *dst++ = src[i+0]*tmp[(0<<2)+2]+src[i+1]*tmp[(1<<2)+2]
               + src[i+2]*tmp[(2<<2)+2]+src[i+3]*tmp[(3<<2)+2];
        *dst++ = src[i+0]*tmp[(0<<2)+3]+src[i+1]*tmp[(1<<2)+3]
               + src[i+2]*tmp[(2<<2)+3]+src[i+3]*tmp[(3<<2)+3];
    }
    return dst;
}

EON_PRIVATE
void eon_matrix4x4Apply(EON_Float *m,
                        EON_Float x, EON_Float y, EON_Float z,
                        EON_Float *outx, EON_Float *outy, EON_Float *outz)
{
    *outx = x*m[0] + y*m[1] + z*m[2 ] + m[3 ];
    *outy = x*m[4] + y*m[5] + z*m[6 ] + m[7 ];
    *outz = x*m[8] + y*m[9] + z*m[10] + m[11];
}


/*************************************************************************/
/* Faces (internal use only)                                             */
/*************************************************************************/

static void eon_faceInit(EON_Face *f)
{
    f->_processFlatLightining  = eon_processFaceNull;
    f->_processFillEnvironment = eon_processFaceNull;
    f->_processGouradShading   = eon_processFaceNull;
    return;
}

static void eon_faceSetMaterial(EON_Face *f, EON_Material *m)
{
    if (m->_shadeMode & (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE)) {
        f->_processFlatLightining = eon_processFaceFlatLightining;
    }

    if (m->_fillMode & EON_FILL_ENVIRONMENT) {
        f->_processFillEnvironment = eon_processFaceFillEnvironment;
    }

    if (m->_shadeMode & (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE)) {
        f->_processGouradShading = eon_processFaceGouradShading;
    }

    return;
}

/*************************************************************************/
/* Materials                                                             */
/*************************************************************************/

/* FIXME: magic numbers */
EON_Material *EON_newMaterial(void)
{
    EON_Material *m = CX_zalloc(sizeof(EON_Material));
    if (m) { /* FIXME magic numbers */
        m->EnvScaling = 1.0f;
        m->TexScaling = 1.0f;
        EON_RGBSet(&m->Ambient,  0,   0,   0);
        EON_RGBSet(&m->Diffuse,  128, 128, 128);
        EON_RGBSet(&m->Specular, 128, 128, 128);
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
            CX_free(m->_reMapTable);
        }
        if (m->_requestedColors) {
            CX_free(m->_requestedColors);
        }
        if (m->_addTable) {
            CX_free(m->_addTable);
        }
        free(m);
    }
}

static void eon_freePalette(EON_Material *m)
{
    /* single solid color special case */
    if (m && m->_requestedColors != &m->_solidColor) {
        CX_free(m->_requestedColors);
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
    m->_requestedColors = CX_malloc(m->_colorsUsed * sizeof(EON_RGB));

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


EON_Status EON_materialSeal(EON_Material *m)
{
    EON_Status ret = EON_OK;
    if (!m) {
        return EON_ERROR;
    }

    if (m->Shininess < 1)
        m->Shininess = 1;

    m->_shadeMode = m->Shade;
    m->_fillMode = m->Fill;
    if (m->_fillMode == EON_FILL_DEFAULT) {
        m->_fillMode = EON_FILL_SOLID;
    }

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
        m->_renderFace = eon_renderFaceNull;
    }
    return ret;
}

/*************************************************************************/
/* Objects and primitives                                                */
/*************************************************************************/

static void eon_objectInitFaces(EON_Object *o, EON_Material *m)
{
    EON_UInt32 x = 0;

    for (x = 0; x < o->NumFaces; x++) {
        eon_faceInit(&(o->Faces[x]));
        eon_faceSetMaterial(&(o->Faces[x]), m);
    }

    return;
}

static void *eon_delObjectData(EON_Object *obj)
{
    if (obj->Vertexes) {
        CX_free(obj->Vertexes);
    }
    if (obj->Faces) {
        CX_free(obj->Faces);
    }
    CX_free(obj);
    return NULL;
}

EON_Object *eon_objectAllocItems(EON_Object *obj, void **p,
                                 size_t size, EON_UInt32 num)
{
    if (obj && size && num) {
        *p = CX_zalloc(size * num);
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


EON_Object *EON_newObject(EON_UInt32 vertices, EON_UInt32 faces,
                          EON_Material *material)
{
    EON_Object *object = NULL;
    
    /* TODO sanity checks; material CANNOT be NULL */

    object = CX_zalloc(sizeof(EON_Object));
    if (object) {
        object->GenMatrix    = EON_TRUE;
        object->BackfaceCull = EON_TRUE;
        object->NumVertexes  = vertices;
        object->NumFaces     = faces;
        object = eon_objectAllocItems(object, (void**)&(object->Vertexes),
                                      sizeof(EON_Vertex), vertices);
        object = eon_objectAllocItems(object, (void**)&(object->Faces),
                                      sizeof(EON_Face), faces);
        
        eon_objectInitFaces(object, material);
  }
  return object;
}

static void eon_resetVertexesNormals(EON_Object *obj)
{
    EON_UInt32 i;
    for (i = 0; i < obj->NumVertexes; i++) {
        EON_Vertex *v = &(obj->Vertexes[i]);
        v->Norm.X = 0.0;
        v->Norm.Y = 0.0;
        v->Norm.Z = 0.0;
    }
    return;
}


int EON_objectCalcNormals(EON_Object *object)
{
    EON_UInt32 i;

    if (!object) {
        return EON_ERROR;
    }

    for (i = 0; i < object->NumFaces; i++) {
        EON_Face *f = &(object->Faces[i]);
        EON_Vector3 d1, d2;

        eon_Vector3Diff(&(f->Vertexes[0]->Coords),
                        &(f->Vertexes[1]->Coords), &d1);
        eon_Vector3Diff(&(f->Vertexes[0]->Coords),
                      &(f->Vertexes[2]->Coords), &d2);

        f->Norm.X = (EON_Float) (d1.Y * d2.Z - d1.Z * d2.Y);
        f->Norm.Y = (EON_Float) (d1.Z * d2.X - d1.X * d2.Z);
        f->Norm.Z = (EON_Float) (d1.X * d2.Y - d1.Y * d2.X);

        eon_Vector3Normalize(&(f->Norm));

        eon_Vector3RAdd(&(f->Vertexes[0]->Norm), &(f->Norm));
        eon_Vector3RAdd(&(f->Vertexes[1]->Norm), &(f->Norm));
        eon_Vector3RAdd(&(f->Vertexes[2]->Norm), &(f->Norm));
    }

    for (i = 0; i < object->NumVertexes; i++) {
        EON_Vertex *v = &(object->Vertexes[i]);
        eon_Vector3Normalize(&(v->Norm));
    }

    for (i = 0; i < EON_MAX_CHILDREN; i++) {
        EON_objectCalcNormals(object->Children[i]);
    }
    return EON_OK;
}

EON_Status EON_objectSetMaterial(EON_Object *object,
                                 EON_Material *material)
{
    EON_UInt32 x = 0;

    /* TODO sanity checks; material CANNOT be NULL */

    for (x = 0; x < object->NumFaces; x++) {
        eon_faceSetMaterial(&(object->Faces[x]), material);
    }

    return EON_OK;
}

EON_Object *EON_newBox(EON_Float w, EON_Float d, EON_Float h,
                       EON_Material *material)
{
    static const EON_Byte verts[6*6] = {
        0,4,1, 1,4,5, 0,1,2, 3,2,1, 2,3,6, 3,7,6,
        6,7,4, 4,7,5, 1,7,3, 7,1,5, 2,6,0, 4,0,6
    };
/*
    static const EON_Byte map[24*2*3] = {
        1,0, 1,1, 0,0, 0,0, 1,1, 0,1,
        0,0, 1,0, 0,1, 1,1, 0,1, 1,0,
        0,0, 1,0, 0,1, 1,0, 1,1, 0,1,
        0,0, 1,0, 0,1, 0,1, 1,0, 1,1,
        1,0, 0,1, 0,0, 0,1, 1,0, 1,1,
        1,0, 1,1, 0,0, 0,1, 0,0, 1,1
    };
*/
    const EON_Byte *vv = verts;

    EON_Object *o = EON_newObject(8, 12, material);
    if (o) {
        EON_Vertex *v = o->Vertexes;
        EON_Face *f = o->Faces;
        EON_UInt32 x;

        w /= 2;
        h /= 2;
        d /= 2;

        eon_Vector3Set(&(v[0].Coords), -w,  h,  d);
        eon_Vector3Set(&(v[1].Coords),  w,  h,  d);
        eon_Vector3Set(&(v[2].Coords), -w,  h, -d);
        eon_Vector3Set(&(v[3].Coords),  w,  h, -d);
        eon_Vector3Set(&(v[4].Coords), -w, -h,  d);
        eon_Vector3Set(&(v[5].Coords),  w, -h,  d);
        eon_Vector3Set(&(v[6].Coords), -w, -h, -d);
        eon_Vector3Set(&(v[7].Coords),  w, -h, -d);

        for (x = 0; x < 12; x ++) {
            f->Vertexes[0] = o->Vertexes + *vv++;
            f->Vertexes[1] = o->Vertexes + *vv++;
            f->Vertexes[2] = o->Vertexes + *vv++;

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
    EON_Light *light = CX_zalloc(sizeof(EON_Light));
    if (light) {
        EON_Float m[4 * 4], m2[4 * 4];

        light->Type = mode;
        light->Intensity = intensity;
        light->HalfDistSquared = halfDist * halfDist;

        if (mode == EON_LIGHT_VECTOR) {
            eon_matrix4x4MakeRotation(m,  1, x);
            eon_matrix4x4MakeRotation(m2, 2, y);
            eon_matrix4x4Multiply(m,         m2);
            eon_matrix4x4MakeRotation(m2, 3, z);
            eon_matrix4x4Multiply(m,         m2);
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
        CX_free(light);
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
    EON_Byte *data = CX_zalloc(sizeof(EON_Frame) + size);
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
    CX_free(frame);
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
    EON_Camera *cam = CX_zalloc(sizeof(EON_Camera));
    if (cam) {
        cam->FieldOfView    = fieldOfView;
        cam->AspectRatio    = aspectRatio;
        cam->Sort           = EON_SORT_BACK_TO_FRONT;
        /* FIXME: if ZBuffer Sort=0 */
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
        CX_free(camera);
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
  EON_Light     *Light;
  EON_Vector3   Pos;
} eon_lightInfo;

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

static void eon_findNormal(EON_Double x2, EON_Double x3,
                           EON_Double y2, EON_Double y3,
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
    dot = CI[0].newVertexes[vIdx].Formed.X * plane[0] +
          CI[0].newVertexes[vIdx].Formed.Y * plane[1] +
          CI[0].newVertexes[vIdx].Formed.Z * plane[2];
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
    CI[1].newVertexes[outV] = CI[0].newVertexes[inV];
    return;
}

/* yes, this is one of the ugliest names I ever gave to a function */
static EON_Float eon_clipCalcScaled(EON_Float aX, EON_Float bX, EON_Double scale)
{
    EON_Float f = (EON_Float) (aX + (bX - aX) * scale);
    return f;
}

static EON_Vector3 *eon_clipGetVertexFormed(eon_clipInfo *CI, int j, int vi)
{
    return &(CI[j].newVertexes[vi].Formed);
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

            /* Points corresponding to Vertexes */
            EON_Vector3 *iP = eon_clipGetVertexFormed(CI, 0, iV);
            EON_Vector3 *nP = eon_clipGetVertexFormed(CI, 0, nV);
            EON_Vector3 *oP = eon_clipGetVertexFormed(CI, 1, oV); /* careful! */

            oP->X = eon_clipCalcScaled(iP->X, nP->X, scale);
            oP->Y = eon_clipCalcScaled(iP->Y, nP->Y, scale);
            oP->Z = eon_clipCalcScaled(iP->Z, nP->Z, scale);

            /* XXX */
            CI[1].Shades[oV]      = eon_clipCalcScaled(CI[0].Shades[iV],      CI[0].Shades[nV],      scale);

            oV++;
        }
        cur = next; /* XXX */
        iV++;
    }
    return oV;
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

    clipPlane[3] = EON_ZEROF;
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
    clip->Fov = EON_Clamp(cam->FieldOfView, 1.0, 179.0);
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
        clip->ClipInfo[0].newVertexes[a] = *(face->Vertexes[a]);
        clip->ClipInfo[0].Shades[a]      =   face->Shades[a];
    }

    return;
}

/* XXX: maybe misleading name? */
static EON_UInt eon_clipCountVertexes(eon_clipContext *clip,
                                      EON_UInt numVerts)
{
    EON_UInt a = (clip->ClipPlanes[0][3] < 0.0 ? 0 : 1);

    while (a < EON_NUM_CLIP_PLANES && numVerts > 2) {
        numVerts = eon_clipToPlane(clip, numVerts, clip->ClipPlanes[a]);
        memcpy(&clip->ClipInfo[0], &clip->ClipInfo[1],
               sizeof(clip->ClipInfo[0]));
        a++;
    }

    return numVerts;
}

/* FIXME: explain the `2' */
static void eon_clipDoRenderFace(eon_clipContext *clip, EON_UInt numVerts,
                                 EON_Renderer *rend, EON_Face *face,
                                 EON_Frame *frame)
{
    EON_Face newface;
    EON_UInt a, k;

    memcpy(&newface, face, sizeof(EON_Face));
    for (k = 2; k < numVerts; k ++) {
        newface.FlatShade = EON_Clamp(face->FlatShade, 0, 1);
        for (a = 0; a < 3; a ++) {
            eon_clipInfo *CI = &(clip->ClipInfo[0]);
            EON_UInt w = (a == 0) ?0 :(a + (k - 2));
            newface.Vertexes[a]    =            CI->newVertexes + w;
            newface.Shades[a]      = (EON_Float)CI->Shades[w];
        }
        newface.Material->_renderFace(rend, &newface, frame);
        rend->TriStats[EON_TRI_STAT_TESSELLED]++;
    }
    rend->TriStats[EON_TRI_STAT_CLIPPED]++;

    return;
}

void eon_clipRenderFace(eon_clipContext *clip,
                        EON_Renderer *rend, EON_Face *face, EON_Frame *frame)
{
    EON_UInt numVerts = 3;

    eon_clipCopyFaceInfo(clip, face);

    numVerts = eon_clipCountVertexes(clip, 3);

    // FIXME: explain the `2'
    if (numVerts > 2) {
        eon_clipDoRenderFace(clip, numVerts, rend, face, frame);
    }
    return;
}

/* FIXME: misleading name? */
EON_Int eon_clipIsNeeded(eon_clipContext *clip, EON_Face *face)
{
    EON_Rectangle *center = &(clip->Cam->Center);
    EON_Area *clipArea = &(clip->Cam->Clip);
    EON_Float clipBack = clip->Cam->ClipBack;
    EON_Double f  = clip->Fov * clip->AdjAsp;
    EON_Int needed = 0;

    EON_Double dr = (clipArea->Right  - center->Width);
    EON_Double dl = (clipArea->Left   - center->Width);
    EON_Double db = (clipArea->Bottom - center->Height);
    EON_Double dt = (clipArea->Top    - center->Height);

    EON_Float XFov[3] = { 0.0, 0.0, 0.0 };
    EON_Float YFov[3] = { 0.0, 0.0, 0.0 };
    EON_Float VZ[3]   = { 0.0, 0.0, 0.0 };

    XFov[0] = face->Vertexes[0]->Formed.X * clip->Fov;
    XFov[1] = face->Vertexes[1]->Formed.X * clip->Fov;
    XFov[2] = face->Vertexes[2]->Formed.X * clip->Fov;

    YFov[0] = face->Vertexes[0]->Formed.Y * f;
    YFov[1] = face->Vertexes[1]->Formed.Y * f;
    YFov[2] = face->Vertexes[2]->Formed.Y * f;

    VZ[0]   = face->Vertexes[0]->Formed.Z;
    VZ[1]   = face->Vertexes[1]->Formed.Z;
    VZ[2]   = face->Vertexes[2]->Formed.Z;

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
    EON_Renderer *rend = CX_zalloc(sizeof(EON_Renderer));
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
        CX_free(rend);
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

    rend->Camera = camera;

    eon_arrayReset(&(rend->Faces));
    eon_arrayReset(&(rend->Lights));

    /* rend->CMatrix initialized as side effect */
    eon_matrix4x4MakeRotation(rend->CMatrix, 2, -camera->Pan);
    /* tempMatrix initialized as side effect */
    eon_matrix4x4MakeRotation(tempMatrix,    1, -camera->Pitch);
    eon_matrix4x4Multiply(rend->CMatrix,        tempMatrix);
    /* tempMatrix initialized as side effect */
    eon_matrix4x4MakeRotation(tempMatrix,    3, -camera->Roll);
    eon_matrix4x4Multiply(rend->CMatrix,        tempMatrix);

    eon_clipInit(&(rend->Clip));
    eon_clipSetFrustum(&(rend->Clip), camera);

    return ret;
}

EON_Status EON_rendererTeardown(EON_Renderer *rend)
{
    eon_arrayReset(&(rend->Faces));
    eon_arrayReset(&(rend->Lights));

    return EON_OK;
}



EON_Status EON_rendererLight(EON_Renderer *rend,
                             EON_Light *light)
{
    EON_Float Xp = 0.0, Yp = 0.0, Zp = 0.0;
    EON_Int32 nLights = 0;

    if (!rend && !light) {
        return EON_ERROR;
    }
    eon_arrayLength(&(rend->Lights), &nLights);
    if (light->Type == EON_LIGHT_NONE || nLights > EON_MAX_LIGHTS) {
        return EON_OK;
    }

    if (light->Type == EON_LIGHT_VECTOR) {
        Xp = light->Coords.X;
        Yp = light->Coords.Y;
        Zp = light->Coords.Z;
    } else if (light->Type & EON_LIGHT_POINT) {
        Xp = light->Coords.X - rend->Camera->Position.X;
        Yp = light->Coords.Y - rend->Camera->Position.Y;
        Zp = light->Coords.Z - rend->Camera->Position.Z;
    }

    eon_lightInfo li;
    li.Light = light;
    eon_matrix4x4Apply(rend->CMatrix,
                       Xp, Yp, Zp,
                       &(li.Pos.X), &(li.Pos.Y), &(li.Pos.Z));

    eon_arrayAppend(&(rend->Lights), &li);
    return EON_OK;
}


typedef struct eon_polysortcontext_ {
    eon_faceInfo *Base;
} eon_polySortContext;

static int eon_siftCmp(eon_faceInfo *fx, eon_faceInfo *fy)
{
    return (fx->ZD < fy->ZD) ?1 :0;
}

static void eon_faceSwap(eon_faceInfo *fx, eon_faceInfo *fy)
{
    eon_faceInfo tmp = *fx;
    *fx              = *fy;
    *fy              = tmp;
    return;
}

static void eon_siftDown(eon_polySortContext *SC,
                         int L, int U, int dir)
{	
    int c = 0;
    while (1) {
        c = L + L;
        if (c > U) {
            break;
        }
        if ((c < U) && dir ^ eon_siftCmp(&SC->Base[c + 1], &SC->Base[c])) {
            c++;
        }
        if (dir ^ eon_siftCmp(&SC->Base[L], &SC->Base[c])) {
            return;
        }
        eon_faceSwap(&(SC->Base[L]), &(SC->Base[c]));
        L = c;
    }
}

static void eon_polySort(eon_polySortContext *SC,
                         eon_faceInfo *base, int nel, EON_SortMode dir)
{
    int i = 0;

    if (dir == 0) {
        return;
    }

    SC->Base = base-1;
    for (i = nel/2; i > 0; i--)
        eon_siftDown(SC, i, nel, dir);
    for (i = nel; i > 1; i--) {
        eon_faceSwap(&(base[0]), &(SC->Base[i]));
        eon_siftDown(SC, 1, i - 1, dir);
    }
    return;
}



EON_Status EON_rendererProcess(EON_Renderer *rend,
                               EON_Frame *frame)
{
    EON_Int32 j = 0, nFaces = 0;
    eon_polySortContext SC = { NULL };

    if (!rend && !frame) {
        return EON_ERROR;
    }
 
    eon_arrayLength(&(rend->Faces), &nFaces);

    eon_polySort(&SC, rend->Faces.Data, nFaces, rend->Camera->Sort); /* FIXME */

    for (j = 0; j < nFaces; j++) {
        eon_faceInfo *fi = eon_arrayGet(&(rend->Faces), j);
        eon_clipRenderFace(&(rend->Clip), rend, fi->Face, frame);
    }

    return EON_OK;
}

static void eon_rendererSetupObjectMatrixes(EON_Renderer *rend,
                                            EON_Object *obj,
                                            EON_Float *objMatrix,
                                            EON_Float *normMatrix,
                                            EON_Float *bmatrix,
                                            EON_Float *bnmatrix)
{
    EON_Float tempMatrix[4 * 4];

    if (obj->GenMatrix) {
        eon_matrix4x4MakeRotation(normMatrix, 1, obj->Rotation.X);
        eon_matrix4x4MakeRotation(tempMatrix, 2, obj->Rotation.Y);
        eon_matrix4x4Multiply(normMatrix,        tempMatrix);
        eon_matrix4x4MakeRotation(tempMatrix, 3, obj->Rotation.Z);
        eon_matrix4x4Multiply(normMatrix,        tempMatrix);
        memcpy(objMatrix, normMatrix, sizeof(EON_Float) * 4 * 4);
    } else {
        memcpy(normMatrix, obj->RMatrix, sizeof(EON_Float) * 4 * 4);
    }

    if (bnmatrix) {
        eon_matrix4x4Multiply(normMatrix, bnmatrix);
    }

    if (obj->GenMatrix) {
        eon_matrix4x4MakeTranslation(tempMatrix,
                                     obj->Position.X,
                                     obj->Position.Y,
                                     obj->Position.Z);
        eon_matrix4x4Multiply(objMatrix, tempMatrix);
    } else {
        memcpy(objMatrix, obj->TMatrix, sizeof(EON_Float) * 4 * 4);
    }

    if (bmatrix) {
        eon_matrix4x4Multiply(objMatrix, bmatrix);
    }
    return;
}

static EON_Status eon_rendererProcessObjectChildrens(EON_Renderer *rend,
                                                     EON_Object *object,
                                                     EON_Float *objMatrix,
                                                     EON_Float *normMatrix)
{
    int i = 0;
    if (!object) {
        return EON_ERROR;
    }

    for (i = 0; i < EON_MAX_CHILDREN; i ++) {
        if (object->Children[i]) {
            eon_rendererProcessObject(rend, object->Children[i],
                                      objMatrix, normMatrix);
        }
    }

    return EON_OK;
}

static int eon_processFaceNull(EON_Face *face, EON_Renderer *rend)
{
    return 0; /* NOP! */
}

static int eon_processFaceFlatLightining(EON_Face *face,
                                         EON_Renderer *rend)
{
    /* TODO */
    return 0;
}

static int eon_processFaceGouradShading(EON_Face *face,
                                        EON_Renderer *rend)
{
    /* TODO */
    return 0;
}

static int eon_processFaceFillEnvironment(EON_Face *face,
                                          EON_Renderer *rend)
{
    return 0;
}

static void eon_rendererAdjustVertexMatrix(EON_Renderer *rend,
                                           EON_Object *obj,
                                           EON_Float *objMatrix,
                                           EON_Float *normMatrix)
{
    EON_UInt32 x = obj->NumVertexes;
    EON_Vertex *V = obj->Vertexes;

    do {
        eon_matrix4x4Apply(objMatrix,
                            V->Coords.X,  V->Coords.Y,  V->Coords.Z,
                           &V->Formed.X, &V->Formed.Y, &V->Formed.Z);
        eon_matrix4x4Apply(normMatrix,
                            V->Norm.X,        V->Norm.Y,        V->Norm.Z,
                           &V->NormFormed.X, &V->NormFormed.Y, &V->NormFormed.Z);
        V++;
    } while (--x);

    return;
}

static EON_Status eon_rendererAppendFace(EON_Renderer *rend,
                                         EON_Face *face)
{
    eon_faceInfo fi = {
        .ZD = face->Vertexes[0]->Formed.Z + 
              face->Vertexes[1]->Formed.Z +
              face->Vertexes[2]->Formed.Z,
        .Face = face,
    };
    return eon_arrayAppend(&(rend->Faces), &fi);
}

static int eon_rendererIsFaceVisible(EON_Face *face, EON_Vector3 *N)
{
    EON_Vector3 *V = (EON_Vector3 *)&(face->Vertexes[0]->Formed);
    EON_Double p = eon_Vector3DotProduct(N, V);
    return p < EON_ZEROF;
}

static void eon_rendererAdjustByCamera(EON_Renderer *rend,
                                       EON_Float *objMatrix,
                                       EON_Float *normMatrix)
{
    EON_Float tempMatrix[4 * 4];
    EON_Vector3 *P = &(rend->Camera->Position);

    eon_matrix4x4MakeTranslation(tempMatrix, P->X, P->Y, P->Z);
    eon_matrix4x4Multiply(objMatrix, tempMatrix);
    eon_matrix4x4Multiply(objMatrix, rend->CMatrix);
    eon_matrix4x4Multiply(normMatrix, rend->CMatrix);
}

static int eon_faceIsFlatShaded(EON_Face *face)
{
    return (face && face->Material
         && (face->Material->_shadeMode & EON_SHADE_FLAT));
}

static EON_Status eon_rendererProcessObject(EON_Renderer *rend,
                                            EON_Object *object,
                                            EON_Float *bmatrix,
                                            EON_Float *bnmatrix)
{
    EON_Int32 j = 0, nFaces = 0;
    EON_Float objMatrix[4 * 4], normMatrix[4 * 4];/*, tempMatrix[4 * 4];*/
    EON_Vector3 N = { .X = 0.0f, .Y = 0.0f, .Z = 0.0f };

    if (!rend || !object || !object->NumFaces || !object->NumVertexes) {
        return EON_ERROR; /* log */
    }
    eon_arrayLength(&(rend->Faces), &nFaces);
    nFaces += object->NumFaces;
    if (nFaces >= EON_MAX_TRIANGLES) {
        return EON_ERROR; /* log */
    }

    rend->TriStats[EON_TRI_STAT_INITIAL] += object->NumFaces;

    eon_rendererSetupObjectMatrixes(rend, object,
                                    objMatrix, normMatrix,
                                    bmatrix, bnmatrix);
    
    eon_rendererProcessObjectChildrens(rend, object, objMatrix, normMatrix);

    eon_rendererAdjustByCamera(rend, objMatrix, normMatrix);
    eon_rendererAdjustVertexMatrix(rend, object, objMatrix, normMatrix);

    for (j = 0; j < object->NumFaces; j++) {
        EON_Face *face = &(object->Faces[j]);
        if (object->BackfaceCull || eon_faceIsFlatShaded(face)) {
            eon_matrix4x4Apply(normMatrix,
                               face->Norm.X, face->Norm.Y, face->Norm.Z,
                               &N.X, &N.Y, &N.Z);
        }
        if ((!object->BackfaceCull || eon_rendererIsFaceVisible(face, &N))
         && eon_clipIsNeeded(&(rend->Clip), face)) {
            face->FlatShade = 0.0;

            face->_processFlatLightining(face, rend);
            face->_processFillEnvironment(face, rend);
            face->_processGouradShading(face, rend);

            eon_rendererAppendFace(rend, face);

            rend->TriStats[EON_TRI_STAT_CULLED]++;
        }
    }

    return EON_OK;
}

EON_Status EON_rendererObject(EON_Renderer *rend, EON_Object *object)
{
    return eon_rendererProcessObject(rend, object, 0, 0);
}

static int eon_renderFaceNull(EON_Renderer *renderer,
                              EON_Face *face, EON_Frame *frame)
{
    return 0; /* NOP! */
}


static int eon_cameraProjectPoint(EON_Camera *camera,
                                  const EON_Vector3 *worldPoint,
                                  EON_ScreenPoint *screenPoint)
{
    /* TODO */
    return 0;
}

static int eon_renderFaceVertexes(EON_Renderer *renderer,
                                  EON_Face *face, EON_Frame *frame)
{
    static const EON_RGB rgb = { .R = 255, .G = 255, .B = 255, .A = 255 };
    EON_UInt32 color = EON_RGBPack(&rgb);
    EON_Camera *cam = renderer->Camera; /* shortcut */
    EON_ScreenPoint point;

    /* loop unrolled */
    eon_cameraProjectPoint(cam, &(face->Vertexes[EON_X]->Formed), &point);
    EON_framePutPixel(frame, point.X, point.Y, color);

    eon_cameraProjectPoint(cam, &(face->Vertexes[EON_Y]->Formed), &point);
    EON_framePutPixel(frame, point.X, point.Y, color);

    eon_cameraProjectPoint(cam, &(face->Vertexes[EON_Z]->Formed), &point);
    EON_framePutPixel(frame, point.X, point.Y, color);

    return 0;
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

