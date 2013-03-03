/******************************************************************************
  plush.h
  PLUSH 3D VERSION 1.2 MAIN HEADER
  Copyright (c) 1996-2000 Justin Frankel
  Copyright (c) 1998-2000 Nullsoft, Inc.

  For more information on Plush and the latest updates, please visit
    http://www.nullsoft.com

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Justin Frankel
  justin@nullsoft.com

******************************************************************************/

#ifndef _PLUSH_H_
#define _PLUSH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* Maximum children per object */
    PL_MAX_CHILDREN = 16,
    /* Maximum lights per scene -- if you exceed this, they will be ignored */
    PL_MAX_LIGHTS = 32,
    /* Maximum number of triangles per scene -- if you exceed this, entire
       objects will be ignored. You can increase this if you need it. It takes
       approximately 8*PL_MAX_TRIANGLES bytes of memory. i.e. the default of
       16384 consumes 128kbytes of memory. not really a big deal,
    */
    PL_MAX_TRIANGLES = 16384
};

typedef float pl_ZBuffer;        /* z-buffer type (must be float) */
typedef float pl_Float;          /* General floating point */
typedef float pl_IEEEFloat32;    /* IEEE 32 bit floating point */
typedef int32_t pl_sInt32;       /* signed 32 bit integer */
typedef uint32_t pl_uInt32;      /* unsigned 32 bit integer */
typedef int16_t pl_sInt16;       /* signed 16 bit integer */
typedef uint16_t pl_uInt16;      /* unsigned 16 bit integer */
typedef signed int pl_sInt;      /* signed optimal integer */
typedef unsigned int pl_uInt;    /* unsigned optimal integer */
typedef int pl_Bool;             /* boolean */
typedef uint8_t pl_uChar;        /* unsigned 8 bit integer */
typedef int8_t pl_sChar;         /* signed 8 bit integer */

/* pi! */
#define PL_PI 3.14159265359

/* Utility min() and max() functions */
#define plMin(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define plMax(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))

/*
** Shade modes. Used with plMat.ShadeType
** Note that (PL_SHADE_GOURAUD|PL_SHADE_GOURAUD_DISTANCE) and
** (PL_SHADE_FLAT|PL_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
#define PL_SHADE_NONE (1)
#define PL_SHADE_FLAT (2)
#define PL_SHADE_FLAT_DISTANCE (4)
#define PL_SHADE_GOURAUD (8)
#define PL_SHADE_GOURAUD_DISTANCE (16)

/*
** Light modes. Used with plLight.Type or plLightSet().
** Note that PL_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, PL_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see plLightSet() for setting it), PL_LIGHT_POINT does both.
*/
#define PL_LIGHT_NONE (0x0)
#define PL_LIGHT_VECTOR (0x1)
#define PL_LIGHT_POINT (0x2|0x4)
#define PL_LIGHT_POINT_DISTANCE (0x2)
#define PL_LIGHT_POINT_ANGLE (0x4)

/* Used internally; PL_FILL_* are stored in plMat._st. */
#define PL_FILL_SOLID (0x0)
#define PL_FILL_TEXTURE (0x1)
#define PL_FILL_ENVIRONMENT (0x2)
#define PL_FILL_TRANSPARENT (0x4)

#define PL_TEXENV_ADD (0)
#define PL_TEXENV_MUL (1)
#define PL_TEXENV_AVG (2)
#define PL_TEXENV_TEXMINUSENV (3)
#define PL_TEXENV_ENVMINUSTEX (4)
#define PL_TEXENV_MIN (5)
#define PL_TEXENV_MAX (6)

/*
** Texture type. Read textures with plReadPCXTex(), and assign them to
** plMat.Environment or plMat.Texture.
*/
typedef struct _pl_Texture {
  pl_uChar *Data;            /* Texture data */
  pl_uChar *PaletteData;     /* Palette data (NumColors bytes) */
  pl_uChar Width, Height;    /* Log2 of dimensions */
  pl_uInt iWidth, iHeight;   /* Integer dimensions */
  pl_Float uScale, vScale;   /* Scaling (usually 2**Width, 2**Height) */
  pl_uInt NumColors;         /* Number of colors used in texture */
} pl_Texture;

/*
** Material type. Create materials with plMatCreate().
*/
typedef struct _pl_Mat {
  pl_sInt Ambient[3];          /* RGB of surface (0-255 is a good range) */
  pl_sInt Diffuse[3];          /* RGB of diffuse (0-255 is a good range) */
  pl_sInt Specular[3];         /* RGB of "specular" highlights (0-255) */
  pl_uInt Shininess;           /* Shininess of material. 1 is dullest */
  pl_Float FadeDist;           /* For distance fading, distance at
                                  which intensity is 0 */
  pl_uChar ShadeType;          /* Shade type: PL_SHADE_* */
  pl_uChar Transparent;        /* Transparency index (0 = none), 4 = alot
                                  Note: transparencies disable textures */
  pl_uChar PerspectiveCorrect; /* Correct textures every n pixels */
  pl_Texture *Texture;         /* Texture map (see pl_Texture) above */
  pl_Texture *Environment;     /* Environment map (ditto) */
  pl_Float TexScaling;         /* Texture map scaling */
  pl_Float EnvScaling;         /* Environment map scaling */
  pl_uChar TexEnvMode;         /* TexEnv combining mode (PL_TEXENV_*) */
  pl_Bool zBufferable;         /* Can this material be zbuffered? */
  pl_uInt NumGradients;        /* Desired number of gradients to be used */
                 /* The following are used mostly internally */
  pl_uInt _ColorsUsed;         /* Number of colors actually used */
  pl_uChar _st, _ft;           /* The shadetype and filltype */
  pl_uInt _tsfact;             /* Translucent shading factor */
  pl_uInt16 *_AddTable;        /* Shading/Translucent/etc table */
  pl_uChar *_ReMapTable;       /* Table to remap colors to palette */
  pl_uChar *_RequestedColors;  /* _ColorsUsed colors, desired colors */
  void (*_PutFace)();          /* Function that renders the triangle with this
                                  material */
} pl_Mat;

/*
** Vertex, used within pl_Obj
*/
typedef struct _pl_Vertex {
  pl_Float x, y, z;              /* Vertex coordinate (objectspace) */
  pl_Float xformedx, xformedy, xformedz;
                                 /* Transformed vertex
                                    coordinate (cameraspace) */
  pl_Float nx, ny, nz;           /* Unit vertex normal (objectspace) */
  pl_Float xformednx, xformedny, xformednz;
                                 /* Transformed unit vertex normal
                                    (cameraspace) */
} pl_Vertex;

/*
** Face
*/
typedef struct _pl_Face {
  pl_Vertex *Vertices[3];      /* Vertices of triangle */
  pl_Float nx, ny, nz;         /* Normal of triangle (object space) */
  pl_Mat *Material;            /* Material of triangle */
  pl_sInt32 Scrx[3], Scry[3];  /* Projected screen coordinates
                                  (12.20 fixed point) */
  pl_Float Scrz[3];            /* Projected 1/Z coordinates */
  pl_sInt32 MappingU[3], MappingV[3];
                               /* 16.16 Texture mapping coordinates */
  pl_sInt32 eMappingU[3], eMappingV[3];
                               /* 16.16 Environment map coordinates */
  pl_Float fShade;             /* Flat intensity */
  pl_Float sLighting;          /* Face static lighting. Should usually be 0.0 */
  pl_Float Shades[3];          /* Vertex intensity */
  pl_Float vsLighting[3];      /* Vertex static lighting. Should be 0.0 */
} pl_Face;

/*
** Object
*/
typedef struct _pl_Obj {
  pl_uInt32 NumVertices;              /* Number of vertices */
  pl_uInt32 NumFaces;                 /* Number of faces */
  pl_Vertex *Vertices;                /* Array of vertices */
  pl_Face *Faces;                     /* Array of faces */
  struct _pl_Obj *Children[PL_MAX_CHILDREN];
                                      /* Children */
  pl_Bool BackfaceCull;               /* Are backfacing polys drawn? */
  pl_Bool BackfaceIllumination;       /* Illuminated by lights behind them? */
  pl_Bool GenMatrix;                  /* Generate Matrix from the following
                                         if set */
  pl_Float Xp, Yp, Zp, Xa, Ya, Za;    /* Position and rotation of object:
                                         Note: rotations are around
                                         X then Y then Z. Measured in degrees */
  pl_Float Matrix[16];                /* Transformation matrix */
  pl_Float RotMatrix[16];             /* Rotation only matrix (for normals) */
} pl_Obj;

/*
** Spline type. See plSpline*().
*/
typedef struct _pl_Spline {
  pl_Float *keys;              /* Key data, keyWidth*numKeys */
  pl_sInt keyWidth;            /* Number of floats per key */
  pl_sInt numKeys;             /* Number of keys */
  pl_Float cont;               /* Continuity. Should be -1.0 -> 1.0 */
  pl_Float bias;               /* Bias. -1.0 -> 1.0 */
  pl_Float tens;               /* Tension. -1.0 -> 1.0 */
} pl_Spline;

/*
** Light type. See plLight*().
*/
typedef struct _pl_Light {
  pl_uChar Type;               /* Type of light: PL_LIGHT_* */
  pl_Float Xp, Yp, Zp;         /* If Type=PL_LIGHT_POINT*,
                                  this is Position (PL_LIGHT_POINT_*),
                                  otherwise if PL_LIGHT_VECTOR,
                                  Unit vector */
  pl_Float Intensity;           /* Intensity. 0.0 is off, 1.0 is full */
  pl_Float HalfDistSquared;     /* Distance squared at which
                                   PL_LIGHT_POINT_DISTANCE is 50% */
} pl_Light;

/*
** Camera Type.
*/
typedef struct _pl_Cam {
  pl_Float Fov;                  /* FOV in degrees valid range is 1-179 */
  pl_Float AspectRatio;          /* Aspect ratio (usually 1.0) */
  pl_sChar Sort;                 /* Sort polygons, -1 f-t-b, 1 b-t-f, 0 no */
  pl_Float ClipBack;             /* Far clipping ( < 0.0 is none) */
  pl_sInt ClipTop, ClipLeft;     /* Screen Clipping */
  pl_sInt ClipBottom, ClipRight;
  pl_uInt ScreenWidth, ScreenHeight; /* Screen dimensions */
  pl_sInt CenterX, CenterY;      /* Center of screen */
  pl_Float X, Y, Z;              /* Camera position in worldspace */
  pl_Float Pitch, Pan, Roll;     /* Camera angle in degrees in worldspace */
  pl_uChar *frameBuffer;         /* Framebuffer (ScreenWidth*ScreenHeight) */
  pl_ZBuffer *zBuffer;           /* Z Buffer (NULL if none) */
} pl_Cam;


extern pl_uChar plText_DefaultFont[256*16]; /* Default 8x16 font for plText* */
extern pl_uInt32 plRender_TriStats[4]; /* Three different triangle counts from
                                          the last plRender() block:
                                          0: initial tris
                                          1: tris after culling
                                          2: final polys after real clipping
                                          3: final tris after tesselation
                                       */

/******************************************************************************
** Material Functions (mat.c)
******************************************************************************/

/*
  plMatCreate() creates a material.
  Parameters:
    none
  Returns:
    a pointer to the material on success, 0 on failure
*/
pl_Mat *plMatCreate();

/*
  plMatDelete() deletes a material that was created with plMatCreate().
  Parameters:
    m: a pointer to the material to be deleted
  Returns:
    nothing
*/
void plMatDelete(pl_Mat *m);

/*
  plMatInit() initializes a material that was created with plMatCreate().
  Parameters:
    m: a pointer to the material to be intialized
  Returns:
    nothing
  Notes:
    you *must* do this before calling plMatMapToPal() or plMatMakeOptPal().
*/
void plMatInit(pl_Mat *m);

/*
  plMatMapToPal() maps a material that was created with plMatCreate() and
    initialized with plMatInit() to a palette.
  Parameters:
    mat: material to map
    pal: a 768 byte array of unsigned chars, each 3 being a rgb triplet
         (0-255, *not* the cheesy vga 0-63)
    pstart: starting offset to use colors of, usually 0
    pend: ending offset to use colors of, usually 255
  Returns:
    nothing
  Notes:
    Mapping a material with > 2000 colors can take up to a second or two.
      Be careful, and go easy on plMat.NumGradients ;)
*/
void plMatMapToPal(pl_Mat *m, pl_uChar *pal, pl_sInt pstart, pl_sInt pend);


/*
  plMatMakeOptPal() makes an almost optimal palette from materials
    created with plMatCreate() and initialized with plMatInit().
  Paramters:
    p: palette to create
    pstart: first color entry to use
    pend: last color entry to use
    materials: an array of pointers to materials to generate the palette from
    nmats: number of materials
  Returns:
    nothing
*/
void plMatMakeOptPal(pl_uChar *p, pl_sInt pstart,
                     pl_sInt pend, pl_Mat **materials, pl_sInt nmats);


/******************************************************************************
** Object Functions (obj.c)
******************************************************************************/

/*
  plObjCreate() allocates an object
  Paramters:
    np: Number of vertices in object
    nf: Number of faces in object
  Returns:
    a pointer to the object on success, 0 on failure
*/
pl_Obj *plObjCreate(pl_uInt32 np, pl_uInt32 nf);

/*
  plObjDelete() frees an object and all of it's subobjects
    that was allocated with plObjCreate();
  Paramters:
    o: object to delete
  Returns:
    nothing
*/
void plObjDelete(pl_Obj *o);

/*
  plObjClone() creates an exact but independent duplicate of an object and
    all of it's subobjects
  Paramters:
    o: the object to clone
  Returns:
    a pointer to the new object on success, 0 on failure
*/
pl_Obj *plObjClone(pl_Obj *o);

/*
  plObjScale() scales an object, and all of it's subobjects.
  Paramters:
    o: a pointer to the object to scale
    s: the scaling factor
  Returns:
    a pointer to o.
  Notes: This scales it slowly, by going through each vertex and scaling it's
    position. Avoid doing this in realtime.
*/
pl_Obj *plObjScale(pl_Obj *o, pl_Float s);

/*
  plObjStretch() stretches an object, and all of it's subobjects
  Parameters:
    o: a pointer to the object to stretch
    x,y,z: the x y and z stretch factors
  Returns:
    a pointer to o.
  Notes: same as plObjScale(). Note that the normals are preserved.
*/
pl_Obj *plObjStretch(pl_Obj *o, pl_Float x, pl_Float y, pl_Float z);

/*
   plObjTranslate() translates an object
   Parameters:
     o: a pointer to the object to translate
     x,y,z: translation in object space
   Returns:
     a pointer to o
   Notes: same has plObjScale().
*/
pl_Obj *plObjTranslate(pl_Obj *o, pl_Float x, pl_Float y, pl_Float z);

/*
  plObjFlipNormals() flips all vertex and face normals of and object
    and allo of it's subobjects.
  Parameters:
    o: a pointer to the object to flip normals of
  Returns:
    a pointer to o
  Notes:
    Not especially fast.
    A call to plObjFlipNormals() or plObjCalcNormals() will restore the normals
*/
pl_Obj *plObjFlipNormals(pl_Obj *o);

/*
  plObjSetMat() sets the material of all faces in an object.
  Paramters:
    o: the object to set the material of
    m: the material to set it to
    th: "transcend hierarchy". If set, it will set the
        material of all subobjects too.
  Returns:
    nothing
*/
void plObjSetMat(pl_Obj *o, pl_Mat *m, pl_Bool th);

/*
   plObjCalcNormals() calculates all face and vertex normals for an object
     and all subobjects.
   Paramters:
     obj: the object
   Returns:
     nothing
*/
pl_Obj *plObjCalcNormals(pl_Obj *obj);

/******************************************************************************
** Frustum Clipping Functions (clip.c)
******************************************************************************/

/*
  plClipSetFrustum() sets up the clipping frustum.
  Parameters:
    cam: a camera allocated with plCamCreate().
  Returns:
    nothing
  Notes:
    Sets up the internal structures.
    DO NOT CALL THIS ROUTINE FROM WITHIN A plRender*() block.
*/
void plClipSetFrustum(pl_Cam *cam);

/*
  plClipRenderFace() renders a face and clips it to the frustum initialized
    with plClipSetFrustum().
  Parameters:
    face: the face to render
  Returns:
    nothing
  Notes: this is used internally by plRender*(), so be careful. Kinda slow too.
*/
void plClipRenderFace(pl_Face *face);

/*
  plClipNeeded() decides whether the face is in the frustum, intersecting
    the frustum, or completely out of the frustum craeted with
    plClipSetFrustum().
  Parameters:
    face: the face to check
  Returns:
    0: the face is out of the frustum, no drawing necessary
    1: the face is intersecting the frustum, splitting and drawing necessary
  Notes: this is used internally by plRender*(), so be careful. Kinda slow too.
*/
pl_sInt plClipNeeded(pl_Face *face);

/******************************************************************************
** Light Handling Routines (light.c)
******************************************************************************/

/*
  plLightCreate() creates a new light
  Parameters:
    none
  Returns:
    a pointer to the light
*/
pl_Light *plLightCreate();

/*
  plLightSet() sets up a light allocated with plLightCreate()
  Parameters:
    light: the light to set up
    mode: the mode of the light (PL_LIGHT_*)
    x,y,z: either the position of the light (PL_LIGHT_POINT*) or the angle
           in degrees of the light (PL_LIGHT_VECTOR)
    intensity: the intensity of the light (0.0-1.0)
    halfDist: the distance at which PL_LIGHT_POINT_DISTANCE is 1/2 intensity
  Returns:
    a pointer to light.
*/
pl_Light *plLightSet(pl_Light *light, pl_uChar mode, pl_Float x, pl_Float y,
                     pl_Float z, pl_Float intensity, pl_Float halfDist);

/*
  WRITEME
*/
pl_Light *plLightNew(pl_uChar mode, pl_Float x, pl_Float y, pl_Float z,
                     pl_Float intensity, pl_Float halfDist);

/*
  plLightDelete() frees a light allocated with plLightCreate().
  Paramters:
    l: light to delete
  Returns:
    nothing
*/
void plLightDelete(pl_Light *l);

/* PUT ME SOMEWHERE */
/*
** plTexDelete() frees all memory associated with "t"
*/
void plTexDelete(pl_Texture *t);


/******************************************************************************
** Camera Handling Routines (cam.c)
******************************************************************************/

/*
  plCamCreate() allocates a new camera
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
pl_Cam *plCamCreate(pl_uInt sw, pl_uInt sh, pl_Float ar, pl_Float fov,
                    pl_uChar *fb, pl_ZBuffer *zb);

/*
  plCamSetTarget() sets the target of a camera allocated with plCamCreate().
  Parameters:
    c: the camera to set the target of
    x,y,z: the worldspace coordinate of the target
  Returns:
    nothing
  Notes:
    Sets the pitch and pan of the camera. Does not touch the roll.
*/
void plCamSetTarget(pl_Cam *c, pl_Float x, pl_Float y, pl_Float z);

/*
   plCamDelete() frees all memory associated with a camera excluding
     framebuffers and Z buffers
   Paramters:
     c: camera to free
   Returns:
     nothing
*/
void plCamDelete(pl_Cam *c);

/******************************************************************************
** Easy Rendering Interface (render.c)
******************************************************************************/

/*
 plRenderBegin() begins the rendering process.
   Parameters:
     Camera: camera to use for rendering
   Returns:
     nothing
   Notes:
     Only one rendering process can occur at a time.
     Uses plClip*(), so don't use them within or around a plRender() block.
*/
void plRenderBegin(pl_Cam *Camera);

/*
   plRenderLight() adds a light to the scene.
   Parameters:
     light: light to add to scene
   Returns:
     nothing
   Notes: Any objects rendered before will be unaffected by this.
*/
void plRenderLight(pl_Light *light);

/*
   plRenderObj() adds an object and all of it's subobjects to the scene.
   Parameters:
     obj: object to render
   Returns:
     nothing
   Notes: if Camera->Sort is zero, objects are rendered in the order that
     they are added to the scene.
*/
void plRenderObj(pl_Obj *obj);

/*
   plRenderEnd() actually does the rendering, and closes the rendering process
   Paramters:
     none
   Returns:
     nothing
*/
void plRenderEnd();

/******************************************************************************
** Object Primitives Code (make.c)
******************************************************************************/

/*
  plMakePlane() makes a plane centered at the origin facing up the y axis.
  Parameters:
    w: width of the plane (along the x axis)
    d: depth of the plane (along the z axis)
    res: resolution of plane, i.e. subdivisions
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Obj *plMakePlane(pl_Float w, pl_Float d, pl_uInt res, pl_Mat *m);

/*
  plMakeBox() makes a box centered at the origin
  Parameters:
    w: width of the box (x axis)
    d: depth of the box (z axis)
    h: height of the box (y axis)
  Returns:
    pointer to object created.
*/
pl_Obj *plMakeBox(pl_Float w, pl_Float d, pl_Float h, pl_Mat *m);

/*
  plMakeCone() makes a cone centered at the origin
  Parameters:
    r: radius of the cone (x-z axis)
    h: height of the cone (y axis)
    div: division of cone (>=3)
    cap: close the big end?
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Obj *plMakeCone(pl_Float r, pl_Float h, pl_uInt div, pl_Bool cap, pl_Mat *m);

/*
  plMakeCylinder() makes a cylinder centered at the origin
  Parameters:
    r: radius of the cylinder (x-z axis)
    h: height of the cylinder (y axis)
    divr: division of of cylinder (around the circle) (>=3)
    captop: close the top
    capbottom: close the bottom
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Obj *plMakeCylinder(pl_Float r, pl_Float h, pl_uInt divr, pl_Bool captop,
                       pl_Bool capbottom, pl_Mat *m);

/*
  plMakeSphere() makes a sphere centered at the origin.
  Parameters:
    r: radius of the sphere
    divr: division of the sphere (around the y axis) (>=3)
    divh: division of the sphere (around the x,z axis) (>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Obj *plMakeSphere(pl_Float r, pl_uInt divr, pl_uInt divh, pl_Mat *m);

/*
  plMakeTorus() makes a torus centered at the origin
  Parameters:
    r1: inner radius of the torus
    r2: outer radius of the torus
    divrot: division of the torus (around the y axis) (>=3)
    divrad: division of the radius of the torus (x>=3)
    m: material to use
  Returns:
    pointer to object created.
*/
pl_Obj *plMakeTorus(pl_Float r1, pl_Float r2, pl_uInt divrot,
                    pl_uInt divrad, pl_Mat *m);

/******************************************************************************
** File Readers (read_*.c)
******************************************************************************/

/*
  plRead3DSObj() reads a 3DS object
  Parameters:
    fn: filename of object to read
    m: material to assign it
  Returns:
    pointer to object
  Notes:
    This reader organizes multiple objects like so:
      1) the first object is returned
      2) the second object is the first's first child
      3) the third object is the second's first child
      4) etc
*/
pl_Obj *plRead3DSObj(char *fn, pl_Mat *m);

/*
  plReadCOBObj() reads an ascii .COB object
  Parameters:
    fn: filename of object to read
    mat: material to assign it
  Returns:
    pointer to object
  Notes:
    This is Caligari's ASCII object format.
    This reader doesn't handle multiple objects. It just reads the first one.
    Polygons with lots of sides are not always tesselated correctly. Just
      use the "Tesselate" button from within truespace to improve the results.
*/
pl_Obj *plReadCOBObj(char *fn, pl_Mat *mat);

/*
  plReadJAWObj() reads a .JAW object.
  Parameters:
    fn: filename of object to read
    m: material to assign it
  Returns:
    pointer to object
  Notes:
    For information on the .JAW format, please see the jaw3D homepage,
      http://www.tc.umn.edu/nlhome/g346/kari0022/jaw3d/
*/
pl_Obj *plReadJAWObj(char *fn, pl_Mat *m);

/*
  plReadPCXTex() reads a 8bpp PCX texture
  Parameters:
    fn: filename of texture to read
    rescale: will rescale image if not whole log2 dimensions (USE THIS)
    optimize: will optimize colors (USE THIS TOO)
  Returns:
    pointer to texture
  Notes:
    The PCX must be a 8bpp zSoft version 5 PCX. The texture's palette will
      be optimized, and the texture might be scaled up so that it's dimensions
      will be a nice power of two.
*/
pl_Texture *plReadPCXTex(char *fn, pl_Bool rescale, pl_Bool optimize);

/******************************************************************************
** Math Code (math.c)
******************************************************************************/

/*
  plMatrixRotate() generates a rotation matrix
  Parameters:
    matrix: an array of 16 pl_Floats that is a 4x4 matrix
    m: the axis to rotate around, 1=X, 2=Y, 3=Z.
    Deg: the angle in degrees to rotate
  Returns:
    nothing
*/
void plMatrixRotate(pl_Float matrix[], pl_uChar m, pl_Float Deg);

/*
  plMatrixTranslate() generates a translation matrix
  Parameters:
    m: the matrix (see plMatrixRotate for more info)
    x,y,z: the translation coordinates
  Returns:
    nothing
*/
void plMatrixTranslate(pl_Float m[], pl_Float x, pl_Float y, pl_Float z);

/*
  plMatrixMultiply() multiplies two matrices
  Parameters:
    dest: destination matrix will be multipled by src
    src: source matrix
  Returns:
    nothing
  Notes:
    this is the same as dest = dest*src (since the order *does* matter);
*/
void plMatrixMultiply(pl_Float *dest, pl_Float src[]);

/*
   plMatrixApply() applies a matrix.
  Parameters:
    m: matrix to apply
    x,y,z: input coordinate
    outx,outy,outz: pointers to output coords.
  Returns:
    nothing
  Notes:
    applies the matrix to the 3d point to produce the transformed 3d point
*/
void plMatrixApply(pl_Float *m, pl_Float x, pl_Float y, pl_Float z,
                   pl_Float *outx, pl_Float *outy, pl_Float *outz);

/*
  plNormalizeVector() makes a vector a unit vector
  Parameters:
    x,y,z: pointers to the vector
  Returns:
    nothing
*/
void plNormalizeVector(pl_Float *x, pl_Float *y, pl_Float *z);

/*
  plDotProduct() returns the dot product of two vectors
  Parameters:
    x1,y1,z1: the first vector
    x2,y2,z2: the second vector
  Returns:
    the dot product of the two vectors
*/
pl_Float plDotProduct(pl_Float x1, pl_Float y1, pl_Float z1,
                      pl_Float x2, pl_Float y2, pl_Float z2);

/******************************************************************************
** Spline Interpolation (spline.c)
******************************************************************************/

/*
  plSplineInit() initializes a spline
  Parameters:
    s: the spline
  Returns:
    nothing
  Notes:
    Intializes the spline. Do this once, or when you change any of the settings
*/
void plSplineInit(pl_Spline *s);

/*
  plSplineGetPoint() gets a point on the spline
  Parameters:
    s: spline
    frame: time into spline. 0.0 is start, 1.0 is second key point, etc.
    out: a pointer to an array of s->keyWidth floats that will be filled in.
  Returns:
    nothing
*/
void plSplineGetPoint(pl_Spline *s, pl_Float frame, pl_Float *out);

/******************************************************************************
** 8xX  Bitmapped Text
******************************************************************************/
/*
  plTextSetFont() sets the font to be used by the plText*() functions.
    Parameters:
      font: a pointer to a 8xX bitmapped font
      height: the height of the font (X)
    Returns:
      nothing
*/

void plTextSetFont(pl_uChar *font, pl_uChar height);

/*
  plTextPutChar() puts a character to a camera
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    c: the character to put. Special characters such as '\n' aren't handled.
  Returns:
    nothing
*/

void plTextPutChar(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                   pl_uChar color, pl_uChar c);

/*
  plTextPutString() puts an array of characters to a camera
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    string:
      the characters to put. '\n' and '\t' are handled as one would expect
  Returns:
    nothing
*/
void plTextPutStr(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                  pl_uChar color, pl_sChar *string);

/*
  plTextPrintf() is printf() for graphics
  Parameters:
    cam: The camera. If the camera has a zBuffer, it will be used.
    x: the x screen position of the left of the text
    y: the y screen position of the top of the text
    z: the depth of the text (used when cam->zBuffer is set)
    color: the color to make the text
    format:
      the characters to put, with printf() formatting codes.
      '\n' and '\t' are handled as one would expect
    ...: any additional parameters specified by format
  Returns:
    nothing
*/
void plTextPrintf(pl_Cam *cam, pl_sInt x, pl_sInt y, pl_Float z,
                  pl_uChar color, pl_sChar *format, ...);

/******************************************************************************
** Built-in Rasterizers
******************************************************************************/

void plPF_SolidF(pl_Cam *, pl_Face *);
void plPF_SolidG(pl_Cam *, pl_Face *);
void plPF_TexF(pl_Cam *, pl_Face *);
void plPF_TexG(pl_Cam *, pl_Face *);
void plPF_TexEnv(pl_Cam *, pl_Face *);
void plPF_PTexF(pl_Cam *, pl_Face *);
void plPF_PTexG(pl_Cam *, pl_Face *);
void plPF_TransF(pl_Cam *, pl_Face *);
void plPF_TransG(pl_Cam *, pl_Face *);

#ifdef __cplusplus
}
#endif

#endif /* !_PLUSH_H_ */
