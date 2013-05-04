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

/* 
  Worldspace is in a unusual coordinate system.
  For example, if the camera is at the origin and not rotated,
  X is positive to the right,
  Y is positive upward, and
  Z is positive going into the screen.
  I.e. behind the camera is negative Z. 
*/

/******************************************************************************/

enum {
    /* Maximum children per object */
    EON_MAX_CHILDREN = 4096,
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
typedef double EON_Double;        /* Double-precision floating point */
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
typedef uint8_t EON_Byte;         /* generic binary data */

#define EON_ZERO 0.0000001
#define EON_PI   3.14159265359

/* Utility min() and max() functions */
#define EON_Min(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define EON_Max(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))
#define EON_Clamp(x,m,M) ( EON_Min(EON_Max(( x ), ( m )), ( M )) )

/*
** Shade modes. Used with EON_Mat.ShadeType
** Note that (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE) and
** (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
enum {
    EON_SHADE_NONE             = 1,
    EON_SHADE_FLAT             = 2,
    EON_SHADE_FLAT_DISTANCE    = 4,
    EON_SHADE_GOURAUD          = 8,
    EON_SHADE_GOURAUD_DISTANCE = 16,
    EON_SHADE_WIREFRAME        = 32
};

/*
** Light modes. Used with EON_Light.Type or EON_LightSet().
** Note that EON_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, EON_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see EON_LightSet() for setting it), EON_LIGHT_POINT does both.
*/
enum {
    EON_LIGHT_NONE           =  0x0,
    EON_LIGHT_VECTOR         =  0x1,
    EON_LIGHT_POINT          = (0x2|0x4),
    EON_LIGHT_POINT_DISTANCE =  0x2,
    EON_LIGHT_POINT_ANGLE    =  0x4
};

typedef struct _EON_ScrPoint {
    /* Projected screen coordinates (12.20 fixed point) */
    EON_sInt32 X;
    EON_sInt32 Y;
    EON_Float Z; /* 1/Z coordinates */
} EON_ScrPoint;

typedef struct _EON_3DPoint {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_3DPoint;

typedef struct _EON_Color {
    EON_Byte R;
    EON_Byte G;
    EON_Byte B;
    EON_Byte A;
} EON_Color;

typedef struct _EON_Frame {
    EON_Byte *Data;
    EON_ZBuffer *ZBuffer;
    EON_uInt32 Width;
    EON_uInt32 Height;
    EON_uInt32 Bpp; /* Bytes Per Pixel */
} EON_Frame;

/* Forward declarations needed for _PutFace */
typedef struct _EON_Face EON_Face;
typedef struct _EON_Cam EON_Cam;

/*
** Texture type. Read textures with EON_ReadPCXTex(), and assign them to
** EON_Mat.Texture.
*/
typedef struct _EON_Texture {
    EON_Byte *Data;             /* Texture data */
    EON_Byte *PaletteData;      /* Palette data (NumColors bytes) */
    EON_uInt16 Width, Height;   /* Log2 of dimensions */
    EON_uInt iWidth, iHeight;   /* Integer dimensions */
    EON_Float uScale, vScale;   /* Scaling (usually 2**Width, 2**Height) */
    EON_uInt NumColors;         /* Number of colors used in texture */
} EON_Texture;

/*
** Material type. Create materials with EON_MatCreate().
*/
typedef struct _EON_Mat {
    EON_Color Ambient;
    EON_Color Diffuse;
    EON_Color Specular;
    EON_uInt Shininess;           /* Shininess of material. 1 is dullest */
    EON_Float FadeDist;           /* For distance fading, distance at
                                     which intensity is 0 */
    EON_uInt16 ShadeType;         /* Shade type: EON_SHADE_* */
    EON_uInt16 PerspectiveCorrect;/* Correct textures every n pixels */
    EON_Texture *Texture;         /* Texture map (see EON_Texture) above */
    EON_Float TexScaling;         /* Texture map scaling */
    EON_Bool zBufferable;         /* Can this material be zbuffered? */
    /* The following are used mostly internally */
    EON_uInt16 _st, _ft;          /* The shadetype and filltype */
    void (*_PutFace)(EON_Cam *, EON_Face *, EON_Frame *);
    /* Renders the triangle with this material */
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
struct _EON_Face {
    EON_Vertex *Vertices[3];    /* Vertices of triangle */
    EON_ScrPoint Scr[3];
    EON_Float nx, ny, nz;       /* Normal of triangle (object space) */
    EON_Mat *Material;          /* Material of triangle */
    EON_Float Shades[3];        /* Vertex intensity */
    EON_Float vsLighting[3];    /* Vertex static lighting. Should be 0.0 */
    /* 16.16 Texture mapping coordinates */
    EON_sInt32 MappingU[3];
    EON_sInt32 MappingV[3];
};

/*
** Object
*/
typedef struct _EON_Obj {
    EON_uInt32 NumVertices;         /* Number of vertices */
    EON_uInt32 NumFaces;            /* Number of faces */
    EON_Vertex *Vertices;           /* Array of vertices */
    EON_Face *Faces;                /* Array of faces */
    EON_Bool BackfaceCull;          /* Are backfacing polys drawn? */
    EON_Bool BackfaceIllumination;  /* Illuminated by lights behind them? */
    EON_Bool GenMatrix;             /* Generate Matrix from the following
                                       if set */
    EON_Float Xp, Yp, Zp;           /* Position of the object */
    EON_Float Xa, Ya, Za;           /* Rotation of object:
                                       Note: rotations are around
                                       X then Y then Z. Measured in degrees */
    EON_Float Pad;                  /* Padding */
    EON_Float Matrix[16];           /* Transformation matrix */
    EON_Float RotMatrix[16];        /* Rotation only matrix (for normals) */
    struct _EON_Obj *Children[EON_MAX_CHILDREN];
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
    EON_uInt Type;                /* Type of light: EON_LIGHT_* */
    EON_3DPoint Pos;              /* If Type=EON_LIGHT_POINT*,
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
struct _EON_Cam {
    EON_Float Fov;                  /* FOV in degrees valid range is 1-179 */
    EON_Float AspectRatio;          /* Aspect ratio (usually 1.0) */
    EON_Float ClipBack;             /* Far clipping ( < 0.0 is none) */
    EON_sInt ClipTop, ClipLeft;     /* Screen Clipping */
    EON_sInt ClipBottom, ClipRight;
    EON_uInt16 ScreenWidth, ScreenHeight; /* Screen dimensions */
    EON_sInt CenterX, CenterY;      /* Center of screen */
    EON_3DPoint Pos;                /* Camera position in worldspace */
    EON_Float Pitch, Pan, Roll;     /* Camera angle in degrees in worldspace */
};

enum {
    TRI_STAT_INITIAL = 0,
    TRI_STAT_CULLING,       /* after the culling */
    TRI_STAT_CLIPPING,      /* after the real clipping */
    TRI_STAT_TESSELLATION,  /* after the tessellation */
    TRI_STAT_NUM            /* MUST be the last one! */
};

typedef struct _EON_RenderInfo {
    EON_uInt32 TriStats[TRI_STAT_NUM];
} EON_RenderInfo;

enum {
    NUM_CLIP_PLANES = 5
};

typedef struct _EON_ClipInfo {
    EON_Vertex newVertices[8];
    EON_Double Shades[8];
    EON_Double MappingU[8];
    EON_Double MappingV[8];
    EON_Double eMappingU[8];
    EON_Double eMappingV[8];
} EON_ClipInfo;

typedef struct _EON_Clip {
    EON_RenderInfo *Info;
    EON_ClipInfo CL[2];
    EON_Double ClipPlanes[NUM_CLIP_PLANES][4];
    EON_Cam *Cam;
    EON_sInt32 Cx;
    EON_sInt32 Cy;
    EON_Double Fov;
    EON_Double AdjAsp;
} EON_Clip;

typedef struct _EON_FaceInfo {
    EON_Face *face;
    EON_Float zd;
} EON_FaceInfo;

typedef struct _EON_LightInfo {
    EON_Light *light;
    EON_3DPoint pos;
} EON_LightInfo;

typedef struct _EON_Rend {
    EON_RenderInfo Info;
    EON_Clip Clip;
    EON_Float CMatrix[16];
    EON_uInt32 NumFaces;
    EON_FaceInfo Faces[EON_MAX_TRIANGLES];
    EON_uInt32 NumLights;
    EON_LightInfo Lights[EON_MAX_LIGHTS];
    EON_Cam *Cam;
} EON_Rend;


/******************************************************************************
** Frame Functions
******************************************************************************/
/*
  EON_FrameCreate() creates a frame with the specified dimensions
  Parameters:
    Width: the frame width (pixels).
    Height: the frame height (pixels).
    Bpp: Bytes per pixel. How many bytes takes a pixel?
  Returns:
    a pointer to the frame on success, 0 on failure
*/
EON_Frame *EON_FrameCreate(EON_uInt32 Width, EON_uInt32 Height,
                           EON_uInt32 Bpp);

/*
  EON_FrameDelete() deletes a frame that was created with EON_FrameCreate().
  Parameters:
    f: a pointer to the frame to be deleted
  Returns:
    nothing
*/
void EON_FrameDelete(EON_Frame *f);

/*
  EON_FrameSize() tells how much storage space in bytes takes a given frame.
  Parameters:
    f: the frame to be inspected
  Returns:
    nothing
*/
EON_uInt32 EON_FrameSize(EON_Frame *f);

/*
  EON_FrameClear() `blanks' a given frame.
  Parameters:
    f: the frame to be blanked (cleared)
  Returns:
    nothing
*/
void EON_FrameClear(EON_Frame *f);

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
    you *must* call this before calling any rendering function.
*/
void EON_MatInit(EON_Mat *m);

/*
  EON_MatInfo() dumps the internal informations about a Material to a
    logger. See cxkit/logkit.h for details.
    The logger datatype is void* to remove a header dependency.
  Parameters:
    m: material to be inspected
    logger: logger instance to be used
  Returns:
    nothing
*/
void EON_MatInfo(EON_Mat *m, void *logger);

/******************************************************************************
** Object Functions (obj.c)
******************************************************************************/

/*
  EON_ObjCreate() allocates an object
  Parameters:
    np: Number of vertices in object
    nf: Number of faces in object
  Returns:
    a pointer to the object on success, 0 on failure
*/
EON_Obj *EON_ObjCreate(EON_uInt32 np, EON_uInt32 nf);

/*
  EON_ObjDelete() frees an object and all of it's subobjects
    that was allocated with EON_ObjCreate();
  Parameters:
    o: object to delete
  Returns:
    nothing
*/
void EON_ObjDelete(EON_Obj *o);

/*
  EON_ObjClone() creates an exact but independent duEON_icate of an object and
    all of it's subobjects
  Parameters:
    o: the object to clone
  Returns:
    a pointer to the new object on success, 0 on failure
*/
EON_Obj *EON_ObjClone(EON_Obj *o);

/*
  EON_ObjScale() scales an object, and all of it's subobjects.
  Parameters:
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
  EON_ObjCentroid() calculates the centroid of a given object
  Parameters:
    o: a pointer to the object to calculate the centroid for
    x,y,z: pointer to the output coordinates.
  Returns:
    nothing
*/
void EON_ObjCentroid(EON_Obj *o, EON_Float *x, EON_Float *y, EON_Float *z);

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
  Parameters:
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
   Parameters:
     obj: the object
   Returns:
     nothing
*/
EON_Obj *EON_ObjCalcNormals(EON_Obj *obj);

/*
  EON_ObjInfo() dumps the internal informations about an Object to a
    logger. See cxkit/logkit.h for details.
    The logger datatype is void* to remove a header dependency.
  Parameters:
    o: object to be inspected
    logger: logger instance to be used
  Returns:
    nothing
*/
void EON_ObjInfo(EON_Obj *o, void *logger);

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
  Parameters:
    l: light to delete
  Returns:
    nothing
*/
void EON_LightDelete(EON_Light *l);

/*
  EON_TexDelete() frees all memory associated with "t"
*/
void EON_TexDelete(EON_Texture *t);

void EON_TexInfo(EON_Texture *t, void *logger);

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
  Returns:
    a pointer to the newly allocated camera
*/
EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov);

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
   Parameters:
     c: camera to free
   Returns:
     nothing
*/
void EON_CamDelete(EON_Cam *c);

/******************************************************************************
** Easy Rendering Interface (render.c)
******************************************************************************/

/*
  EON_RendCreate() allocates a new renderer context.
  Parameters:
    Camera: an already initialized and ready EON_Cam.
  Returns:
    a pointer to the newly allocated context.
*/
EON_Rend *EON_RendCreate(EON_Cam *Camera);

/*
   EON_RendDelete() frees all memory associated with a rendering context.
   Parameters:
     rend: context to free
   Returns:
     nothing
*/
void EON_RendDelete(EON_Rend *rend);

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
void EON_RenderBegin(EON_Rend *rend);

/*
   EON_RenderLight() adds a light to the scene.
   Parameters:
     light: light to add to scene
   Returns:
     nothing
   Notes: Any objects rendered before will be unaffected by this.
*/
void EON_RenderLight(EON_Rend *rend, EON_Light *light);

/*
   EON_RenderObj() adds an object and all of it's subobjects to the scene.
   Parameters:
     obj: object to render
   Returns:
     nothing
*/
void EON_RenderObj(EON_Rend *rend, EON_Obj *obj);

/*
   EON_RenderEnd() actually does the rendering, and closes the rendering process
   Parameters:
     none
   Returns:
     nothing
*/
void EON_RenderEnd(EON_Rend *rend, EON_Frame *frame);


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

/*************************************************************************/

#endif /* ! EON3D_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

