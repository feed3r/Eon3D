/**************************************************************************
 * eon3d.h -- Eon3D is a simplistic 3D software renderer.                 *
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

#ifndef EON3D_H
#define EON3D_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

/** \file eon3d.h
    \brief eon3d: a simplistic 3D software renderer (interface).
*/

/******************************************************************************/

enum {
    /* Maximum children per object */
    EON_MAX_CHILDREN = 128,
    /* Maximum lights per scene -- if you exceed this, they will be ignored */
    EON_MAX_LIGHTS = 32,
    /* Maximum number of triangles per scene -- if you exceed this, entire
       objects will be ignored. You can increase this if you need it. It takes
       approximately 8*EON_MAX_TRIANGLES bytes of memory. i.e. the default of
       16384 consumes 128kbytes of memory. not really a big deal,
    */
    EON_MAX_TRIANGLES = 1048576
};

typedef float EON_ZBuffer;        /* z-buffer type (must be float) */
typedef float EON_Float;          /* General floating point */
typedef float EON_IEEEFloat32;    /* IEEE 32 bit floating point */
typedef int32_t EON_sInt32;       /* signed 32 bit integer */
typedef uint32_t EON_uInt32;      /* unsigned 32 bit integer */
typedef int16_t EON_sInt16;       /* signed 16 bit integer */
typedef uint16_t EON_uInt16;      /* unsigned 16 bit integer */
typedef signed int EON_sInt;      /* signed optimal integer */
typedef unsigned int EON_uInt;    /* unsigned optimal integer */
typedef int EON_Bool;             /* boolean */
typedef uint8_t EON_uChar;        /* unsigned 8 bit integer */
typedef int8_t EON_sChar;         /* signed 8 bit integer */

/* pi! */
#define EON_PI 3.14159265359

/* Utility min() and max() functions */
#define EON_Min(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define EON_Max(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))

/*
** Shade modes. Used with EON_Mat.ShadeType
** Note that (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE) and
** (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
#define EON_SHADE_NONE (1)
#define EON_SHADE_FLAT (2)
#define EON_SHADE_FLAT_DISTANCE (4)
#define EON_SHADE_GOURAUD (8)
#define EON_SHADE_GOURAUD_DISTANCE (16)

/*
** Light modes. Used with EON_Light.Type or EON_LightSet().
** Note that EON_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, EON_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see EON_LightSet() for setting it), EON_LIGHT_POINT does both.
*/
#define EON_LIGHT_NONE (0x0)
#define EON_LIGHT_VECTOR (0x1)
#define EON_LIGHT_POINT (0x2|0x4)
#define EON_LIGHT_POINT_DISTANCE (0x2)
#define EON_LIGHT_POINT_ANGLE (0x4)

/* Used internally; EON_FILL_* are stored in EON_Mat._st. */
#define EON_FILL_SOLID (0x0)
#define EON_FILL_TEXTURE (0x1)
#define EON_FILL_ENVIRONMENT (0x2)
#define EON_FILL_TRANSPARENT (0x4)

#define EON_TEXENV_ADD (0)
#define EON_TEXENV_MUL (1)
#define EON_TEXENV_AVG (2)
#define EON_TEXENV_TEXMINUSENV (3)
#define EON_TEXENV_ENVMINUSTEX (4)
#define EON_TEXENV_MIN (5)
#define EON_TEXENV_MAX (6)

/*
** Texture type. Read textures with EON_ReadPCXTex(), and assign them to
** EON_Mat.Environment or EON_Mat.Texture.
*/
typedef struct _EON_Texture {
  EON_uChar *Data;            /* Texture data */
  EON_uChar *PaletteData;     /* Palette data (NumColors bytes) */
  EON_uChar Width, Height;    /* Log2 of dimensions */
  EON_uInt iWidth, iHeight;   /* Integer dimensions */
  EON_Float uScale, vScale;   /* Scaling (usually 2**Width, 2**Height) */
  EON_uInt NumColors;         /* Number of colors used in texture */
} EON_Texture;

/*
** Material type. Create materials with EON_MatCreate().
*/
typedef struct _EON_Mat {
  EON_sInt Ambient[3];          /* RGB of surface (0-255 is a good range) */
  EON_sInt Diffuse[3];          /* RGB of diffuse (0-255 is a good range) */
  EON_sInt Specular[3];         /* RGB of "specular" highlights (0-255) */
  EON_uInt Shininess;           /* Shininess of material. 1 is dullest */
  EON_Float FadeDist;           /* For distance fading, distance at
                                  which intensity is 0 */
  EON_uChar ShadeType;          /* Shade type: EON_SHADE_* */
  EON_uChar Transparent;        /* Transparency index (0 = none), 4 = alot
                                  Note: transparencies disable textures */
  EON_uChar PerspectiveCorrect; /* Correct textures every n pixels */
  EON_Texture *Texture;         /* Texture map (see EON_Texture) above */
  EON_Texture *Environment;     /* Environment map (ditto) */
  EON_Float TexScaling;         /* Texture map scaling */
  EON_Float EnvScaling;         /* Environment map scaling */
  EON_uChar TexEnvMode;         /* TexEnv combining mode (EON_TEXENV_*) */
  EON_Bool zBufferable;         /* Can this material be zbuffered? */
  EON_uInt NumGradients;        /* Desired number of gradients to be used */
                 /* The following are used mostly internally */
  EON_uInt _ColorsUsed;         /* Number of colors actually used */
  EON_uChar _st, _ft;           /* The shadetype and filltype */
  EON_uInt _tsfact;             /* Translucent shading factor */
  EON_uInt16 *_AddTable;        /* Shading/Translucent/etc table */
  EON_uChar *_ReMapTable;       /* Table to remap colors to palette */
  EON_uChar *_RequestedColors;  /* _ColorsUsed colors, desired colors */
  void (*_PutFace)();          /* Function that renders the triangle with this
                                  material */
} EON_Mat;

/*
** Vertex, used within EON_Obj
*/
typedef struct _EON_Vertex {
  EON_Float x, y, z;              /* Vertex coordinate (objectspace) */
  EON_Float xformedx, xformedy, xformedz;
                                 /* Transformed vertex
                                    coordinate (cameraspace) */
  EON_Float nx, ny, nz;           /* Unit vertex normal (objectspace) */
  EON_Float xformednx, xformedny, xformednz;
                                 /* Transformed unit vertex normal
                                    (cameraspace) */
} EON_Vertex;

/*
** Face
*/
typedef struct _EON_Face {
  EON_Vertex *Vertices[3];      /* Vertices of triangle */
  EON_Float nx, ny, nz;         /* Normal of triangle (object space) */
  EON_Mat *Material;            /* Material of triangle */
  EON_sInt32 Scrx[3], Scry[3];  /* Projected screen coordinates
                                  (12.20 fixed point) */
  EON_Float Scrz[3];            /* Projected 1/Z coordinates */
  EON_sInt32 MappingU[3], MappingV[3];
                               /* 16.16 Texture mapping coordinates */
  EON_sInt32 eMappingU[3], eMappingV[3];
                               /* 16.16 Environment map coordinates */
  EON_Float fShade;             /* Flat intensity */
  EON_Float sLighting;          /* Face static lighting. Should usually be 0.0 */
  EON_Float Shades[3];          /* Vertex intensity */
  EON_Float vsLighting[3];      /* Vertex static lighting. Should be 0.0 */
} EON_Face;

/*
** Object
*/
typedef struct _EON_Obj {
  EON_uInt32 NumVertices;              /* Number of vertices */
  EON_uInt32 NumFaces;                 /* Number of faces */
  EON_Vertex *Vertices;                /* Array of vertices */
  EON_Face *Faces;                     /* Array of faces */
  struct _EON_Obj *Children[EON_MAX_CHILDREN];
                                      /* Children */
  EON_Bool BackfaceCull;               /* Are backfacing polys drawn? */
  EON_Bool BackfaceIllumination;       /* Illuminated by lights behind them? */
  EON_Bool GenMatrix;                  /* Generate Matrix from the following
                                         if set */
  EON_Float Xp, Yp, Zp, Xa, Ya, Za;    /* Position and rotation of object:
                                         Note: rotations are around
                                         X then Y then Z. Measured in degrees */
  EON_Float Matrix[16];                /* Transformation matrix */
  EON_Float RotMatrix[16];             /* Rotation only matrix (for normals) */
} EON_Obj;

/*
** Spline type. See EON_Spline*().
*/
typedef struct _EON_Spline {
  EON_Float *keys;              /* Key data, keyWidth*numKeys */
  EON_sInt keyWidth;            /* Number of floats per key */
  EON_sInt numKeys;             /* Number of keys */
  EON_Float cont;               /* Continuity. Should be -1.0 -> 1.0 */
  EON_Float bias;               /* Bias. -1.0 -> 1.0 */
  EON_Float tens;               /* Tension. -1.0 -> 1.0 */
} EON_Spline;

/*
** Light type. See EON_Light*().
*/
typedef struct _EON_Light {
  EON_uChar Type;               /* Type of light: EON_LIGHT_* */
  EON_Float Xp, Yp, Zp;         /* If Type=EON_LIGHT_POINT*,
                                  this is Position (EON_LIGHT_POINT_*),
                                  otherwise if EON_LIGHT_VECTOR,
                                  Unit vector */
  EON_Float Intensity;           /* Intensity. 0.0 is off, 1.0 is full */
  EON_Float HalfDistSquared;     /* Distance squared at which
                                   EON_LIGHT_POINT_DISTANCE is 50% */
} EON_Light;

/*
** Camera Type.
*/
typedef struct _EON_Cam {
  EON_Float Fov;                  /* FOV in degrees valid range is 1-179 */
  EON_Float AspectRatio;          /* Aspect ratio (usually 1.0) */
  EON_sChar Sort;                 /* Sort polygons, -1 f-t-b, 1 b-t-f, 0 no */
  EON_Float ClipBack;             /* Far clipping ( < 0.0 is none) */
  EON_sInt ClipTop, ClipLeft;     /* Screen Clipping */
  EON_sInt ClipBottom, ClipRight;
  EON_uInt ScreenWidth, ScreenHeight; /* Screen dimensions */
  EON_sInt CenterX, CenterY;      /* Center of screen */
  EON_Float X, Y, Z;              /* Camera position in worldspace */
  EON_Float Pitch, Pan, Roll;     /* Camera angle in degrees in worldspace */
  EON_uChar *frameBuffer;         /* Framebuffer (ScreenWidth*ScreenHeight) */
  EON_ZBuffer *zBuffer;           /* Z Buffer (NULL if none) */
} EON_Cam;


extern EON_uInt32 EON_Render_TriStats[4]; /* Three different triangle counts from
                                          the last EON_Render() block:
                                          0: initial tris
                                          1: tris after culling
                                          2: final polys after real clipping
                                          3: final tris after tesselation
                                       */

/******************************************************************************
** Material Functions (mat.c)
******************************************************************************/

/*
  EON_MatCreate() creates a material.
  Parameters:
    none
  Returns:
    a pointer to the material on success, 0 on failure
*/
EON_Mat *EON_MatCreate();

/*
  EON_MatDelete() deletes a material that was created with EON_MatCreate().
  Parameters:
    m: a pointer to the material to be deleted
  Returns:
    nothing
*/
void EON_MatDelete(EON_Mat *m);

/*
  EON_MatInit() initializes a material that was created with EON_MatCreate().
  Parameters:
    m: a pointer to the material to be intialized
  Returns:
    nothing
  Notes:
    you *must* do this before calling EON_MatMapToPal() or EON_MatMakeOptPal().
*/
void EON_MatInit(EON_Mat *m);

/*
  EON_MatMapToPal() maps a material that was created with EON_MatCreate() and
    initialized with EON_MatInit() to a palette.
  Parameters:
    mat: material to map
    pal: a 768 byte array of unsigned chars, each 3 being a rgb triEON_et
         (0-255, *not* the cheesy vga 0-63)
    pstart: starting offset to use colors of, usually 0
    pend: ending offset to use colors of, usually 255
  Returns:
    nothing
  Notes:
    Mapping a material with > 2000 colors can take up to a second or two.
      Be careful, and go easy on EON_Mat.NumGradients ;)
*/
void EON_MatMapToPal(EON_Mat *m, EON_uChar *pal, EON_sInt pstart, EON_sInt pend);


/*
  EON_MatMakeOptPal() makes an almost optimal palette from materials
    created with EON_MatCreate() and initialized with EON_MatInit().
  Paramters:
    p: palette to create
    pstart: first color entry to use
    pend: last color entry to use
    materials: an array of pointers to materials to generate the palette from
    nmats: number of materials
  Returns:
    nothing
*/
void EON_MatMakeOptPal(EON_uChar *p, EON_sInt pstart,
                     EON_sInt pend, EON_Mat **materials, EON_sInt nmats);


/******************************************************************************
** Object Functions (obj.c)
******************************************************************************/

/*
  EON_ObjCreate() allocates an object
  Paramters:
    np: Number of vertices in object
    nf: Number of faces in object
  Returns:
    a pointer to the object on success, 0 on failure
*/
EON_Obj *EON_ObjCreate(EON_uInt32 np, EON_uInt32 nf);

/*
  EON_ObjDelete() frees an object and all of it's subobjects
    that was allocated with EON_ObjCreate();
  Paramters:
    o: object to delete
  Returns:
    nothing
*/
void EON_ObjDelete(EON_Obj *o);

/*
  EON_ObjClone() creates an exact but independent duEON_icate of an object and
    all of it's subobjects
  Paramters:
    o: the object to clone
  Returns:
    a pointer to the new object on success, 0 on failure
*/
EON_Obj *EON_ObjClone(EON_Obj *o);

/*
  EON_ObjScale() scales an object, and all of it's subobjects.
  Paramters:
    o: a pointer to the object to scale
    s: the scaling factor
  Returns:
    a pointer to o.
  Notes: This scales it slowly, by going through each vertex and scaling it's
    position. Avoid doing this in realtime.
*/
EON_Obj *EON_ObjScale(EON_Obj *o, EON_Float s);

/*
  EON_ObjStretch() stretches an object, and all of it's subobjects
  Parameters:
    o: a pointer to the object to stretch
    x,y,z: the x y and z stretch factors
  Returns:
    a pointer to o.
  Notes: same as EON_ObjScale(). Note that the normals are preserved.
*/
EON_Obj *EON_ObjStretch(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z);

/*
   EON_ObjTranslate() translates an object
   Parameters:
     o: a pointer to the object to translate
     x,y,z: translation in object space
   Returns:
     a pointer to o
   Notes: same has EON_ObjScale().
*/
EON_Obj *EON_ObjTranslate(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z);

/*
  EON_ObjFlipNormals() flips all vertex and face normals of and object
    and allo of it's subobjects.
  Parameters:
    o: a pointer to the object to flip normals of
  Returns:
    a pointer to o
  Notes:
    Not especially fast.
    A call to EON_ObjFlipNormals() or EON_ObjCalcNormals() will restore the normals
*/
EON_Obj *EON_ObjFlipNormals(EON_Obj *o);

/*
  EON_ObjSetMat() sets the material of all faces in an object.
  Paramters:
    o: the object to set the material of
    m: the material to set it to
    th: "transcend hierarchy". If set, it will set the
        material of all subobjects too.
  Returns:
    nothing
*/
void EON_ObjSetMat(EON_Obj *o, EON_Mat *m, EON_Bool th);

/*
   EON_ObjCalcNormals() calculates all face and vertex normals for an object
     and all subobjects.
   Paramters:
     obj: the object
   Returns:
     nothing
*/
EON_Obj *EON_ObjCalcNormals(EON_Obj *obj);

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
void EON_ClipSetFrustum(EON_Cam *cam);

/*
  EON_ClipRenderFace() renders a face and clips it to the frustum initialized
    with EON_ClipSetFrustum().
  Parameters:
    face: the face to render
  Returns:
    nothing
  Notes: this is used internally by EON_Render*(), so be careful. Kinda slow too.
*/
void EON_ClipRenderFace(EON_Face *face);

/*
  EON_ClipNeeded() decides whether the face is in the frustum, intersecting
    the frustum, or comEON_etely out of the frustum craeted with
    EON_ClipSetFrustum().
  Parameters:
    face: the face to check
  Returns:
    0: the face is out of the frustum, no drawing necessary
    1: the face is intersecting the frustum, sEON_itting and drawing necessary
  Notes: this is used internally by EON_Render*(), so be careful. Kinda slow too.
*/
EON_sInt EON_ClipNeeded(EON_Face *face);

/******************************************************************************
** Light Handling Routines (light.c)
******************************************************************************/

/*
  EON_LightCreate() creates a new light
  Parameters:
    none
  Returns:
    a pointer to the light
*/
EON_Light *EON_LightCreate();

/*
  EON_LightSet() sets up a light allocated with EON_LightCreate()
  Parameters:
    light: the light to set up
    mode: the mode of the light (EON_LIGHT_*)
    x,y,z: either the position of the light (EON_LIGHT_POINT*) or the angle
           in degrees of the light (EON_LIGHT_VECTOR)
    intensity: the intensity of the light (0.0-1.0)
    halfDist: the distance at which EON_LIGHT_POINT_DISTANCE is 1/2 intensity
  Returns:
    a pointer to light.
*/
EON_Light *EON_LightSet(EON_Light *light, EON_uChar mode, EON_Float x, EON_Float y,
                     EON_Float z, EON_Float intensity, EON_Float halfDist);

/*
  WRITEME
*/
EON_Light *EON_LightNew(EON_uChar mode, EON_Float x, EON_Float y, EON_Float z,
                     EON_Float intensity, EON_Float halfDist);

/*
  EON_LightDelete() frees a light allocated with EON_LightCreate().
  Paramters:
    l: light to delete
  Returns:
    nothing
*/
void EON_LightDelete(EON_Light *l);

/* PUT ME SOMEWHERE */
/*
** EON_DeleteTexture() frees all memory associated with "t"
*/
void EON_DeleteTexture(EON_Texture *t);


/******************************************************************************
** Camera Handling Routines (cam.c)
******************************************************************************/

/*
  EON_CamCreate() allocates a new camera
  Parameters:
    sw: screen width
    sh: screen height
    ar: aspect ratio (usually 1.0)
    fov: field of view (usually 45-120)
    fb: pointer to framebuffer
    zb: pointer to Z buffer (or NULL)
  Returns:
    a pointer to the newly allocated camera
*/
EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov,
                       EON_uChar *fb, EON_ZBuffer *zb);

/*
  EON_CamSetTarget() sets the target of a camera allocated with EON_CamCreate().
  Parameters:
    c: the camera to set the target of
    x,y,z: the worldspace coordinate of the target
  Returns:
    nothing
  Notes:
    Sets the pitch and pan of the camera. Does not touch the roll.
*/
void EON_CamSetTarget(EON_Cam *c, EON_Float x, EON_Float y, EON_Float z);

/*
   EON_CamDelete() frees all memory associated with a camera excluding
     framebuffers and Z buffers
   Paramters:
     c: camera to free
   Returns:
     nothing
*/
void EON_CamDelete(EON_Cam *c);

/******************************************************************************
** Easy Rendering Interface (render.c)
******************************************************************************/

/*
 EON_RenderBegin() begins the rendering process.
   Parameters:
     Camera: camera to use for rendering
   Returns:
     nothing
   Notes:
     Only one rendering process can occur at a time.
     Uses EON_Clip*(), so don't use them within or around a EON_Render() block.
*/
void EON_RenderBegin(EON_Cam *Camera);

/*
   EON_RenderLight() adds a light to the scene.
   Parameters:
     light: light to add to scene
   Returns:
     nothing
   Notes: Any objects rendered before will be unaffected by this.
*/
void EON_RenderLight(EON_Light *light);

/*
   EON_RenderObj() adds an object and all of it's subobjects to the scene.
   Parameters:
     obj: object to render
   Returns:
     nothing
   Notes: if Camera->Sort is zero, objects are rendered in the order that
     they are added to the scene.
*/
void EON_RenderObj(EON_Obj *obj);

/*
   EON_RenderEnd() actually does the rendering, and closes the rendering process
   Paramters:
     none
   Returns:
     nothing
*/
void EON_RenderEnd();


/******************************************************************************
** Math Code (math.c)
******************************************************************************/

/*
  EON_MatrixRotate() generates a rotation matrix
  Parameters:
    matrix: an array of 16 EON_Floats that is a 4x4 matrix
    m: the axis to rotate around, 1=X, 2=Y, 3=Z.
    Deg: the angle in degrees to rotate
  Returns:
    nothing
*/
void EON_MatrixRotate(EON_Float matrix[], EON_uChar m, EON_Float Deg);

/*
  EON_MatrixTranslate() generates a translation matrix
  Parameters:
    m: the matrix (see EON_MatrixRotate for more info)
    x,y,z: the translation coordinates
  Returns:
    nothing
*/
void EON_MatrixTranslate(EON_Float m[], EON_Float x, EON_Float y, EON_Float z);

/*
  EON_MatrixMultiply() multiEON_ies two matrices
  Parameters:
    dest: destination matrix will be multiEON_ed by src
    src: source matrix
  Returns:
    nothing
  Notes:
    this is the same as dest = dest*src (since the order *does* matter);
*/
void EON_MatrixMultiply(EON_Float *dest, EON_Float src[]);

/*
   EON_MatrixApply() apEON_ies a matrix.
  Parameters:
    m: matrix to apply
    x,y,z: input coordinate
    outx,outy,outz: pointers to output coords.
  Returns:
    nothing
  Notes:
    apEON_ies the matrix to the 3d point to produce the transformed 3d point
*/
void EON_MatrixApply(EON_Float *m, EON_Float x, EON_Float y, EON_Float z,
                     EON_Float *outx, EON_Float *outy, EON_Float *outz);

/*
  EON_NormalizeVector() makes a vector a unit vector
  Parameters:
    x,y,z: pointers to the vector
  Returns:
    nothing
*/
void EON_NormalizeVector(EON_Float *x, EON_Float *y, EON_Float *z);

/*
  EON_DotProduct() returns the dot product of two vectors
  Parameters:
    x1,y1,z1: the first vector
    x2,y2,z2: the second vector
  Returns:
    the dot product of the two vectors
*/
EON_Float EON_DotProduct(EON_Float x1, EON_Float y1, EON_Float z1,
                         EON_Float x2, EON_Float y2, EON_Float z2);

/******************************************************************************
** Spline Interpolation (spline.c)
******************************************************************************/

/*
  EON_SplineInit() initializes a spline
  Parameters:
    s: the spline
  Returns:
    nothing
  Notes:
    Intializes the spline. Do this once, or when you change any of the settings
*/
void EON_SplineInit(EON_Spline *s);

/*
  EON_SplineGetPoint() gets a point on the spline
  Parameters:
    s: spline
    frame: time into spline. 0.0 is start, 1.0 is second key point, etc.
    out: a pointer to an array of s->keyWidth floats that will be filled in.
  Returns:
    nothing
*/
void EON_SplineGetPoint(EON_Spline *s, EON_Float frame, EON_Float *out);

/******************************************************************************
** Built-in Rasterizers
******************************************************************************/

void EON_PF_SolidF(EON_Cam *, EON_Face *);
void EON_PF_SolidG(EON_Cam *, EON_Face *);
void EON_PF_TexF(EON_Cam *, EON_Face *);
void EON_PF_TexG(EON_Cam *, EON_Face *);
void EON_PF_TexEnv(EON_Cam *, EON_Face *);
void EON_PF_PTexF(EON_Cam *, EON_Face *);
void EON_PF_PTexG(EON_Cam *, EON_Face *);
void EON_PF_TransF(EON_Cam *, EON_Face *);
void EON_PF_TransG(EON_Cam *, EON_Face *);

/*************************************************************************/

#endif /* ! EON3D_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

