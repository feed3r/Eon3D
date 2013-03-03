/**************************************************************************
 * eon3d.h -- Eon3D is a simplistic 3D software renderer.               *
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

#include "eon3d.h"

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
                free(o->Vertices);
            if (o->Faces)
                free(o->Faces);
        free(o);
    }
    return;
}

EON_Obj *EON_ObjCreate(EON_uInt32 nv, EON_uInt32 nf)
{
    EON_Obj *o;
    if (!(o = (EON_Obj *) malloc(sizeof(EON_Obj))))
        return 0;
    memset(o,0,sizeof(EON_Obj));
    o->GenMatrix = 1;
    o->BackfaceCull = 1;
    o->NumVertices = nv;
    o->NumFaces = nf;
    if (nv && !(o->Vertices=(EON_Vertex *) malloc(sizeof(EON_Vertex)*nv))) {
        free(o);
        return 0;
    }
    if (nf && !(o->Faces = (EON_Face *) malloc(sizeof(EON_Face)*nf))) {
        free(o->Vertices);
        free(o);
        return 0;
    }
    memset(o->Vertices,0,sizeof(EON_Vertex)*nv);
    memset(o->Faces,0,sizeof(EON_Face)*nf);
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
    EON_Mat *m;
    m = (EON_Mat *) malloc(sizeof(EON_Mat));
    if (!m)
        return 0;
    memset(m,0,sizeof(EON_Mat));
    m->EnvScaling = 1.0f;
    m->TexScaling = 1.0f;
    m->Ambient[0] = m->Ambient[1] = m->Ambient[2] = 0;
    m->Diffuse[0] = m->Diffuse[1] = m->Diffuse[2] = 128;
    m->Specular[0] = m->Specular[1] = m->Specular[2] = 128;
    m->Shininess = 4;
    m->NumGradients = 32;
    m->FadeDist = 1000.0;
    m->zBufferable = 1;
    return m;
}

void EON_MatDelete(EON_Mat *m)
{
    if (m) {
        if (m->_ReMapTable)
            free(m->_ReMapTable);
        if (m->_RequestedColors)
            free(m->_RequestedColors);
        if (m->_AddTable)
            free(m->_AddTable);
        free(m);
    }
    return;
}

void EON_MatInit(EON_Mat *m)
{
    if (m->Shininess < 1)
        m->Shininess = 1;
    m->_ft = ((m->Environment ? EON_FILL_ENVIRONMENT : 0) |
               (m->Texture ? EON_FILL_TEXTURE : 0));
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
            free(m->_AddTable);
        m->_AddTable = (EON_uInt16 *) malloc(256*sizeof(EON_uInt16));
        for (x = 0; x < 256; x ++) {
            intensity = *pal++;
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
  if (!m->_RequestedColors) EON_MatInit(m);
  if (!m->_RequestedColors) return;
  if (m->_ReMapTable) free(m->_ReMapTable);
  m->_ReMapTable = (EON_uChar *) malloc(m->_ColorsUsed);
  for (i = 0; i < m->_ColorsUsed; i ++) {
    bestdiff = 1000000000;
    bestpos = pstart;
    r = m->_RequestedColors[i*3];
    g = m->_RequestedColors[i*3+1];
    b = m->_RequestedColors[i*3+2];
    p = pal + pstart*3;
    for (k = pstart; k <= (EON_sInt)pend; k ++) {
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
  if (m->_RequestedColors) free(m->_RequestedColors);
  m->_RequestedColors = (EON_uChar *) malloc(3);
  m->_RequestedColors[0] = EON_Min(EON_Max(m->Ambient[0],0),255);
  m->_RequestedColors[1] = EON_Min(EON_Max(m->Ambient[1],0),255);
  m->_RequestedColors[2] = EON_Min(EON_Max(m->Ambient[2],0),255);
}

static void eon_GeneratePhongPalette(EON_Mat *m)
{
  EON_uInt i = m->NumGradients, x;
  EON_sInt c;
  EON_uChar *pal;
  double a, da, ca, cb;
  m->_ColorsUsed = m->NumGradients;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal =  m->_RequestedColors = (EON_uChar *) malloc(m->_ColorsUsed*3);
  a = EON_PI/2.0;

  if (m->NumGradients > 1) da = -EON_PI/((m->NumGradients-1)<<1);
  else da=0.0;

  do {
    if (m->NumGradients == 1) ca = 1;
    else {
      ca = cos((double) a);
      a += da;
    }
    cb = pow((double) ca, (double) m->Shininess);
    for (x = 0; x < 3; x ++) {
      c = (EON_sInt) ((cb*m->Specular[x])+(ca*m->Diffuse[x])+m->Ambient[x]);
      *(pal++) = EON_Max(0,EON_Min(c,255));
    }
  } while (--i);
}

static void eon_GenerateTextureEnvPalette(EON_Mat *m)
{
  EON_sInt c;
  EON_uInt whichlevel,whichindex;
  EON_uChar *texpal, *envpal, *pal;
  m->_ColorsUsed = m->Texture->NumColors*m->Environment->NumColors;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (EON_uChar *) malloc(m->_ColorsUsed*3);
  envpal = m->Environment->PaletteData;
  if (m->_AddTable) free(m->_AddTable);
  m->_AddTable = (EON_uInt16 *) malloc(m->Environment->NumColors*sizeof(EON_uInt16));
  for (whichlevel = 0; whichlevel < m->Environment->NumColors; whichlevel++) {
    texpal = m->Texture->PaletteData;
    switch (m->TexEnvMode)
    {
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
          c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[0]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[1]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = (EON_sInt) (*texpal++) - (EON_sInt) envpal[2]; *pal++ = EON_Max(0,EON_Min(255,c));
        }
      break;
      case EON_TEXENV_ENVMINUSTEX: // env-tex
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[0]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[1]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = -(EON_sInt) (*texpal++) - (EON_sInt) envpal[2]; *pal++ = EON_Max(0,EON_Min(255,c));
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
      break;
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          *pal++ = EON_Max(texpal[0],envpal[0]);
          *pal++ = EON_Max(texpal[1],envpal[1]);
          *pal++ = EON_Max(texpal[2],envpal[2]);
          texpal+=3;
        }
      default: // add
        for (whichindex = 0; whichindex < m->Texture->NumColors; whichindex++) {
          c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[0]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[1]; *pal++ = EON_Max(0,EON_Min(255,c));
          c = (EON_sInt) (*texpal++) + (EON_sInt) envpal[2]; *pal++ = EON_Max(0,EON_Min(255,c));
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
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (EON_uChar *) malloc(m->_ColorsUsed*3);
  ppal = t->PaletteData;
  i = t->NumColors;
  do {
    for (x = 0; x < 3; x ++) {
      c = m->Ambient[x] + *ppal++;
      *(pal++) = EON_Max(0,EON_Min(c,255));
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

  if (t->NumColors) num_shades = (m->NumGradients / t->NumColors);
  else num_shades=1;

  if (!num_shades) num_shades = 1;
  m->_ColorsUsed = num_shades*t->NumColors;
  if (m->_RequestedColors) free(m->_RequestedColors);
  pal = m->_RequestedColors = (EON_uChar *) malloc(m->_ColorsUsed*3);
  a = EON_PI/2.0;
  if (num_shades>1) da = (-EON_PI/2.0)/(num_shades-1);
  else da=0.0;
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
        *(pal++) = EON_Max(0,EON_Min(c,255));
      }
    } while (--i);
  } while (--i2);
  ca = 0;
  if (m->_AddTable) free(m->_AddTable);
  m->_AddTable = (EON_uInt16 *) malloc(256*sizeof(EON_uInt16));
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

static void  eon_GenerateTransparentPalette(EON_Mat *m)
{
  m->_tsfact = 0;
  eon_GeneratePhongPalette(m);
}

static void eon_SetMaterialPutFace(EON_Mat *m)
{
  m->_PutFace = 0;
  switch (m->_ft) {
    case EON_FILL_TRANSPARENT: switch(m->_st) {
      case EON_SHADE_NONE: case EON_SHADE_FLAT:
      case EON_SHADE_FLAT_DISTANCE: case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
        m->_PutFace = EON_PF_TransF;
      break;
      case EON_SHADE_GOURAUD: case EON_SHADE_GOURAUD_DISTANCE:
      case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
        m->_PutFace = EON_PF_TransG;
      break;
    }
    break;
    case EON_FILL_SOLID: switch(m->_st) {
      case EON_SHADE_NONE: case EON_SHADE_FLAT:
      case EON_SHADE_FLAT_DISTANCE: case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
        m->_PutFace = EON_PF_SolidF;
      break;
      case EON_SHADE_GOURAUD: case EON_SHADE_GOURAUD_DISTANCE:
      case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
        m->_PutFace = EON_PF_SolidG;
      break;
    }
    break;
    case EON_FILL_ENVIRONMENT:
    case EON_FILL_TEXTURE:
      if (m->PerspectiveCorrect) switch (m->_st) {
        case EON_SHADE_NONE: case EON_SHADE_FLAT:
        case EON_SHADE_FLAT_DISTANCE: case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
          m->_PutFace = EON_PF_PTexF;
        break;
        case EON_SHADE_GOURAUD: case EON_SHADE_GOURAUD_DISTANCE:
        case EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE:
          m->_PutFace = EON_PF_PTexG;
        break;
      }
      else switch (m->_st) {
        case EON_SHADE_NONE: case EON_SHADE_FLAT:
        case EON_SHADE_FLAT_DISTANCE: case EON_SHADE_FLAT_DISTANCE|EON_SHADE_FLAT:
          m->_PutFace = EON_PF_TexF;
        break;
        case EON_SHADE_GOURAUD: case EON_SHADE_GOURAUD_DISTANCE:
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
  EON_uChar r,g,b;
  EON_Bool visited;
  struct __ct *next;
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
      if (!materials[x]->_RequestedColors) EON_MatInit(materials[x]);
      if (materials[x]->_RequestedColors) numColors+=materials[x]->_ColorsUsed;
    }
  }
  if (!numColors) return;

  allColors=(EON_uChar*)malloc(numColors*3);
  numColors=0;

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
    free(allColors);
    return;
  }

  colorBlock = (_ct *) malloc(sizeof(_ct)*numColors);
  for (x = 0; x < numColors; x++) {
    colorBlock[x].r = allColors[x*3];
    colorBlock[x].g = allColors[x*3+1];
    colorBlock[x].b = allColors[x*3+2];
    colorBlock[x].visited = 0;
    colorBlock[x].next = 0;
  }
  free(allColors);

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
        if (thisdist < 5) { colorBlock[x].visited = 1; numColors--; }
        else if (thisdist < bestdist) { bestdist = thisdist; newnext = x; }
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
  free(colorBlock);
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
      EON_MatrixRotate(m,1,x);
      EON_MatrixRotate(m2,2,y);
      EON_MatrixMultiply(m,m2);
      EON_MatrixRotate(m2,3,z);
      EON_MatrixMultiply(m,m2);
      EON_MatrixApply(m,0.0,0.0,-1.0,&light->Xp, &light->Yp, &light->Zp);
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
  return calloc(1, sizeof(EON_Light));
}

void EON_LightDelete(EON_Light *l)
{
  free(l);
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
    free(c);
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
}

EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov,
                    EON_uChar *fb, EON_ZBuffer *zb)
{
  EON_Cam *c = calloc(1, sizeof(EON_Cam));
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
    c->Sort = (zb != NULL);
  }
  return c;
}

// clip.c
//

#define NUM_CLIP_PLANES 5

typedef struct
{
  EON_Vertex newVertices[8];
  double Shades[8];
  double MappingU[8];
  double MappingV[8];
  double eMappingU[8];
  double eMappingV[8];
} _clipInfo;


static _clipInfo m_cl[2];


static double m_clipPlanes[NUM_CLIP_PLANES][4];
static EON_Cam *m_cam;
static EON_sInt32 m_cx, m_cy;
static double m_fov;
static double m_adj_asp;

static void _FindNormal(double x2, double x3,
                        double y2, double y3,
                        double zv,
                        double *res);

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt _ClipToPlane(EON_uInt numVerts, double *plane);

void EON_ClipSetFrustum(EON_Cam *cam)
{
  m_adj_asp = 1.0 / cam->AspectRatio;
  m_fov = EON_Min(EON_Max(cam->Fov,1.0),179.0);
  m_fov = (1.0/tan(m_fov*(EON_PI/360.0)))*(double) (cam->ClipRight-cam->ClipLeft);
  m_cx = cam->CenterX<<20;
  m_cy = cam->CenterY<<20;
  m_cam = cam;
  memset(m_clipPlanes,0,sizeof(m_clipPlanes));

  /* Back */
  m_clipPlanes[0][2] = -1.0;
  m_clipPlanes[0][3] = -cam->ClipBack;

  /* Left */
  m_clipPlanes[1][3] = 0.00000001;
  if (cam->ClipLeft == cam->CenterX) {
    m_clipPlanes[1][0] = 1.0;
  }
  else _FindNormal(-100,-100,
                100, -100,
                m_fov*-100.0/(cam->ClipLeft-cam->CenterX),
                m_clipPlanes[1]);
  if (cam->ClipLeft > cam->CenterX) {
    m_clipPlanes[1][0] = -m_clipPlanes[1][0];
    m_clipPlanes[1][1] = -m_clipPlanes[1][1];
    m_clipPlanes[1][2] = -m_clipPlanes[1][2];
  }

  /* Right */
  m_clipPlanes[2][3] = 0.00000001;
  if (cam->ClipRight == cam->CenterX) {
    m_clipPlanes[2][0] = -1.0;
  }
  else _FindNormal(100,100,
                -100, 100,
                m_fov*100.0/(cam->ClipRight-cam->CenterX),
                m_clipPlanes[2]);
  if (cam->ClipRight < cam->CenterX) {
    m_clipPlanes[2][0] = -m_clipPlanes[2][0];
    m_clipPlanes[2][1] = -m_clipPlanes[2][1];
    m_clipPlanes[2][2] = -m_clipPlanes[2][2];
  }
  /* Top */
  m_clipPlanes[3][3] = 0.00000001;
  if (cam->ClipTop == cam->CenterY) {
    m_clipPlanes[3][1] = -1.0;
  } else _FindNormal(100, -100,
                100, 100,
                m_fov*m_adj_asp*100.0/(cam->CenterY-cam->ClipTop),
                m_clipPlanes[3]);
  if (cam->ClipTop > cam->CenterY) {
    m_clipPlanes[3][0] = -m_clipPlanes[3][0];
    m_clipPlanes[3][1] = -m_clipPlanes[3][1];
    m_clipPlanes[3][2] = -m_clipPlanes[3][2];
  }

  /* Bottom */
  m_clipPlanes[4][3] = 0.00000001;
  if (cam->ClipBottom == cam->CenterY) {
    m_clipPlanes[4][1] = 1.0;
  } else _FindNormal(-100, 100,
                -100, -100,
                m_fov*m_adj_asp*-100.0/(cam->CenterY-cam->ClipBottom),
                m_clipPlanes[4]);
  if (cam->ClipBottom < cam->CenterY) {
    m_clipPlanes[4][0] = -m_clipPlanes[4][0];
    m_clipPlanes[4][1] = -m_clipPlanes[4][1];
    m_clipPlanes[4][2] = -m_clipPlanes[4][2];
  }
}


void EON_ClipRenderFace(EON_Face *face)
{
  EON_uInt k, a, w, numVerts = 3;
  double tmp, tmp2;
  EON_Face newface;

  for (a = 0; a < 3; a ++) {
    m_cl[0].newVertices[a] = *(face->Vertices[a]);
    m_cl[0].Shades[a] = face->Shades[a];
    m_cl[0].MappingU[a] = face->MappingU[a];
    m_cl[0].MappingV[a] = face->MappingV[a];
    m_cl[0].eMappingU[a] = face->eMappingU[a];
    m_cl[0].eMappingV[a] = face->eMappingV[a];
  }

  a = (m_clipPlanes[0][3] < 0.0 ? 0 : 1);
  while (a < NUM_CLIP_PLANES && numVerts > 2)
  {
    numVerts = _ClipToPlane(numVerts, m_clipPlanes[a]);
    memcpy(&m_cl[0],&m_cl[1],sizeof(m_cl)/2);
    a++;
  }
  if (numVerts > 2) {
    memcpy(&newface,face,sizeof(EON_Face));
    for (k = 2; k < numVerts; k ++) {
      newface.fShade = EON_Max(0,EON_Min(face->fShade,1));
      for (a = 0; a < 3; a ++) {
        if (a == 0) w = 0;
        else w = a+(k-2);
        newface.Vertices[a] = m_cl[0].newVertices+w;
        newface.Shades[a] = (EON_Float) m_cl[0].Shades[w];
        newface.MappingU[a] = (EON_sInt32)m_cl[0].MappingU[w];
        newface.MappingV[a] = (EON_sInt32)m_cl[0].MappingV[w];
        newface.eMappingU[a] = (EON_sInt32)m_cl[0].eMappingU[w];
        newface.eMappingV[a] = (EON_sInt32)m_cl[0].eMappingV[w];
        newface.Scrz[a] = 1.0f/newface.Vertices[a]->xformedz;
        tmp2 = m_fov * newface.Scrz[a];
        tmp = tmp2*newface.Vertices[a]->xformedx;
        tmp2 *= newface.Vertices[a]->xformedy;
        newface.Scrx[a] = m_cx + ((EON_sInt32)((tmp*(float) (1<<20))));
        newface.Scry[a] = m_cy - ((EON_sInt32)((tmp2*m_adj_asp*(float) (1<<20))));
      }
      newface.Material->_PutFace(m_cam,&newface);
      EON_Render_TriStats[3] ++;
    }
    EON_Render_TriStats[2] ++;
  }
}

EON_sInt EON_ClipNeeded(EON_Face *face)
{
  double dr,dl,db,dt;
  double f;
  dr = (m_cam->ClipRight-m_cam->CenterX);
  dl = (m_cam->ClipLeft-m_cam->CenterX);
  db = (m_cam->ClipBottom-m_cam->CenterY);
  dt = (m_cam->ClipTop-m_cam->CenterY);
  f = m_fov*m_adj_asp;
  return ((m_cam->ClipBack <= 0.0 ||
           face->Vertices[0]->xformedz <= m_cam->ClipBack ||
           face->Vertices[1]->xformedz <= m_cam->ClipBack ||
           face->Vertices[2]->xformedz <= m_cam->ClipBack) &&
          (face->Vertices[0]->xformedz >= 0 ||
           face->Vertices[1]->xformedz >= 0 ||
           face->Vertices[2]->xformedz >= 0) &&
          (face->Vertices[0]->xformedx*m_fov<=dr*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*m_fov<=dr*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*m_fov<=dr*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedx*m_fov>=dl*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedx*m_fov>=dl*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedx*m_fov>=dl*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f<=db*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f<=db*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f<=db*face->Vertices[2]->xformedz) &&
          (face->Vertices[0]->xformedy*f>=dt*face->Vertices[0]->xformedz ||
           face->Vertices[1]->xformedy*f>=dt*face->Vertices[1]->xformedz ||
           face->Vertices[2]->xformedy*f>=dt*face->Vertices[2]->xformedz));
}



static void _FindNormal(double x2, double x3,double y2, double y3,
                        double zv, double *res)
{
  res[0] = zv*(y2-y3);
  res[1] = zv*(x3-x2);
  res[2] = x2*y3 - y2*x3;
}

 /* Returns: 0 if nothing gets in,  1 or 2 if pout1 & pout2 get in */
static EON_uInt _ClipToPlane(EON_uInt numVerts, double *plane)
{
  EON_uInt i, nextvert, curin, nextin;
  double curdot, nextdot, scale;
  EON_uInt invert, outvert;
  invert = 0;
  outvert = 0;
  curdot = m_cl[0].newVertices[0].xformedx*plane[0] +
           m_cl[0].newVertices[0].xformedy*plane[1] +
           m_cl[0].newVertices[0].xformedz*plane[2];
  curin = (curdot >= plane[3]);

  for (i=0 ; i < numVerts; i++) {
    nextvert = (i + 1) % numVerts;
    if (curin) {
      m_cl[1].Shades[outvert] = m_cl[0].Shades[invert];
      m_cl[1].MappingU[outvert] = m_cl[0].MappingU[invert];
      m_cl[1].MappingV[outvert] = m_cl[0].MappingV[invert];
      m_cl[1].eMappingU[outvert] = m_cl[0].eMappingU[invert];
      m_cl[1].eMappingV[outvert] = m_cl[0].eMappingV[invert];
      m_cl[1].newVertices[outvert++] = m_cl[0].newVertices[invert];
    }
    nextdot = m_cl[0].newVertices[nextvert].xformedx*plane[0] +
              m_cl[0].newVertices[nextvert].xformedy*plane[1] +
              m_cl[0].newVertices[nextvert].xformedz*plane[2];
    nextin = (nextdot >= plane[3]);
    if (curin != nextin) {
      scale = (plane[3] - curdot) / (nextdot - curdot);
      m_cl[1].newVertices[outvert].xformedx = (EON_Float) (m_cl[0].newVertices[invert].xformedx +
           (m_cl[0].newVertices[nextvert].xformedx - m_cl[0].newVertices[invert].xformedx)
             * scale);
      m_cl[1].newVertices[outvert].xformedy = (EON_Float) (m_cl[0].newVertices[invert].xformedy +
           (m_cl[0].newVertices[nextvert].xformedy - m_cl[0].newVertices[invert].xformedy)
             * scale);
      m_cl[1].newVertices[outvert].xformedz = (EON_Float) (m_cl[0].newVertices[invert].xformedz +
           (m_cl[0].newVertices[nextvert].xformedz - m_cl[0].newVertices[invert].xformedz)
             * scale);
      m_cl[1].Shades[outvert] = m_cl[0].Shades[invert] +
                        (m_cl[0].Shades[nextvert] - m_cl[0].Shades[invert]) * scale;
      m_cl[1].MappingU[outvert] = m_cl[0].MappingU[invert] +
           (m_cl[0].MappingU[nextvert] - m_cl[0].MappingU[invert]) * scale;
      m_cl[1].MappingV[outvert] = m_cl[0].MappingV[invert] +
           (m_cl[0].MappingV[nextvert] - m_cl[0].MappingV[invert]) * scale;
      m_cl[1].eMappingU[outvert] = m_cl[0].eMappingU[invert] +
           (m_cl[0].eMappingU[nextvert] - m_cl[0].eMappingU[invert]) * scale;
      m_cl[1].eMappingV[outvert] = m_cl[0].eMappingV[invert] +
           (m_cl[0].eMappingV[nextvert] - m_cl[0].eMappingV[invert]) * scale;
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
  while (i_1 < 0) i_1 += s->numKeys;
  i1 = i0 + 1;
  while (i1 >= s->numKeys) i1 -= s->numKeys;
  i2 = i0 + 2;
  while (i2 >= s->numKeys) i2 -= s->numKeys;
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

EON_uChar EON_Text_DefaultFont[256*16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 60, 66, 129, 231, 165, 153, 129, 153, 66, 60, 0, 0, 0, 0,
  0, 0, 60, 126, 255, 153, 219, 231, 255, 231, 126, 60, 0, 0, 0, 0,
  0, 0, 0, 0, 102, 255, 255, 255, 255, 126, 60, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 60, 126, 255, 126, 60, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 24, 60, 60, 90, 255, 255, 90, 24, 60, 0, 0, 0, 0,
  0, 0, 0, 24, 60, 126, 255, 255, 255, 90, 24, 60, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 24, 60, 60, 24, 0, 0, 0, 0, 0, 0,
  255,255,255, 255, 255, 255, 231, 195, 195, 231, 255, 255, 255, 255, 255, 255,
  0, 0, 0, 0, 0, 60, 102, 66, 66, 102, 60, 0, 0, 0, 0, 0,
  255,255,255, 255, 255, 195, 153, 189, 189, 153, 195, 255, 255, 255, 255, 255,
  0, 0, 15, 7, 13, 24, 62, 99, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 126, 195, 195, 195, 126, 24, 24, 30, 120, 24, 0, 0, 0, 0,
  0, 0, 8, 12, 14, 11, 8, 8, 8, 120, 248, 112, 0, 0, 0, 0,
  0, 16, 24, 28, 22, 26, 22, 18, 114, 242, 98, 14, 30, 12, 0, 0,
  0, 0, 24, 24, 219, 126, 60, 255, 60, 126, 219, 24, 24, 0, 0, 0,
  0, 0, 96, 112, 120, 124, 126, 126, 124, 120, 112, 96, 0, 0, 0, 0,
  0, 0, 6, 14, 30, 62, 126, 126, 62, 30, 14, 6, 0, 0, 0, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 56, 254, 124, 56, 16, 0, 0,
  0, 0, 102, 102, 102, 102, 102, 102, 102, 0, 102, 102, 0, 0, 0, 0,
  0, 0, 63, 123, 219, 219, 219, 127, 59, 27, 27, 27, 0, 0, 0, 0,
  0, 31, 48, 120, 220, 206, 231, 115, 59, 30, 12, 24, 48, 224, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 126, 126, 126, 126, 0, 0, 0, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 254, 124, 56, 16, 0, 254, 0,
  0, 0, 16, 56, 124, 254, 56, 56, 56, 56, 56, 56, 56, 56, 0, 0,
  0, 0, 56, 56, 56, 56, 56, 56, 56, 56, 254, 124, 56, 16, 0, 0,
  0, 0, 0, 0, 0, 8, 12, 254, 255, 254, 12, 8, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 16, 48, 127, 255, 127, 48, 16, 0, 0, 0, 0,
  0, 0, 204, 102, 51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 36, 102, 255, 255, 102, 36, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 60, 60, 126, 126, 255, 255, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 255, 255, 126, 126, 60, 60, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 24, 24, 24, 24, 24, 24, 24, 0, 24, 24, 0, 0, 0, 0,
  0, 0, 51, 102, 204, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 51, 51, 255, 102, 102, 102, 102, 255, 204, 204, 0, 0, 0, 0,
  0, 24, 126, 219, 216, 120, 28, 30, 27, 219, 219, 126, 24, 0, 0, 0,
  0, 0, 96, 209, 179, 102, 12, 24, 54, 109, 203, 6, 0, 0, 0, 0,
  0, 0, 28, 54, 102, 60, 56, 108, 199, 198, 110, 59, 0, 0, 0, 0,
  0, 0, 12, 24, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 12, 24, 48, 48, 96, 96, 96, 96, 48, 48, 24, 12, 0, 0, 0,
  0, 48, 24, 12, 12, 6, 6, 6, 6, 12, 12, 24, 48, 0, 0, 0,
  0, 0, 102, 60, 255, 60, 102, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 24, 48, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0,
  0, 0, 6, 4, 12, 8, 24, 16, 48, 32, 96, 64, 192, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 207, 219, 243, 198, 124, 0, 0, 0, 0,
  0, 0, 12, 28, 60, 108, 12, 12, 12, 12, 12, 12, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 3, 6, 12, 24, 48, 99, 255, 0, 0, 0, 0,
  0, 0, 255, 198, 12, 24, 62, 3, 3, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 6, 14, 30, 54, 102, 199, 222, 246, 6, 6, 0, 0, 0, 0,
  0, 0, 31, 240, 192, 220, 246, 3, 3, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 192, 220, 246, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 255, 195, 6, 12, 12, 24, 24, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 108, 62, 99, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 60, 102, 198, 198, 222, 118, 6, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 0, 0, 24, 24, 48, 0, 0, 0, 0,
  0, 0, 6, 12, 24, 48, 96, 96, 48, 24, 12, 6, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 96, 48, 24, 12, 6, 6, 12, 24, 48, 96, 0, 0, 0, 0,
  0, 0, 62, 99, 198, 12, 24, 48, 48, 0, 48, 48, 0, 0, 0, 0,
  0, 0, 30, 51, 111, 219, 219, 219, 222, 216, 198, 220, 112, 0, 0, 0,
  0, 0, 24, 24, 60, 36, 102, 110, 122, 227, 195, 195, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 198, 220, 247, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 30, 51, 96, 192, 192, 192, 192, 192, 99, 62, 0, 0, 0, 0,
  0, 0, 252, 198, 195, 195, 195, 195, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 30, 240, 192, 192, 220, 240, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 30, 240, 192, 192, 220, 240, 192, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 62, 99, 192, 192, 192, 207, 195, 195, 102, 60, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 207, 251, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 28, 56, 24, 24, 24, 24, 24, 24, 28, 56, 0, 0, 0, 0,
  0, 0, 3, 3, 3, 3, 3, 3, 195, 195, 99, 62, 0, 0, 0, 0,
  0, 0, 195, 198, 220, 240, 224, 240, 216, 204, 198, 195, 0, 0, 0, 0,
  0, 0, 192, 192, 192, 192, 192, 192, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 195, 195, 231, 239, 251, 211, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 227, 243, 211, 219, 207, 199, 195, 195, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 220, 240, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 195, 243, 222, 204, 124, 6, 3, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 252, 216, 204, 198, 195, 0, 0, 0, 0,
  0, 0, 126, 195, 192, 112, 28, 6, 3, 195, 195, 126, 0, 0, 0, 0,
  0, 0, 15, 248, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 195, 195, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 102, 102, 124, 56, 48, 48, 0, 0, 0, 0,
  0, 0, 195, 195, 195, 195, 219, 219, 219, 255, 231, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 60, 102, 102, 195, 195, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 31, 246, 4, 12, 24, 16, 48, 32, 111, 248, 0, 0, 0, 0,
  0, 62, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 62, 0, 0, 0,
  0, 0, 192, 64, 96, 32, 48, 16, 24, 8, 12, 4, 6, 0, 0, 0,
  0, 124, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 124, 0, 0, 0,
  0, 24, 60, 102, 192, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0,
  0, 0, 48, 24, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 192, 192, 192, 220, 246, 195, 195, 198, 220, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 30, 51, 96, 192, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 3, 3, 3, 31, 115, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 30, 51, 48, 48, 60, 240, 48, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 63, 99, 195, 199, 207, 219, 115, 3, 195, 126, 0,
  0, 0, 192, 192, 192, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 12, 12, 0, 12, 12, 12, 12, 12, 12, 12, 204, 204, 120, 0,
  0, 0, 192, 192, 192, 198, 204, 216, 248, 236, 198, 195, 0, 0, 0, 0,
  0, 0, 56, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 230, 219, 219, 219, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 227, 195, 198, 252, 192, 192, 192, 0,
  0, 0, 0, 0, 0, 115, 219, 207, 199, 195, 99, 63, 3, 3, 3, 0,
  0, 0, 0, 0, 0, 206, 219, 243, 224, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 126, 195, 112, 30, 3, 195, 126, 0, 0, 0, 0,
  0, 0, 16, 48, 48, 60, 240, 48, 48, 54, 60, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 102, 108, 56, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 219, 219, 255, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 102, 60, 24, 60, 102, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 195, 195, 195, 195, 195, 102, 62, 12, 216, 112, 0,
  0, 0, 0, 0, 0, 255, 6, 12, 24, 48, 96, 255, 0, 0, 0, 0,
  0, 14, 24, 24, 24, 24, 112, 112, 24, 24, 24, 24, 14, 0, 0, 0,
  0, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0,
  0, 112, 24, 24, 24, 24, 14, 14, 24, 24, 24, 24, 112, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 118, 220, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 16, 56, 108, 198, 198, 198, 254, 0, 0, 0, 0, 0,
  0, 0, 30, 51, 96, 192, 192, 192, 192, 192, 99, 62, 12, 24, 240, 0,
  0, 0, 102, 102, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  12, 30, 51, 96, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 54, 54, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 48, 24, 12, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  28, 54, 54, 28, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 30, 51, 96, 192, 192, 195, 126, 12, 24, 240, 0,
  24, 60, 102, 192, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 62, 99, 206, 248, 192, 195, 126, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  102, 102, 24, 24, 60, 36, 102, 102, 126, 195, 195, 195, 0, 0, 0, 0,
  24, 36, 36, 24, 60, 36, 102, 102, 126, 195, 195, 195, 0, 0, 0, 0,
  24, 48, 96, 30, 240, 192, 222, 240, 192, 192, 222, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 126, 219, 59, 126, 220, 219, 110, 0, 0, 0, 0,
  0, 0, 63, 62, 60, 108, 111, 110, 124, 204, 207, 206, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  24, 60, 102, 192, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 96, 48, 24, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 0, 102, 102, 0, 195, 195, 195, 195, 195, 102, 62, 12, 216, 112, 0,
  102, 0, 62, 99, 195, 195, 195, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  102, 0, 195, 195, 195, 195, 195, 195, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 8, 8, 30, 59, 104, 200, 200, 203, 126, 8, 8, 0, 0,
  0, 0, 60, 102, 96, 248, 96, 248, 96, 96, 99, 126, 0, 0, 0, 0,
  0, 0, 195, 195, 102, 102, 60, 24, 126, 24, 126, 24, 24, 0, 0, 0,
  0, 0, 30, 51, 227, 195, 198, 220, 248, 204, 222, 204, 15, 14, 4, 0,
  0, 0, 30, 51, 48, 48, 60, 240, 48, 48, 48, 48, 224, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 6, 12, 24, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 0, 0,
  0, 12, 24, 48, 0, 195, 195, 195, 199, 207, 219, 115, 0, 0, 0, 0,
  0, 3, 118, 220, 0, 206, 219, 243, 227, 195, 195, 195, 0, 0, 0, 0,
  3, 118, 220, 0, 195, 227, 243, 211, 219, 207, 199, 195, 0, 0, 0, 0,
  0, 0, 62, 99, 31, 115, 195, 207, 123, 0, 0, 255, 0, 0, 0, 0,
  0, 0, 62, 99, 195, 195, 195, 198, 124, 0, 0, 255, 0, 0, 0, 0,
  0, 0, 12, 12, 0, 12, 12, 24, 48, 99, 198, 124, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 254, 192, 192, 192, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 254, 6, 6, 6, 0, 0, 0, 0, 0, 0,
  0, 96, 225, 99, 102, 108, 24, 48, 96, 206, 155, 6, 13, 31, 0, 0,
  0, 96, 225, 99, 102, 108, 24, 48, 102, 206, 151, 62, 6, 6, 0, 0,
  0, 0, 24, 24, 0, 24, 24, 24, 24, 24, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 0, 3, 54, 108, 216, 216, 108, 54, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 108, 54, 27, 27, 54, 108, 192, 0, 0, 0, 0,
  130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16, 130, 16,
  0, 149, 0, 169, 0, 149, 0, 169, 0, 149, 0, 169, 0, 149, 0, 169,
  146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73, 146, 73,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 248, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 248, 248, 248, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 60, 60, 252, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 0, 0, 252, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 248, 248, 248, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 252, 252, 252, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 252, 252, 252, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 252, 252, 252, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 60, 60, 252, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 248, 248, 248, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 248, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 31, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 24, 24, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 31, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 24, 24, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 31, 31, 31, 24, 24, 24, 24, 24, 24, 24,
  60, 60, 60, 60, 60, 60, 60, 60, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 63, 63, 63, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 63, 63, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 63, 63, 63, 60, 60, 60, 60, 60, 60, 60,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 255, 255, 255, 60, 60, 60, 60, 60, 60, 60,
  24, 24, 24, 24, 24, 24, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0,
  60, 60, 60, 60, 60, 60, 60, 60, 255, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 255, 255, 255, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 63, 0, 0, 0, 0, 0, 0, 0,
  24, 24, 24, 24, 24, 24, 31, 31, 31, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 31, 31, 31, 24, 24, 24, 24, 24, 24, 24,
  0, 0, 0, 0, 0, 0, 0, 0, 63, 60, 60, 60, 60, 60, 60, 60,
  60, 60, 60, 60, 60, 60, 60, 60, 255, 60, 60, 60, 60, 60, 60, 60,
  24, 24, 24, 24, 24, 24, 255, 255, 255, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 248, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 31, 24, 24, 24, 24, 24, 24, 24,
  255, 255, 255,255,255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
  0, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255,
  240, 240, 240, 240,240,240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
  255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 118, 204, 204, 204, 222, 115, 0, 0, 0, 0,
  0, 0, 30, 51, 227, 194, 204, 194, 195, 195, 206, 216, 192, 192, 0, 0,
  0, 0, 31, 243, 195, 192, 192, 192, 192, 192, 192, 192, 0, 0, 0, 0,
  0, 0, 0, 0, 3, 126, 230, 102, 102, 102, 102, 68, 0, 0, 0, 0,
  0, 0, 31, 240, 96, 48, 24, 48, 96, 192, 223, 240, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 127, 240, 216, 216, 216, 216, 112, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 99, 99, 99, 99, 103, 111, 123, 96, 96, 192, 0,
  0, 0, 0, 0, 111, 184, 48, 48, 48, 48, 48, 48, 0, 0, 0, 0,
  0, 0, 24, 24, 126, 219, 219, 219, 219, 126, 24, 24, 0, 0, 0, 0,
  0, 0, 0, 60, 102, 195, 195, 255, 195, 195, 102, 60, 0, 0, 0, 0,
  0, 0, 60, 102, 195, 195, 195, 195, 102, 36, 165, 231, 0, 0, 0, 0,
  0, 7, 28, 48, 24, 12, 62, 102, 198, 198, 204, 120, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 110, 219, 219, 219, 118, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 2, 4, 124, 206, 214, 230, 124, 64, 128, 0, 0, 0, 0,
  0, 0, 30, 48, 96, 96, 126, 96, 96, 96, 48, 30, 0, 0, 0, 0,
  0, 0, 0, 126, 195, 195, 195, 195, 195, 195, 195, 195, 0, 0, 0, 0,
  0, 0, 0, 0, 255, 0, 0, 255, 0, 0, 255, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 126, 24, 24, 0, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 48, 24, 12, 6, 12, 24, 48, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 12, 24, 48, 96, 48, 24, 12, 0, 126, 0, 0, 0, 0,
  0, 0, 0, 14, 27, 27, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 216, 216, 112, 0, 0, 0, 0,
  0, 0, 0, 0, 24, 24, 0, 255, 0, 24, 24, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 118, 220, 0, 118, 220, 0, 0, 0, 0, 0, 0,
  0, 0, 60, 102, 102, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 3, 2, 6, 4, 12, 8, 216, 80, 112, 32, 0, 0, 0, 0,
  0, 0, 220, 246, 230, 198, 198, 198, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 120, 204, 24, 48, 100, 252, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 126, 126, 126, 126, 126, 126, 126, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void EON_TexDelete(EON_Texture *t)
{
  if (t) {
    if (t->Data) free(t->Data);
    if (t->PaletteData) free(t->PaletteData);
    free(t);
  }
}

// text.c
//

static EON_uChar font_height = 16;

static EON_uChar *current_font = EON_Text_DefaultFont;

void EON_TextSetFont(EON_uChar *font, EON_uChar height)
{
  current_font = font;
  font_height = height;
}

void EON_TextPutChar(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                   EON_uChar color, EON_uChar c)
{
  EON_uChar *font = current_font + (c*font_height);
  EON_sInt offset = x+(y*cam->ScreenWidth);
  EON_ZBuffer zz = (EON_ZBuffer) (1.0/z);
  EON_sInt xx = x, a;
  EON_uChar len = font_height;
  EON_uChar ch;
  EON_uChar *outmem;
  EON_ZBuffer *zbuffer;
  if (y+font_height < cam->ClipTop || y >= cam->ClipBottom) return;
  if (y < cam->ClipTop) {
    font += (cam->ClipTop-y);
    offset += (cam->ClipTop-y)*cam->ScreenWidth;
    len -= (cam->ClipTop-y);
    y = cam->ClipTop;
  }
  if (y+font_height >= cam->ClipBottom) {
    len = cam->ClipBottom-y;
  }
  if (len > 0) {
    if (cam->zBuffer && z != 0.0) do {
      outmem = cam->frameBuffer + offset;
      zbuffer = cam->zBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft)
          if (ch & a)
            if (zz > *zbuffer) {
              *zbuffer = zz;
              *outmem = color;
            }
        zbuffer++;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
    else do {
      outmem = cam->frameBuffer + offset;
      offset += cam->ScreenWidth;
      xx = x;
      ch = *font++;
      a = 128;
      while (a) {
        if (xx >= cam->ClipRight) break;
        if (xx++ >= cam->ClipLeft) if (ch & a) *outmem = color;
        outmem++;
        a >>= 1;
      }
      if (a) break;
    } while (--len);
  }
}

void EON_TextPutStr(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                    EON_uChar color, EON_sChar *string)
{
  EON_sInt xx = x;
  while (*string) {
    switch (*string) {
      case '\n': y += font_height; xx = x; break;
      case ' ': xx += 8; break;
      case '\r': break;
      case '\t': xx += 8*5; break;
      default:
        EON_TextPutChar(cam,xx,y,z,color,(EON_uChar) *string);
        xx += 8;
      break;
    }
    string++;
  }
}

void EON_TextPrintf(EON_Cam *cam, EON_sInt x, EON_sInt y, EON_Float z,
                    EON_uChar color, EON_sChar *format, ...)
{
  va_list arglist;
  EON_sChar str[256];
  va_start(arglist, format);
  vsprintf((char *)str, (char *) format,arglist);
  va_end(arglist);
  EON_TextPutStr(cam,x,y,z,color,str);
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

// pf_solid.c
//

void EON_PF_SolidF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;

  EON_uChar *gmem = cam->frameBuffer;
  EON_ZBuffer *zbuf = cam->zBuffer;

  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_ZBuffer dZL=0, dZ1=0, dZ2=0, Z1, ZL, Z2, Z3;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uChar stat;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;
  EON_uChar bc;
  EON_sInt32 shade;

  PUTFACE_SORT();

  shade=(EON_sInt32) (TriFace->fShade*(TriFace->Material->_ColorsUsed-1));
  if (shade < 0) shade=0;
  if (shade > (EON_sInt32) TriFace->Material->_ColorsUsed-1) shade=TriFace->Material->_ColorsUsed-1;
  bc=TriFace->Material->_ReMapTable[shade];

  X2 = X1 = TriFace->Scrx[i0];
  Z1 = TriFace->Scrz[i0];
  Z2 = TriFace->Scrz[i1];
  Z3 = TriFace->Scrz[i2];
  Y0 = (TriFace->Scry[i0]+(1<<19)) >> 20;
  Y1 = (TriFace->Scry[i1]+(1<<19)) >> 20;
  Y2 = (TriFace->Scry[i2]+(1<<19)) >> 20;

  dY = Y2-Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (Z3 - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (Z2 - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
    Z2 = Z1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      ZL = Z1; Z1 = Z2; Z2 = ZL;
      stat = 1|8;
    }
  }

  if (zb) {
    XL1 = ((dX1-dX2)*dY+(1<<19))>>20;
    if (XL1) dZL = ((dZ1-dZ2)*dY)/XL1;
    else {
      XL1 = (X2-X1+(1<<19))>>20;
      if (zb && XL1) dZL = (Z2-Z1)/XL1;
      else dZL = 0.0;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);

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
        dZ1 = (Z3-Z1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    XL2 -= XL1;
    if (XL2 > 0) {
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = bc;
          }
          gmem++;
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      else do *gmem++ = bc; while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    Y0++;
  }
}

void EON_PF_SolidG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
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
      gmem += XL1;
      zbuf += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            if (CL >= maxColor) *gmem=remap[maxColorNonShift];
            else if (CL > 0) *gmem = remap[CL>>16];
            else *gmem = remap[0];
          }
          gmem++;
          zbuf++;
          ZL += dZL;
          CL += dCL;
        } while (--XL2);
      else do {
          if (CL >= maxColor) *gmem++=remap[maxColorNonShift];
          else if (CL > 0) *gmem++ = remap[CL>>16];
          else *gmem++ = remap[0];
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

void EON_PF_TexEnv(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
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
            *gmem = remap[addtable[environment[
                ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>vshift)&MappingV_AND)]];
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
          *gmem++ = remap[addtable[environment[
              ((eUL>>16)&eMappingU_AND)+((eVL>>evshift)&eMappingV_AND)]] +
                          texture[((UL>>16)&MappingU_AND) +
                                  ((VL>>vshift)&MappingV_AND)]];
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

void EON_PF_TexF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
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
            *gmem = remap[bc + texture[((UL >> 16)&MappingU_AND) +
                                ((VL>>vshift)&MappingV_AND)]];
          }
          zbuf++;
          gmem++;
          ZL += dZL;
          UL += dUL;
          VL += dVL;
        } while (--XL2);
      else do {
          *gmem++ = remap[bc + texture[((UL >> 16)&MappingU_AND) +
                                ((VL>>vshift)&MappingV_AND)]];
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

void EON_PF_TexG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
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
            *gmem = remap[av +
                            texture[((UL>>16)&MappingU_AND) +
                                    ((VL>>vshift)&MappingV_AND)]];
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
          *gmem++ = remap[av +
                          texture[((UL>>16)&MappingU_AND) +
                                  ((VL>>vshift)&MappingV_AND)]];
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

void EON_PF_PTexF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
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
              *gmem = remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]];
            }
            zbuf++;
            gmem++;
            ZL += dZL;
            iUL += idUL;
            iVL += idVL;
          } while (--n);
        else do {
            *gmem++ = remap[bc + texture[((iUL>>16)&MappingU_AND) +
                                   ((iVL>>vshift)&MappingV_AND)]];
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

void EON_PF_PTexG(EON_Cam *cam, EON_Face *TriFace)
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
  EON_uChar *gmem = cam->frameBuffer;
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
              *gmem = remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]];
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
            *gmem++ = remap[av +
                      texture[((iUL>>16)&MappingU_AND) +
                              ((iVL>>vshift)&MappingV_AND)]];
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


// pf_trans.c
//

void EON_PF_TransF(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
  EON_uChar *remap = TriFace->Material->_ReMapTable;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_ZBuffer Z1, ZL, dZ1=0, dZL=0, dZ2=0, Z2;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_uInt16 *lookuptable = TriFace->Material->_AddTable;
  EON_uChar stat;
  EON_sInt32 bc = (EON_sInt32) TriFace->fShade*TriFace->Material->_tsfact;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;

  PUTFACE_SORT();

  if (bc < 0) bc=0;
  if (bc > (EON_sInt32) TriFace->Material->_tsfact-1) bc=TriFace->Material->_tsfact-1;
  remap+=bc;

  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      stat= 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      stat= 1|8;
    }
  }

  gmem += (Y0 * cam->ScreenWidth);
  zbuf += (Y0 * cam->ScreenWidth);
  if (zb) {
    XL1 = (((dX1-dX2)*dY+(1<<19))>>20);
    if (XL1) dZL = ((dZ1-dZ2)*dY)/XL1;
    else {
      XL1 = ((X2-X1+(1<<19))>>20);
      if (XL1) dZL = (Z2-Z1)/XL1;
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
        dZ1 = (TriFace->Scrz[i2]- Z1)/dY;
      }
    }
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) {
        do {
          if (*zbuf < ZL) {
            *zbuf = ZL;
            *gmem = remap[lookuptable[*gmem]];
          }
          gmem++;
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      } else do {
          EON_uChar value = remap[lookuptable[*gmem]];
          *gmem++ = value;
      } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    Y0 ++;
  }
}

void EON_PF_TransG(EON_Cam *cam, EON_Face *TriFace)
{
  EON_uChar i0, i1, i2;
  EON_uChar *gmem = cam->frameBuffer;
  EON_uChar *remap = TriFace->Material->_ReMapTable;
  EON_ZBuffer *zbuf = cam->zBuffer;
  EON_sInt32 X1, X2, dX1=0, dX2=0, XL1, XL2;
  EON_ZBuffer Z1, ZL, dZ1=0, dZL=0, dZ2=0, Z2;
  EON_sInt32 dC1=0, dCL=0, CL, C1, C2, dC2=0;
  EON_sInt32 Y1, Y2, Y0, dY;
  EON_Float nc = (TriFace->Material->_tsfact*65536.0f);
  EON_uInt16 *lookuptable = TriFace->Material->_AddTable;
  EON_Bool zb = (zbuf&&TriFace->Material->zBufferable) ? 1 : 0;
  EON_uChar stat;

  EON_sInt32 maxColor=((TriFace->Material->_tsfact-1)<<16);
  EON_sInt32 maxColorNonShift=TriFace->Material->_tsfact-1;

  PUTFACE_SORT();

  C1 = C2 = (EON_sInt32) (TriFace->Shades[i0]*nc);
  X2 = X1 = TriFace->Scrx[i0];
  Z2 = Z1 = TriFace->Scrz[i0];
  Y0 = (TriFace->Scry[i0]+(1<<19))>>20;
  Y1 = (TriFace->Scry[i1]+(1<<19))>>20;
  Y2 = (TriFace->Scry[i2]+(1<<19))>>20;

  dY = Y2 - Y0;
  if (dY) {
    dX2 = (TriFace->Scrx[i2] - X1) / dY;
    dC2 = (EON_sInt32) ((TriFace->Shades[i2]*nc - C1) / dY);
    dZ2 = (TriFace->Scrz[i2] - Z1) / dY;
  }
  dY = Y1-Y0;
  if (dY) {
    dX1 = (TriFace->Scrx[i1] - X1) / dY;
    dZ1 = (TriFace->Scrz[i1] - Z1) / dY;
    dC1 = (EON_sInt32) ((TriFace->Shades[i1]*nc - C1) / dY);
    if (dX2 < dX1) {
      dX2 ^= dX1; dX1 ^= dX2; dX2 ^= dX1;
      dC2 ^= dC1; dC1 ^= dC2; dC2 ^= dC1;
      dZL = dZ1; dZ1 = dZ2; dZ2 = dZL;
      stat = 2;
    } else stat = 1;
  } else {
    if (TriFace->Scrx[i1] > X1) {
      X2 = TriFace->Scrx[i1];
      Z2 = TriFace->Scrz[i1];
      C2 = (EON_sInt32) (TriFace->Shades[i1]*nc);
      stat = 2|4;
    } else {
      X1 = TriFace->Scrx[i1];
      Z1 = TriFace->Scrz[i1];
      C1 = (EON_sInt32) (TriFace->Shades[i1]*nc);
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
        dC1 = (EON_sInt32) ((TriFace->Shades[i2]*nc - C1) / dY);
      }
    }
    CL = C1;
    XL1 = (X1+(1<<19))>>20;
    XL2 = (X2+(1<<19))>>20;
    ZL = Z1;
    if ((XL2-XL1) > 0) {
      XL2 -= XL1;
      zbuf += XL1;
      gmem += XL1;
      XL1 += XL2;
      if (zb) do {
          if (*zbuf < ZL) {
            EON_sInt av;
            if (CL >= maxColor) av=maxColorNonShift;
            else if (CL > 0) av=CL>>16;
            else av=0;
            *zbuf = ZL;
            *gmem = remap[av + lookuptable[*gmem]];
          }
          gmem++;
          CL += dCL;
          zbuf++;
          ZL += dZL;
        } while (--XL2);
      else do {
          EON_sInt av;
          EON_uChar value;
          if (CL >= maxColor) av=maxColorNonShift;
          else if (CL > 0) av=CL>>16;
          else av=0;
          value = remap[av + lookuptable[*gmem]];
          *gmem++ = value;
          CL += dCL;
        } while (--XL2);
      gmem -= XL1;
      zbuf -= XL1;
    }
    gmem += cam->ScreenWidth;
    zbuf += cam->ScreenWidth;
    Z1 += dZ1;
    X1 += dX1;
    X2 += dX2;
    C1 += dC1;
    Y0++;
  }
}

// render.c
//

typedef struct {
  EON_Float zd;
  EON_Face *face;
} _faceInfo;

typedef struct {
  EON_Light *light;
  EON_Float l[3];
} _lightInfo;

#define MACRO_eon_MatrixApply(m,x,y,z,outx,outy,outz) \
      ( outx ) = ( x )*( m )[0] + ( y )*( m )[1] + ( z )*( m )[2] + ( m )[3];\
      ( outy ) = ( x )*( m )[4] + ( y )*( m )[5] + ( z )*( m )[6] + ( m )[7];\
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

EON_uInt32 EON_Render_TriStats[4];

static EON_uInt32 _numfaces;
static _faceInfo _faces[EON_MAX_TRIANGLES];

static EON_Float _cMatrix[16];
static EON_uInt32 _numlights;
static _lightInfo _lights[EON_MAX_LIGHTS];
static EON_Cam *_cam;
static void _RenderObj(EON_Obj *, EON_Float *, EON_Float *);
static void _sift_down(int L, int U, int dir);
static void _hsort(_faceInfo *base, int nel, int dir);

void EON_RenderBegin(EON_Cam *Camera)
{
  EON_Float tempMatrix[16];
  memset(EON_Render_TriStats,0,sizeof(EON_Render_TriStats));
  _cam = Camera;
  _numlights = 0;
  _numfaces = 0;
  EON_MatrixRotate(_cMatrix,2,-Camera->Pan);
  EON_MatrixRotate(tempMatrix,1,-Camera->Pitch);
  EON_MatrixMultiply(_cMatrix,tempMatrix);
  EON_MatrixRotate(tempMatrix,3,-Camera->Roll);
  EON_MatrixMultiply(_cMatrix,tempMatrix);
  EON_ClipSetFrustum(_cam);
}

void EON_RenderLight(EON_Light *light)
{
  EON_Float *pl, xp, yp, zp;
  if (light->Type == EON_LIGHT_NONE || _numlights >= EON_MAX_LIGHTS) return;
  pl = _lights[_numlights].l;
  if (light->Type == EON_LIGHT_VECTOR) {
    xp = light->Xp;
    yp = light->Yp;
    zp = light->Zp;
    MACRO_eon_MatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  } else if (light->Type & EON_LIGHT_POINT) {
    xp = light->Xp-_cam->X;
    yp = light->Yp-_cam->Y;
    zp = light->Zp-_cam->Z;
    MACRO_eon_MatrixApply(_cMatrix,xp,yp,zp,pl[0],pl[1],pl[2]);
  }
  _lights[_numlights++].light = light;
}

static void _RenderObj(EON_Obj *obj, EON_Float *bmatrix, EON_Float *bnmatrix)
{
  EON_uInt32 i, x, facepos;
  EON_Float nx = 0.0, ny = 0.0, nz = 0.0;
  double tmp, tmp2;
  EON_Float oMatrix[16], nMatrix[16], tempMatrix[16];

  EON_Vertex *vertex;
  EON_Face *face;
  EON_Light *light;

  if (obj->GenMatrix) {
    EON_MatrixRotate(nMatrix,1,obj->Xa);
    EON_MatrixRotate(tempMatrix,2,obj->Ya);
    EON_MatrixMultiply(nMatrix,tempMatrix);
    EON_MatrixRotate(tempMatrix,3,obj->Za);
    EON_MatrixMultiply(nMatrix,tempMatrix);
    memcpy(oMatrix,nMatrix,sizeof(EON_Float)*16);
  } else memcpy(nMatrix,obj->RotMatrix,sizeof(EON_Float)*16);

  if (bnmatrix) EON_MatrixMultiply(nMatrix,bnmatrix);

  if (obj->GenMatrix) {
    EON_MatrixTranslate(tempMatrix, obj->Xp, obj->Yp, obj->Zp);
    EON_MatrixMultiply(oMatrix,tempMatrix);
  } else memcpy(oMatrix,obj->Matrix,sizeof(EON_Float)*16);
  if (bmatrix) EON_MatrixMultiply(oMatrix,bmatrix);

  for (i = 0; i < EON_MAX_CHILDREN; i ++)
    if (obj->Children[i]) _RenderObj(obj->Children[i],oMatrix,nMatrix);
  if (!obj->NumFaces || !obj->NumVertices) return;

  EON_MatrixTranslate(tempMatrix, -_cam->X, -_cam->Y, -_cam->Z);
  EON_MatrixMultiply(oMatrix,tempMatrix);
  EON_MatrixMultiply(oMatrix,_cMatrix);
  EON_MatrixMultiply(nMatrix,_cMatrix);

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
  facepos = _numfaces;

  if (_numfaces + obj->NumFaces >= EON_MAX_TRIANGLES) // exceeded maximum face coutn
  {
    return;
  }

  EON_Render_TriStats[0] += obj->NumFaces;
  _numfaces += obj->NumFaces;
  x = obj->NumFaces;

  do {
    if (obj->BackfaceCull || face->Material->_st & EON_SHADE_FLAT)
    {
      MACRO_eon_MatrixApply(nMatrix,face->nx,face->ny,face->nz,nx,ny,nz);
    }
    if (!obj->BackfaceCull || (MACRO_eon_DotProduct(nx,ny,nz,
        face->Vertices[0]->xformedx, face->Vertices[0]->xformedy,
        face->Vertices[0]->xformedz) < 0.0000001)) {
      if (EON_ClipNeeded(face)) {
        if (face->Material->_st & (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE)) {
          tmp = face->sLighting;
          if (face->Material->_st & EON_SHADE_FLAT) {
            for (i = 0; i < _numlights; i ++) {
              tmp2 = 0.0;
              light = _lights[i].light;
              if (light->Type & EON_LIGHT_POINT_ANGLE) {
                double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
                double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
                double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
                MACRO_eon_NormalizeVector(nx2,ny2,nz2);
                tmp2 = MACRO_eon_DotProduct(nx,ny,nz,nx2,ny2,nz2)*light->Intensity;
              }
              if (light->Type & EON_LIGHT_POINT_DISTANCE) {
                double nx2 = _lights[i].l[0] - face->Vertices[0]->xformedx;
                double ny2 = _lights[i].l[1] - face->Vertices[0]->xformedy;
                double nz2 = _lights[i].l[2] - face->Vertices[0]->xformedz;
                if (light->Type & EON_LIGHT_POINT_ANGLE) {
                   nx2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/
                           light->HalfDistSquared));
                  tmp2 *= EON_Max(0,EON_Min(1.0,nx2))*light->Intensity;
                } else {
                  tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/
                    light->HalfDistSquared));
                  tmp2 = EON_Max(0,EON_Min(1.0,tmp2))*light->Intensity;
                }
              }
              if (light->Type == EON_LIGHT_VECTOR)
                tmp2 = MACRO_eon_DotProduct(nx,ny,nz,_lights[i].l[0],_lights[i].l[1],_lights[i].l[2])
                  * light->Intensity;
              if (tmp2 > 0.0) tmp += tmp2;
              else if (obj->BackfaceIllumination) tmp -= tmp2;
            } /* End of light loop */
          } /* End of flat shading if */
          if (face->Material->_st & EON_SHADE_FLAT_DISTANCE)
            tmp += 1.0-(face->Vertices[0]->xformedz+face->Vertices[1]->xformedz+
                        face->Vertices[2]->xformedz) /
                       (face->Material->FadeDist*3.0);
          face->fShade = (EON_Float) tmp;
        } else face->fShade = 0.0; /* End of flatmask lighting if */
        if (face->Material->_ft & EON_FILL_ENVIRONMENT) {
          face->eMappingU[0] = 32768 + (EON_sInt32) (face->Vertices[0]->xformednx*32768.0);
          face->eMappingV[0] = 32768 - (EON_sInt32) (face->Vertices[0]->xformedny*32768.0);
          face->eMappingU[1] = 32768 + (EON_sInt32) (face->Vertices[1]->xformednx*32768.0);
          face->eMappingV[1] = 32768 - (EON_sInt32) (face->Vertices[1]->xformedny*32768.0);
          face->eMappingU[2] = 32768 + (EON_sInt32) (face->Vertices[2]->xformednx*32768.0);
          face->eMappingV[2] = 32768 - (EON_sInt32) (face->Vertices[2]->xformedny*32768.0);
        }
        if (face->Material->_st &(EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE)) {
          register EON_uChar a;
          for (a = 0; a < 3; a ++) {
            tmp = face->vsLighting[a];
            if (face->Material->_st & EON_SHADE_GOURAUD) {
              for (i = 0; i < _numlights ; i++) {
                tmp2 = 0.0;
                light = _lights[i].light;
                if (light->Type & EON_LIGHT_POINT_ANGLE) {
                  nx = _lights[i].l[0] - face->Vertices[a]->xformedx;
                  ny = _lights[i].l[1] - face->Vertices[a]->xformedy;
                  nz = _lights[i].l[2] - face->Vertices[a]->xformedz;
                  MACRO_eon_NormalizeVector(nx,ny,nz);
                  tmp2 = MACRO_eon_DotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      nx,ny,nz) * light->Intensity;
                }
                if (light->Type & EON_LIGHT_POINT_DISTANCE) {
                  double nx2 = _lights[i].l[0] - face->Vertices[a]->xformedx;
                  double ny2 = _lights[i].l[1] - face->Vertices[a]->xformedy;
                  double nz2 = _lights[i].l[2] - face->Vertices[a]->xformedz;
                  if (light->Type & EON_LIGHT_POINT_ANGLE) {
                     double t= (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
                     tmp2 *= EON_Max(0,EON_Min(1.0,t))*light->Intensity;
                  } else {
                    tmp2 = (1.0 - 0.5*((nx2*nx2+ny2*ny2+nz2*nz2)/light->HalfDistSquared));
                    tmp2 = EON_Max(0,EON_Min(1.0,tmp2))*light->Intensity;
                  }
                }
                if (light->Type == EON_LIGHT_VECTOR)
                  tmp2 = MACRO_eon_DotProduct(face->Vertices[a]->xformednx,
                                      face->Vertices[a]->xformedny,
                                      face->Vertices[a]->xformednz,
                                      _lights[i].l[0],_lights[i].l[1],_lights[i].l[2])
                                        * light->Intensity;
                if (tmp2 > 0.0) tmp += tmp2;
                else if (obj->BackfaceIllumination) tmp -= tmp2;
              } /* End of light loop */
            } /* End of gouraud shading if */
            if (face->Material->_st & EON_SHADE_GOURAUD_DISTANCE)
              tmp += 1.0-face->Vertices[a]->xformedz/face->Material->FadeDist;
            face->Shades[a] = (EON_Float) tmp;
          } /* End of vertex loop for */
        } /* End of gouraud shading mask if */
        _faces[facepos].zd = face->Vertices[0]->xformedz+
        face->Vertices[1]->xformedz+face->Vertices[2]->xformedz;
        _faces[facepos++].face = face;
        EON_Render_TriStats[1] ++;
      } /* Is it in our area Check */
    } /* Backface Check */
    _numfaces = facepos;
    face++;
  } while (--x); /* Face loop */
}

void EON_RenderObj(EON_Obj *obj)
{
  _RenderObj(obj,0,0);
}

void EON_RenderEnd()
{
  _faceInfo *f;
  if (_cam->Sort > 0) _hsort(_faces,_numfaces,0);
  else if (_cam->Sort < 0) _hsort(_faces,_numfaces,1);
  f = _faces;
  while (_numfaces--) {
    if (f->face->Material && f->face->Material->_PutFace)
    {
      EON_ClipRenderFace(f->face);
    }
    f++;
  }
  _numfaces=0;
  _numlights = 0;
}

static _faceInfo *Base, tmp;

static void _hsort(_faceInfo *base, int nel, int dir)
{
  static int i;
  Base=base-1;
  for (i=nel/2; i>0; i--) _sift_down(i,nel,dir);
  for (i=nel; i>1; ) {
    tmp = base[0]; base[0] = Base[i]; Base[i] = tmp;
    _sift_down(1,i-=1,dir);
  }
}

#define Comp(x,y) (( x ).zd < ( y ).zd ? 1 : 0)

static void _sift_down(int L, int U, int dir)
{
  static int c;
  while (1) {
    c=L+L;
    if (c>U) break;
    if ( (c < U) && dir^Comp(Base[c+1],Base[c])) c++;
    if (dir^Comp(Base[L],Base[c])) return;
    tmp = Base[L]; Base[L] = Base[c]; Base[c] = tmp;
    L=c;
  }
}
#undef Comp

/*************************************************************************/

// make.c
//

EON_Obj *EON_MakeTorus(EON_Float r1, EON_Float r2, EON_uInt divrot, EON_uInt divrad,
                    EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  double ravg, rt, a, da, al, dal;
  EON_sInt32 U,V,dU,dV;
  if (divrot < 3) divrot = 3;
  if (divrad < 3) divrad = 3;
  ravg = (r1+r2)*0.5;
  rt = (r2-r1)*0.5;
  o = EON_ObjCreate(divrad*divrot,divrad*divrot*2);
  if (!o) return 0;
  v = o->Vertices;
  a = 0.0;
  da = 2*EON_PI/divrot;
  for (y = 0; y < divrot; y ++) {
    al = 0.0;
    dal = 2*EON_PI/divrad;
    for (x = 0; x < divrad; x ++) {
      v->x = (EON_Float) (cos((double) a)*(ravg + cos((double) al)*rt));
      v->z = (EON_Float) (sin((double) a)*(ravg + cos((double) al)*rt));
      v->y = (EON_Float) (sin((double) al)*rt);
      v++;
      al += dal;
    }
    a += da;
  }
  v = o->Vertices;
  f = o->Faces;
  dV = 65535/divrad;
  dU = 65535/divrot;
  U = 0;
  for (y = 0; y < divrot; y ++) {
    V = -32768;
    for (x = 0; x < divrad; x ++) {
      f->Vertices[0] = v+x+y*divrad;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V;
      f->Material = m;
      f++;
      f->Vertices[0] = v+x+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[0] = U+dU;
      f->MappingV[0] = V;
      f->Vertices[1] = v+(x+1==divrad?0:x+1)+y*divrad;
      f->MappingU[1] = U;
      f->MappingV[1] = V+dV;
      f->Vertices[2] = v+(x+1==divrad?0:x+1)+(y+1==divrot?0:(y+1)*divrad);
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m;
      f++;
      V += dV;
    }
    U += dU;
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeSphere(EON_Float r, EON_uInt divr, EON_uInt divh, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  double a, da, yp, ya, yda, yf;
  EON_sInt32 U,V,dU,dV;
  if (divh < 3) divh = 3;
  if (divr < 3) divr = 3;
  o = EON_ObjCreate(2+(divh-2)*(divr),2*divr+(divh-3)*divr*2);
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0.0; v->y = r; v++;
  v->x = v->z = 0.0; v->y = -r; v++;
  ya = 0.0;
  yda = EON_PI/(divh-1);
  da = (EON_PI*2.0)/divr;
  for (y = 0; y < divh - 2; y ++) {
    ya += yda;
    yp = cos((double) ya)*r;
    yf = sin((double) ya)*r;
    a = 0.0;
    for (x = 0; x < divr; x ++) {
      v->y = (EON_Float) yp;
      v->x = (EON_Float) (cos((double) a)*yf);
      v->z = (EON_Float) (sin((double) a)*yf);
      v++;
      a += da;
    }
  }
  f = o->Faces;
  v = o->Vertices + 2;
  a = 0.0;
  U = 0;
  dU = 65535/divr;
  dV = V = 65535/divh;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = v + (x+1==divr ? 0 : x+1);
    f->Vertices[2] = v + x;
    f->MappingU[0] = U;
    f->MappingV[0] = 0;
    f->MappingU[1] = U+dU;
    f->MappingV[1] = V;
    f->MappingU[2] = U;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  da = 1.0/(divr+1);
  v = o->Vertices + 2;
  for (x = 0; x < (divh-3); x ++) {
    U = 0;
    for (y = 0; y < divr; y ++) {
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+divr+(y+1==divr?0:y+1);
      f->Vertices[2] = v+y+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V+dV;
      f->MappingU[2] = U;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      f->Vertices[0] = v+y;
      f->Vertices[1] = v+(y+1==divr?0:y+1);
      f->Vertices[2] = v+(y+1==divr?0:y+1)+divr;
      f->MappingU[0] = U;
      f->MappingV[0] = V;
      f->MappingU[1] = U+dU;
      f->MappingV[1] = V;
      f->MappingU[2] = U+dU;
      f->MappingV[2] = V+dV;
      f->Material = m; f++;
      U += dU;
    }
    V += dV;
    v += divr;
  }
  v = o->Vertices + o->NumVertices - divr;
  U = 0;
  for (x = 0; x < divr; x ++) {
    f->Vertices[0] = o->Vertices + 1;
    f->Vertices[1] = v + x;
    f->Vertices[2] = v + (x+1==divr ? 0 : x+1);
    f->MappingU[0] = U;
    f->MappingV[0] = 65535;
    f->MappingU[1] = U;
    f->MappingV[1] = V;
    f->MappingU[2] = U+dU;
    f->MappingV[2] = V;
    f->Material = m;
    f++;
    U += dU;
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeCylinder(EON_Float r, EON_Float h, EON_uInt divr, EON_Bool captop,
                          EON_Bool capbottom, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v, *topverts, *bottomverts, *topcapvert=0, *bottomcapvert=0;
  EON_Face *f;
  EON_uInt32 i;
  double a, da;
  if (divr < 3) divr = 3;
  o = EON_ObjCreate(divr*2+((divr==3)?0:(captop?1:0)+(capbottom?1:0)),
                  divr*2+(divr==3 ? (captop ? 1 : 0) + (capbottom ? 1 : 0) :
                  (captop ? divr : 0) + (capbottom ? divr : 0)));
  if (!o) return 0;
  a = 0.0;
  da = (2.0*EON_PI)/divr;
  v = o->Vertices;
  topverts = v;
  for (i = 0; i < divr; i ++) {
    v->y = h/2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float)(r*sin(a));
    v->xformedx = (EON_Float) (32768.0 + (32768.0*cos((double) a))); // temp
    v->xformedy = (EON_Float) (32768.0 + (32768.0*sin((double) a))); // use xf
    v++;
    a += da;
  }
  bottomverts = v;
  a = 0.0;
  for (i = 0; i < divr; i ++) {
    v->y = -h/2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float) (r*sin(a));
    v->xformedx = (EON_Float) (32768.0 + (32768.0*cos((double) a)));
    v->xformedy = (EON_Float) (32768.0 + (32768.0*sin((double) a)));
    v++; a += da;
  }
  if (captop && divr != 3) {
    topcapvert = v;
    v->y = h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  if (capbottom && divr != 3) {
    bottomcapvert = v;
    v->y = -h / 2.0f;
    v->x = v->z = 0.0f;
    v++;
  }
  f = o->Faces;
  for (i = 0; i < divr; i ++) {
    f->Vertices[0] = bottomverts + i;
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[0] = f->MappingV[2] = 65535; f->MappingV[1] = 0;
    f->MappingU[0] = f->MappingU[1] = (i<<16)/divr;
    f->MappingU[2] = ((i+1)<<16)/divr;
    f->Material = m; f++;
    f->Vertices[0] = bottomverts + (i == divr-1 ? 0 : i+1);
    f->Vertices[1] = topverts + i;
    f->Vertices[2] = topverts + (i == divr-1 ? 0 : i+1);
    f->MappingV[1] = f->MappingV[2] = 0; f->MappingV[0] = 65535;
    f->MappingU[0] = f->MappingU[2] = ((i+1)<<16)/divr;
    f->MappingU[1] = (i<<16)/divr;
    f->Material = m; f++;
  }
  if (captop) {
    if (divr == 3) {
      f->Vertices[0] = topverts + 0;
      f->Vertices[1] = topverts + 2;
      f->Vertices[2] = topverts + 1;
      f->MappingU[0] = (EON_sInt32) topverts[0].xformedx;
      f->MappingV[0] = (EON_sInt32) topverts[0].xformedy;
      f->MappingU[1] = (EON_sInt32) topverts[1].xformedx;
      f->MappingV[1] = (EON_sInt32) topverts[1].xformedy;
      f->MappingU[2] = (EON_sInt32) topverts[2].xformedx;
      f->MappingV[2] = (EON_sInt32) topverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = topverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[1] = topverts + i;
        f->Vertices[2] = topcapvert;
        f->MappingU[0] = (EON_sInt32) topverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[0] = (EON_sInt32) topverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[1] = (EON_sInt32) topverts[i].xformedx;
        f->MappingV[1] = (EON_sInt32) topverts[i].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  if (capbottom) {
    if (divr == 3) {
      f->Vertices[0] = bottomverts + 0;
      f->Vertices[1] = bottomverts + 1;
      f->Vertices[2] = bottomverts + 2;
      f->MappingU[0] = (EON_sInt32) bottomverts[0].xformedx;
      f->MappingV[0] = (EON_sInt32) bottomverts[0].xformedy;
      f->MappingU[1] = (EON_sInt32) bottomverts[1].xformedx;
      f->MappingV[1] = (EON_sInt32) bottomverts[1].xformedy;
      f->MappingU[2] = (EON_sInt32) bottomverts[2].xformedx;
      f->MappingV[2] = (EON_sInt32) bottomverts[2].xformedy;
      f->Material = m; f++;
    } else {
      for (i = 0; i < divr; i ++) {
        f->Vertices[0] = bottomverts + i;
        f->Vertices[1] = bottomverts + (i == divr-1 ? 0 : i + 1);
        f->Vertices[2] = bottomcapvert;
        f->MappingU[0] = (EON_sInt32) bottomverts[i].xformedx;
        f->MappingV[0] = (EON_sInt32) bottomverts[i].xformedy;
        f->MappingU[1] = (EON_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedx;
        f->MappingV[1] = (EON_sInt32) bottomverts[(i==divr-1?0:i+1)].xformedy;
        f->MappingU[2] = f->MappingV[2] = 32768;
        f->Material = m; f++;
      }
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakeCone(EON_Float r, EON_Float h, EON_uInt div,
                      EON_Bool cap, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt32 i;
  double a, da;
  if (div < 3) div = 3;
  o = EON_ObjCreate(div + (div == 3 ? 1 : (cap ? 2 : 1)),
                  div + (div == 3 ? 1 : (cap ? div : 0)));
  if (!o) return 0;
  v = o->Vertices;
  v->x = v->z = 0; v->y = h/2;
  v->xformedx = 1<<15;
  v->xformedy = 1<<15;
  v++;
  a = 0.0;
  da = (2.0*EON_PI)/div;
  for (i = 1; i <= div; i ++) {
    v->y = h/-2.0f;
    v->x = (EON_Float) (r*cos((double) a));
    v->z = (EON_Float) (r*sin((double) a));
    v->xformedx = (EON_Float) (32768.0 + (cos((double) a)*32768.0));
    v->xformedy = (EON_Float) (32768.0 + (sin((double) a)*32768.0));
    a += da;
    v++;
  }
  if (cap && div != 3) {
    v->y = h / -2.0f;
    v->x = v->z = 0.0f;
    v->xformedx = (EON_Float) (1<<15);
    v->xformedy = (EON_Float) (1<<15);
    v++;
  }
  f = o->Faces;
  for (i = 1; i <= div; i ++) {
    f->Vertices[0] = o->Vertices;
    f->Vertices[1] = o->Vertices + (i == div ? 1 : i + 1);
    f->Vertices[2] = o->Vertices + i;
    f->MappingU[0] = (EON_sInt32) o->Vertices[0].xformedx;
    f->MappingV[0] = (EON_sInt32) o->Vertices[0].xformedy;
    f->MappingU[1] = (EON_sInt32) o->Vertices[(i==div?1:i+1)].xformedx;
    f->MappingV[1] = (EON_sInt32) o->Vertices[(i==div?1:i+1)].xformedy;
    f->MappingU[2] = (EON_sInt32) o->Vertices[i].xformedx;
    f->MappingV[2] = (EON_sInt32) o->Vertices[i].xformedy;
    f->Material = m;
    f++;
  }
  if (cap) {
    if (div == 3) {
      f->Vertices[0] = o->Vertices + 1;
      f->Vertices[1] = o->Vertices + 2;
      f->Vertices[2] = o->Vertices + 3;
      f->MappingU[0] = (EON_sInt32) o->Vertices[1].xformedx;
      f->MappingV[0] = (EON_sInt32) o->Vertices[1].xformedy;
      f->MappingU[1] = (EON_sInt32) o->Vertices[2].xformedx;
      f->MappingV[1] = (EON_sInt32) o->Vertices[2].xformedy;
      f->MappingU[2] = (EON_sInt32) o->Vertices[3].xformedx;
      f->MappingV[2] = (EON_sInt32) o->Vertices[3].xformedy;
      f->Material = m;
      f++;
    } else {
      for (i = 1; i <= div; i ++) {
        f->Vertices[0] = o->Vertices + div + 1;
        f->Vertices[1] = o->Vertices + i;
        f->Vertices[2] = o->Vertices + (i==div ? 1 : i+1);
        f->MappingU[0] = (EON_sInt32) o->Vertices[div+1].xformedx;
        f->MappingV[0] = (EON_sInt32) o->Vertices[div+1].xformedy;
        f->MappingU[1] = (EON_sInt32) o->Vertices[i].xformedx;
        f->MappingV[1] = (EON_sInt32) o->Vertices[i].xformedy;
        f->MappingU[2] = (EON_sInt32) o->Vertices[i==div?1:i+1].xformedx;
        f->MappingV[2] = (EON_sInt32) o->Vertices[i==div?1:i+1].xformedy;
        f->Material = m;
        f++;
      }
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

static EON_uChar verts[6*6] = {
  0,4,1, 1,4,5, 0,1,2, 3,2,1, 2,3,6, 3,7,6,
  6,7,4, 4,7,5, 1,7,3, 7,1,5, 2,6,0, 4,0,6
};
static EON_uChar map[24*2*3] = {
  1,0, 1,1, 0,0, 0,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 1,1, 0,1, 1,0,
  0,0, 1,0, 0,1, 1,0, 1,1, 0,1,
  0,0, 1,0, 0,1, 0,1, 1,0, 1,1,
  1,0, 0,1, 0,0, 0,1, 1,0, 1,1,
  1,0, 1,1, 0,0, 0,1, 0,0, 1,1
};


EON_Obj *EON_MakeBox(EON_Float w, EON_Float d, EON_Float h, EON_Mat *m)
{
  EON_uChar *mm = map;
  EON_uChar *vv = verts;
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x;
  o = EON_ObjCreate(8,12);
  if (!o) return 0;
  v = o->Vertices;
  v->x = -w/2; v->y = h/2; v->z = d/2; v++;
  v->x = w/2; v->y = h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = h/2; v->z = -d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = d/2; v++;
  v->x = -w/2; v->y = -h/2; v->z = -d/2; v++;
  v->x = w/2; v->y = -h/2; v->z = -d/2; v++;
  f = o->Faces;
  for (x = 0; x < 12; x ++) {
    f->Vertices[0] = o->Vertices + *vv++;
    f->Vertices[1] = o->Vertices + *vv++;
    f->Vertices[2] = o->Vertices + *vv++;
    f->MappingU[0] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[0] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[1] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[1] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingU[2] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->MappingV[2] = (EON_sInt32) ((double)*mm++ * 65535.0);
    f->Material = m;
    f++;
  }

  EON_ObjCalcNormals(o);
  return (o);
}

EON_Obj *EON_MakePlane(EON_Float w, EON_Float d, EON_uInt res, EON_Mat *m)
{
  EON_Obj *o;
  EON_Vertex *v;
  EON_Face *f;
  EON_uInt x, y;
  o = EON_ObjCreate((res+1)*(res+1),res*res*2);
  if (!o) return 0;
  v = o->Vertices;
  for (y = 0; y <= res; y ++) {
    for (x = 0; x <= res; x ++) {
      v->y = 0;
      v->x = ((x*w)/res) - w/2;
      v->z = ((y*d)/res) - d/2;
      v++;
    }
  }
  f = o->Faces;
  for (y = 0; y < res; y ++) {
    for (x = 0; x < res; x ++) {
      f->Vertices[0] = o->Vertices + x+(y*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = (y<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[1] = (x<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
      f->Vertices[0] = o->Vertices + x+((y+1)*(res+1));
      f->MappingU[0] = (x<<16)/res;
      f->MappingV[0] = ((y+1)<<16)/res;
      f->Vertices[2] = o->Vertices + x+1+(y*(res+1));
      f->MappingU[2] = ((x+1)<<16)/res;
      f->MappingV[2] = (y<<16)/res;
      f->Vertices[1] = o->Vertices + x+1+((y+1)*(res+1));
      f->MappingU[1] = ((x+1)<<16)/res;
      f->MappingV[1] = ((y+1)<<16)/res;
      f->Material = m;
      f++;
    }
  }
  EON_ObjCalcNormals(o);
  return (o);
}

/*************************************************************************/

/* vim: set ts=4 sw=4 et */
/* EOF */

