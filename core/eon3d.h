/**************************************************************************
 * eon3d.h -- Eon3D is a simplistic 3D software renderer.                 *
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

/**************************************************************************
 * Configuration section                                                  *
 **************************************************************************/

/* implementation limits */
enum {
    EON_MAX_LOG_LINE_LEN = 1024, /* Maximum user message length */
    EON_MAX_CHILDREN     = 64,   /* Maximum children per object */
    EON_MAX_LIGHTS       = 32,   /* Maximum lights per scene --
                                if you exceed this, they will be ignored */
    EON_MAX_TRIANGLES    = 1073741824
    /* Maximum number of triangles per scene -- if you exceed this,
       entire objects will be ignored.
       You can increase this if you need it.
       It takes approximately 8*EON_MAX_TRIANGLES bytes of memory.
       Not really a big deal.
     */
};


typedef float         EON_ZBuffer;     /* Z-buffer type (must be float)    */
typedef float         EON_Float;       /* General floating point           */
typedef float         EON_IEEEFloat32; /* IEEE 32 bit floating point       */ 
typedef double        EON_Double;      /* Double precision floating point  */
typedef double        EON_IEEEFloat64; /* IEEE 64 bit floating point       */ 
typedef int32_t       EON_Int32;       /* signed 32 bit integer            */
typedef uint32_t      EON_UInt32;      /* unsigned 32 bit integer          */
typedef int16_t       EON_Int16;       /* signed 16 bit integer            */
typedef uint16_t      EON_UInt16;      /* unsigned 16 bit integer          */
typedef int8_t        EON_Int8;        /* signed 8 bit integer             */
typedef uint8_t       EON_UInt8;       /* unsigned 8 bit integer           */
typedef int           EON_Int;         /* signed optimal integer           */
typedef unsigned int  EON_UInt;        /* unsigned optimal integer         */
typedef unsigned char EON_UChar;       /* unsigned 8 bit integer           */
typedef signed char   EON_Char;        /* signed 8 bit integer             */
typedef unsigned char EON_Byte;        /* generic 8 bit byte type          */

/* pi! */
#define EON_PI 3.14159265359

/* Utility min() and max() functions */
#define EON_Min(x, y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define EON_Max(x, y) (( ( x ) < ( y ) ? ( y ) : ( x )))
#define EON_Clamp(a, x, y) EON_Min(EON_Max(( a ), ( x )), ( y ))

typedef enum eon_boolean_ {
    EON_FALSE = 0,
    EON_TRUE  = 1
} EON_Boolean;

typedef enum eon_status_ {
    EON_OK    =  0,
    EON_ERROR = -1
} EON_Status; 

typedef enum eon_loglevel_ {
    EON_LOG_CRITICAL = 0,   /* this MUST be the first */
    EON_LOG_ERROR,
    EON_LOG_WARNING,
    EON_LOG_INFO,
    EON_LOG_DEBUG,
    EON_LOG_LAST            /* this MUST be the last 
                               and should'nt be used */ 
} EON_LogLevel;


typedef void (*EON_logHandler)(void *userData,
                               const char *where, int level,
                               const char *fmt, va_list ap);

/*************************************************************************/

/* 
** Note that (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE) and
** (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
typedef enum eon_shademode_ {
    EON_SHADE_NONE             = 1,
    EON_SHADE_FLAT             = 2,
    EON_SHADE_FLAT_DISTANCE    = 4,
    EON_SHADE_GOURAUD          = 8,
    EON_SHADE_GOURAUD_DISTANCE = 16
} EON_ShadeMode;

/*
** Note that EON_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
** the light and the point, EON_LIGHT_POINT_DISTANCE has falloff with proportion
** to distance**2 (see EON_LightSet() for setting it), EON_LIGHT_POINT does both.
*/
typedef enum eon_lightmode_ {
    EON_LIGHT_NONE           = 0x0,
    EON_LIGHT_VECTOR         = 0x1,
    EON_LIGHT_POINT          = 0x2|0x4,
    EON_LIGHT_POINT_DISTANCE = 0x2,
    EON_LIGHT_POINT_ANGLE    = 0x4
} EON_LightMode;

/* TODO WRITEME
** those can't be combined
*/
typedef enum eon_fillmode_ {
    EON_FILL_DEFAULT     = 0,
    EON_FILL_NONE        = 1, /* just the vertexes */
    EON_FILL_WIREFRAME   = 2,
    EON_FILL_SOLID       = 4,
    EON_FILL_TEXTURE     = 8,
    EON_FILL_ENVIRONMENT = 16,
} EON_FillMode;

typedef enum eon_texenvop_ {
    EON_TEXENV_ADD          = 0,
    EON_TEXENV_MUL          = 1,
    EON_TEXENV_AVG          = 2,
    EON_TEXENV_TEXMINUSENV  = 3,
    EON_TEXENV_ENVMINUSTEX  = 4,
    EON_TEXENV_MIN          = 5,
    EON_TEXENV_MAX          = 6
} EON_TexEnvOp;

/* Polygon sort mode wrt the Camera */
typedef enum eon_sort_mode_ {
    EON_SORT_FRONT_TO_BACK = -1,
    EON_SORT_NONE          =  0,
    EON_SORT_BACK_TO_FRONT = +1
} EON_SortMode;

typedef enum eon_frameflags_ {
    EON_FRAME_FLAG_NONE     = 0x0,
    EON_FRAME_FLAG_DR       = 0x1, /* direct rendering */
} EON_FrameFlags;

/*************************************************************************/


typedef struct eon_rectangle_ {
    EON_Int Width;
    EON_Int Height;
} EON_Rectangle;

typedef struct eon_area_ {
    EON_Int Top;
    EON_Int Left;
    EON_Int Bottom;
    EON_Int Right; 
} EON_Area;


/* 
** Texture type. Read textures with EON_TextureReadPCX(), and assign them to
** Material.Environment or Material.Texture. 
*/
typedef struct eon_texture_ {
    EON_Byte        *Data;  /* Texture data */
    EON_Rectangle   L2Dim;  /* Log2 of dimensions */
    EON_Rectangle   Dim;    /* Integer dimensions */
    EON_Float       UScale; /* Scaling (usually 2**Width, 2**Height) */
    EON_Float       VScale; /* ditto */
} EON_Texture;

typedef struct eon_rgb_ {
    EON_UInt8 R;    /* Red      */
    EON_UInt8 G;    /* Green    */
    EON_UInt8 B;    /* Blue     */
    EON_UInt8 Pad;  /* for future usage */
} EON_RGB;

enum {
    EON_DIMENSIONS = 3,    /* of course */
    EON_RGB_BPP    = 4,    /* Bytes Per Pixel (RGBA) */
    EON_RGB_BLACK  = 0xFF  /* per component (XXX) */
};

/*************************************************************************/
/* I hate the forward declarations,
** but this is like the chicken/egg problem.
 */

typedef struct eon_renderer_ EON_Renderer;
typedef struct eon_frame_ EON_Frame;
typedef struct eon_face_ EON_Face;

/*************************************************************************/

typedef int (*EON_RenderFaceFn)(EON_Renderer *renderer,
                                EON_Face *face, EON_Frame *frame);

/* yes, there is a better name */
typedef int (*EON_ProcessFaceFn)(EON_Face *face, EON_Renderer *rend);

/*************************************************************************/
/* 
** Material type. Create materials with EON_newMaterial().
*/
typedef struct eon_material_ {
    EON_RGB          Ambient;            /* RGB of surface */
    EON_RGB          Diffuse;            /* RGB of diffuse */
    EON_RGB          Specular;           /* RGB of "specular" highlights */
    EON_UInt         Shininess;          /* Shininess of material. 1 is dullest */
    EON_Float        FadeDist;           /* For distance fading, distance at 
                                           which intensity is 0 */
    EON_FillMode     Fill;
    EON_ShadeMode    Shade;
    EON_UInt         Transparent;        /* Transparency index (0 = none), 4 = alot 
                                           Note: transparencies disable textures */
    EON_UInt         PerspectiveCorrect; /* Correct textures every n pixels */
    EON_Texture      *Texture;           /* Texture map (see EON_Texture) above */
    EON_Texture      *Environment;       /* Environment map (ditto) */
    EON_Float        TexScaling;         /* Texture map scaling */
    EON_Float        EnvScaling;         /* Environment map scaling */
    EON_TexEnvOp     TexEnvMode;         /* TexEnv combining mode (EON_TEXENV_*) */
    EON_Boolean      ZBufferable;        /* Can this material be zbuffered? */
    EON_UInt         NumGradients;       /* Desired number of gradients to be used */
  
    /* The following are used mostly internally */
    EON_ShadeMode    _shadeMode;
    EON_FillMode     _fillMode;      
    EON_UInt         _tsfact;            /* Translucent shading factor */
    EON_UInt16       *_addTable;         /* Shading/Translucent/etc table */
    EON_Byte         *_reMapTable;       /* Table to remap colors to palette */
    EON_UInt         _colorsUsed;
    EON_RGB          _solidColor;
    EON_RGB          *_requestedColors;  /* _colorsUsed colors, desired colors */

    EON_RenderFaceFn _renderFace;        /* Function that renders the triangle 
                                           with this material */
} EON_Material;

typedef struct eon_point_ {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_Point;


/* we store just one endpoint since the other one is always the origin 
 * we don't include the EON_Point because we want to play dirty later.
 */
typedef struct eon_vector_ {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_Vector;

/*
** Vertex, used within EON_Object
*/
typedef struct eon_vertex_ {
    EON_Point Coords;      /* Vertex coordinate (objectspace) */
    EON_Point Formed;      /* Transformed vertex coordinate (cameraspace) */
    EON_Point Norm;        /* Unit vertex normal (objectspace) */
    EON_Point NormFormed;  /* Transformed unit vertex normal (cameraspace) */
} EON_Vertex;

/*
** Face
*/
struct eon_face_ {
    EON_Vertex      *Vertexes[EON_DIMENSIONS];   /* Vertexes of triangle */
    EON_Point       Norm;                        /* Normal of triangle (object space) */
    EON_Material    *Material;                   /* Material of triangle */
    EON_Int32       ScrX[EON_DIMENSIONS];        /* FXP12.20 Projected screen coords */
    EON_Int32       ScrY[EON_DIMENSIONS];        /* Ditto */
    EON_Float       ScrZ[EON_DIMENSIONS];        /* Projected 1/Z coordinates */
    EON_Int32       MappingU[EON_DIMENSIONS];    /* FXP16.16 Texture mapping coords */ 
    EON_Int32       MappingV[EON_DIMENSIONS];    /* Ditto */
    EON_Int32       EnvMappingU[EON_DIMENSIONS]; /* FXP16.16 Environment map coords */
    EON_Int32       EnvMappingV[EON_DIMENSIONS]; /* Ditto */
    EON_Float       FlatShade;                   /* Flat intensity */
    EON_Float       StaticLighting;              /* Face static lighting. Should usually be 0.0 */
    EON_Float       Shades[EON_DIMENSIONS];      /* Vertex intensity */
    EON_Float       VSLighting[EON_DIMENSIONS];  /* Vertex static lighting. Should be 0.0 */

    EON_ProcessFaceFn   _processFlatLightining;
    EON_ProcessFaceFn   _processFillEnvironment;
    EON_ProcessFaceFn   _processGouradShading;
};


/* 
** Object 
*/
typedef struct eon_object_ {
    EON_UInt32  NumVertexes;            /* Number of vertexes */
    EON_UInt32  NumFaces;               /* Number of faces */
    EON_Vertex  *Vertexes;              /* Array of vertexes */
    EON_Face    *Faces;                 /* Array of faces */
    struct eon_object_ *Children[EON_MAX_CHILDREN];
    EON_Boolean BackfaceCull;           /* Are backfacing polys drawn? */
    EON_Boolean BackfaceIllumination;   /* Illuminated by lights behind them? */ 
    EON_Boolean GenMatrix;              /* Generate Matrix from the following
                                           if set */
    EON_Point   Position;
    EON_Point   Rotation;
                                        /* Position and rotation of object:
                                           Note: rotations are around 
                                           X then Y then Z. Measured in degrees */
    EON_Float   TMatrix[4 * 4];         /* Transformation matrix */
    EON_Float   RMatrix[4 * 4];         /* Rotation only matrix (for normals) */
} EON_Object;


/*
** Light type. See EON_Light*().
*/
typedef struct eon_light_ {
    EON_LightMode Type; 
    EON_Point     Coords;          /* If Type=EON_LIGHT_POINT*,
                                      this is Position (EON_LIGHT_POINT_*),
                                      otherwise if EON_LIGHT_VECTOR,
                                      Unit vector */
    EON_Float     Intensity;       /* Intensity. 0.0 is off, 1.0 is full */
    EON_Float     HalfDistSquared; /* Distance squared at which 
                                      EON_LIGHT_POINT_DISTANCE is 50% */
} EON_Light;

/*
** Camera Type.
*/
typedef struct eon_camera_ {
    EON_Float       Fov;            /* FOV in degrees valid range is 1-179 */
    EON_Float       AspectRatio;    /* Aspect ratio (usually 1.0) */
    EON_SortMode    Sort;           /* Sort polygons */
    EON_Float       ClipBack;       /* Far clipping ( < 0.0 is none) */
    EON_Area        Clip;           /* Screen Clipping */
    EON_Rectangle   Screen;         /* Screen dimensions */
    EON_Rectangle   Center;         /* Center of screen */
    EON_Point       Position;       /* Camera position in worldspace */
    EON_Float       Pitch;          /* Camera angle in degrees in worldspace */
    EON_Float       Pan;            /* ditto */
    EON_Float       Roll;           /* ditto */
} EON_Camera;


struct eon_frame_ {
    EON_Rectangle   F;
    EON_Byte        *Pixels; /* RGB24, not EON_RGB */
    EON_UInt32      Flags;

    /* direct rendering */
    EON_Status      (*PutPixel)(EON_Frame *frame,
                                EON_Int x, EON_Int y, EON_UInt32 color);
    void            *_private;
};


/* Implementation will be in flux for a while */
/* see fordward declarations above */



/*************************************************************************
 * Initialization and Finalization                                       *
 *************************************************************************/

void EON_startup();
void EON_shutdown();

/**************************************************************************
 * Memory manipulation                                                    *
 **************************************************************************/

void *EON_malloc(size_t size);
void *EON_zalloc(size_t size);
void *EON_free(void *ptr);


/*************************************************************************
 * Error Handling                                                        *
 *************************************************************************/

void EON_log(const char *where, int level, const char *fmt, ...);
void EON_vlog(const char *where, int level, const char *fmt, va_list ap);

void EON_logSetHandler(EON_logHandler logHandler, void *userData);

EON_logHandler EON_logGetHandler(void);
void *EON_logGetUserData(void);

void EON_logDefaultHandler(void *userData,
                           const char *where, int level,
                           const char* fmt, va_list ap);


/*************************************************************************/
/* Materials                                                             */
/*************************************************************************/

EON_Material    *EON_newMaterial(void);
void            EON_delMaterial(EON_Material *material);
EON_Status      EON_materialInit(EON_Material *material);

/*************************************************************************/
/* Objects and primitives                                                */
/*************************************************************************/

EON_Object      *EON_delObject(EON_Object *object);
EON_Object      *EON_newObject(EON_UInt32 vertexes, EON_UInt32 faces,
                               EON_Material *material);
EON_Object      *EON_newBox(EON_Float w, EON_Float d, EON_Float h,
                            EON_Material *material);
EON_Status      EON_objectSetMaterial(EON_Object *object,
                                      EON_Material *material);
EON_Status      EON_objectCalcNormals(EON_Object *object);
EON_Status      EON_objectCenter(EON_Object *object);

/*************************************************************************/
/* Lights                                                                */
/*************************************************************************/

void       EON_delLight(EON_Light *light);
EON_Light *EON_newLight(EON_LightMode mode,
                        EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity,
                        EON_Float halfDist);

/*************************************************************************/
/* Frames                                                                */
/*************************************************************************/

EON_Frame       *EON_newFrame(EON_Int width, EON_Int height);
void            EON_delFrame(EON_Frame *frame);
void            EON_frameClean(EON_Frame *frame);
EON_Status  EON_framePutPixel(EON_Frame *frame,
                                  EON_Int x, EON_Int y, EON_UInt32 color);


/*************************************************************************/
/* Camera                                                                */
/*************************************************************************/

EON_Camera      *EON_newCamera(EON_Int width, EON_Int height,
                               EON_Float aspectRatio,
                               EON_Float fieldOfView);
void            EON_delCamera(EON_Camera *camera);

/*************************************************************************/
/* Rendering                                                             */
/*************************************************************************/

EON_Renderer    *EON_newRenderer(void);
void            EON_delRenderer(EON_Renderer *rend);

EON_Status      EON_rendererSetup(EON_Renderer *rend,
                                  EON_Camera *camera);
EON_Status      EON_rendererLight(EON_Renderer *rend,
                                  EON_Light *light);
EON_Status      EON_rendererObject(EON_Renderer *rend,
                                   EON_Object *object);
EON_Status      EON_rendererProcess(EON_Renderer *rend,
                                    EON_Frame *frame);
EON_Status      EON_rendererTeardown(EON_Renderer *rend);

/*************************************************************************/

#endif /* ! EON3D_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

