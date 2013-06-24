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

/** 
 * Worldspace is in a unusual coordinate system.
 * For example, if the camera is at the origin and not rotated,
 * X is positive to the right,
 * Y is positive upward, and
 * Z is positive going into the screen.
 * I.e. behind the camera is negative Z. 
 */

/******************************************************************************/

enum {
    /** Maximum lights per scene -- if you exceed this, they will be ignored */
    EON_MAX_LIGHTS = 32,
    /** Maximum number of triangles per scene -- if you exceed this, entire
     *  objects will be ignored. You can increase this if you need it. It takes
     *  approximately 8*EON_MAX_TRIANGLES bytes of memory. i.e. the default of
     *  16384 consumes 128kbytes of memory. not really a big deal,
     */
    EON_MAX_TRIANGLES = 1048576
};

typedef float EON_ZBuffer;        //!< z-buffer type (must be float)
typedef float EON_Float;          //!< General floating point
typedef double EON_Double;        //!< Double-precision floating point
typedef float EON_IEEEFloat32;    //!< IEEE 32 bit floating point
typedef int32_t EON_sInt32;       //!< signed 32 bit integer
typedef uint32_t EON_uInt32;      //!< unsigned 32 bit integer
typedef int16_t EON_sInt16;       //!< signed 16 bit integer
typedef uint16_t EON_uInt16;      //!< unsigned 16 bit integer
typedef signed int EON_sInt;      //!< signed optimal integer
typedef unsigned int EON_uInt;    //!< unsigned optimal integer
typedef int EON_Bool;             //!< boolean
typedef uint8_t EON_uChar;        //!< unsigned 8 bit integer
typedef int8_t EON_sChar;         //!< signed 8 bit integer
typedef uint8_t EON_Byte;         //!< generic binary data

#define EON_ZERO 0.00000000001
#define EON_PI   3.14159265359

#define EON_Min(x,y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define EON_Max(x,y) (( ( x ) < ( y ) ? ( y ) : ( x )))
#define EON_Clamp(x,m,M) ( EON_Min(EON_Max(( x ), ( m )), ( M )) )

/**
 * shade modes.
 * @see EON_Mat
 * @note (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE) and
 * (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE) are valid shading modes.
 */
enum {
    EON_SHADE_NONE             = 1,
    EON_SHADE_FLAT             = 2,
    EON_SHADE_FLAT_DISTANCE    = 4,
    EON_SHADE_GOURAUD          = 8,
    EON_SHADE_GOURAUD_DISTANCE = 16,
    EON_SHADE_WIREFRAME        = 32
};

/**
 * light modes.
 * @see EON_Light
 * @see EON_LightSet
 * @note EON_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
 * the light and the point, EON_LIGHT_POINT_DISTANCE has falloff with proportion
 * to distance**2 (see EON_LightSet() for setting it), EON_LIGHT_POINT does both.
 */
enum {
    EON_LIGHT_NONE           =  0x0,
    EON_LIGHT_VECTOR         =  0x1,
    EON_LIGHT_POINT          = (0x2|0x4),
    EON_LIGHT_POINT_DISTANCE =  0x2,
    EON_LIGHT_POINT_ANGLE    =  0x4
};

/** represent a point on the 2D screen. */
typedef struct _EON_ScrPoint {
    EON_sInt32 X; //!< Projected screen coordinates (12.20 fixed point)
    EON_sInt32 Y; //!< Projected screen coordinates (12.20 fixed point)
    EON_Float Z;  //!< 1/Z coordinates
} EON_ScrPoint;

/** represent a point into the 3D space. */
typedef struct _EON_3DPoint {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_3DPoint;

/** RGBA32 color. 8 bits per channel. */
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
    EON_uInt32 Bpp; //!< Bytes Per Pixel
} EON_Frame;

/* Forward declarations needed for _PutFace */
typedef struct _EON_Face EON_Face;
typedef struct _EON_Cam EON_Cam;

/**
 * texture type.
 * Read textures and assign them to EON_Mat.Texture.
 * @see EON_ReadPCXTex
 */
typedef struct _EON_Texture {
    EON_Byte *Data;             //!< Texture data
    EON_Byte *PaletteData;      //!< Palette data (NumColors bytes)
    EON_uInt16 Width, Height;   //!< Log2 of dimensions
    EON_uInt iWidth, iHeight;   //!< Integer dimensions
    EON_Float uScale, vScale;   //!< Scaling (usually 2**Width, 2**Height)
    EON_uInt NumColors;         //!< Number of colors used in texture
} EON_Texture;

/**
 * material type.
 * @see EON_MatCreate.
 */
typedef struct _EON_Mat {
    EON_Color Ambient;
    EON_Color Diffuse;
    EON_Color Specular;
    EON_uInt Shininess;           //!< Shininess of material. 1 is dullest
    EON_Float FadeDist;           //!< For distance fading, distance at
                                  //!<  which intensity is 0
    EON_uInt16 ShadeType;         //!< Shade type: see EON_SHADE_*
    EON_uInt16 PerspectiveCorrect;//!< Correct textures every n pixels
    EON_Texture *Texture;         //!< Texture map (see EON_Texture) above
    EON_Float TexScaling;         //!< Texture map scaling
    EON_Bool zBufferable;         //!< Can this material be zbuffered?
    /* The following are used mostly internally */
    EON_uInt16 _st, _ft;          //!< The shadetype and filltype
    /** Renders the triangle with this material */
    void (*_PutFace)(EON_Cam *, EON_Face *, EON_Frame *);
} EON_Mat;

/**
 * Vertex, used within objects.
 * @see EON_Obj.
 */
typedef struct _EON_Vertex {
    EON_Float x, y, z;      //!< Vertex coordinate (objectspace)
    EON_Float nx, ny, nz;   //!< Unit vertex normal (objectspace)
    /** Transformed vertex coordinate (cameraspace) */
    EON_Float xformedx, xformedy, xformedz;
    /** Transformed unit vertex normal (cameraspace) */
    EON_Float xformednx, xformedny, xformednz;
} EON_Vertex;

/**
 * face
 */
struct _EON_Face {
    EON_Vertex *Vertices[3];    //!< vertices of triangle.
    EON_ScrPoint Scr[3];        //!< screen coordinates of the vertices.
    EON_Float nx, ny, nz;       //!< Normal of triangle (object space)
    EON_Mat *Material;          //!< Material of triangle
    EON_Float Shades[3];        //!< Vertex intensity
    EON_Float vsLighting[3];    //!< Vertex static lighting. Should be 0.0
    /** 16.16 Texture mapping coordinates */
    EON_sInt32 MappingU[3];
    EON_sInt32 MappingV[3];
};

/*
 * object
 */
typedef struct _EON_Obj {
    EON_uInt32 NumVertices;         //!< Number of vertices
    EON_uInt32 NumFaces;            //!< Number of faces
    EON_Vertex *Vertices;           //!< Array of vertices
    EON_Face *Faces;                //!< Array of faces
    EON_Bool BackfaceCull;          //!< Are backfacing polys drawn?
    EON_Bool BackfaceIllumination;  //!< Illuminated by lights behind them?
    EON_Bool GenMatrix;             //!< Generate Matrix from the following
                                    //!<  if set
    EON_Float Xp, Yp, Zp;           //!< Position of the object
    EON_Float Xa, Ya, Za;           //!< Rotation of object:
                                    //!<  Note: rotations are around
                                    //!<  X then Y then Z. Measured in degrees
    EON_Float Pad;                  //!< Padding
    EON_Float Matrix[16];           //!< Transformation matrix
    EON_Float RotMatrix[16];        //!< Rotation only matrix (for normals)
} EON_Obj;

/**
 * light type.
 */
typedef struct _EON_Light {
    EON_uInt Type;                //!< Type of light: EON_LIGHT_*
    EON_3DPoint Pos;              //!< If Type=EON_LIGHT_POINT*,
                                  //!<  this is Position (EON_LIGHT_POINT_*),
                                  //!<  otherwise if EON_LIGHT_VECTOR,
                                  //!<  Unit vector
    EON_Float Intensity;          //!< Intensity. 0.0 is off, 1.0 is full
    EON_Float HalfDistSquared;    //!< Distance squared at which
                                  //!< EON_LIGHT_POINT_DISTANCE is 50%
} EON_Light;

/**
 * camera type.
 */
struct _EON_Cam {
    EON_Float Fov;                  //!< FOV in degrees valid range is 1-179
    EON_Float AspectRatio;          //!< Aspect ratio (usually 1.0)
    EON_Float ClipBack;             //!< Far clipping ( < 0.0 is none)
    EON_sInt ClipTop, ClipLeft;     //!< Screen Clipping
    EON_sInt ClipBottom, ClipRight; //!< Screen Clipping
    EON_uInt16 ScreenWidth, ScreenHeight; //!< Screen dimensions
    EON_sInt CenterX, CenterY;      //!< Center of screen
    EON_3DPoint Pos;                //!< Camera position in worldspace
    EON_Float Pitch, Pan, Roll;     //!< Camera angle in degrees in worldspace
};

enum {
    TRI_STAT_INITIAL = 0,
    TRI_STAT_CULLING,       //!< after the culling
    TRI_STAT_CLIPPING,      //!< after the real clipping
    TRI_STAT_TESSELLATION,  //!< after the tessellation
    TRI_STAT_NUM            //!< MUST be the last one!
};

typedef struct _EON_RenderInfo {
    EON_uInt32 TriStats[TRI_STAT_NUM]; //!< triangle count
} EON_RenderInfo;

enum {
    NUM_CLIP_PLANES = 5
};

typedef struct _EON_ClipInfo {
    EON_Vertex newVertices[8];
    EON_Double Shades[8];
    EON_Double MappingU[8];
    EON_Double MappingV[8];
} EON_ClipInfo;

typedef struct _EON_Clip {
    EON_RenderInfo *Info;
    EON_Double ClipPlanes[NUM_CLIP_PLANES][4];
    EON_Cam *Cam;
    EON_sInt32 Cx;      //!< Screen center (x)
    EON_sInt32 Cy;      //!< Screen center (y)
    EON_Double Fov;     //!< Field Of View
    EON_Double AdjAsp;  //!< Adjustment of Aspect Ratio
} EON_Clip;

typedef struct _EON_FaceInfo {
    EON_Face *face;
    EON_Float zd;   //!< Z distance
} EON_FaceInfo;

typedef struct _EON_LightInfo {
    EON_Light *light;
    EON_3DPoint pos;
} EON_LightInfo;

/**
 * carefully reordered to exploit cachelines.
 * remember to check with pahole after any change.
 */
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

/**
 * creates a frame with the specified dimensions.
 * @param Width the frame width (pixels).
 * @param Height the frame height (pixels).
 * @param Bpp Bytes per pixel. How many bytes takes a pixel?
 * @return a pointer to the frame on success, NULL on failure.
 */
EON_Frame *EON_FrameCreate(EON_uInt32 Width, EON_uInt32 Height,
                           EON_uInt32 Bpp);

/**
 * deletes a frame.
 * @see EON_FrameCreate.
 * @param f a pointer to the frame to be deleted.
 */
void EON_FrameDelete(EON_Frame *f);

/**
 * tells how much storage space in bytes takes a given frame.
 * @param f the frame to be inspected.
 * @return the size in bytes of the argument frame.
 */
EON_uInt32 EON_FrameSize(EON_Frame *f);

/**
 * blanks a given frame.
 * @param f the frame to be blanked (cleared).
 */
void EON_FrameClear(EON_Frame *f);

/******************************************************************************
** Material Functions (mat.c)
******************************************************************************/

/**
 * creates a material.
 * @return a pointer to the material on success, NULL on failure.
 */
EON_Mat *EON_MatCreate();

/**
 * deletes a material.
 * @see EON_MatCreate.
 * @param m a pointer to the material to be deleted.
 */
void EON_MatDelete(EON_Mat *m);

/**
 * initializes a material after the parameters are been (re)set.
 * @note you *must* call this before calling any rendering function.
 * @see EON_MatCreate.
 * @param m a pointer to the material to be intialized.
 */
void EON_MatInit(EON_Mat *m);

/**
 * dumps the internal informations about a Material to a logger.
 * @see cxkit/logkit.h for details.
 * @note the logger datatype is void* to remove a header dependency.
 * @param m material to be inspected.
 * @param logger logger instance to be used.
 */
void EON_MatInfo(EON_Mat *m, void *logger);

/******************************************************************************
** Object Functions (obj.c)
******************************************************************************/

/**
 * allocates an object.
 * @param np Number of vertices in object.
 * @param nf Number of faces in object.
 * @return a pointer to the object on success, NULL on failure.
 */
EON_Obj *EON_ObjCreate(EON_uInt32 np, EON_uInt32 nf);

/**
 * frees an object and all of it's subobjects.
 * @see EON_ObjCreate.
 * @param o object to delete.
 */
void EON_ObjDelete(EON_Obj *o);

/**
 * creates an exact but independent duplicate of an object and
 * all of it's subobjects.
 * @param o the object to clone.
 * @return a pointer to the new object on success, NULL on failure
 */
EON_Obj *EON_ObjClone(EON_Obj *o);

/**
 * scales an object, and all of it's subobjects.
 * @note This scales it slowly, by going through each vertex
 *       and scaling it's position. Avoid doing this in realtime.
 * @param o a pointer to the object to scale.
 * @param s the scaling factor.
 * @return a pointer to o.
 */
EON_Obj *EON_ObjScale(EON_Obj *o, EON_Float s);

/**
 * stretches an object, and all of it's subobjects.
 * @note: same as EON_ObjScale. Note that the normals are preserved.
 * @see EON_ObjScale.
 * @param o a pointer to the object to stretch.
 * @param x stretch factor.
 * @param y stretch factor.
 * @param z stretch factor.
 * @return a pointer to o.
 */
EON_Obj *EON_ObjStretch(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z);

/**
 * calculates the centroid of a given object.
 * @param[in] o a pointer to the object to calculate the centroid for.
 * @param[out] x pointer to the output coordinate.
 * @param[out] y pointer to the output coordinate.
 * @param[out] z pointer to the output coordinate.
 */
void EON_ObjCentroid(EON_Obj *o, EON_Float *x, EON_Float *y, EON_Float *z);

/**
 * translates an object.
 * @note same has EON_ObjScale.
 * @see EON_ObjScale.
 * @param o a pointer to the object to translate.
 * @param x translation in object space.
 * @param y translation in object space.
 * @param z translation in object space.
 * @return a pointer to o.
*/
EON_Obj *EON_ObjTranslate(EON_Obj *o, EON_Float x, EON_Float y, EON_Float z);

/**
 * flips all vertex and face normals of and object
 * and all of it's subobjects.
 * @note Not especially fast.
 *       A call to EON_ObjFlipNormals or EON_ObjCalcNormals
 *       will restore the normals
 * @see EON_ObjCalcNormals
 * @param o a pointer to the object to flip normals of.
 * @return a pointer to o.
*/
EON_Obj *EON_ObjFlipNormals(EON_Obj *o);

/**
 * sets the material of all faces in an object.
 * @param o the object to set the material of.
 * @param m the material to set it to.
 * @param th "transcend hierarchy". If set, it will set the
 *        material of all subobjects too.
 */
void EON_ObjSetMat(EON_Obj *o, EON_Mat *m, EON_Bool th);

/**
 * calculates all face and vertex normals for an object and all subobjects.
 * @param obj the object.
 */
EON_Obj *EON_ObjCalcNormals(EON_Obj *obj);

/**
 * dumps the internal informations about an Object to a logger.
 * @see cxkit/logkit.h for details.
 * @note the logger datatype is void* to remove a header dependency.
 * @param o object to be inspected.
 * @param logger logger instance to be used.
 */
void EON_ObjInfo(EON_Obj *o, void *logger);

/******************************************************************************
** Light Handling Routines (light.c)
******************************************************************************/

/**
 * creates a new light.
 * @return a pointer to the light or NULL on error.
*/
EON_Light *EON_LightCreate();

/**
 * sets up a light.
 * @see EON_LightCreate.
 * @param light the light to set up.
 * @param mode: the mode of the light (EON_LIGHT_*).
 * @param x either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param y either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param z either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param intensity the intensity of the light (0.0-1.0).
 * @param halfDist the distance at which EON_LIGHT_POINT_DISTANCE is 1/2 intensity
 * @return a pointer to light.
 */
EON_Light *EON_LightSet(EON_Light *light, EON_uChar mode, EON_Float x, EON_Float y,
                        EON_Float z, EON_Float intensity, EON_Float halfDist);

/**
 * allocate and set a new light in a single operation.
 * @see EON_LightCreate
 * @see EON_LightSet
 * @param light the light to set up.
 * @param mode: the mode of the light (EON_LIGHT_*).
 * @param x either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param y either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param z either the position of the light (EON_LIGHT_POINT*) or the angle
 *        in degrees of the light (EON_LIGHT_VECTOR).
 * @param intensity the intensity of the light (0.0-1.0).
 * @param halfDist the distance at which EON_LIGHT_POINT_DISTANCE is 1/2 intensity
 * @return a pointer to light or NULL on error.
 */
EON_Light *EON_LightNew(EON_uChar mode, EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity, EON_Float halfDist);

/**
 * frees a light.
 * @see EON_LightCreate
 * @see EON_LightNew
 * @param l light to delete.
 */
void EON_LightDelete(EON_Light *l);

/******************************************************************************
** Texture utilities
******************************************************************************/

/**
 * frees all memory associated with the given texture.
 * @param t the texture to be freed.
 */
void EON_TexDelete(EON_Texture *t);

/**
 * dumps the internal informations about a Texture to a logger.
 * @see cxkit/logkit.h for details.
 * @note the logger datatype is void* to remove a header dependency.
 * @param t texture to be inspected.
 * @param logger logger instance to be used.
 */
void EON_TexInfo(EON_Texture *t, void *logger);

/******************************************************************************
** Camera Handling Routines (cam.c)
******************************************************************************/

/**
 * allocates a new camera.
 * @param sw screen width
 * @param sh screen height
 * @param ar aspect ratio (usually 1.0)
 * @param fov field of view (usually 45-120)
 * @return a pointer to the newly allocated camera or NULL on error.
 */
EON_Cam *EON_CamCreate(EON_uInt sw, EON_uInt sh, EON_Float ar, EON_Float fov);

/**
 * sets the target of a camera.
 * sets the pitch and pan of the camera. Does not touch the roll.
 * @see EON_CamCreate
 * @param c the camera to set the target of.
 * @param x the worldspace coordinate of the target.
 * @param y the worldspace coordinate of the target.
 * @param z the worldspace coordinate of the target.
 */
void EON_CamSetTarget(EON_Cam *c, EON_Float x, EON_Float y, EON_Float z);

/**
 * frees all memory associated with a camera excluding framebuffers
 * and Z buffers
 * @param c camera to free
 */
void EON_CamDelete(EON_Cam *c);

/******************************************************************************
** Easy Rendering Interface (render.c)
******************************************************************************/

/**
 * allocates a new renderer context.
 * @see EON_Cam
 * @param Camera an already initialized and ready EON_Cam.
 * @return a pointer to the newly allocated context or NULL on error.
 */
EON_Rend *EON_RendCreate(EON_Cam *Camera);

/**
 * frees all memory associated with a rendering context.
 * @param rend context to free.
 */
void EON_RendDelete(EON_Rend *rend);

/**
 * begins the rendering process.
 * Only one rendering process per thread can occur at any given time.
 * Uses EON_Clip*(), so don't use them within or around a EON_Render() block.
 * @param rend an initialized renderer instance.
 */
void EON_RenderBegin(EON_Rend *rend);

/**
 * adds a light to the scene.
 * Any objects rendered before will be unaffected by this.
 * @param rend an initialized renderer instance.
 * @param light light to add to scene
 */
void EON_RenderLight(EON_Rend *rend, EON_Light *light);

/**
 * adds an object and all of it's subobjects to the scene.
 * @param rend an initialized renderer instance.
 * @param obj object to render
 */
void EON_RenderObj(EON_Rend *rend, EON_Obj *obj);

/**
 * actually does the rendering, and closes the rendering process
 * @param[in] rend an initialized renderer instance.
 * @param[out] frame frame instance to hold the rendered scene.
 */
void EON_RenderEnd(EON_Rend *rend, EON_Frame *frame);


/******************************************************************************
** Math Code (math.c)
******************************************************************************/

/**
 * generates a rotation matrix.
 * @param[out] matrix an array of 16 EON_Floats that is a 4x4 matrix.
 * @param[in] m the axis to rotate around, 1=X, 2=Y, 3=Z.
 * @param[in] Deg the angle in degrees to rotate.
 */
void EON_MatrixRotate(EON_Float matrix[], EON_uChar m, EON_Float Deg);

/**
 * generates a translation matrix.
 * @see EON_MatrixRotate.
 * @param[out] m the matrix.
 * @param[in] x the translation coordinate
 * @param[in] y the translation coordinate
 * @param[in] z the translation coordinate
 */
void EON_MatrixTranslate(EON_Float m[], EON_Float x, EON_Float y, EON_Float z);

/**
 * multiplies two matrices.
 * this is the same as dest = dest*src (since the order *does* matter);
 * @param[in,out] dest destination matrix will be multiplied by src
 * @param[in] src source matrix
 */
void EON_MatrixMultiply(EON_Float *dest, EON_Float src[]);

/**
 * applies a matrix.
 * applies the matrix to the 3d point to produce the transformed 3d point.
 * @param[in] m matrix to apply
 * @param[in] x input coordinate
 * @param[in] y input coordinate
 * @param[in] z input coordinate
 * @param[out] outx pointers to output coords.
 * @param[out] outy pointers to output coords.
 * @param[out] outz pointers to output coords.
 */
void EON_MatrixApply(EON_Float *m, EON_Float x, EON_Float y, EON_Float z,
                     EON_Float *outx, EON_Float *outy, EON_Float *outz);

/**
 * makes a vector a unit vector
 * @param[in,out] x pointers to the vector coordinate.
 * @param[in,out] y pointers to the vector coordinate.
 * @param[in,out] z pointers to the vector coordinate.
 */
void EON_NormalizeVector(EON_Float *x, EON_Float *y, EON_Float *z);

/**
 * returns the dot product of two vectors.
 * @param x1 the first vector coordinate.
 * @param y1 the first vector coordinate.
 * @param z1 the first vector coordinate.
 * @param x2 the second vector coordinate.
 * @param y2 the second vector coordinate.
 * @param z2 the second vector coordinate.
 * @return the dot product of the two vectors.
 */
EON_Float EON_DotProduct(EON_Float x1, EON_Float y1, EON_Float z1,
                         EON_Float x2, EON_Float y2, EON_Float z2);

/*************************************************************************/

#endif /* ! EON3D_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

