/**************************************************************************
 * eon3d.c -- Eon3D is a simplistic 3D software renderer.                 *
 * (C) 2010-2013 Francesco Romani <fromani at gmail dot com>              *
 *                                                                        *
 * derived from                                                           *
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
 * Meaning: all good stuff is credited to the plush author(s).            *
 * All the bugs, misdesigns and pessimizations are credited to me. ;)     *
 *                                                                        *
 **************************************************************************/

#include "memorykit.h"
#include "logkit.h"
#include "eon3d.h"


#define EON_TAG "EON"


/******************************************************************************
** Forward Declarations
******************************************************************************/

/******************************************************************************
** Frustum Clipping Functions (clip.c)
******************************************************************************/

/*
  EON_ClipSetFrustum() sets up the clipping frustum.
  Parameters:
    cam: a camera allocated with EON_CamCreate().
  Returns:
    nothing
  Notes:
    Sets up the internal structures.
    DO NOT CALL THIS ROUTINE FROM WITHIN A EON_Render*() block.
*/
void EON_ClipSetFrustum(EON_Clip *clip, EON_Cam *cam);

/*
  EON_ClipRenderFace() renders a face and clips it to the frustum initialized
    with EON_ClipSetFrustum().
  Parameters:
    face: the face to render
  Returns:
    nothing
  Notes: this is used internally by EON_Render*(), so be careful. Kinda slow too.
*/
void EON_ClipRenderFace(EON_Clip *clip, EON_Face *face, EON_Frame *frame);

/*
  EON_ClipNeeded() decides whether the face is in the frustum, intersecting
    the frustum, or comEON_etely out of the frustum craeted with
    EON_ClipSetFrustum().
  Parameters:
    face: the face to check
  Returns:
    0: the face is out of the frustum, no drawing necessary
    1: the face is intersecting the frustum, splitting and drawing necessary
  Notes: this is used internally by EON_Render*(), so be careful. Kinda slow too.
*/
EON_sInt EON_ClipNeeded(EON_Clip *clip, EON_Face *face);


/******************************************************************************
** Built-in Rasterizers
******************************************************************************/

static void EON_PF_Null(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_WireF(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_SolidF(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_SolidG(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_TexF(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_TexG(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_PTexF(EON_Cam *, EON_Face *, EON_Frame *);
static void EON_PF_PTexG(EON_Cam *, EON_Face *, EON_Frame *);


/* Used internally; EON_FILL_* are stored in EON_Mat._st. */
enum {
    EON_FILL_SOLID       = 0x0,
    EON_FILL_TEXTURE     = 0x1,
    EON_FILL_ENVIRONMENT = 0x2
};


// math.c
//
void EON_MatrixRotate(EON_Float matrix[], EON_uChar m, EON_Float Deg)
{
    EON_uChar m1, m2;
    EON_Double c,s;
    EON_Double d = Deg * EON_PI / 180.0;
    memset(matrix, 0, sizeof(EON_Float) * 16);
    matrix[((m-1)<<2)+m-1] = matrix[15] = 1.0;
    m1 = (m % 3);
    m2 = ((m1+1) % 3);
    c = cos(d);
    s = sin(d);
    matrix[(m1<<2)+m1] = (EON_Float)c;
    matrix[(m1<<2)+m2] = (EON_Float)s;
    matrix[(m2<<2)+m2] = (EON_Float)c;
    matrix[(m2<<2)+m1] = (EON_Float)-s;
    return;
}

void EON_MatrixTranslate(EON_Float m[], EON_Float x, EON_Float y, EON_Float z)
{
    memset(m, 0, sizeof(EON_Float)*16);
    m[0] = m[4+1] = m[8+2] = m[12+3] = 1.0;
    m[0+3] = x;
    m[4+3] = y;
    m[8+3] = z;
    return;
}

void EON_MatrixMultiply(EON_Float *dest, EON_Float src[])
{
    EON_Float temp[16];
    EON_uInt i;
    memcpy(temp,dest,sizeof(EON_Float)*16);
    for (i = 0; i < 16; i += 4) {
        *dest++ = src[i+0]*temp[(0<<2)+0]+src[i+1]*temp[(1<<2)+0]+
                  src[i+2]*temp[(2<<2)+0]+src[i+3]*temp[(3<<2)+0];
        *dest++ = src[i+0]*temp[(0<<2)+1]+src[i+1]*temp[(1<<2)+1]+
                  src[i+2]*temp[(2<<2)+1]+src[i+3]*temp[(3<<2)+1];
        *dest++ = src[i+0]*temp[(0<<2)+2]+src[i+1]*temp[(1<<2)+2]+
                  src[i+2]*temp[(2<<2)+2]+src[i+3]*temp[(3<<2)+2];
        *dest++ = src[i+0]*temp[(0<<2)+3]+src[i+1]*temp[(1<<2)+3]+
                  src[i+2]*temp[(2<<2)+3]+src[i+3]*temp[(3<<2)+3];
    }
    return;
}

void EON_MatrixApply(EON_Float *m, EON_Float x, EON_Float y, EON_Float z,
                     EON_Float *outx, EON_Float *outy, EON_Float *outz)
{
    *outx = x*m[0] + y*m[1] + z*m[2] + m[3];
    *outy = x*m[4] + y*m[5] + z*m[6] + m[7];
    *outz = x*m[8] + y*m[9] + z*m[10] + m[11];
    return;
}

EON_Float EON_DotProduct(EON_Float x1, EON_Float y1, EON_Float z1,
                         EON_Float x2, EON_Float y2, EON_Float z2)
{
    return ((x1*x2)+(y1*y2)+(z1*z2));
}

void EON_NormalizeVector(EON_Float *x, EON_Float *y, EON_Float *z)
{
    EON_Double length = (*x)*(*x)+(*y)*(*y)+(*z)*(*z);
    if (length > 0.0000000001) {
        EON_Float t = (EON_Float)sqrt(length);
        *x /= t;
        *y /= t;
        *z /= t;
    } else {
        *x = *y = *z = 0.0;
    }
    return;
}

// frame

static inline EON_uInt32 eon_FrameSizeZBuffer(EON_Frame *f)
{
    return f->Width * f->Height * sizeof(EON_ZBuffer);
}

static inline void *eon_FrameDel(EON_Frame *f)
{
    if (f) {
        CX_pafree(f->ZBuffer);
        CX_pafree(f->Data);
        CX_free(f);
    }
    return NULL;
}

EON_Frame *EON_FrameCreate(EON_uInt32 Width, EON_uInt32 Height,
                           EON_uInt32 Bpp)
{
    EON_Frame *f = CX_zalloc(sizeof(*f));
    if (f) {
        f->Width = Width;
        f->Height = Height;
        f->Bpp = Bpp;
        f->Data = CX_paalloc(EON_FrameSize(f));
        f->ZBuffer = CX_paalloc(eon_FrameSizeZBuffer(f));

        if (f->Data == NULL || f->ZBuffer == NULL ) {
            f = eon_FrameDel(f);
        }
    }
    return f;
}

void EON_FrameDelete(EON_Frame *f)
{
    eon_FrameDel(f);
}

EON_uInt32 EON_FrameSize(EON_Frame *f)
{
    EON_uInt32 size = 0;
    if (f) {
        size = f->Width * f->Height * f->Bpp;
    }
    return size;
}

void EON_FrameClear(EON_Frame *f)
{
    if (f) {
        // clear framebuffer for next frame
        memset(f->Data, 0, EON_FrameSize(f));
        memset(f->ZBuffer, 0, eon_FrameSizeZBuffer(f));
    }
    return;
}

// obj.c
//
EON_Obj *EON_ObjScale(EON_Obj *o, EON_Float s)
{
    EON_uInt32 i = o->NumVertices;
    EON_Vertex *v = o->Vertices;
    while (i--) {
        v->x *= s;
        v->y *= s;
        v->z *= s;
        v++;
    }
    for (i = 0; i < EON_MAX_CHILDREN; i ++)
        if (o->Children[i])
            EON_ObjScale(o->Children[i],s);
    return o;
}

EON_Obj *EON_ObjStretch(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z)
{
    EON_uInt32 i = o->NumVertices;
    EON_Vertex *v = o->Vertices;
    while (i--) {
        v->x *= x;
        v->y *= y;
        v->z *= z;
        v++;
    }
    for (i = 0; i < EON_MAX_CHILDREN; i ++)
        if (o->Children[i])
            EON_ObjStretch(o->Children[i],x,y,z);
    return o;
}

void EON_ObjCentroid(EON_Obj *o, EON_Float *x, EON_Float *y, EON_Float *z)
{
    EON_uInt32 i = 0;
    EON_uInt32 n = o->NumVertices;
    EON_Double tX = 0.0, tY = 0.0, tZ = 0.0;
    for (i = 0; i < n; i++) {
        tX += o->Vertices[i].x;
        tY += o->Vertices[i].y;
        tZ += o->Vertices[i].z;
    }
    *x = tX / (EON_Double)n;
    *y = tY / (EON_Double)n;
    *z = tZ / (EON_Double)n;
    return;
}

EON_Obj *EON_ObjTranslate(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z)
{
    EON_uInt32 i = o->NumVertices;
    EON_Vertex *v = o->Vertices;
    while (i--) {
        v->x += x;
        v->y += y;
        v->z += z;
        v++;
    }
    return o;
}

EON_Obj *EON_ObjFlipNormals(EON_Obj *o)
{
    EON_uInt32 i = o->NumVertices;
    EON_Vertex *v = o->Vertices;
    EON_Face *f = o->Faces;
    while (i--) {
        v->nx = - v->nx;
        v->ny = - v->ny;
        v->nz = - v->nz;
        v++;
    }
    i = o->NumFaces;
    while (i--) {
        f->nx = - f->nx;
        f->ny = - f->ny;
        f->nz = - f->nz;
        f++;
    }
    for (i = 0; i < EON_MAX_CHILDREN; i ++)
        if (o->Children[i])
            EON_ObjFlipNormals(o->Children[i]);
    return o;
}

void EON_ObjDelete(EON_Obj *o)
{
    EON_uInt i;
    if (o) {
        for (i = 0; i < EON_MAX_CHILDREN; i ++)
            if (o->Children[i])
                EON_ObjDelete(o->Children[i]);
            if (o->Vertices)
                CX_free(o->Vertices);
            if (o->Faces)
                CX_free(o->Faces);
        CX_free(o);
    }
    return;
}

EON_Obj *EON_ObjCreate(EON_uInt32 nv, EON_uInt32 nf)
{
    EON_Obj *o = CX_zalloc(sizeof(EON_Obj));
    if (!o)
        return 0;
    o->GenMatrix = 1;
    o->BackfaceCull = 1;
    o->NumVertices = nv;
    o->NumFaces = nf;
    if (nv && !(o->Vertices = CX_zalloc(sizeof(EON_Vertex)*nv))) {
        CX_free(o);
        return 0;
    }
    if (nf && !(o->Faces = CX_zalloc(sizeof(EON_Face)*nf))) {
        CX_free(o->Vertices);
        CX_free(o);
        return 0;
    }
    return o;
}

void EON_ObjInfo(EON_Obj *o, void *logger)
{
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG, "Object (%p)", o);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Vertices: %i", o->NumVertices);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Faces: %i", o->NumFaces);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Generate Matrix: %s", (o->GenMatrix) ?"Yes" :"No");
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Backface Cull: %s", (o->GenMatrix) ?"Yes" :"No");
    return;
}

EON_Obj *EON_ObjClone(EON_Obj *o)
{
    EON_Face *iff, *of;
    EON_uInt32 i;
    EON_Obj *out = EON_ObjCreate(o->NumVertices,o->NumFaces);
    if (out) {
        return 0;
    }
    for (i = 0; i < EON_MAX_CHILDREN; i ++) {
        if (o->Children[i]) {
            out->Children[i] = EON_ObjClone(o->Children[i]);
        }
    }
    out->Xa = o->Xa; out->Ya = o->Ya; out->Za = o->Za;
    out->Xp = o->Xp; out->Yp = o->Yp; out->Zp = o->Zp;
    out->BackfaceCull = o->BackfaceCull;
    out->BackfaceIllumination = o->BackfaceIllumination;
    out->GenMatrix = o->GenMatrix;
    memcpy(out->Vertices, o->Vertices, sizeof(EON_Vertex) * o->NumVertices);
    iff = o->Faces;
    of = out->Faces;
    i = out->NumFaces;
    while (i--) {
        of->Vertices[0] = (EON_Vertex *)
          out->Vertices + (iff->Vertices[0] - o->Vertices);
        of->Vertices[1] = (EON_Vertex *)
          out->Vertices + (iff->Vertices[1] - o->Vertices);
        of->Vertices[2] = (EON_Vertex *)
          out->Vertices + (iff->Vertices[2] - o->Vertices);
        of->MappingU[0] = iff->MappingU[0];
        of->MappingV[0] = iff->MappingV[0];
        of->MappingU[1] = iff->MappingU[1];
        of->MappingV[1] = iff->MappingV[1];
        of->MappingU[2] = iff->MappingU[2];
        of->MappingV[2] = iff->MappingV[2];
        of->nx = iff->nx;
        of->ny = iff->ny;
        of->nz = iff->nz;
        of->Material = iff->Material;
        of++;
        iff++;
    }
    return out;
}

void EON_ObjSetMat(EON_Obj *o, EON_Mat *m, EON_Bool th)
{
    EON_sInt32 i = o->NumFaces;
    EON_Face *f = o->Faces;
    while (i--) {
        (f++)->Material = m;
    }
    if (th) {
        for (i = 0; i < EON_MAX_CHILDREN; i++) {
            if (o->Children[i]) {
                EON_ObjSetMat(o->Children[i],m,th);
            }
        }
    }
    return;
}

EON_Obj *EON_ObjCalcNormals(EON_Obj *obj)
{
    EON_uInt32 i;
    EON_Vertex *v = obj->Vertices;
    EON_Face *f = obj->Faces;
    EON_Double x1, x2, y1, y2, z1, z2;
    i = obj->NumVertices;
    while (i--) {
        v->nx = 0.0;
        v->ny = 0.0;
        v->nz = 0.0;
        v++;
    }
    i = obj->NumFaces;
    while (i--) {
        x1 = f->Vertices[0]->x-f->Vertices[1]->x;
        x2 = f->Vertices[0]->x-f->Vertices[2]->x;
        y1 = f->Vertices[0]->y-f->Vertices[1]->y;
        y2 = f->Vertices[0]->y-f->Vertices[2]->y;
        z1 = f->Vertices[0]->z-f->Vertices[1]->z;
        z2 = f->Vertices[0]->z-f->Vertices[2]->z;
        f->nx = (EON_Float) (y1*z2 - z1*y2);
        f->ny = (EON_Float) (z1*x2 - x1*z2);
        f->nz = (EON_Float) (x1*y2 - y1*x2);
        EON_NormalizeVector(&f->nx, &f->ny, &f->nz);
        f->Vertices[0]->nx += f->nx;
        f->Vertices[0]->ny += f->ny;
        f->Vertices[0]->nz += f->nz;
        f->Vertices[1]->nx += f->nx;
        f->Vertices[1]->ny += f->ny;
        f->Vertices[1]->nz += f->nz;
        f->Vertices[2]->nx += f->nx;
        f->Vertices[2]->ny += f->ny;
        f->Vertices[2]->nz += f->nz;
        f++;
    }
    v = obj->Vertices;
    i = obj->NumVertices;
    do {
        EON_NormalizeVector(&v->nx, &v->ny, &v->nz);
        v++;
    } while (--i);
    for (i = 0; i < EON_MAX_CHILDREN; i ++) {
        if (obj->Children[i]) {
            EON_ObjCalcNormals(obj->Children[i]);
        }
    }
    return obj;
}

// mat.c
//

static void eon_SetMaterialPutFace(EON_Mat *m);

EON_Mat *EON_MatCreate()
{
    EON_Mat *m = CX_zalloc(sizeof(EON_Mat));
    if (m) {
        m->TexScaling = 1.0f;
        m->Ambient.R = m->Ambient.G = m->Ambient.B = 0;
        m->Diffuse.R = m->Diffuse.G = m->Diffuse.B = 128;
        m->Specular.R = m->Specular.G = m->Specular.B = 128;
        m->Shininess = 4;
        m->FadeDist = 1000.0;
        m->zBufferable = 1;
    }
    return m;
}

void EON_MatDelete(EON_Mat *m)
{
    if (m) {
        CX_free(m);
    }
    return;
}

void EON_MatInit(EON_Mat *m)
{
    if (m->Shininess < 1)
        m->Shininess = 1;

    m->_ft = ((m->Texture) ?EON_FILL_TEXTURE :EON_FILL_SOLID);
    m->_st = m->ShadeType;

    eon_SetMaterialPutFace(m);

    return;
}


static void eon_SetMaterialPutFace(EON_Mat *m)
{
    m->_PutFace = EON_PF_Null;
    switch (m->_ft) {
    case EON_FILL_SOLID:
        switch(m->_st) {
        case EON_SHADE_WIREFRAME:
            m->_PutFace = EON_PF_WireF;
            break;
        case EON_SHADE_NONE:
        case EON_SHADE_FLAT:
        case EON_SHADE_FLAT_DISTANCE:
        case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
            m->_PutFace = EON_PF_SolidF;
            break;
        case EON_SHADE_GOURAUD:
        case EON_SHADE_GOURAUD_DISTANCE:
        case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
            m->_PutFace = EON_PF_SolidG;
            break;
        }
        break;
    case EON_FILL_ENVIRONMENT: // fallthrough
    case EON_FILL_TEXTURE:
        if (m->PerspectiveCorrect)
            switch (m->_st) {
            case EON_SHADE_NONE:
            case EON_SHADE_FLAT:
            case EON_SHADE_FLAT_DISTANCE:
            case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
                m->_PutFace = EON_PF_PTexF;
                break;
            case EON_SHADE_GOURAUD:
            case EON_SHADE_GOURAUD_DISTANCE:
            case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
                m->_PutFace = EON_PF_PTexG;
                break;
            }
        else
            switch (m->_st) {
            case EON_SHADE_NONE:
            case EON_SHADE_FLAT:
            case EON_SHADE_FLAT_DISTANCE:
            case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
                m->_PutFace = EON_PF_TexF;
                break;
            case EON_SHADE_GOURAUD:
            case EON_SHADE_GOURAUD_DISTANCE:
            case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
                m->_PutFace = EON_PF_TexG;
                break;
            }
        break;
    }
    return;
}

static const char *eon_PutFaceName(void *addr)
{
    const char *name = "N/A";
    if (addr == EON_PF_Null) {
        name = "Null";
    } else if (addr == EON_PF_SolidF) {
        name = "SolidF";
    } else if (addr == EON_PF_SolidG) {
        name = "SolidG";
    } else if (addr == EON_PF_TexF) {
        name = "TexF";
    } else if (addr == EON_PF_TexG) {
        name = "TexG";
    } else if (addr == EON_PF_PTexF) {
        name = "PTexF";
    } else if (addr == EON_PF_PTexG) {
        name = "PTexG";
    } else if (addr == EON_PF_WireF) {
        name = "WireF";
    }
    return name;
}

void EON_MatInfo(EON_Mat *m, void *logger)
{
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG, "Material (%p)", m);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Rasterizer: %s",
                 eon_PutFaceName(m->_PutFace));
    return;
}

// light.c
//

EON_Light *EON_LightSet(EON_Light *light, EON_uChar mode, EON_Float x, EON_Float y,
                        EON_Float z, EON_Float intensity, EON_Float halfDist)
{
    EON_Float m[16], m2[16];
    light->Type = mode;
    light->Intensity = intensity;
    light->HalfDistSquared = halfDist*halfDist;
    switch (mode) {
    case EON_LIGHT_VECTOR:
        EON_MatrixRotate(m, 1, x);
        EON_MatrixRotate(m2, 2, y);
        EON_MatrixMultiply(m, m2);
        EON_MatrixRotate(m2, 3, z);
        EON_MatrixMultiply(m, m2);
        EON_MatrixApply(m, 0.0, 0.0, -1.0,
                        &light->Pos.X, &light->Pos.Y, &light->Pos.Z);
        break;
    case EON_LIGHT_POINT_ANGLE:
    case EON_LIGHT_POINT_DISTANCE:
    case EON_LIGHT_POINT:
        light->Pos.X = x;
        light->Pos.Y = y;
        light->Pos.Z = z;
        break;
    }
    return light;
}

EON_Light *EON_LightCreate()
{
    return CX_zalloc(sizeof(EON_Light));
}

void EON_LightDelete(EON_Light *l)
{
    CX_free(l);
}

EON_Light *EON_LightNew(EON_uChar mode, EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity, EON_Float halfDist)
{
    EON_Light *l = EON_LightCreate();
    if (l) {
        l = EON_LightSet(l, mode, x, y, z, intensity, halfDist);
    }
    return l;
}

// cam.c
//

void EON_CamDelete(EON_Cam *c)
{
    CX_free(c);
    return;
}

void EON_CamSetTarget(EON_Cam *c, EON_Float x, EON_Float y, EON_Float z)
{
    EON_Double dx, dy, dz;
    dx = x - c->Pos.X;
    dy = y - c->Pos.Y;
    dz = z - c->Pos.Z;
    c->Roll = 0;
    if (dz > 0.0001f) {
        c->Pan = (EON_Float) (-atan(dx/dz)*(180.0/EON_PI));
        dz /= cos(c->Pan*(EON_PI/180.0));
        c->Pitch = (EON_Float) (atan(dy/dz)*(180.0/EON_PI));
    } else if (dz < -0.0001f) {
        c->Pan = (EON_Float) (180.0-atan(dx/dz)*(180.0/EON_PI));
        dz /= cos((c->Pan-180.0f)*(EON_PI/180.0));
        c->Pitch = (EON_Float) (-atan(dy/dz)*(180.0/EON_PI));
    } else {
        c->Pan = 0.0f;
        c->Pitch = -90.0f;
    }
    return;
}


EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov)
{
    EON_Cam *c = CX_zalloc(sizeof(EON_Cam));
    if (c) {
        c->Fov = fov;
        c->AspectRatio = ar;
        c->ClipRight = c->ScreenWidth = sw;
        c->ClipBottom = c->ScreenHeight = sh;
        c->CenterX = sw>>1;
        c->CenterY = sh>>1;
        c->ClipBack = 8.0e30f;
    }
    return c;
}

// clip.c
//

static void eon_FindNormal(EON_Double x2, EON_Double x3,
                           EON_Double y2, EON_Double y3,
                           EON_Double zv,
                           EON_Double *res);

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt eon_ClipToPlane(EON_Clip *clip, EON_uInt numVerts, EON_Double *plane);

void EON_ClipSetFrustum(EON_Clip *clip, EON_Cam *cam)
{
    clip->AdjAsp = 1.0 / cam->AspectRatio;
    clip->Fov = EON_Clamp(cam->Fov,1.0,179.0);
    clip->Fov = (1.0/tan(clip->Fov*(EON_PI/360.0)))*(EON_Double) (cam->ClipRight-cam->ClipLeft);
    clip->Cx = cam->CenterX<<20;
    clip->Cy = cam->CenterY<<20;
    clip->Cam = cam;
    memset(clip->ClipPlanes, 0, sizeof(clip->ClipPlanes));

    /* Back */
    clip->ClipPlanes[0][2] = -1.0;
    clip->ClipPlanes[0][3] = -cam->ClipBack;

    /* Left */
    clip->ClipPlanes[1][3] = 0.00000001;
    if (cam->ClipLeft == cam->CenterX) {
        clip->ClipPlanes[1][0] = 1.0;
    }
    else
        eon_FindNormal(-100,-100,
                    100, -100,
                    clip->Fov*-100.0/(cam->ClipLeft-cam->CenterX),
                    clip->ClipPlanes[1]);
    if (cam->ClipLeft > cam->CenterX) {
        clip->ClipPlanes[1][0] = -clip->ClipPlanes[1][0];
        clip->ClipPlanes[1][1] = -clip->ClipPlanes[1][1];
        clip->ClipPlanes[1][2] = -clip->ClipPlanes[1][2];
    }

    /* Right */
    clip->ClipPlanes[2][3] = 0.00000001;
    if (cam->ClipRight == cam->CenterX) {
        clip->ClipPlanes[2][0] = -1.0;
    } else {
        eon_FindNormal(100,100,
                    -100, 100,
                    clip->Fov*100.0/(cam->ClipRight-cam->CenterX),
                    clip->ClipPlanes[2]);
    }
    if (cam->ClipRight < cam->CenterX) {
        clip->ClipPlanes[2][0] = -clip->ClipPlanes[2][0];
        clip->ClipPlanes[2][1] = -clip->ClipPlanes[2][1];
        clip->ClipPlanes[2][2] = -clip->ClipPlanes[2][2];
    }
    /* Top */
    clip->ClipPlanes[3][3] = 0.00000001;
    if (cam->ClipTop == cam->CenterY) {
        clip->ClipPlanes[3][1] = -1.0;
    } else
        eon_FindNormal(100, -100,
                    100, 100,
                    clip->Fov*clip->AdjAsp*100.0/(cam->CenterY-cam->ClipTop),
                    clip->ClipPlanes[3]);
    if (cam->ClipTop > cam->CenterY) {
        clip->ClipPlanes[3][0] = -clip->ClipPlanes[3][0];
        clip->ClipPlanes[3][1] = -clip->ClipPlanes[3][1];
        clip->ClipPlanes[3][2] = -clip->ClipPlanes[3][2];
    }

    /* Bottom */
    clip->ClipPlanes[4][3] = 0.00000001;
    if (cam->ClipBottom == cam->CenterY) {
        clip->ClipPlanes[4][1] = 1.0;
    } else
        eon_FindNormal(-100, 100,
                    -100, -100,
                    clip->Fov*clip->AdjAsp*-100.0/(cam->CenterY-cam->ClipBottom),
                    clip->ClipPlanes[4]);
    if (cam->ClipBottom < cam->CenterY) {
        clip->ClipPlanes[4][0] = -clip->ClipPlanes[4][0];
        clip->ClipPlanes[4][1] = -clip->ClipPlanes[4][1];
        clip->ClipPlanes[4][2] = -clip->ClipPlanes[4][2];
    }
    return;
}

inline static void eon_ClipVertexToScreen(EON_Face *face, EON_uInt a, EON_Clip *clip)
{
    EON_Double xf, yf, zf;
    face->Scr[a].Z = 1.0f/face->Vertices[a]->xformedz;
    zf = clip->Fov * face->Scr[a].Z;
    xf = zf * face->Vertices[a]->xformedx;
    yf = zf * face->Vertices[a]->xformedy;
    face->Scr[a].X = clip->Cx + ((EON_sInt32)((xf *                (float)(1<<20))));
    face->Scr[a].Y = clip->Cy - ((EON_sInt32)((yf * clip->AdjAsp * (float)(1<<20))));
    return;
}


void EON_ClipRenderFace(EON_Clip *clip, EON_Face *face, EON_Frame *frame)
{
    EON_uInt a, numVerts = 3;

    for (a = 0; a < 3; a ++) {
        clip->CL[0].newVertices[a] = *(face->Vertices[a]);
        clip->CL[0].Shades[a] = face->Shades[a];
        clip->CL[0].MappingU[a] = face->MappingU[a];
        clip->CL[0].MappingV[a] = face->MappingV[a];
    }

    a = (clip->ClipPlanes[0][3] < 0.0 ? 0 : 1);
    while (a < NUM_CLIP_PLANES && numVerts > 2) {
        numVerts = eon_ClipToPlane(clip, numVerts, clip->ClipPlanes[a]);
        memcpy(&clip->CL[0],&clip->CL[1],sizeof(clip->CL)/2);
        a++;
    }
    if (numVerts > 2) {
        EON_uInt k, w;
        EON_Face newface;
        memcpy(&newface, face, sizeof(EON_Face));
        for (k = 2; k < numVerts; k ++) {
            for (a = 0; a < 3; a++) {
                if (a == 0)
                    w = 0;
                else
                    w = a+(k-2);
                newface.Vertices[a] = clip->CL[0].newVertices+w;
                newface.Shades[a] = (EON_Float) clip->CL[0].Shades[w];
                newface.MappingU[a] = (EON_sInt32)clip->CL[0].MappingU[w];
                newface.MappingV[a] = (EON_sInt32)clip->CL[0].MappingV[w];
                eon_ClipVertexToScreen(&newface, a, clip);
            }
            newface.Material->_PutFace(clip->Cam, &newface, frame);
            clip->Info->TriStats[TRI_STAT_TESSELLATION]++;
        }
        clip->Info->TriStats[TRI_STAT_CLIPPING]++;
    }
}

EON_sInt EON_ClipNeeded(EON_Clip *clip, EON_Face *face)
{
    EON_Double dr = (clip->Cam->ClipRight - clip->Cam->CenterX);
    EON_Double dl = (clip->Cam->ClipLeft - clip->Cam->CenterX);
    EON_Double db = (clip->Cam->ClipBottom - clip->Cam->CenterY);
    EON_Double dt = (clip->Cam->ClipTop - clip->Cam->CenterY);
    EON_Double f = clip->Fov * clip->AdjAsp;
    return ((clip->Cam->ClipBack <= 0.0 ||
            face->Vertices[0]->xformedz <= clip->Cam->ClipBack ||
            face->Vertices[1]->xformedz <= clip->Cam->ClipBack ||
            face->Vertices[2]->xformedz <= clip->Cam->ClipBack) &&
            (face->Vertices[0]->xformedz >= 0 ||
            face->Vertices[1]->xformedz >= 0 ||
            face->Vertices[2]->xformedz >= 0) &&
            (face->Vertices[0]->xformedx*clip->Fov<=dr*face->Vertices[0]->xformedz ||
            face->Vertices[1]->xformedx*clip->Fov<=dr*face->Vertices[1]->xformedz ||
            face->Vertices[2]->xformedx*clip->Fov<=dr*face->Vertices[2]->xformedz) &&
            (face->Vertices[0]->xformedx*clip->Fov>=dl*face->Vertices[0]->xformedz ||
            face->Vertices[1]->xformedx*clip->Fov>=dl*face->Vertices[1]->xformedz ||
            face->Vertices[2]->xformedx*clip->Fov>=dl*face->Vertices[2]->xformedz) &&
            (face->Vertices[0]->xformedy*f<=db*face->Vertices[0]->xformedz ||
            face->Vertices[1]->xformedy*f<=db*face->Vertices[1]->xformedz ||
            face->Vertices[2]->xformedy*f<=db*face->Vertices[2]->xformedz) &&
            (face->Vertices[0]->xformedy*f>=dt*face->Vertices[0]->xformedz ||
            face->Vertices[1]->xformedy*f>=dt*face->Vertices[1]->xformedz ||
            face->Vertices[2]->xformedy*f>=dt*face->Vertices[2]->xformedz));
}



static void eon_FindNormal(EON_Double x2, EON_Double x3,EON_Double y2, EON_Double y3,
                           EON_Double zv, EON_Double *res)
{
    res[0] = zv*(y2-y3);
    res[1] = zv*(x3-x2);
    res[2] = x2*y3 - y2*x3;
}

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt eon_ClipToPlane(EON_Clip *clip,
                                EON_uInt numVerts, EON_Double *plane)
{
    EON_uInt i, nextvert, curin, nextin;
    EON_Double curdot, nextdot, scale;
    EON_uInt invert = 0, outvert = 0;
    curdot = clip->CL[0].newVertices[0].xformedx*plane[0] +
             clip->CL[0].newVertices[0].xformedy*plane[1] +
             clip->CL[0].newVertices[0].xformedz*plane[2];
    curin = (curdot >= plane[3]);

    for (i = 0 ; i < numVerts; i++) {
        nextvert = (i + 1) % numVerts;
        if (curin) {
            clip->CL[1].Shades[outvert] = clip->CL[0].Shades[invert];
            clip->CL[1].MappingU[outvert] = clip->CL[0].MappingU[invert];
            clip->CL[1].MappingV[outvert] = clip->CL[0].MappingV[invert];
            clip->CL[1].newVertices[outvert++] = clip->CL[0].newVertices[invert];
        }
        nextdot = clip->CL[0].newVertices[nextvert].xformedx*plane[0] +
                  clip->CL[0].newVertices[nextvert].xformedy*plane[1] +
                  clip->CL[0].newVertices[nextvert].xformedz*plane[2];
        nextin = (nextdot >= plane[3]);
        if (curin != nextin) {
            scale = (plane[3] - curdot) / (nextdot - curdot);
            clip->CL[1].newVertices[outvert].xformedx = (EON_Float) (clip->CL[0].newVertices[invert].xformedx +
                (clip->CL[0].newVertices[nextvert].xformedx - clip->CL[0].newVertices[invert].xformedx)
                 * scale);
            clip->CL[1].newVertices[outvert].xformedy = (EON_Float) (clip->CL[0].newVertices[invert].xformedy +
                (clip->CL[0].newVertices[nextvert].xformedy - clip->CL[0].newVertices[invert].xformedy)
                 * scale);
            clip->CL[1].newVertices[outvert].xformedz = (EON_Float) (clip->CL[0].newVertices[invert].xformedz +
                (clip->CL[0].newVertices[nextvert].xformedz - clip->CL[0].newVertices[invert].xformedz)
                 * scale);
            clip->CL[1].Shades[outvert] = clip->CL[0].Shades[invert] +
                        (clip->CL[0].Shades[nextvert] - clip->CL[0].Shades[invert]) * scale;
            clip->CL[1].MappingU[outvert] = clip->CL[0].MappingU[invert] +
                (clip->CL[0].MappingU[nextvert] - clip->CL[0].MappingU[invert]) * scale;
           clip->CL[1].MappingV[outvert] = clip->CL[0].MappingV[invert] +
                (clip->CL[0].MappingV[nextvert] - clip->CL[0].MappingV[invert]) * scale;
            outvert++;
        }
        curdot = nextdot;
        curin = nextin;
        invert++;
    }
    return outvert;
}

// plush.c
//


void EON_TexDelete(EON_Texture *t)
{
    if (t) {
        if (t->Data)
            CX_free(t->Data);
        if (t->PaletteData)
            CX_free(t->PaletteData);
        CX_free(t);
    }
}

void EON_TexInfo(EON_Texture *t, void *logger)
{
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG, "Texture (%p)", t);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Colors: %i", t->NumColors);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Data: %p (Palette=%p)", t->Data, t->PaletteData);
    CX_log_trace(logger, CX_LOG_INFO, EON_TAG,
                 "* Dimensions: %ix%i", t->iWidth, t->iHeight);
    return;
}

// putface.h
//

#define PUTFACE_SORT() \
    i0 = 0; i1 = 1; i2 = 2; \
    if (Face->Scr[0].Y > Face->Scr[1].Y) { \
        i0 = 1; i1 = 0; \
    } \
    if (Face->Scr[i0].Y > Face->Scr[2].Y) { \
        i2 ^= i0; i0 ^= i2; i2 ^= i0; \
    } \
    if (Face->Scr[i1].Y > Face->Scr[i2].Y) { \
        i2 ^= i1; i1 ^= i2; i2 ^= i1; \
    }


#define PUTFACE_SORT_TEX() \
    PUTFACE_SORT(); \
    MappingU1=Face->MappingU[i0]*Texture->uScale*\
              Face->Material->TexScaling;\
    MappingV1=Face->MappingV[i0]*Texture->vScale*\
              Face->Material->TexScaling;\
    MappingU2=Face->MappingU[i1]*Texture->uScale*\
              Face->Material->TexScaling;\
    MappingV2=Face->MappingV[i1]*Texture->vScale*\
              Face->Material->TexScaling;\
    MappingU3=Face->MappingU[i2]*Texture->uScale*\
              Face->Material->TexScaling;\
    MappingV3=Face->MappingV[i2]*Texture->vScale*\
              Face->Material->TexScaling;

inline static EON_uInt32 eon_PickColorPS(const EON_Byte *pal, EON_Byte v, EON_Float shade)
{
    EON_uInt32 R = ((EON_uInt32)(pal[3 * v + 0] * shade)) & 0xFF;
    EON_uInt32 G = ((EON_uInt32)(pal[3 * v + 1] * shade)) & 0xFF;
    EON_uInt32 B = ((EON_uInt32)(pal[3 * v + 2] * shade)) & 0xFF;
    EON_uInt32 A = 0xFF000000;
    return (A | R << 16 | G << 8| B);
}

inline static EON_uInt32 eon_PickColorS(const EON_Face *f, EON_Float shade)
{
    EON_uInt32 R = ((EON_uInt32)(f->Material->Ambient.R * shade)) & 0xFF;
    EON_uInt32 G = ((EON_uInt32)(f->Material->Ambient.G * shade)) & 0xFF;
    EON_uInt32 B = ((EON_uInt32)(f->Material->Ambient.B * shade)) & 0xFF;
    EON_uInt32 A = 0xFF000000;
    return (A | R << 16 | G << 8| B);
}

inline static EON_sInt32 eon_ToScreen(EON_sInt32 val)
{
    return (val + (1<<19)) >> 20;
}

inline static void eon_DrawLine(EON_uInt32 *fb, EON_sInt32 pitch,
                                EON_sInt32 x0, EON_sInt32 y0,
                                EON_sInt32 x1, EON_sInt32 y1,
                                EON_uInt32 pixel)
{
    EON_sInt32 dx = abs(x0 - x1);
    EON_sInt32 dy = abs(y0 - y1);
    EON_sInt32 sx = (x0 < x1) ?+1 :-1;
    EON_sInt32 sy = (y0 < y1) ?+1 :-1;
    EON_sInt32 err = dx - dy;

    while (x0 != x1 || y0 != y1) {
        EON_sInt32 e2 = 2 * err;
        EON_sInt32 offset = x0 + y0 * pitch;

        fb[offset] = pixel;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 <  dx) {
            err += dx;
            y0 += sy;
        }
    }

    return;
}

// pf misc

static void EON_PF_Null(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
    return; /* nothing to do */
}

static void EON_PF_WireF(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
    EON_uChar i0 = 0, i1 = 1, i2 = 2;

    EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;

    EON_sInt32 X0, X1, X2;
    EON_sInt32 Y0, Y1, Y2;
    EON_uInt32 color = eon_PickColorS(Face, 1.0);

    X0 = eon_ToScreen(Face->Scr[i0].X);
    X1 = eon_ToScreen(Face->Scr[i1].X);
    X2 = eon_ToScreen(Face->Scr[i2].X);
    X0 = EON_Clamp(X0, 0, Frame->Width-1);
    X1 = EON_Clamp(X1, 0, Frame->Width-1);
    X2 = EON_Clamp(X2, 0, Frame->Width-1);
    Y0 = eon_ToScreen(Face->Scr[i0].Y);
    Y1 = eon_ToScreen(Face->Scr[i1].Y);
    Y2 = eon_ToScreen(Face->Scr[i2].Y);
    Y0 = EON_Clamp(Y0, 0, Frame->Height-1);
    Y1 = EON_Clamp(Y1, 0, Frame->Height-1);
    Y2 = EON_Clamp(Y2, 0, Frame->Height-1);

    eon_DrawLine(gmem, cam->ScreenWidth, X0, Y0, X1, Y1, color);
    eon_DrawLine(gmem, cam->ScreenWidth, X0, Y0, X2, Y2, color);
    eon_DrawLine(gmem, cam->ScreenWidth, X1, Y1, X2, Y2, color);

    return;
}

// pf_solid.c
//

static void EON_PF_SolidF(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
    EON_uChar i0, i1, i2;

    EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
    EON_ZBuffer *zbuf = Frame->ZBuffer;

    EON_sInt32 X1, X2;
    EON_sInt32 XL1, XL2;
    EON_sInt32 dX1=0, dX2=0;
    EON_ZBuffer dZL=0, dZ1=0, dZ2=0;
    EON_ZBuffer Z0, Z1, Z2, ZL;
    EON_sInt32 Y1, Y2, Y0, dY;
    EON_uChar stat;
    EON_Bool zb = Face->Material->zBufferable;
    EON_uInt32 color = eon_PickColorS(Face, Face->Shades[0]);

    PUTFACE_SORT();

    X1 = Face->Scr[i0].X;
    X2 = Face->Scr[i0].X;
    Z0 = Face->Scr[i0].Z;
    Z1 = Face->Scr[i1].Z;
    Z2 = Face->Scr[i2].Z;
    Y0 = eon_ToScreen(Face->Scr[i0].Y);
    Y1 = eon_ToScreen(Face->Scr[i1].Y);
    Y2 = eon_ToScreen(Face->Scr[i2].Y);

    dY = Y2-Y0;
    if (dY) {
        dX2 = (Face->Scr[i2].X - X1) / dY;
        dZ2 = (Z2 - Z0) / dY;
    }
    dY = Y1-Y0;
    if (dY) {
        dX1 = (Face->Scr[i1].X - X1) / dY;
        dZ1 = (Z1 - Z0) / dY;
        if (dX2 < dX1) {
            dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
            dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
            stat = 2;
        } else
            stat = 1;
        Z1 = Z0;
    } else {
        if (Face->Scr[i1].X > X1) {
            X2 = Face->Scr[i1].X;
            stat = 2|4;
        } else {
            X1 = Face->Scr[i1].X;
            ZL = Z0; Z0 = Z1; Z1 = ZL;
            stat = 1|8;
        }
    }

    if (zb) {
        XL1 = eon_ToScreen((dX1-dX2)*dY);
        if (XL1)
            dZL = ((dZ1-dZ2)*dY)/XL1;
        else {
            XL1 = eon_ToScreen(X2-X1);
            if (zb && XL1)
                dZL = (Z1-Z0)/XL1;
            else
                dZL = 0.0;
        }
    }

    gmem += (Y0 * cam->ScreenWidth);
    zbuf += (Y0 * cam->ScreenWidth);

    while (Y0 < Y2) {
        if (Y0 == Y1) {
            dY = Y2 - eon_ToScreen(Face->Scr[i1].Y);
            if (dY) {
                if (stat & 1) {
                    X1 = Face->Scr[i1].X;
                    dX1 = (Face->Scr[i2].X - Face->Scr[i1].X)/dY;
                }
                if (stat & 2) {
                    X2 = Face->Scr[i1].X;
                    dX2 = (Face->Scr[i2].X - Face->Scr[i1].X)/dY;
                }
                if (stat & 4) {
                    X1 = Face->Scr[i0].X;
                    dX1 = (Face->Scr[i2].X - Face->Scr[i0].X)/dY;
                }
                if (stat & 8) {
                    X2 = Face->Scr[i0].X;
                    dX2 = (Face->Scr[i2].X - Face->Scr[i0].X)/dY;
                }
                dZ1 = (Z2-Z0)/dY;
            }
        }
        XL1 = eon_ToScreen(X1);
        XL2 = eon_ToScreen(X2);
        ZL = Z0;
        XL2 -= XL1;
        if (XL2 > 0) {
            zbuf += XL1;
            gmem += XL1;
            XL1 += XL2;
            if (zb)
                do {
                    if (*zbuf < ZL) {
                        *zbuf = ZL;
                        *gmem = color;
                    }
                    gmem++;
                    zbuf++;
                    ZL += dZL;
                } while (--XL2);
            else
                do {
                    *gmem = color;
                    gmem++;
                } while (--XL2);
            gmem -= XL1;
            zbuf -= XL1;
        }
        gmem += cam->ScreenWidth;
        zbuf += cam->ScreenWidth;
        Z0 += dZ1;
        X1 += dX1;
        X2 += dX2;
        Y0++;
    }
}

// full(er) Phong Eq:
//  c = (EON_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x]);
static void EON_PF_SolidG(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
    EON_uChar i0, i1, i2;
    EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
    EON_ZBuffer *zbuf = Frame->ZBuffer;
    EON_ZBuffer dZL=0, dZ1=0, dZ2=0, Z1, Z2, ZL, Z3;
    EON_sInt32 dX1=0, dX2=0, X1, X2, XL1, XL2;
    EON_Float C1, C2, dC1=0, dC2=0, dCL=0, CL, C3;
    EON_sInt32 Y1, Y2, Y0, dY;
    EON_uChar stat;
    EON_Bool zb = Face->Material->zBufferable;

    PUTFACE_SORT();

    C1 = Face->Shades[i0];
    C2 = Face->Shades[i1];
    C3 = Face->Shades[i2];
    X1 = Face->Scr[i0].X;
    X2 = Face->Scr[i0].X;
    Z1 = Face->Scr[i0].Z;
    Z2 = Face->Scr[i1].Z;
    Z3 = Face->Scr[i2].Z;

    Y0 = eon_ToScreen(Face->Scr[i0].Y);
    Y1 = eon_ToScreen(Face->Scr[i1].Y);
    Y2 = eon_ToScreen(Face->Scr[i2].Y);

    dY = Y2 - Y0;
    if (dY) {
        dX2 = (Face->Scr[i2].X - X1) / dY;
        dC2 = (C3 - C1) / dY;
        dZ2 = (Z3 - Z1) / dY;
    }
    dY = Y1 - Y0;
    if (dY) {
        dX1 = (Face->Scr[i1].X - X1) / dY;
        dC1 = (C2 - C1) / dY;
        dZ1 = (Z2 - Z1) / dY;
        if (dX2 < dX1) {
            EON_Float dCT = dC2; dC2 = dC1; dC1 = dCT;
            dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
            dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
            stat = 2;
        } else {
            stat = 1;
        }
        Z2 = Z1;
        C2 = C1;
    } else {
        if (Face->Scr[i1].X > X1) {
            X2 = Face->Scr[i1].X;
            stat = 2|4;
        } else {
            X1 = C1; C1 = C2; C2 = X1;
            ZL = Z1; Z1 = Z2; Z2 = ZL;
            X1 = Face->Scr[i1].X;
            stat = 1|8;
        }
    }

    gmem += (Y0 * cam->ScreenWidth);
    zbuf += (Y0 * cam->ScreenWidth);

    XL1 = eon_ToScreen((dX1-dX2)*dY);
    if (XL1) {
        dCL = ((dC1-dC2)*dY)/XL1;
        dZL = ((dZ1-dZ2)*dY)/XL1;
    } else {
        XL1 = ((X2-X1+(1<<19))>>20);
        if (XL1) {
            dCL = (C2-C1)/XL1;
            dZL = (Z2-Z1)/XL1;
        }
    }

    while (Y0 < Y2) {
        if (Y0 == Y1) {
            dY = Y2 - eon_ToScreen(Face->Scr[i1].Y);
            if (dY) {
                dZ1 = (Z3-Z1)/dY;
                dC1 = (C3-C1) / dY;
                if (stat & 1) {
                    X1 = Face->Scr[i1].X;
                    dX1 = (Face->Scr[i2].X - Face->Scr[i1].X)/dY;
                }
                if (stat & 2) {
                    X2 = Face->Scr[i1].X;
                    dX2 = (Face->Scr[i2].X - Face->Scr[i1].X)/dY;
                }
                if (stat & 4) {
                    X1 = Face->Scr[i0].X;
                    dX1 = (Face->Scr[i2].X - Face->Scr[i0].X)/dY;
                }
                if (stat & 8) {
                    X2 = Face->Scr[i0].X;
                    dX2 = (Face->Scr[i2].X - Face->Scr[i0].X)/dY;
                }
            }
        }
        CL = C1;
        XL1 = eon_ToScreen(X1);
        XL2 = eon_ToScreen(X2);
        ZL = Z1;
        XL2 -= XL1;
        if (XL2 > 0) {
            gmem += XL1;
            zbuf += XL1;
            XL1 += XL2;
            if (zb)
                do {
                    if (*zbuf < ZL) {
                        *zbuf = ZL;
                        *gmem = eon_PickColorS(Face, CL);
                    }
                    gmem++;
                    zbuf++;
                    ZL += dZL;
                    CL += dCL;
                } while (--XL2);
            else
                do {
                    *gmem = eon_PickColorS(Face, CL);
                    gmem++;
                    CL += dCL;
                } while (--XL2);
            gmem -= XL1;
            zbuf -= XL1;
        }
        gmem += cam->ScreenWidth;
        zbuf += cam->ScreenWidth;
        X1 += dX1;
        X2 += dX2;
        C1 += dC1;
        Z1 += dZ1;
        Y0++;
    }
}

// pf_tex.c
//

static void EON_PF_TexF(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
  EON_ZBuffer *zbuf = Frame->ZBuffer;
  EON_sInt32 MappingU1, MappingU2, MappingU3;
  EON_sInt32 MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar vshift;
  EON_Texture *Texture;
  EON_uChar stat;

  EON_ZBuffer Z1, ZL, dZ1=0, dZL=0, Z2, dZ2=0;
  EON_sInt32 dU1=0, dV1=0, dU2=0, dV2=0, U1, V1, U2, V2;
  EON_sInt32 dUL=0, dVL=0, UL, VL;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_Bool zb = Face->Material->zBufferable;

  Texture = Face->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;

  PUTFACE_SORT_TEX();

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = Face->Scr[i0].X;
  Z2 = Z1 = Face->Scr[i0].Z;
  Y0 = (Face->Scr[i0].Y+(1<<19))>>20;
  Y1 = (Face->Scr[i1].Y+(1<<19))>>20;
  Y2 = (Face->Scr[i2].Y+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (Face->Scr[i2].X - X1) / dY;
    dV2 = (MappingV3 - V1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dZ2 = (Face->Scr[i2].Z - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (Face->Scr[i1].X - X1) / dY;
    dZ1 = (Face->Scr[i1].Z - Z1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (Face->Scr[i1].X > X1) {
      X2 = Face->Scr[i1].X;
      Z2 = Face->Scr[i1].Z;
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = Face->Scr[i1].X;
      Z1 = Face->Scr[i1].Z;
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
    }
  }

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2 - ((Face->Scr[i1].Y+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = Face->Scr[i1].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 2) {
          X2 = Face->Scr[i1].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 4) {
          X1 = Face->Scr[i0].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        if (stat & 8) {
          X2 = Face->Scr[i0].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        dZ1 = (Face->Scr[i2].Z-Z1) / dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    UL = U1;
    VL = V1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            int tv = ((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND);
            *zbuf = ZL;
            *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], Face->Shades[0]);
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          int tv = ((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND);
          *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], Face->Shades[0]);
          gmem++;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    X1 += dX1;
    X2 += dX2;
    U1 += dU1;
    V1 += dV1;
    Z1 += dZ1;
    Y0++;
  }
}

static void EON_PF_TexG(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
  EON_ZBuffer *zbuf = Frame->ZBuffer;
  EON_sInt32 MappingU1, MappingU2, MappingU3;
  EON_sInt32 MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar vshift;
  EON_Texture *Texture;

  EON_sInt32 U1, V1, U2, V2, dU1=0, dU2=0, dV1=0, dV2=0, dUL=0, dVL=0, UL, VL;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_Float C1, C2, dC1=0, dC2=0, CL, dCL=0;
  EON_ZBuffer Z1, ZL, dZ1=0, dZ2=0, dZL=0, Z2;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;

  EON_Bool zb = Face->Material->zBufferable;

  Texture = Face->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;

  PUTFACE_SORT_TEX();

  C1 = C2 = Face->Shades[i0]*65535.0f;
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = Face->Scr[i0].X;
  Z2 = Z1 = Face->Scr[i0].Z;
  Y0 = (Face->Scr[i0].Y+(1<<19))>>20;
  Y1 = (Face->Scr[i1].Y+(1<<19))>>20;
  Y2 = (Face->Scr[i2].Y+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (Face->Scr[i2].X - X1) / dY;
    dZ2 = (Face->Scr[i2].Z - Z1) / dY;
    dC2 = (Face->Shades[i2]*65535.0f - C1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (Face->Scr[i1].X - X1) / dY;
    dZ1 = (Face->Scr[i1].Z - Z1) / dY;
    dC1 = (Face->Shades[i1]*65535.0f - C1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      EON_Float dCT = dC1; dC1 = dCT; dC2 = dCT;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (Face->Scr[i1].X > X1) {
      X2 = Face->Scr[i1].X;
      Z2 = Face->Scr[i1].Z;
      C2 = Face->Shades[i1]*65535.0f;
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = Face->Scr[i1].X;
      Z1 = Face->Scr[i1].Z;
      C1 = Face->Shades[i1]*65535.0f;
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    if (zb) dZL = ((dZ1-dZ2)*dY)/XL1;
    dCL = ((dC1-dC2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      if (zb) dZL = (Z2-Z1)/XL1;
      dCL = (C2-C1)/(XL1);
    }
  }
  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2 - ((Face->Scr[i1].Y+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = Face->Scr[i1].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 2) {
          X2 = Face->Scr[i1].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 4) {
          X1 = Face->Scr[i0].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        if (stat & 8) {
          X2 = Face->Scr[i0].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        dZ1 = (Face->Scr[i2].Z-Z1)/dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
        dC1 = (Face->Shades[i2]*65535.0f-C1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    CL = C1;
    ZL = Z1;
    UL = U1;
    VL = V1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            int tv = ((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND);
            *zbuf = ZL;
            *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], CL/65536.0f);
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          int tv = ((UL>>16)&MappingU_AND) + ((VL>>vshift)&MappingV_AND);
          *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], CL/65536.0f);
          gmem++;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
    U1 += dU1;
    V1 += dV1;
    Y0++;
  }
}

// pf_ptex.c
//

static void EON_PF_PTexF(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
  EON_ZBuffer *zbuf = Frame->ZBuffer;
  EON_Float MappingU1, MappingU2, MappingU3;
  EON_Float MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar vshift;
  EON_Texture *Texture;

  EON_uChar nm, nmb;
  EON_sInt n;
  EON_Float U1,V1,U2,V2,dU1=0,dU2=0,dV1=0,dV2=0,dUL=0,dVL=0,UL,VL;
  EON_sInt32 iUL, iVL, idUL=0, idVL=0, iULnext, iVLnext;

  EON_sInt32 scrwidth = cam->ScreenWidth;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, Xlen;
  EON_ZBuffer Z1, dZ1=0, dZ2=0, Z2, dZL=0, ZL, pZL, pdZL;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;

  EON_Bool zb = Face->Material->zBufferable;

  Texture = Face->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;

  nm = Face->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = EON_Min(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  PUTFACE_SORT_TEX();

  MappingU1 *= Face->Scr[i0].Z/65536.0f;
  MappingV1 *= Face->Scr[i0].Z/65536.0f;
  MappingU2 *= Face->Scr[i1].Z/65536.0f;
  MappingV2 *= Face->Scr[i1].Z/65536.0f;
  MappingU3 *= Face->Scr[i2].Z/65536.0f;
  MappingV3 *= Face->Scr[i2].Z/65536.0f;

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = Face->Scr[i0].X;
  Z2 = Z1 = Face->Scr[i0].Z;
  Y0 = (Face->Scr[i0].Y+(1<<19))>>20;
  Y1 = (Face->Scr[i1].Y+(1<<19))>>20;
  Y2 = (Face->Scr[i2].Y+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (Face->Scr[i2].X - X1) / dY;
    dZ2 = (Face->Scr[i2].Z - Z1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (Face->Scr[i1].X - X1) / dY;
    dZ1 = (Face->Scr[i1].Z - Z1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      XL1 = dX2; dX2 = dX1; dX1 = XL1;
      dUL = dU1; dU1 = dU2; dU2 = dUL;
      dVL = dV1; dV1 = dV2; dV2 = dVL;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (Face->Scr[i1].X > X1) {
      X2 = Face->Scr[i1].X;
      Z2 = Face->Scr[i1].Z;
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = Face->Scr[i1].X;
      Z1 = Face->Scr[i1].Z;
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = ((dX1-dX2)*dY+(1<<19))>>20;
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
    }
  }

  pdZL = dZL * nm;
  dUL *= nm;
  dVL *= nm;

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2-((Face->Scr[i1].Y+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = Face->Scr[i1].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 2) {
          X2 = Face->Scr[i1].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 4) {
          X1 = Face->Scr[i0].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        if (stat & 8) {
          X2 = Face->Scr[i0].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        dZ1 = (Face->Scr[i2].Z-Z1)/dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    Xlen = ((X2+(1<<19))>>20) - XL1;
    if (Xlen > 0) {
      register EON_Float t;
      pZL = ZL = Z1;
      UL = U1;
      VL = V1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += Xlen-scrwidth;
      t = 65536.0f/ZL;
      iUL = iULnext = ((EON_sInt32) (UL*t));
      iVL = iVLnext = ((EON_sInt32) (VL*t));
      do {
        UL += dUL;
        VL += dVL;
        iUL = iULnext;
        iVL = iVLnext;
        pZL += pdZL;
        t = 65536.0f/pZL;
        iULnext = ((EON_sInt32) (UL*t));
        iVLnext = ((EON_sInt32) (VL*t));
        idUL = (iULnext - iUL)>>nmb;
        idVL = (iVLnext - iVL)>>nmb;
        n = nm;
        Xlen -= n;
        if (Xlen < 0) n += Xlen;
        if (zb) do {
            if (*zbuf < ZL) {
              int tv = ((iUL>>16)&MappingU_AND) + ((iVL>>vshift)&MappingV_AND);
              *zbuf = ZL;
              *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], Face->Shades[0]);
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            int tv = ((iUL>>16)&MappingU_AND) + ((iVL>>vshift)&MappingV_AND);
            *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], Face->Shades[0]);
            gmem++;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
      } while (Xlen > 0);
      gmem -= XL1;
      zbuf -= XL1;
    } else {
      zbuf += cam->ScreenWidth;
      gmem += cam->ScreenWidth;
    }
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
    Y0++;
  }
}

static void EON_PF_PTexG(EON_Cam *cam, EON_Face *Face, EON_Frame *Frame)
{
  EON_uChar i0, i1, i2;
  EON_Float MappingU1, MappingU2, MappingU3;
  EON_Float MappingV1, MappingV2, MappingV3;

  EON_Texture *Texture;
  EON_Bool zb = Face->Material->zBufferable;

  EON_uChar nm, nmb;
  EON_uInt n;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar vshift;
  EON_uChar *texture;
  EON_sInt32 iUL, iVL, idUL, idVL, iULnext, iVLnext;
  EON_Float U2,V2,dU2=0,dV2=0,dUL=0,dVL=0,UL,VL;
  EON_sInt32 XL1, Xlen;
  EON_Float C2, dC2=0, CL, dCL=0;
  EON_Float ZL, Z2, dZ2=0, dZL=0, pdZL, pZL;

  EON_sInt32 Y2, dY;
  EON_uChar stat;

  /* Cache line */
  EON_sInt32 Y0,Y1;
  EON_sInt32 C1, dC1=0, X2, dX2=0, X1, dX1=0;

  /* Cache line */
  EON_Float dU1=0, U1, dZ1=0, Z1, V1, dV1=0;
  EON_uInt32 *gmem = (EON_uInt32 *)Frame->Data;
  EON_ZBuffer *zbuf = Frame->ZBuffer;

  Texture = Face->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;

  nm = Face->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = EON_Min(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  PUTFACE_SORT_TEX();

  MappingU1 *= Face->Scr[i0].Z/65536.0f;
  MappingV1 *= Face->Scr[i0].Z/65536.0f;
  MappingU2 *= Face->Scr[i1].Z/65536.0f;
  MappingV2 *= Face->Scr[i1].Z/65536.0f;
  MappingU3 *= Face->Scr[i2].Z/65536.0f;
  MappingV3 *= Face->Scr[i2].Z/65536.0f;
  Face->Shades[0] *= 65536.0f;
  Face->Shades[1] *= 65536.0f;
  Face->Shades[2] *= 65536.0f;

  C1 = C2 = (EON_sInt32) Face->Shades[i0];
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = Face->Scr[i0].X;
  Z2 = Z1 = Face->Scr[i0].Z;
  Y0 = (Face->Scr[i0].Y+(1<<19))>>20;
  Y1 = (Face->Scr[i1].Y+(1<<19))>>20;
  Y2 = (Face->Scr[i2].Y+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (Face->Scr[i2].X - X1) / dY;
    dZ2 = (Face->Scr[i2].Z - Z1) / dY;
    dC2 = (EON_sInt32) ((Face->Shades[i2] - C1) / dY);
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (Face->Scr[i1].X - X1) / dY;
    dZ1 = (Face->Scr[i1].Z - Z1) / dY;
    dC1 = (EON_sInt32) ((Face->Shades[i1] - C1) / dY);
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dUL = dU1; dU1 = dU2; dU2 = dUL;
      dVL = dV1; dV1 = dV2; dV2 = dVL;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      dCL = dC1; dC1 = dC2; dC2 = dCL;
      stat = 2;
    } else stat = 1;
  } else {
    if (Face->Scr[i1].X > X1) {
      X2 = Face->Scr[i1].X;
      Z2 = Face->Scr[i1].Z;
      C2 = (EON_sInt32)Face->Shades[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = Face->Scr[i1].X;
      Z1 = Face->Scr[i1].Z;
      C1 = (EON_sInt32)Face->Shades[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
    dCL = ((dC1-dC2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
      dCL = (C2-C1)/XL1;
    }
  }

  pdZL = dZL * nm;
  dUL *= nm;
  dVL *= nm;
  Y1 -= Y0;
  Y0 = Y2-Y0;
  while (Y0--) {
    if (!Y1--) {
      dY = Y2-((Face->Scr[i1].Y+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = Face->Scr[i1].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 2) {
          X2 = Face->Scr[i1].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i1].X)/dY;
        }
        if (stat & 4) {
          X1 = Face->Scr[i0].X;
          dX1 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        if (stat & 8) {
          X2 = Face->Scr[i0].X;
          dX2 = (Face->Scr[i2].X-Face->Scr[i0].X)/dY;
        }
        dZ1 = (Face->Scr[i2].Z-Z1)/dY;
        dC1 = (EON_sInt32)((Face->Shades[i2]-C1)/dY);
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    Xlen = ((X2+(1<<19))>>20) - XL1;
    if (Xlen > 0) {
      register EON_Float t;
      CL = C1;
      pZL = ZL = Z1;
      UL = U1;
      VL = V1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += Xlen - cam->ScreenWidth;
      t = 65536.0f / ZL;
      iUL = iULnext = ((EON_sInt32) (UL*t));
      iVL = iVLnext = ((EON_sInt32) (VL*t));
      do {
        UL += dUL;
        VL += dVL;
        iUL = iULnext;
        iVL = iVLnext;
        pZL += pdZL;
        t = 65536.0f/pZL;
        iULnext = ((EON_sInt32) (UL*t));
        iVLnext = ((EON_sInt32) (VL*t));
        idUL = (iULnext - iUL)>>nmb;
        idVL = (iVLnext - iVL)>>nmb;
        n = nm;
        Xlen -= n;
        if (Xlen < 0) n += Xlen;
        if (zb) do {
            if (*zbuf < ZL) {
              int tv = ((iUL>>16)&MappingU_AND) + ((iVL>>vshift)&MappingV_AND);
              *zbuf = ZL;
              *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], CL/65536.0f);
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            int tv = ((iUL>>16)&MappingU_AND) + ((iVL>>vshift)&MappingV_AND);
            *gmem = eon_PickColorPS(Texture->PaletteData, texture[tv], CL/65536.0f);
            gmem++;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
      } while (Xlen > 0);
      gmem -= XL1;
      zbuf -= XL1;
    } else {
      zbuf += cam->ScreenWidth;
      gmem += cam->ScreenWidth;
    }
    Z1 += dZ1;
    U1 += dU1;
    V1 += dV1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
  }
}


// render.c
//

#define MACRO_eon_MatrixApply(m,x,y,z,outx,outy,outz) \
      ( outx ) = ( x )*( m )[0] + ( y )*( m )[1] + ( z )*( m )[2 ] + ( m )[3 ];\
      ( outy ) = ( x )*( m )[4] + ( y )*( m )[5] + ( z )*( m )[6 ] + ( m )[7 ];\
      ( outz ) = ( x )*( m )[8] + ( y )*( m )[9] + ( z )*( m )[10] + ( m )[11]

#define MACRO_eon_DotProduct(x1,y1,z1,x2,y2,z2) \
      ((( x1 )*( x2 ))+(( y1 )*( y2 ))+(( z1 )*( z2 )))

#define MACRO_eon_NormalizeVector(x,y,z) { \
    register EON_Double length; \
    length = ( x )*( x )+( y )*( y )+( z )*( z ); \
    if (length > 0.0000000001) { \
        EON_Float l = (EON_Float) sqrt(length); \
        ( x ) /= l; \
        ( y ) /= l; \
        ( z ) /= l; \
    } \
}



static void eon_RenderObj(EON_Rend *rend, EON_Obj *obj,
                          EON_Float *bmatrix, EON_Float *bnmatrix);

EON_Rend *EON_RendCreate(EON_Cam *Camera)
{
    EON_Rend *rend = CX_zalloc(sizeof(EON_Rend));
    if (rend) {
        rend->Cam = Camera;
        rend->Clip.Info = &rend->Info;
    }
    return rend;
}

void EON_RendDelete(EON_Rend *rend)
{
    CX_free(rend);
    return;
}

void EON_RenderBegin(EON_Rend *rend)
{
    EON_Float tempMatrix[16];
    memset(&rend->Info, 0, sizeof(rend->Info));
    rend->NumLights = 0;
    rend->NumFaces = 0;
    EON_MatrixRotate(rend->CMatrix,2,  -rend->Cam->Pan);
    EON_MatrixRotate(tempMatrix,1, -rend->Cam->Pitch);
    EON_MatrixMultiply(rend->CMatrix, tempMatrix);
    EON_MatrixRotate(tempMatrix,3, -rend->Cam->Roll);
    EON_MatrixMultiply(rend->CMatrix, tempMatrix);
    EON_ClipSetFrustum(&rend->Clip, rend->Cam);
}

void EON_RenderLight(EON_Rend *rend, EON_Light *light)
{
    EON_3DPoint *pl;
    EON_Float xp, yp, zp;
    if (light->Type == EON_LIGHT_NONE || rend->NumLights >= EON_MAX_LIGHTS)
        return;
    pl = &(rend->Lights[rend->NumLights].pos);
    if (light->Type == EON_LIGHT_VECTOR) {
        xp = light->Pos.X;
        yp = light->Pos.Y;
        zp = light->Pos.Z;
        MACRO_eon_MatrixApply(rend->CMatrix,xp,yp,zp,pl->X,pl->Y,pl->Z);
    } else if (light->Type & EON_LIGHT_POINT) {
        xp = light->Pos.X - rend->Cam->Pos.X;
        yp = light->Pos.Y - rend->Cam->Pos.Y;
        zp = light->Pos.Z - rend->Cam->Pos.Z;
        MACRO_eon_MatrixApply(rend->CMatrix,xp,yp,zp,pl->X,pl->Y,pl->Z);
    }
    rend->Lights[rend->NumLights].light = light;
    rend->NumLights++;
}

inline static EON_Double eon_RenderVertexLights(EON_Rend *rend, EON_Vertex *vertex,
                                                EON_Double BaseShade, EON_Bool BackfaceIllumination,
                                                EON_Float nx, EON_Float ny, EON_Float nz)
{
    EON_Double shade = BaseShade;
    EON_uInt32 i;
    for (i = 0; i < rend->NumLights; i ++) {
        EON_Double CurShade = 0.0;
        EON_Light *light = rend->Lights[i].light;
        if (light->Type & EON_LIGHT_POINT_ANGLE) {
            EON_Double nx2 = rend->Lights[i].pos.X - vertex->xformedx;
            EON_Double ny2 = rend->Lights[i].pos.Y - vertex->xformedy;
            EON_Double nz2 = rend->Lights[i].pos.Z - vertex->xformedz;
            MACRO_eon_NormalizeVector(nx2,ny2,nz2);
            CurShade = MACRO_eon_DotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
        }
        if (light->Type & EON_LIGHT_POINT_DISTANCE) {
            EON_Double nx2 = rend->Lights[i].pos.X - vertex->xformedx;
            EON_Double ny2 = rend->Lights[i].pos.Y - vertex->xformedy;
            EON_Double nz2 = rend->Lights[i].pos.Z - vertex->xformedz;
            EON_Double t = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
            CurShade *= EON_Clamp(t,0,1.0)*light->Intensity; // FIXME
        }
        if (light->Type == EON_LIGHT_VECTOR) {
            CurShade = MACRO_eon_DotProduct(nx,ny,nz,
                                            rend->Lights[i].pos.X,
                                            rend->Lights[i].pos.Y,
                                            rend->Lights[i].pos.Z);
            CurShade *= light->Intensity;
        }
        if (CurShade > 0.0) {
            shade += CurShade;
        } else if (BackfaceIllumination) {
            shade -= CurShade;
        }
    } /* End of light loop */
    return shade;
}

inline static void eon_RenderShadeObjFlat(EON_Rend *rend, EON_Face *face,
                                          EON_Bool BackfaceIllumination,
                                          EON_Float nx, EON_Float ny, EON_Float nz)
{
    EON_Double shade = face->vsLighting[0];
    if (face->Material->_st & EON_SHADE_FLAT) {
        shade = eon_RenderVertexLights(rend,
                                       face->Vertices[0],
                                       face->vsLighting[0],
                                       BackfaceIllumination,
                                       nx,
                                       ny,
                                       nz);
    }
    if (face->Material->_st & EON_SHADE_FLAT_DISTANCE) {
        EON_Double avg = (face->Vertices[0]->xformedz +
                          face->Vertices[1]->xformedz +
                          face->Vertices[2]->xformedz) / 3.0;
        shade += 1.0 - (avg / face->Material->FadeDist);
    }
    face->Shades[0] = (EON_Float)shade;
    return;
}

inline static void eon_RenderShadeObjGourad(EON_Rend *rend, EON_Face *face,
                                            EON_Bool BackfaceIllumination,
                                            EON_uInt32 VertexNum)
{
    EON_Double shade = face->vsLighting[VertexNum];
    if (face->Material->_st & EON_SHADE_GOURAUD) {
        shade = eon_RenderVertexLights(rend,
                                       face->Vertices[VertexNum],
                                       face->vsLighting[VertexNum],
                                       BackfaceIllumination,
                                       face->Vertices[VertexNum]->xformednx,
                                       face->Vertices[VertexNum]->xformedny,
                                       face->Vertices[VertexNum]->xformednz);
    }
    if (face->Material->_st & EON_SHADE_GOURAUD_DISTANCE) {
        shade += 1.0 - face->Vertices[VertexNum]->xformedz/face->Material->FadeDist;
    }
    face->Shades[VertexNum] = (EON_Float)shade;
    return;
}

inline static void eon_RenderShadeObjWireframe(EON_Rend *rend, EON_Face *face)
{
    face->Shades[0] = 1.0;
    face->Shades[1] = 1.0;
    face->Shades[2] = 1.0;
    return;
}

static void eon_RenderObj(EON_Rend *rend, EON_Obj *obj,
                          EON_Float *bmatrix, EON_Float *bnmatrix)
{
    EON_uInt32 i, x, facepos;
    EON_Float nx = 0.0, ny = 0.0, nz = 0.0;
    EON_Float oMatrix[16], nMatrix[16], tempMatrix[16];

    EON_Vertex *vertex = NULL;
    EON_Face *face = NULL;

    if (obj->GenMatrix) {
        EON_MatrixRotate(nMatrix,1,obj->Xa);
        EON_MatrixRotate(tempMatrix,2,obj->Ya);
        EON_MatrixMultiply(nMatrix,tempMatrix);
        EON_MatrixRotate(tempMatrix,3,obj->Za);
        EON_MatrixMultiply(nMatrix,tempMatrix);
        memcpy(oMatrix,nMatrix,sizeof(EON_Float)*16);
    } else {
        memcpy(nMatrix,obj->RotMatrix,sizeof(EON_Float)*16);
    }
    if (bnmatrix) {
        EON_MatrixMultiply(nMatrix,bnmatrix);
    }
    if (obj->GenMatrix) {
        EON_MatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
        EON_MatrixMultiply(oMatrix,tempMatrix);
    } else {
        memcpy(oMatrix,obj->Matrix,sizeof(EON_Float)*16);
    }
    if (bmatrix) {
        EON_MatrixMultiply(oMatrix,bmatrix);
    }
    for (i = 0; i < EON_MAX_CHILDREN; i ++) {
        if (obj->Children[i]) {
            eon_RenderObj(rend, obj->Children[i],
                          oMatrix, nMatrix);
        }
    }
    if (!obj->NumFaces || !obj->NumVertices) {
        return;
    }

    EON_MatrixTranslate(tempMatrix,
                        -rend->Cam->Pos.X, -rend->Cam->Pos.Y, -rend->Cam->Pos.Z);
    EON_MatrixMultiply(oMatrix, tempMatrix);
    EON_MatrixMultiply(oMatrix, rend->CMatrix);
    EON_MatrixMultiply(nMatrix, rend->CMatrix);

    x = obj->NumVertices;
    vertex = obj->Vertices;

    do {
        MACRO_eon_MatrixApply(oMatrix,vertex->x,vertex->y,vertex->z,
                              vertex->xformedx, vertex->xformedy, vertex->xformedz);
        MACRO_eon_MatrixApply(nMatrix,vertex->nx,vertex->ny,vertex->nz,
                              vertex->xformednx,vertex->xformedny,vertex->xformednz);
        vertex++;
    } while (--x);

    face = obj->Faces;
    facepos = rend->NumFaces;

    if (rend->NumFaces + obj->NumFaces >= EON_MAX_TRIANGLES) {// exceeded maximum face count
        return;
    }

    rend->Info.TriStats[TRI_STAT_INITIAL] += obj->NumFaces;
    rend->NumFaces += obj->NumFaces;
    x = obj->NumFaces;

    do {
        if (obj->BackfaceCull || face->Material->_st & EON_SHADE_FLAT) {
            MACRO_eon_MatrixApply(nMatrix,face->nx,face->ny,face->nz,nx,ny,nz);
        }
        if (!obj->BackfaceCull || (MACRO_eon_DotProduct(nx,ny,nz,
                face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,
                face->Vertices[0]->xformedz) < 0.0000001)) {
            if (EON_ClipNeeded(&rend->Clip, face)) {
                if (face->Material->_st & EON_SHADE_WIREFRAME) {
                    eon_RenderShadeObjWireframe(rend, face);
                }
                if (face->Material->_st & (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE)) {
                    eon_RenderShadeObjFlat(rend, face, obj->BackfaceIllumination, nx, ny, nz);
                }
                if (face->Material->_st &(EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE)) {
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 0);
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 1);
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 2);
                }
                rend->Faces[facepos].zd = face->Vertices[0]->xformedz + face->Vertices[1]->xformedz + face->Vertices[2]->xformedz;
                rend->Faces[facepos].face = face;
                facepos++;
                rend->Info.TriStats[TRI_STAT_CULLING]++;
            } /* Is it in our area Check */
        } /* Backface Check */
        rend->NumFaces = facepos;
        face++;
    } while (--x); /* Face loop */
    return;
}

void EON_RenderObj(EON_Rend *rend, EON_Obj *obj)
{
    eon_RenderObj(rend, obj, 0, 0);
    return;
}

void EON_RenderEnd(EON_Rend *rend, EON_Frame *frame)
{
    EON_FaceInfo *f = rend->Faces;
    while (rend->NumFaces--) {
        EON_ClipRenderFace(&rend->Clip, f->face, frame);
        f++;
    }
    rend->NumFaces = 0;
    rend->NumLights = 0;
    return;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

