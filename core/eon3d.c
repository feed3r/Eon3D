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
#include "eon3d.h"


/******************************************************************************
** Built-in Rasterizers
******************************************************************************/

static void EON_PF_Null(EON_Cam *, EON_Face *);
static void EON_PF_SolidF(EON_Cam *, EON_Face *);
static void EON_PF_SolidG(EON_Cam *, EON_Face *);
static void EON_PF_TexF(EON_Cam *, EON_Face *);
static void EON_PF_TexG(EON_Cam *, EON_Face *);
static void EON_PF_TexEnv(EON_Cam *, EON_Face *);
static void EON_PF_PTexF(EON_Cam *, EON_Face *);
static void EON_PF_PTexG(EON_Cam *, EON_Face *);

/* Used internally; EON_FILL_* are stored in EON_Mat._st. */
enum {
    EON_FILL_SOLID       = 0x0,
    EON_FILL_TEXTURE     = 0x1,
    EON_FILL_ENVIRONMENT = 0x2,
    EON_FILL_TRANSPARENT = 0x4
};


// math.c
//
void EON_MatrixRotate(EON_Float matrix[], EON_uChar m, EON_Float Deg)
{
    EON_uChar m1, m2;
    double c,s;
    double d = Deg * EON_PI / 180.0;
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
    double length = (*x)*(*x)+(*y)*(*y)+(*z)*(*z);
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

EON_Obj *EON_ObjClone(EON_Obj *o) 
{
    EON_Face *iff, *of;
    EON_uInt32 i;
    EON_Obj *out;
    if (!(out = EON_ObjCreate(o->NumVertices,o->NumFaces))) return 0;
    for (i = 0; i < EON_MAX_CHILDREN; i ++)
        if (o->Children[i]) out->Children[i] = EON_ObjClone(o->Children[i]);
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
    while (i--)
        (f++)->Material = m;
    if (th)
        for (i = 0; i < EON_MAX_CHILDREN; i++)
            if (o->Children[i])
                EON_ObjSetMat(o->Children[i],m,th);
    return;
}

EON_Obj *EON_ObjCalcNormals(EON_Obj *obj)
{
    EON_uInt32 i;
    EON_Vertex *v = obj->Vertices;
    EON_Face *f = obj->Faces;
    double x1, x2, y1, y2, z1, z2;
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
    for (i = 0; i < EON_MAX_CHILDREN; i ++)
        if (obj->Children[i])
            EON_ObjCalcNormals(obj->Children[i]);
    return obj;
}

// mat.c
//

static void eon_GenerateSinglePalette(EON_Mat *);
static void eon_GeneratePhongPalette(EON_Mat *);
static void eon_GenerateTextureEnvPalette(EON_Mat *);
static void eon_GenerateTexturePalette(EON_Mat *, EON_Texture *);
static void eon_GeneratePhongTexturePalette(EON_Mat *, EON_Texture *);
static void eon_GeneratePhongTransparentPalette(EON_Mat *m);
static void eon_GenerateTransparentPalette(EON_Mat *);
static void eon_SetMaterialPutFace(EON_Mat *m);
static void eon_MatSetupTransparent(EON_Mat *m, EON_uChar *pal);

EON_Mat *EON_MatCreate()
{
    EON_Mat *m = CX_zalloc(sizeof(EON_Mat));
    if (m) {
        m->EnvScaling = 1.0f;
        m->TexScaling = 1.0f;
        m->Ambient[0] = m->Ambient[1] = m->Ambient[2] = 0;
        m->Diffuse[0] = m->Diffuse[1] = m->Diffuse[2] = 128;
        m->Specular[0] = m->Specular[1] = m->Specular[2] = 128;
        m->Shininess = 4;
        m->NumGradients = 32;
        m->FadeDist = 1000.0;
        m->zBufferable = 1;
    }
    return m;
}

void EON_MatDelete(EON_Mat *m)
{
    if (m) {
        if (m->_ReMapTable)
            CX_free(m->_ReMapTable);
        if (m->_RequestedColors)
            CX_free(m->_RequestedColors);
        if (m->_AddTable)
            CX_free(m->_AddTable);
        CX_free(m);
    }
    return;
}

void EON_MatInit(EON_Mat *m)
{
    if (m->Shininess < 1)
        m->Shininess = 1;
    m->_ft = ((m->Environment ? EON_FILL_ENVIRONMENT : EON_FILL_SOLID) |
               (m->Texture ? EON_FILL_TEXTURE : EON_FILL_SOLID));
    m->_st = m->ShadeType;

    if (m->Transparent)
        m->_ft = EON_FILL_TRANSPARENT;

    if (m->_ft == (EON_FILL_TEXTURE|EON_FILL_ENVIRONMENT))
        m->_st = EON_SHADE_NONE;

    if (m->_ft == EON_FILL_SOLID) {
        if (m->_st == EON_SHADE_NONE)
            eon_GenerateSinglePalette(m);
        else
            eon_GeneratePhongPalette(m);
    } else if (m->_ft == EON_FILL_TEXTURE) {
        if (m->_st == EON_SHADE_NONE)
            eon_GenerateTexturePalette(m,m->Texture);
        else
            eon_GeneratePhongTexturePalette(m,m->Texture);
    } else if (m->_ft == EON_FILL_ENVIRONMENT) {
        if (m->_st == EON_SHADE_NONE)
            eon_GenerateTexturePalette(m,m->Environment);
        else
            eon_GeneratePhongTexturePalette(m,m->Environment);
    } else if (m->_ft == (EON_FILL_ENVIRONMENT|EON_FILL_TEXTURE)) {
        eon_GenerateTextureEnvPalette(m);
    } else if (m->_ft == EON_FILL_TRANSPARENT) {
        if (m->_st == EON_SHADE_NONE)
            eon_GenerateTransparentPalette(m);
        else
            eon_GeneratePhongTransparentPalette(m);
    }
    eon_SetMaterialPutFace(m);
}

static void eon_MatSetupTransparent(EON_Mat *m, EON_uChar *pal)
{
    EON_uInt x, intensity;
    if (m->Transparent) {
        if (m->_AddTable)
            CX_free(m->_AddTable);
        m->_AddTable = CX_malloc(256 * sizeof(EON_uInt16));
        for (x = 0; x < 256; x++) {
            intensity  = *pal++;
            intensity += *pal++;
            intensity += *pal++;
            m->_AddTable[x] = ((intensity*(m->_ColorsUsed-m->_tsfact))/768);
        }
    }
    return;
}

void EON_MatMapToPal(EON_Mat *m, EON_uChar *pal, EON_sInt pstart, EON_sInt pend)
{
    EON_sInt32 j, r, g, b, bestdiff, r2, g2, b2;
    EON_sInt bestpos,k;
    EON_uInt32 i;
    EON_uChar *p;
    if (!m->_RequestedColors)
        EON_MatInit(m);
    if (!m->_RequestedColors)
        return;
    if (m->_ReMapTable)
        CX_free(m->_ReMapTable);
    m->_ReMapTable = CX_malloc(m->_ColorsUsed);
    for (i = 0; i < m->_ColorsUsed; i ++) {
        bestdiff = 1000000000;
        bestpos = pstart;
        r = m->_RequestedColors[i*3];
        g = m->_RequestedColors[i*3+1];
        b = m->_RequestedColors[i*3+2];
        p = pal + pstart*3;
        for (k = pstart; k <= (EON_sInt)pend; k++) {
            r2 = p[0] - r;
            g2 = p[1] - g;
            b2 = p[2] - b;
            p += 3;
            j = r2*r2+g2*g2+b2*b2;
            if (j < bestdiff) {
                bestdiff = j;
                bestpos = k;
            }
        }
        m->_ReMapTable[i] = bestpos;
    }
    eon_MatSetupTransparent(m,pal);
}

static void eon_GenerateSinglePalette(EON_Mat *m)
{
    m->_ColorsUsed = 1;
    if (m->_RequestedColors)
        CX_free(m->_RequestedColors);
    m->_RequestedColors = CX_malloc(3); // FIXME
    m->_RequestedColors[0] = EON_Clamp(m->Ambient[0],0,255);
    m->_RequestedColors[1] = EON_Clamp(m->Ambient[1],0,255);
    m->_RequestedColors[2] = EON_Clamp(m->Ambient[2],0,255);
}

static void eon_GeneratePhongPalette(EON_Mat *m)
{
    EON_uInt i = m->NumGradients, x;
    EON_sInt c;
    EON_uChar *pal;
    double a, da, ca, cb;
    m->_ColorsUsed = m->NumGradients;
    if (m->_RequestedColors)
        CX_free(m->_RequestedColors);
    pal =  m->_RequestedColors = CX_malloc(m->_ColorsUsed*3);
    a = EON_PI/2.0;

    if (m->NumGradients > 1)
        da = -EON_PI/((m->NumGradients-1)<<1);
    else
        da=0.0;

    do {
        if (m->NumGradients == 1)
            ca = 1;
        else {
            ca = cos((double) a);
            a += da;
        }
        cb = pow((double) ca, (double) m->Shininess);
        for (x = 0; x < 3; x ++) {
            c = (EON_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x]);
            *(pal++) = EON_Clamp(c,0,255);
        }
    } while (--i);
}

static void eon_GenerateTextureEnvPalette(EON_Mat *m)
{
    EON_sInt c;
    EON_uInt whichlevel,whichindex;
    EON_uChar *texpal, *envpal, *pal;
    m->_ColorsUsed = m->Texture->NumColors*m->Environment->NumColors;
    if (m->_RequestedColors)
        CX_free(m->_RequestedColors);
    pal = m->_RequestedColors = CX_malloc(m->_ColorsUsed*3);
    envpal = m->Environment->PaletteData;
    if (m->_AddTable)
        CX_free(m->_AddTable);
    m->_AddTable = CX_malloc(m->Environment->NumColors*sizeof(EON_uInt16));
    for (whichlevel = 0; whichlevel < m->Environment->NumColors; whichlevel++) {
        texpal = m->Texture->PaletteData;
        switch (m->TexEnvMode) {
        case EON_TEXENV_MUL: // multiply
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) * (EON_sInt) envpal[0])>>8);
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) * (EON_sInt) envpal[1])>>8);
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) * (EON_sInt) envpal[2])>>8);
            }
            break;
        case EON_TEXENV_AVG: // average
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) + (EON_sInt) envpal[0])>>1);
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) + (EON_sInt) envpal[1])>>1);
                *pal++ = (EON_uChar) (((EON_sInt) (*texpal++) + (EON_sInt) envpal[2])>>1);
            }
            break;
        case EON_TEXENV_TEXMINUSENV: // tex-env
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[0]; *pal++ = EON_Clamp(c,0,255);
                c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[1]; *pal++ = EON_Clamp(c,0,255);
                c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[2]; *pal++ = EON_Clamp(c,0,255);
            }
            break;
        case EON_TEXENV_ENVMINUSTEX: // env-tex
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[0]; *pal++ = EON_Clamp(c,0,255);
                c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[1]; *pal++ = EON_Clamp(c,0,255);
                c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[2]; *pal++ = EON_Clamp(c,0,255);
            }
            break;
        case EON_TEXENV_MIN:
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                *pal++ = EON_Min(texpal[0],envpal[0]);
                *pal++ = EON_Min(texpal[1],envpal[1]);
                *pal++ = EON_Min(texpal[2],envpal[2]);
                texpal+=3;
            }
            break;
        case EON_TEXENV_MAX:
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                *pal++ = EON_Max(texpal[0],envpal[0]);
                *pal++ = EON_Max(texpal[1],envpal[1]);
                *pal++ = EON_Max(texpal[2],envpal[2]);
                texpal+=3;
            }
            break;
        default: // add
            for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
                c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[0]; *pal++ = EON_Clamp(c,0,255);
                c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[1]; *pal++ = EON_Clamp(c,0,255);
                c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[2]; *pal++ = EON_Clamp(c,0,255);
            }
            break;
        }
        envpal += 3;
        m->_AddTable[whichlevel] = whichlevel*m->Texture->NumColors;
    }
}

static void eon_GenerateTexturePalette(EON_Mat *m, EON_Texture *t)
{
    EON_uChar *ppal, *pal;
    EON_sInt c, i, x;
    m->_ColorsUsed = t->NumColors;
    if (m->_RequestedColors)
        CX_free(m->_RequestedColors);
    pal = m->_RequestedColors = CX_malloc(m->_ColorsUsed * 3);
    ppal = t->PaletteData;
    i = t->NumColors;
    do {
        for (x = 0; x < 3; x ++) {
            c = m->Ambient[x] + *ppal++;
            *(pal++) = EON_Clamp(c,0,255);
        }
    } while (--i);
}

static void eon_GeneratePhongTexturePalette(EON_Mat *m, EON_Texture *t)
{
    double a, ca, da, cb;
    EON_uInt16 *addtable;
    EON_uChar *ppal, *pal;
    EON_sInt c, i, i2, x;
    EON_uInt num_shades;

    if (t->NumColors)
        num_shades = (m->NumGradients / t->NumColors);
    else
        num_shades=1;

    if (!num_shades)
        num_shades = 1;
    m->_ColorsUsed = num_shades*t->NumColors;
    if (m->_RequestedColors)
        CX_free(m->_RequestedColors);
    pal = m->_RequestedColors = CX_malloc(m->_ColorsUsed * 3);
    a = EON_PI/2.0;
    if (num_shades > 1)
        da = (-EON_PI/2.0)/(num_shades-1);
    else
        da = 0.0;
    i2 = num_shades;
    do {
        ppal = t->PaletteData;
        ca = cos((double) a);
        a += da;
        cb = pow(ca, (double) m->Shininess);
        i = t->NumColors;
        do {
            for (x = 0; x < 3; x ++) {
                c = (EON_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x] + *ppal++);
                *(pal++) = EON_Clamp(c,0,255);
            }
        } while (--i);
    } while (--i2);
    ca = 0;
    if (m->_AddTable)
        CX_free(m->_AddTable);
    m->_AddTable = CX_malloc(256 * sizeof(EON_uInt16));
    addtable = m->_AddTable;
    i = 256;
    do {
        a = sin(ca) * num_shades;
        ca += EON_PI/512.0;
        *addtable++ = ((EON_sInt) a)*t->NumColors;
    } while (--i);
}

static void eon_GeneratePhongTransparentPalette(EON_Mat *m)
{
    m->_tsfact = (EON_sInt) (m->NumGradients*(1.0/(1+m->Transparent)));
    eon_GeneratePhongPalette(m);
}

static void eon_GenerateTransparentPalette(EON_Mat *m)
{
    m->_tsfact = 0;
    eon_GeneratePhongPalette(m);
}

static void eon_SetMaterialPutFace(EON_Mat *m)
{
    m->_PutFace = EON_PF_Null;
    switch (m->_ft) {
    case EON_FILL_TRANSPARENT:
        switch(m->_st) {
        case EON_SHADE_NONE:
        case EON_SHADE_FLAT:
        case EON_SHADE_FLAT_DISTANCE:
        case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
//            m->_PutFace = EON_PF_TransF;
            break;
        case EON_SHADE_GOURAUD:
        case EON_SHADE_GOURAUD_DISTANCE:
        case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
//            m->_PutFace = EON_PF_TransG;
            break;
        }
        break;
    case EON_FILL_SOLID:
        switch(m->_st) {
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
    case EON_FILL_ENVIRONMENT:
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
    case EON_FILL_TEXTURE|EON_FILL_ENVIRONMENT:
        m->_PutFace = EON_PF_TexEnv;
        break;
    }
}

typedef struct __ct {
    struct __ct *next;
    EON_uChar r,g,b;
    EON_Bool visited;
} _ct;

static int mdist(_ct *a, _ct *b)
{
    return ((a->r-b->r)*(a->r-b->r)+(a->g-b->g)*(a->g-b->g)+(a->b-b->b)*(a->b-b->b));
}

void EON_MatMakeOptPal(EON_uChar *p, EON_sInt pstart,
                       EON_sInt pend, EON_Mat **materials, EON_sInt nmats)
{
    EON_uChar *allColors = 0;
    EON_sInt numColors = 0, nc, x;
    EON_sInt len = pend + 1 - pstart;
    EON_sInt32 current, newnext, bestdist, thisdist;
    _ct *colorBlock, *best, *cp;

    for (x = 0; x < nmats; x ++) {
        if (materials[x]) {
            if (!materials[x]->_RequestedColors)
                EON_MatInit(materials[x]);
            if (materials[x]->_RequestedColors)
                numColors+=materials[x]->_ColorsUsed;
        }
    }
    if (!numColors)
        return;

    allColors = CX_malloc(numColors * 3);
    numColors = 0;

    for (x = 0; x < nmats; x ++) {
        if (materials[x]) {
            if (materials[x]->_RequestedColors)
                memcpy(allColors + (numColors*3), materials[x]->_RequestedColors,
            materials[x]->_ColorsUsed*3);
            numColors += materials[x]->_ColorsUsed;
        }
    }

    if (numColors <= len) {
        memcpy(p+pstart*3,allColors,numColors*3);
        CX_free(allColors);
        return;
    }

    colorBlock = CX_malloc(sizeof(_ct) * numColors);
    for (x = 0; x < numColors; x++) {
        colorBlock[x].r = allColors[x*3];
        colorBlock[x].g = allColors[x*3+1];
        colorBlock[x].b = allColors[x*3+2];
        colorBlock[x].visited = 0;
        colorBlock[x].next = 0;
    }
    CX_free(allColors);

    /* Build a list, starting at color 0 */
    current = 0;
    nc = numColors;
    do {
        newnext = -1;
        bestdist = 300000000;
        colorBlock[current].visited = 1;
        for (x = 0; x < nc; x ++) {
            if (!colorBlock[x].visited) {
                thisdist = mdist(colorBlock + x, colorBlock + current);
                if (thisdist < 5) {
                    colorBlock[x].visited = 1;
                    numColors--;
                } else if (thisdist < bestdist) {
                    bestdist = thisdist;
                    newnext = x;
                }
            }
        }
        if (newnext != -1) {
            colorBlock[current].next = colorBlock + newnext;
            current = newnext;
        }
    } while (newnext != -1);
    colorBlock[current].next = 0; /* terminate the list */

    /* we now have a linked list starting at colorBlock, which is each one and
       it's closest neighbor */

    while (numColors > len) {
        bestdist = mdist(colorBlock,colorBlock->next);
        for (best = cp = colorBlock; cp->next; cp = cp->next) {
            if (bestdist > (thisdist = mdist(cp,cp->next))) {
                best = cp;
                bestdist = thisdist;
            }
        }
        best->r = ((int) best->r + (int) best->next->r)>>1;
        best->g = ((int) best->g + (int) best->next->g)>>1;
        best->b = ((int) best->b + (int) best->next->b)>>1;
        best->next = best->next->next;
        numColors--;
    }
    x = pstart*3;
    for (cp = colorBlock; cp; cp = cp->next) {
        p[x++] = cp->r;
        p[x++] = cp->g;
        p[x++] = cp->b;
    }
    CX_free(colorBlock);
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
                        &light->Xp, &light->Yp, &light->Zp);
        break;
    case EON_LIGHT_POINT_ANGLE:
    case EON_LIGHT_POINT_DISTANCE:
    case EON_LIGHT_POINT:
        light->Xp = x;
        light->Yp = y;
        light->Zp = z;
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
    double dx, dy, dz;
    dx = x - c->X;
    dy = y - c->Y;
    dz = z - c->Z;
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


void EON_CamSetPalette(EON_Cam *c, const uint8_t *palette, int numcolors)
{
    c->Palette = palette;
    return;
}

EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov,
                       EON_uChar *fb, EON_ZBuffer *zb)
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
        c->frameBuffer = fb;
        c->zBuffer = zb;
    }
    return c;
}

// clip.c
//

static void eon_FindNormal(double x2, double x3,
                           double y2, double y3,
                           double zv,
                           double *res);

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt eon_ClipToPlane(EON_Clip *clip, EON_uInt numVerts, double *plane);

void EON_ClipSetFrustum(EON_Clip *clip, EON_Cam *cam)
{
    clip->AdjAsp = 1.0 / cam->AspectRatio;
    clip->Fov = EON_Clamp(cam->Fov,1.0,179.0);
    clip->Fov = (1.0/tan(clip->Fov*(EON_PI/360.0)))*(double) (cam->ClipRight-cam->ClipLeft);
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
    }
    else
        eon_FindNormal(100,100,
                    -100, 100,
                    clip->Fov*100.0/(cam->ClipRight-cam->CenterX),
                    clip->ClipPlanes[2]);
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
}


void EON_ClipRenderFace(EON_Clip *clip, EON_Face *face)
{
    EON_uInt k, a, w, numVerts = 3;
    double tmp, tmp2;
    EON_Face newface;

    for (a = 0; a < 3; a ++) {
        clip->CL[0].newVertices[a] = *(face->Vertices[a]);
        clip->CL[0].Shades[a] = face->Shades[a];
        clip->CL[0].MappingU[a] = face->MappingU[a];
        clip->CL[0].MappingV[a] = face->MappingV[a];
        clip->CL[0].eMappingU[a] = face->eMappingU[a];
        clip->CL[0].eMappingV[a] = face->eMappingV[a];
    }

    a = (clip->ClipPlanes[0][3] < 0.0 ? 0 : 1);
    while (a < NUM_CLIP_PLANES && numVerts > 2) {
        numVerts = eon_ClipToPlane(clip, numVerts, clip->ClipPlanes[a]);
        memcpy(&clip->CL[0],&clip->CL[1],sizeof(clip->CL)/2);
        a++;
    }
    if (numVerts > 2) {
        memcpy(&newface,face,sizeof(EON_Face));
        for (k = 2; k < numVerts; k ++) {
            newface.fShade = EON_Clamp(face->fShade,0,1);
            for (a = 0; a < 3; a ++) {
                if (a == 0)
                    w = 0;
                else
                    w = a+(k-2);
                newface.Vertices[a] = clip->CL[0].newVertices+w;
                newface.Shades[a] = (EON_Float) clip->CL[0].Shades[w];
                newface.MappingU[a] = (EON_sInt32)clip->CL[0].MappingU[w];
                newface.MappingV[a] = (EON_sInt32)clip->CL[0].MappingV[w];
                newface.eMappingU[a] = (EON_sInt32)clip->CL[0].eMappingU[w];
                newface.eMappingV[a] = (EON_sInt32)clip->CL[0].eMappingV[w];
                newface.Scrz[a] = 1.0f/newface.Vertices[a]->xformedz;
                tmp2 = clip->Fov * newface.Scrz[a];
                tmp = tmp2*newface.Vertices[a]->xformedx;
                tmp2 *= newface.Vertices[a]->xformedy;
                newface.Scrx[a] = clip->Cx + ((EON_sInt32)((tmp*(float) (1<<20))));
                newface.Scry[a] = clip->Cy - ((EON_sInt32)((tmp2*clip->AdjAsp*(float) (1<<20))));
            }
            newface.Material->_PutFace(clip->Cam,&newface);
            clip->Info->TriStats[3] ++;
        }
        clip->Info->TriStats[2] ++;
    }
}

EON_sInt EON_ClipNeeded(EON_Clip *clip, EON_Face *face)
{
    double dr = (clip->Cam->ClipRight - clip->Cam->CenterX);
    double dl = (clip->Cam->ClipLeft - clip->Cam->CenterX);
    double db = (clip->Cam->ClipBottom - clip->Cam->CenterY);
    double dt = (clip->Cam->ClipTop - clip->Cam->CenterY);
    double f = clip->Fov * clip->AdjAsp;
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



static void eon_FindNormal(double x2, double x3,double y2, double y3,
                           double zv, double *res)
{
    res[0] = zv*(y2-y3);
    res[1] = zv*(x3-x2);
    res[2] = x2*y3 - y2*x3;
}

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt eon_ClipToPlane(EON_Clip *clip,
                                EON_uInt numVerts, double *plane)
{
    EON_uInt i, nextvert, curin, nextin;
    double curdot, nextdot, scale;
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
            clip->CL[1].eMappingU[outvert] = clip->CL[0].eMappingU[invert];
            clip->CL[1].eMappingV[outvert] = clip->CL[0].eMappingV[invert];
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
           clip->CL[1].eMappingU[outvert] = clip->CL[0].eMappingU[invert] +
                (clip->CL[0].eMappingU[nextvert] - clip->CL[0].eMappingU[invert]) * scale;
           clip->CL[1].eMappingV[outvert] = clip->CL[0].eMappingV[invert] +
                (clip->CL[0].eMappingV[nextvert] - clip->CL[0].eMappingV[invert]) * scale;
            outvert++;
        }
        curdot = nextdot;
        curin = nextin;
        invert++;
    }
    return outvert;
}

// spline.c
//

void EON_SplineGetPoint(EON_Spline *s, EON_Float frame, EON_Float *out)
{
    EON_sInt32 i, i_1, i0, i1, i2;
    EON_Float time1,time2,time3;
    EON_Float t1,t2,t3,t4,u1,u2,u3,u4,v1,v2,v3;
    EON_Float a,b,c,d;

    EON_Float *keys = s->keys;

    a = (1-s->tens)*(1+s->cont)*(1+s->bias);
    b = (1-s->tens)*(1-s->cont)*(1-s->bias);
    c = (1-s->tens)*(1-s->cont)*(1+s->bias);
    d = (1-s->tens)*(1+s->cont)*(1-s->bias);
    v1 = t1 = -a / 2.0; u1 = a;
    u2 = (-6-2*a+2*b+c)/2.0; v2 = (a-b)/2.0; t2 = (4+a-b-c) / 2.0;
    t3 = (-4+b+c-d) / 2.0;
    u3 = (6-2*b-c+d)/2.0;
    v3 = b/2.0;
    t4 = d/2.0; u4 = -t4;

    i0 = (EON_uInt) frame;
    i_1 = i0 - 1;
    while (i_1 < 0)
        i_1 += s->numKeys;
    i1 = i0 + 1;
    while (i1 >= s->numKeys)
        i1 -= s->numKeys;
    i2 = i0 + 2;
    while (i2 >= s->numKeys)
        i2 -= s->numKeys;
    time1 = frame - (EON_Float) ((EON_uInt) frame);
    time2 = time1*time1;
    time3 = time2*time1;
    i0 *= s->keyWidth;
    i1 *= s->keyWidth;
    i2 *= s->keyWidth;
    i_1 *= s->keyWidth;
    for (i = 0; i < s->keyWidth; i ++) {
        a = t1*keys[i+i_1]+t2*keys[i+i0]+t3*keys[i+i1]+t4*keys[i+i2];
        b = u1*keys[i+i_1]+u2*keys[i+i0]+u3*keys[i+i1]+u4*keys[i+i2];
        c = v1*keys[i+i_1]+v2*keys[i+i0]+v3*keys[i+i1];
        *out++ = a*time3 + b*time2 + c*time1 + keys[i+i0];
    }
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

// putface.h
//

#define PUTFACE_SORT() \
    i0 = 0; i1 = 1; i2 = 2; \
    if (TriFace->Scry[0] > TriFace->Scry[1]) { \
        i0 = 1; i1 = 0; \
    } \
    if (TriFace->Scry[i0] > TriFace->Scry[2]) { \
        i2 ^= i0; i0 ^= i2; i2 ^= i0; \
    } \
    if (TriFace->Scry[i1] > TriFace->Scry[i2]) { \
        i2 ^= i1; i1 ^= i2; i2 ^= i1; \
    }


#define PUTFACE_SORT_ENV() \
    PUTFACE_SORT(); \
    MappingU1=TriFace->eMappingU[i0]*Texture->uScale*\
              TriFace->Material->EnvScaling;\
    MappingV1=TriFace->eMappingV[i0]*Texture->vScale*\
              TriFace->Material->EnvScaling;\
    MappingU2=TriFace->eMappingU[i1]*Texture->uScale*\
              TriFace->Material->EnvScaling;\
    MappingV2=TriFace->eMappingV[i1]*Texture->vScale*\
              TriFace->Material->EnvScaling;\
    MappingU3=TriFace->eMappingU[i2]*Texture->uScale*\
              TriFace->Material->EnvScaling;\
    MappingV3=TriFace->eMappingV[i2]*Texture->vScale*\
              TriFace->Material->EnvScaling;

#define PUTFACE_SORT_TEX() \
    PUTFACE_SORT(); \
    MappingU1=TriFace->MappingU[i0]*Texture->uScale*\
              TriFace->Material->TexScaling;\
    MappingV1=TriFace->MappingV[i0]*Texture->vScale*\
              TriFace->Material->TexScaling;\
    MappingU2=TriFace->MappingU[i1]*Texture->uScale*\
              TriFace->Material->TexScaling;\
    MappingV2=TriFace->MappingV[i1]*Texture->vScale*\
              TriFace->Material->TexScaling;\
    MappingU3=TriFace->MappingU[i2]*Texture->uScale*\
              TriFace->Material->TexScaling;\
    MappingV3=TriFace->MappingV[i2]*Texture->vScale*\
              TriFace->Material->TexScaling;

static void EON_PF_Null(EON_Cam *cam, EON_Face *TriFace)
{
    return; /* nothing to do */
}

inline static EON_uInt32 eon_PickColorP(EON_Cam *cam, EON_Byte value)
{
    EON_uInt32 R = cam->Palette[3 * value + 0];
    EON_uInt32 G = cam->Palette[3 * value + 1];
    EON_uInt32 B = cam->Palette[3 * value + 2];
    EON_uInt32 A = 0xFF000000;
    return (A | R << 16 | G << 8| B);
}

inline static EON_uInt32 eon_PickColorF(const EON_Face *f)
{
    EON_uInt32 R = ((EON_sInt32)(f->Material->Ambient[0] * f->fShade)) & 0xFF;
    EON_uInt32 G = ((EON_sInt32)(f->Material->Ambient[1] * f->fShade)) & 0xFF;
    EON_uInt32 B = ((EON_sInt32)(f->Material->Ambient[2] * f->fShade)) & 0xFF;
    EON_uInt32 A = 0xFF000000;
    return (A | R << 16 | G << 8| B);
}

inline static EON_sInt32 eon_ToScreen(EON_sInt32 val)
{
    return (val + (1<<19)) >> 20;
}

// pf_solid.c
//

static void EON_PF_SolidF(EON_Cam *cam, EON_Face *TriFace)
{
    EON_uChar i0, i1, i2;

    EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
    EON_ZBuffer *zbuf = cam->zBuffer;

    EON_sInt32 X1, X2;
    EON_sInt32 XL1, XL2;
    EON_sInt32 dX1=0, dX2=0;
    EON_ZBuffer dZL=0, dZ1=0, dZ2=0;
    EON_ZBuffer Z0, Z1, Z2, ZL;
    EON_sInt32 Y1, Y2, Y0, dY;
    EON_uChar stat;
    EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;
    EON_uInt32 color = eon_PickColorF(TriFace);

    PUTFACE_SORT();

    X1 = TriFace->Scrx[i0];
    X2 = TriFace->Scrx[i0];
    Z0 = TriFace->Scrz[i0];
    Z1 = TriFace->Scrz[i1];
    Z2 = TriFace->Scrz[i2];
    Y0 = eon_ToScreen(TriFace->Scry[i0]);
    Y1 = eon_ToScreen(TriFace->Scry[i1]);
    Y2 = eon_ToScreen(TriFace->Scry[i2]);

    dY = Y2-Y0;
    if (dY) {
        dX2 = (TriFace->Scrx[i2] - X1) / dY;
        dZ2 = (Z2 - Z0) / dY;
    }
    dY = Y1-Y0;
    if (dY) {
        dX1 = (TriFace->Scrx[i1] - X1) / dY;
        dZ1 = (Z1 - Z0) / dY;
        if (dX2 < dX1) {
            dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
            dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
            stat = 2;
        } else
            stat = 1;
        Z1 = Z0;
    } else {
        if (TriFace->Scrx[i1] > X1) {
            X2 = TriFace->Scrx[i1];
            stat = 2|4;
        } else {
            X1 = TriFace->Scrx[i1];
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
            dY = Y2 - eon_ToScreen(TriFace->Scry[i1]);
            if (dY) {
                if (stat & 1) {
                    X1 = TriFace->Scrx[i1];
                    dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
                }
                if (stat & 2) {
                    X2 = TriFace->Scrx[i1];
                    dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
                }
                if (stat & 4) {
                    X1 = TriFace->Scrx[i0];
                    dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
                }
                if (stat & 8) {
                    X2 = TriFace->Scrx[i0];
                    dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
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

static void EON_PF_SolidG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_uChar *remap = TriFace->Material->_ReMapTable;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_ZBuffer dZL=0, dZ1=0, dZ2=0, Z1, Z2, ZL, Z3;
  EON_sInt32 dX1=0, dX2=0, X1, X2, XL1, XL2;
  EON_sInt32 C1, C2, dC1=0, dC2=0, dCL=0, CL, C3;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  EON_Float nc = (TriFace->Material->_ColorsUsed-1)*65536.0f;
  EON_sInt32 maxColor=((TriFace->Material->_ColorsUsed-1)<<16);
  EON_sInt32 maxColorNonShift=TriFace->Material->_ColorsUsed-1;

  PUTFACE_SORT();

  C1 = (EON_sInt32) (TriFace->Shades[i0]*nc);
  C2 = (EON_sInt32) (TriFace->Shades[i1]*nc);
  C3 = (EON_sInt32) (TriFace->Shades[i2]*nc);
  X2 = X1 = TriFace->Scrx[i0];
  Z1 = TriFace->Scrz[i0];
  Z2 = TriFace->Scrz[i1];
  Z3 = TriFace->Scrz[i2];

  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dC2 = (C3 - C1) / dY;
    dZ2 = (Z3 - Z1) / dY;
  }
  dY = Y1 - Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dC1 = (C2 - C1) / dY;
    dZ1 = (Z2 - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
    Z2 = Z1;
    C2 = C1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      stat = 2|4;
    } else {
      X1 = C1; C1 = C2; C2 = X1;
      ZL = Z1; Z1 = Z2; Z2 = ZL;
      X1 = TriFace->Scrx[i1];
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
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
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        dZ1 = (Z3-Z1)/dY;
        dC1 = (C3-C1) / dY;
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
      }
    }
    CL = C1;
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    XL2 -= XL1;
    if (XL2 > 0) {
      EON_Byte CX = 0;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            if (CL >= maxColor) CX = remap[maxColorNonShift];
            else if (CL > 0) CX = remap[CL>>16];
            else CX = remap[0];
            *gmem = eon_PickColorP(cam, CX);
          }
          gmem++;
          zbuf++;
          ZL += dZL;
          CL += dCL;
        } while (--XL2);
      else do {
          if (CL >= maxColor) CX = remap[maxColorNonShift];
          else if (CL > 0) CX = remap[CL>>16];
          else CX = remap[0];
          *gmem = eon_PickColorP(cam, CX);
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

static void EON_PF_TexEnv(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_uChar *remap;
  EON_ZBuffer *zbuf = cam->zBuffer;

  EON_sInt32 MappingU1, MappingU2, MappingU3;
  EON_sInt32 MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_sInt32 eMappingU1, eMappingU2, eMappingU3;
  EON_sInt32 eMappingV1, eMappingV2, eMappingV3;
  EON_sInt32 eMappingU_AND, eMappingV_AND;

  EON_uChar *texture, *environment;
  EON_uChar vshift;
  EON_uChar evshift;
  EON_uInt16 *addtable;
  EON_Texture *Texture, *Environment;
  EON_uChar stat;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  EON_sInt32 U1, V1, U2, V2, dU1=0, dU2=0, dV1=0, dV2=0, dUL=0, dVL=0, UL, VL;
  EON_sInt32 eU1, eV1, eU2, eV2, edU1=0, edU2=0, edV1=0,
            edV2=0, edUL=0, edVL=0, eUL, eVL;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_Float Z1, ZL, dZ1=0, dZ2=0, dZL=0, Z2;
  EON_sInt32 Y1, Y2, Y0, dY;

  Environment = TriFace->Material->Environment;
  Texture = TriFace->Material->Texture;

  if (!Texture || !Environment) return;
  texture = Texture->Data;
  environment = Environment->Data;
  addtable = TriFace->Material->_AddTable;
  remap = TriFace->Material->_ReMapTable;

  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;
  eMappingV_AND = ((1<<Environment->Height)-1)<<Environment->Width;
  eMappingU_AND = (1<<Environment->Width)-1;
  evshift = 16 - Environment->Width;

  PUTFACE_SORT_TEX();

  eMappingU1=(EON_sInt32) (TriFace->eMappingU[i0]*Environment->uScale*TriFace->Material->EnvScaling);
  eMappingV1=(EON_sInt32) (TriFace->eMappingV[i0]*Environment->vScale*TriFace->Material->EnvScaling);
  eMappingU2=(EON_sInt32) (TriFace->eMappingU[i1]*Environment->uScale*TriFace->Material->EnvScaling);
  eMappingV2=(EON_sInt32) (TriFace->eMappingV[i1]*Environment->vScale*TriFace->Material->EnvScaling);
  eMappingU3=(EON_sInt32) (TriFace->eMappingU[i2]*Environment->uScale*TriFace->Material->EnvScaling);
  eMappingV3=(EON_sInt32) (TriFace->eMappingV[i2]*Environment->vScale*TriFace->Material->EnvScaling);

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  eU1 = eU2 = eMappingU1;
  eV1 = eV2 = eMappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
    edU2 = (eMappingU3 - eU1) / dY;
    edV2 = (eMappingV3 - eV1) / dY;
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    edU1 = (eMappingU2 - eU1) / dY;
    edV1 = (eMappingV2 - eV1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      edU2 ^= edU1; edU1 ^= edU2; edU2 ^= edU1;
      edV2 ^= edV1; edV1 ^= edV2; edV2 ^= edV1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      eU2 = eMappingU2;
      eV2 = eMappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      eU1 = eMappingU2;
      eV1 = eMappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

  XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
  if (XL1) {
    dUL = ((dU1-dU2)*dY)/XL1;
    dVL = ((dV1-dV2)*dY)/XL1;
    edUL = ((edU1-edU2)*dY)/XL1;
    edVL = ((edV1-edV2)*dY)/XL1;
    dZL = ((dZ1-dZ2)*dY)/XL1;
  } else {
    XL1 = ((X2-X1+(1<<19))>>20);
    if (XL1) {
      edUL = (eU2-eU1)/XL1;
      edVL = (eV2-eV1)/XL1;
      dUL = (U2-U1)/XL1;
      dVL = (V2-V1)/XL1;
      dZL = (Z2-Z1)/XL1;
    }
  }

  while (Y0 < Y2) {
    if (Y0 == Y1) {
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
        edV1 = (eMappingV3 - eV1) / dY;
        edU1 = (eMappingU3 - eU1) / dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    UL = U1;
    VL = V1;
    eUL = eU1;
    eVL = eV1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = eon_PickColorP(cam,
                                   remap[addtable[environment[
                ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>vshift)&MappingV_AND)]]);
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
          eUL += edUL;
          eVL += edVL;
        } while (--XL2);
      else do {
          *gmem = eon_PickColorP(cam,
              remap[addtable[environment[
              ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] +
                          texture[((UL>>16)&MappingU_AND) +
                                  ((VL>>vshift)&MappingV_AND)]]);
          gmem++;
          UL += dUL;
          VL += dVL;
          eUL += edUL;
          eVL += edVL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    zbuf += cam->ScreenWidth;
    gmem += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    U1 += dU1;
    V1 += dV1;
    eU1 += edU1;
    eV1 += edV1;
    Y0++;
  }
}

static void EON_PF_TexF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_sInt32 MappingU1, MappingU2, MappingU3;
  EON_sInt32 MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar vshift;
  EON_uInt bc;
  EON_uChar *remap;
  EON_Texture *Texture;
  EON_uChar stat;

  EON_ZBuffer Z1, ZL, dZ1=0, dZL=0, Z2, dZ2=0;
  EON_sInt32 dU1=0, dV1=0, dU2=0, dV2=0, U1, V1, U2, V2;
  EON_sInt32 dUL=0, dVL=0, UL, VL;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;
  EON_sInt shade;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  remap = TriFace->Material->_ReMapTable;
  if (TriFace->Material->_AddTable)
  {
    shade=(EON_sInt)(TriFace->fShade*255.0f);
    if (shade < 0) shade=0;
    if (shade > 255) shade=255;
    bc = TriFace->Material->_AddTable[shade];
  }
  else bc=0;
  texture = Texture->Data;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dV2 = (MappingV3 - V1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
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
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
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
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1) / dY;
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
            *zbuf = ZL;
            *gmem = eon_PickColorP(cam,
                           remap[bc + texture[((UL >> 16)&MappingU_AND) +
                                ((VL>>vshift)&MappingV_AND)]]);
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          *gmem = eon_PickColorP(cam,
                           remap[bc + texture[((UL >> 16)&MappingU_AND) +
                                ((VL>>vshift)&MappingV_AND)]]);
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

static void EON_PF_TexG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_sInt32 MappingU1, MappingU2, MappingU3;
  EON_sInt32 MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar *remap;
  EON_uChar vshift;
  EON_uInt16 *addtable;
  EON_Texture *Texture;

  EON_sInt32 U1, V1, U2, V2, dU1=0, dU2=0, dV1=0, dV2=0, dUL=0, dVL=0, UL, VL;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_sInt32 C1, C2, dC1=0, dC2=0, CL, dCL=0;
  EON_ZBuffer Z1, ZL, dZ1=0, dZ2=0, dZL=0, Z2;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;

  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  remap = TriFace->Material->_ReMapTable;
  texture = Texture->Data;
  addtable = TriFace->Material->_AddTable;
  vshift = 16 - Texture->Width;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  C1 = C2 = TriFace->Shades[i0]*65535.0f;
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dC2 = (TriFace->Shades[i2]*65535.0f - C1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (TriFace->Shades[i1]*65535.0f - C1) / dY;
    dU1 = (MappingU2 - U1) / dY;
    dV1 = (MappingV2 - V1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dU2 ^= dU1; dU1 ^= dU2; dU2 ^= dU1;
      dV2 ^= dV1; dV1 ^= dV2; dV2 ^= dV1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = TriFace->Shades[i1]*65535.0f;
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = TriFace->Shades[i1]*65535.0f;
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
      dY = Y2 - ((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dV1 = (MappingV3 - V1) / dY;
        dU1 = (MappingU3 - U1) / dY;
        dC1 = (TriFace->Shades[i2]*65535.0f-C1)/dY;
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
            int av;
            if (CL < 0) av=addtable[0];
            else if (CL > (255<<8)) av=addtable[255];
            else av=addtable[CL>>8];
            *zbuf = ZL;
            *gmem = eon_PickColorP(cam,
                            remap[av +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>vshift)&MappingV_AND)]]);
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          CL += dCL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          int av;
          if (CL < 0) av=addtable[0];
          else if (CL > (255<<8)) av=addtable[255];
          else av=addtable[CL>>8];
          *gmem = eon_PickColorP(cam,
                          remap[av +
                          texture[((UL>>16)&MappingU_AND) +
                                  ((VL>>vshift)&MappingV_AND)]]);
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

static void EON_PF_PTexF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_uChar *remap = TriFace->Material->_ReMapTable;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_Float MappingU1, MappingU2, MappingU3;
  EON_Float MappingV1, MappingV2, MappingV3;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar *texture;
  EON_uChar vshift;
  EON_uInt16 bc;
  EON_Texture *Texture;
  EON_sInt32 iShade;

  EON_uChar nm, nmb;
  EON_sInt n;
  EON_Float U1,V1,U2,V2,dU1=0,dU2=0,dV1=0,dV2=0,dUL=0,dVL=0,UL,VL;
  EON_sInt32 iUL, iVL, idUL=0, idVL=0, iULnext, iVLnext;

  EON_sInt32 scrwidth = cam->ScreenWidth;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, Xlen;
  EON_ZBuffer Z1, dZ1=0, dZ2=0, Z2, dZL=0, ZL, pZL, pdZL;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;

  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  iShade = (EON_sInt32)(TriFace->fShade*256.0);
  if (iShade < 0) iShade=0;
  if (iShade > 255) iShade=255;

  if (!TriFace->Material->_AddTable) bc=0;
  else bc = TriFace->Material->_AddTable[iShade];
  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = EON_Min(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  MappingU1 *= TriFace->Scrz[i0]/65536.0f;
  MappingV1 *= TriFace->Scrz[i0]/65536.0f;
  MappingU2 *= TriFace->Scrz[i1]/65536.0f;
  MappingV2 *= TriFace->Scrz[i1]/65536.0f;
  MappingU3 *= TriFace->Scrz[i2]/65536.0f;
  MappingV3 *= TriFace->Scrz[i2]/65536.0f;

  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
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
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
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
      dY = Y2-((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
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
              *zbuf = ZL;
              *gmem = eon_PickColorP(cam,
                              remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]]);
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            *gmem = eon_PickColorP(cam,
                              remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]]);
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

static void EON_PF_PTexG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_Float MappingU1, MappingU2, MappingU3;
  EON_Float MappingV1, MappingV2, MappingV3;

  EON_Texture *Texture;
  EON_Bool zb = (cam->zBuffer&&TriFace->Material->zBufferable) ? 1 : 0;

  EON_uChar nm, nmb;
  EON_uInt n;
  EON_sInt32 MappingU_AND, MappingV_AND;
  EON_uChar vshift;
  EON_uChar *texture;
  EON_uInt16 *addtable;
  EON_uChar *remap = TriFace->Material->_ReMapTable;
  EON_sInt32 iUL, iVL, idUL, idVL, iULnext, iVLnext;
  EON_Float U2,V2,dU2=0,dV2=0,dUL=0,dVL=0,UL,VL;
  EON_sInt32 XL1, Xlen;
  EON_sInt32 C2, dC2=0, CL, dCL=0;
  EON_Float ZL, Z2, dZ2=0, dZL=0, pdZL, pZL;

  EON_sInt32 Y2, dY;
  EON_uChar stat;

  /* Cache line */
  EON_sInt32 Y0,Y1;
  EON_sInt32 C1, dC1=0, X2, dX2=0, X1, dX1=0;

  /* Cache line */
  EON_Float dU1=0, U1, dZ1=0, Z1, V1, dV1=0;
  EON_sInt32 scrwidth = cam->ScreenWidth;
  EON_uInt32 *gmem = (EON_uInt32 *)cam->frameBuffer;
  EON_ZBuffer *zbuf = cam->zBuffer;

  if (TriFace->Material->Environment) Texture = TriFace->Material->Environment;
  else Texture = TriFace->Material->Texture;

  if (!Texture) return;
  texture = Texture->Data;
  addtable = TriFace->Material->_AddTable;
  if (!addtable) return;

  nm = TriFace->Material->PerspectiveCorrect;
  nmb = 0; while (nm) { nmb++; nm >>= 1; }
  nmb = EON_Min(6,nmb);
  nm = 1<<nmb;
  MappingV_AND = ((1<<Texture->Height)-1)<<Texture->Width;
  MappingU_AND = (1<<Texture->Width)-1;
  vshift = 16 - Texture->Width;

  if (TriFace->Material->Environment) {
    PUTFACE_SORT_ENV();
  } else {
    PUTFACE_SORT_TEX();
  }

  MappingU1 *= TriFace->Scrz[i0]/65536.0f;
  MappingV1 *= TriFace->Scrz[i0]/65536.0f;
  MappingU2 *= TriFace->Scrz[i1]/65536.0f;
  MappingV2 *= TriFace->Scrz[i1]/65536.0f;
  MappingU3 *= TriFace->Scrz[i2]/65536.0f;
  MappingV3 *= TriFace->Scrz[i2]/65536.0f;
  TriFace->Shades[0] *= 65536.0f;
  TriFace->Shades[1] *= 65536.0f;
  TriFace->Shades[2] *= 65536.0f;

  C1 = C2 = (EON_sInt32) TriFace->Shades[i0];
  U1 = U2 = MappingU1;
  V1 = V2 = MappingV1;
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
    dC2 = (EON_sInt32) ((TriFace->Shades[i2] - C1) / dY);
    dU2 = (MappingU3 - U1) / dY;
    dV2 = (MappingV3 - V1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (EON_sInt32) ((TriFace->Shades[i1] - C1) / dY);
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
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = (EON_sInt32)TriFace->Shades[i1];
      U2 = MappingU2;
      V2 = MappingV2;
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = (EON_sInt32)TriFace->Shades[i1];
      U1 = MappingU2;
      V1 = MappingV2;
      stat = 1|8;
    }
  }

  gmem += (Y0 * scrwidth);
  zbuf += (Y0 * scrwidth);

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
      dY = Y2-((TriFace->Scry[i1]+(1<<19))>>20);
      if (dY) {
        if (stat & 1) {
          X1 = TriFace->Scrx[i1];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 2) {
          X2 = TriFace->Scrx[i1];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i1])/dY;
        }
        if (stat & 4) {
          X1 = TriFace->Scrx[i0];
          dX1 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        if (stat & 8) {
          X2 = TriFace->Scrx[i0];
          dX2 = (TriFace->Scrx[i2]-TriFace->Scrx[i0])/dY;
        }
        dZ1 = (TriFace->Scrz[i2]-Z1)/dY;
        dC1 = (EON_sInt32)((TriFace->Shades[i2]-C1)/dY);
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
      XL1 += Xlen-scrwidth;
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
              int av;
              if (CL < 0) av=addtable[0];
              else if (CL > (255<<8)) av=addtable[255];
              else av=addtable[CL>>8];
              *zbuf = ZL;
              *gmem = eon_PickColorP(cam,
                      remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]]);
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            int av;
            if (CL < 0) av=addtable[0];
            else if (CL > (255<<8)) av=addtable[255];
            else av=addtable[CL>>8];
            *gmem = eon_PickColorP(cam,
                      remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]]);
            gmem++;
            CL += dCL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
      } while (Xlen > 0);
      gmem -= XL1;
      zbuf -= XL1;
    } else {
      zbuf += scrwidth;
      gmem += scrwidth;
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
    register double length; \
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

static void eon_RendReset(EON_Rend *rend)
{
    memset(rend, 0, sizeof(*rend));
    return;
}

EON_Rend *EON_RendCreate(EON_Cam *Camera)
{
    EON_Rend *rend = CX_malloc(sizeof(EON_Rend));
    if (rend) {
        eon_RendReset(rend);
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
    EON_Float *pl, xp, yp, zp;
    if (light->Type == EON_LIGHT_NONE || rend->NumLights >= EON_MAX_LIGHTS)
        return;
    pl = rend->Lights[rend->NumLights].l;
    if (light->Type == EON_LIGHT_VECTOR) {
        xp = light->Xp;
        yp = light->Yp;
        zp = light->Zp;
        MACRO_eon_MatrixApply(rend->CMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
    } else if (light->Type & EON_LIGHT_POINT) {
        xp = light->Xp - rend->Cam->X;
        yp = light->Yp - rend->Cam->Y;
        zp = light->Zp - rend->Cam->Z;
        MACRO_eon_MatrixApply(rend->CMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
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
            EON_Double nx2 = rend->Lights[i].l[0] - vertex->xformedx;
            EON_Double ny2 = rend->Lights[i].l[1] - vertex->xformedy;
            EON_Double nz2 = rend->Lights[i].l[2] - vertex->xformedz;
            MACRO_eon_NormalizeVector(nx2,ny2,nz2);
            CurShade = MACRO_eon_DotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
        }
        if (light->Type & EON_LIGHT_POINT_DISTANCE) {
            EON_Double nx2 = rend->Lights[i].l[0] - vertex->xformedx;
            EON_Double ny2 = rend->Lights[i].l[1] - vertex->xformedy;
            EON_Double nz2 = rend->Lights[i].l[2] - vertex->xformedz;
            EON_Double t = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
            CurShade *= EON_Clamp(t,0,1.0)*light->Intensity; // FIXME
        }
        if (light->Type == EON_LIGHT_VECTOR) {
            CurShade = MACRO_eon_DotProduct(nx,ny,nz,
                                            rend->Lights[i].l[0],
                                            rend->Lights[i].l[1],
                                            rend->Lights[i].l[2]);
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
    EON_Double shade = face->sLighting;
    if (face->Material->_st & EON_SHADE_FLAT) {
        shade = eon_RenderVertexLights(rend,
                                       face->Vertices[0],
                                       face->sLighting,
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
    face->fShade = (EON_Float)shade;
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

inline static void eon_RenderShadeObjEnviron(EON_Rend *rend, EON_Face *face)
{
    face->eMappingU[0] = 32768 + (EON_sInt32) (face->Vertices[0]->xformednx*32768.0);
    face->eMappingV[0] = 32768 - (EON_sInt32) (face->Vertices[0]->xformedny*32768.0);
    face->eMappingU[1] = 32768 + (EON_sInt32) (face->Vertices[1]->xformednx*32768.0);
    face->eMappingV[1] = 32768 - (EON_sInt32) (face->Vertices[1]->xformedny*32768.0);
    face->eMappingU[2] = 32768 + (EON_sInt32) (face->Vertices[2]->xformednx*32768.0);
    face->eMappingV[2] = 32768 - (EON_sInt32) (face->Vertices[2]->xformedny*32768.0);
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
                        -rend->Cam->X, -rend->Cam->Y, -rend->Cam->Z);
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

    rend->Info.TriStats[0] += obj->NumFaces;
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
                if (face->Material->_st & (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE)) {
                    eon_RenderShadeObjFlat(rend, face, obj->BackfaceIllumination, nx, ny, nz);
                }
                if (face->Material->_ft & EON_FILL_ENVIRONMENT) {
                    eon_RenderShadeObjEnviron(rend, face);
                }
                if (face->Material->_st &(EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE)) {
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 0);
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 1);
                    eon_RenderShadeObjGourad(rend, face, obj->BackfaceIllumination, 2);
                }
                rend->Faces[facepos].zd = face->Vertices[0]->xformedz + face->Vertices[1]->xformedz + face->Vertices[2]->xformedz;
                rend->Faces[facepos].face = face;
                facepos++;
                rend->Info.TriStats[1] ++;
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

void EON_RenderEnd(EON_Rend *rend)
{
    EON_FaceInfo *f = rend->Faces;
    while (rend->NumFaces--) {
        EON_ClipRenderFace(&rend->Clip, f->face);
        f++;
    }
    rend->NumFaces = 0;
    rend->NumLights = 0;
    return;
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

