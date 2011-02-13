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

/** \file eon3d.h
    \brief eon3d: a simplistic 3D software renderer (interface).
*/

/**************************************************************************
 * Configuration section                                                  *
 **************************************************************************/

/** \enum implementation limits */
enum {
    EON_MAX_LOG_LINE_LEN = 1024,      /**< Maximum user message length */
    EON_MAX_CHILDREN     = 64,        /**< Maximum children per object */
    EON_MAX_LIGHTS       = 32,        /**< Maximum lights per scene --
                                           if you exceed this, the exceeding
                                           lights will be ignored.
                                       */
    EON_MAX_TRIANGLES    = 1073741824 /**< Maximum number of triangles 
                                           per scene -- if you exceed this,
					   the exceeding OBJECTS will be
                                           ignored. You can increase this
                                           if you need it.
                                           It takes approximately
                                           8*EON_MAX_TRIANGLES bytes
                                           of memory. Not really a big deal.
                                       */
};


typedef float         EON_ZBuffer;     /** Z-buffer type (must be float)   */
typedef float         EON_Float;       /** General floating point          */
typedef float         EON_IEEEFloat32; /** IEEE 32 bit floating point      */ 
typedef double        EON_Double;      /** Double precision floating point */
typedef double        EON_IEEEFloat64; /** IEEE 64 bit floating point      */ 
typedef int32_t       EON_Int32;       /** signed 32 bit integer           */
typedef uint32_t      EON_UInt32;      /** unsigned 32 bit integer         */
typedef int16_t       EON_Int16;       /** signed 16 bit integer           */
typedef uint16_t      EON_UInt16;      /** unsigned 16 bit integer         */
typedef int8_t        EON_Int8;        /** signed 8 bit integer            */
typedef uint8_t       EON_UInt8;       /** unsigned 8 bit integer          */
typedef int           EON_Int;         /** signed optimal integer          */
typedef unsigned int  EON_UInt;        /** unsigned optimal integer        */
typedef unsigned char EON_UChar;       /** unsigned 8 bit integer          */
typedef signed char   EON_Char;        /** signed 8 bit integer            */
typedef unsigned char EON_Byte;        /** generic 8 bit byte type         */

/** pi! */
#define EON_PI 3.14159265359

/** \macro utility min() and max() functions */
#define EON_Min(x, y) (( ( x ) > ( y ) ? ( y ) : ( x )))
#define EON_Max(x, y) (( ( x ) < ( y ) ? ( y ) : ( x )))
#define EON_Clamp(a, x, y) EON_Min(EON_Max(( a ), ( x )), ( y ))

/** \enum the obvious boolean type */
typedef enum eon_boolean_ {
    EON_FALSE = 0,
    EON_TRUE  = 1
} EON_Boolean;

/** \enum status code for all tha API calls */
typedef enum eon_status_ {
    EON_OK    =  0, /**< all clean */
    EON_ERROR = -1  /**< generic error */
} EON_Status; 

/** \enum log levels */
typedef enum eon_loglevel_ {
    EON_LOG_CRITICAL = 0, /**< this MUST be the first and it is
                               the most important. -- PANIC!     */
    EON_LOG_ERROR,        /**< you'll need to see this           */
    EON_LOG_WARNING,      /**< you'd better to see this          */
    EON_LOG_INFO,         /**< informative messages (for tuning) */
    EON_LOG_DEBUG,        /**< debug messages (for devs)         */
    EON_LOG_LAST          /**< this MUST be the last -- 
                               and should'nt be used             */
} EON_LogLevel;


/** \var typedef EON_LogHandler
    \brief logging callback function.

    This callback is invoked by the eon3d runtime whenever is needed
    to log a message.

    eon3d provides a default callback to log to the  stderr.

    \param userData a pointer given at the callback registration
           time. Fully opaque for eon3d.
    \param where string identifying the eon3d module/subsystem.
    \param level the severity of the message.
    \param fmt printf-like format string for the message.
    \param ap va_list of the arguments to complete the format string.
    \return EON_OK on success, a EON_ERROR otherwise.

    \see EON_LogLevel
*/
typedef void (*EON_LogHandler)(void *userData,
                               const char *where, int level,
                               const char *fmt, va_list ap);

/*************************************************************************/

/** \enum shading modes.

    Note (EON_SHADE_GOURAUD|EON_SHADE_GOURAUD_DISTANCE) and
    (EON_SHADE_FLAT|EON_SHADE_FLAT_DISTANCE) are valid shading modes.
*/
typedef enum eon_shademode_ {
    EON_SHADE_NONE             = 1, /**< no shading     */
    EON_SHADE_FLAT             = 2, /**< flat shading   */
    EON_SHADE_FLAT_DISTANCE    = 4, /**< WRITEME        */
    EON_SHADE_GOURAUD          = 8, /**< gourad shading */
    EON_SHADE_GOURAUD_DISTANCE = 16 /**< WRITEME        */
} EON_ShadeMode;


/** \enum lighting mode.

    Note EON_LIGHT_POINT_ANGLE assumes no falloff and uses the angle between
    the light and the point, EON_LIGHT_POINT_DISTANCE has falloff with proportion
    to distance**2 (see EON_lightSet() for setting it), EON_LIGHT_POINT does both.
*/
typedef enum eon_lightmode_ {
    EON_LIGHT_NONE           = 0x0,      /**< no lightining */
    EON_LIGHT_VECTOR         = 0x1,      /**< vector light  */
    EON_LIGHT_POINT          = 0x2|0x4,  /**< point light   */
    EON_LIGHT_POINT_DISTANCE = 0x2,      /**< WRITEME       */
    EON_LIGHT_POINT_ANGLE    = 0x4       /**< WRITEME       */
} EON_LightMode;

/** \enum polygon filling mode.
    Note those cannot be combined
*/
typedef enum eon_fillmode_ {
    EON_FILL_DEFAULT     = 0,  /**< runtime default. FIXME              */
    EON_FILL_NONE        = 1,  /**< just the vertexes (no fill at all)  */
    EON_FILL_WIREFRAME   = 2,  /**< wireframe fill (vertexes and edges) */
    EON_FILL_SOLID       = 4,  /**< solid color fill                    */
    EON_FILL_TEXTURE     = 8,  /**< texture mapping                     */
    EON_FILL_ENVIRONMENT = 16, /**< WRITEME */
} EON_FillMode;

/** \enum texture/environment operation */
typedef enum eon_texenvop_ {
    EON_TEXENV_ADD          = 0, /**< WRITEME */
    EON_TEXENV_MUL          = 1, /**< WRITEME */
    EON_TEXENV_AVG          = 2, /**< WRITEME */
    EON_TEXENV_TEXMINUSENV  = 3, /**< WRITEME */
    EON_TEXENV_ENVMINUSTEX  = 4, /**< WRITEME */
    EON_TEXENV_MIN          = 5, /**< WRITEME */
    EON_TEXENV_MAX          = 6  /**< WRITEME */
} EON_TexEnvOp;

/** \enum polygon sort mode wrt the camera */
typedef enum eon_sort_mode_ {
    EON_SORT_FRONT_TO_BACK = -1, /**< front to back */
    EON_SORT_NONE          =  0, /**< no sort       */
    EON_SORT_BACK_TO_FRONT = +1  /**< back to front */
} EON_SortMode;

/** \enum frame flags */
typedef enum eon_frameflags_ {
    EON_FRAME_FLAG_NONE     = 0x0, /**< no flags */
    EON_FRAME_FLAG_DR       = 0x1, /**< direct rendering (avoid memcpys) */
} EON_FrameFlags;

/*************************************************************************/


/** \struct defines a rectangle. */
typedef struct eon_rectangle_ {
    EON_Int Width;  /**< rectangle width  */
    EON_Int Height; /**< rectangle height */
} EON_Rectangle;

/** \struct defines a rectangular area. 

    the area is defined into the screen plane, having the origin in the
    upper left corner:
    (0,0)
       +-->
       |
       V
 */
typedef struct eon_area_ {
    EON_Int Top;    /**< Y coord of the top left corner     */
    EON_Int Left;   /**< X coord of the top left corner     */
    EON_Int Bottom; /**< Y coord of the bottom right corner */
    EON_Int Right;  /**< X coord of the bottom right corner */
} EON_Area;


/** \struct represents a texture.

   Read textures with EON_textureRead*(), and assign them to
   Material.Environment or Material.Texture. 
*/
typedef struct eon_texture_ {
    EON_Byte        *Data;  /**< texture data                          */
    EON_Rectangle   L2Dim;  /**< log2 of dimensions                    */
    EON_Rectangle   Dim;    /**< integer dimensions                    */
    EON_Float       UScale; /**< scaling (usually 2**Width, 2**Height) */
    EON_Float       VScale; /**< ditto                                 */
} EON_Texture;

/** \struct a RGB24 values. Alpha channel is carried as bonus. */
typedef struct eon_rgb_ {
    EON_UInt8 R;  /**< Red                      */
    EON_UInt8 G;  /**< Green                    */
    EON_UInt8 B;  /**< Blue                     */
    EON_UInt8 A;  /**< Alpha, padding if unused */
} EON_RGB;

/** \enum constant utilities */
enum {
    EON_DIMENSIONS = 3,    /**< of course                           */
    EON_RGB_BPP    = 4,    /**< Bytes Per Pixel (RGBA) \see EON_RGB */
    EON_RGB_BLACK  = 0xFF  /**< per component (XXX)                 */
};

/*************************************************************************/

/* I hate the forward declarations,
 * but this is like the chicken/egg problem.
 */

typedef struct eon_renderer_ EON_Renderer;
typedef struct eon_frame_ EON_Frame;
typedef struct eon_face_ EON_Face;

/*************************************************************************/

/** \var typedef EON_RenderFaceFn
    \brief renders a face (triangle) into a frame.

    This callback is for internal usage of Eon3D only.
    The client shall not override this callback.

    This callback is invoked whenever the EON_Renderer needs to render
    a triangle (face) into a frame.

    \param renderer the renderer context.
    \param face the triangle to render.
    \param frame the frame to be used for rendering.
    \return 0 (zero) on success, !0 otherwise.

    \see EON_renderer*
*/
typedef int (*EON_RenderFaceFn)(EON_Renderer *renderer,
                                EON_Face *face, EON_Frame *frame);

/** \var typedef EON_ProcessFaceFn
    \brief applies (pre)processing (e.g. lightining, shading)
           to a face (triangle).

    This callback is for internal usage of Eon3D only.
    The client shall not override this callback.

    This callback is invoked whenever the EON_Renderer needs to preprocess
    a triangle (face), e.g applying the lightining or the shading, before
    the actual rendering.

    \param face the triangle to be processed.
    \param rend the renderer context.
    \return 0 (zero) on success, !0 otherwise.

    \see EON_renderer*
*/
typedef int (*EON_ProcessFaceFn)(EON_Face *face, EON_Renderer *rend);

/*************************************************************************/

/* \struct material type.
 
   the fields marked with [X] are `private' for internal usage only.
   The client shall not touch those.
*/
typedef struct eon_material_ {
    EON_RGB          Ambient;            /**< RGB of surface               */
    EON_RGB          Diffuse;            /**< RGB of diffuse               */
    EON_RGB          Specular;           /**< RGB of "specular" highlights */
    EON_UInt         Shininess;          /**< shininess of material.
                                              1 is dullest                 */
    EON_Float        FadeDist;           /**< for distance fading, distance
                                              at which intensity is 0      */
    EON_FillMode     Fill;               /**< filling mode                 */
    EON_ShadeMode    Shade;              /**< shading mode                 */
    EON_UInt         Transparent;        /**< transparency index (0=min)
                                              this disable texturing       */
    EON_UInt         PerspectiveCorrect; /**< correct textures 
                                              every n pixels */
    EON_Texture      *Texture;           /**< texture map (see EON_Texture)*/
    EON_Texture      *Environment;       /**< environment map (ditto)      */
    EON_Float        TexScaling;         /**< texture map scaling          */
    EON_Float        EnvScaling;         /**< environment map scaling      */
    EON_TexEnvOp     TexEnvMode;         /**< TexEnv combining mode
                                              (EON_TEXENV_*)               */
    EON_Boolean      ZBufferable;        /**< can this material
                                              be zbuffered?                */
    EON_UInt         NumGradients;       /**< desired number of gradients
                                              to be used                   */
  
    EON_ShadeMode    _shadeMode;         /**< [X] real shading to apply    */
    EON_FillMode     _fillMode;          /**< [X] real filling to apply    */
    EON_UInt         _tsfact;            /**< [X] translucent shading
                                                  factor                   */
    EON_UInt16       *_addTable;         /**< [X] shading/translucent/...
                                                  table                    */
    EON_Byte         *_reMapTable;       /**< [X] table to remap colors
                                              to palette                   */
    EON_UInt         _colorsUsed;        /**< [X] WRITEME                  */
    EON_RGB          _solidColor;        /**< [X] WRITEME                  */
    EON_RGB          *_requestedColors;  /**< [X] _colorsUsed colors,
                                                  desired colors           */
    EON_RenderFaceFn _renderFace;        /**< [X] renders the triangle 
                                                  with this material       */
} EON_Material;

/** \struct point into a three-dimensional space.

    The layout (and thus the ABI) must always be compatibile 
    with EON_Vector.

    \see EON_Vector.
*/
typedef struct eon_point_ {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_Point;


/** \struct vector into a three-dimensional space.

    has just one endpoint since the other one is always the origin. 
    The layout (and thus the ABI) must always be compatibile 
    with EON_Point.

    \see EON_Point.
*/
typedef struct eon_vector_ {
    EON_Float X;
    EON_Float Y;
    EON_Float Z;
} EON_Vector;

/** \struct a vertex, used within EON_Object */
typedef struct eon_vertex_ {
    EON_Point Coords;      /**< vertex coordinate              (objectspace)*/
    EON_Point Formed;      /**< transformed vertex coordinate  (cameraspace)*/
    EON_Point Norm;        /**< unit vertex normal             (objectspace)*/
    EON_Point NormFormed;  /**< transformed unit vertex normal (cameraspace)*/
} EON_Vertex;

/** \struct face (triangle) 

    the fields marked with [X] are `private' for internal usage only.
    The client shall not touch those.
*/
struct eon_face_ {
    EON_Vertex   *Vertexes[EON_DIMENSIONS];   /**< vertexes of triangle    */
    EON_Point    Norm;                        /**< normal (object space)   */
    EON_Material *Material;                   /**< material of triangle    */
    EON_Int32    ScrX[EON_DIMENSIONS];        /**< FXP12.20 projected screen
                                                   coordinates             */
    EON_Int32    ScrY[EON_DIMENSIONS];        /**< ditto                   */
    EON_Float    ScrZ[EON_DIMENSIONS];        /**< Projected 1/Z
                                                   coordinates             */
    EON_Int32    MappingU[EON_DIMENSIONS];    /**< FXP16.16 texture mapping
                                                   coordinatess            */
    EON_Int32    MappingV[EON_DIMENSIONS];    /**< Ditto                   */
    EON_Int32    EnvMappingU[EON_DIMENSIONS]; /**< FXP16.16 environment
                                                   mapping coordinatess    */
    EON_Int32    EnvMappingV[EON_DIMENSIONS]; /**< Ditto                   */
    EON_Float    FlatShade;                   /**< Flat intensity          */
    EON_Float    StaticLighting;              /**< Face static lighting.
                                                   Should usually be 0.0   */
    EON_Float    Shades[EON_DIMENSIONS];      /**< Vertex intensity        */
    EON_Float    VSLighting[EON_DIMENSIONS];  /**< Vertex static lighting.
                                                   Should usually be 0.0   */

    EON_ProcessFaceFn   _processFlatLightining;  /**< [X] to apply flat 
                                                          ligthining       */
    EON_ProcessFaceFn   _processFillEnvironment; /**< [X] to apply
                                                          environment fill */
    EON_ProcessFaceFn   _processGouradShading;   /**< [X] to apply gourad
                                                          shading          */
};


/** \struct object

    An object is the main viewable content.

    TODO: port to eon_array?
*/
typedef struct eon_object_ {
    EON_UInt32  NumVertexes;            /**< number of vertexes            */
    EON_UInt32  NumFaces;               /**< number of faces               */
    EON_Vertex  *Vertexes;              /**< Array of vertexes             */
    EON_Face    *Faces;                 /**< Array of faces                */
    struct eon_object_ *Children[EON_MAX_CHILDREN];
    EON_Boolean BackfaceCull;           /**< are backfacing polys drawn?   */
    EON_Boolean BackfaceIllumination;   /**< illuminated by lights 
                                             behind them?                  */
    EON_Boolean GenMatrix;              /**< Generate Matrix from the 
                                             following if set              */
    EON_Point   Position;
    EON_Point   Rotation;
                                        /**< Position and rotation of object:
                                             Note: rotations are around 
                                             X then Y then Z.
                                             Measured in degrees           */
    EON_Float   TMatrix[4 * 4];         /**< Transformation matrix         */
    EON_Float   RMatrix[4 * 4];         /**< Rotation only matrix
                                             (for normals)                 */
} EON_Object;


/** \struct light */
typedef struct eon_light_ {
    EON_LightMode Type;            /**< ligthining mode                    */   
    EON_Point     Coords;          /**< If Type=EON_LIGHT_POINT*,
                                        this is Position (EON_LIGHT_POINT_*),
                                        otherwise if EON_LIGHT_VECTOR,
                                        Unit vector                        */
    EON_Float     Intensity;       /**< Intensity. 0.0 is off, 1.0 is full */
    EON_Float     HalfDistSquared; /**< Distance squared at which 
                                        EON_LIGHT_POINT_DISTANCE is 50%    */
} EON_Light;

/** \struct camera */
typedef struct eon_camera_ {
    EON_Float       FieldOfView; /**< FOV in degrees valid range is 1-179  */
    EON_Float       AspectRatio; /**< aspect ratio (usually 1.0)           */
    EON_SortMode    Sort;        /**< how to sort polygons                 */
    EON_Float       ClipBack;    /**< far clipping ( < 0.0 is none)        */
    EON_Area        Clip;        /**< screen Clipping                      */
    EON_Rectangle   Screen;      /**< screen dimensions                    */
    EON_Rectangle   Center;      /**< center of screen                     */
    EON_Point       Position;    /**< position (worldspace)                */
    EON_Float       Pitch;       /**< angle in degrees (worldspace)        */
    EON_Float       Pan;         /**< ditto                                */
    EON_Float       Roll;        /**< ditto                                */
} EON_Camera;

/** \struct frame
    \brief a frame is a rendering destination buffer.
    
    the fields marked with [X] are `private' for internal usage only.
    The client shall not touch those.
*/
struct eon_frame_ {
    EON_Rectangle   F;       /**< frame dimensions */
    EON_Byte        *Pixels; /**< the actual data (TODO) */
    EON_UInt32      Flags;   /**< bitmask of options and flags */

    /** direct rendering support */
    EON_Status      (*PutPixel)(EON_Frame *frame,
                                EON_Int x, EON_Int y, EON_UInt32 color);
    void            *_private; /**< [X] PutPixel private data */
};


/* Implementation will be in flux for a while */
/* see fordward declarations above */


/*************************************************************************
 * Initialization and Finalization                                       *
 *************************************************************************/

/** \fn initializes the Eon3D runtime.

    this function does all the resource acquisition and initialization
    needed by the Eon3D runtime.
    The client must invoke this function once before to use any other
    Eon3D function.

    \see EON_shutdown
*/
void EON_startup();

/** \fn finalizes the Eon3D runtime.

    this function frees al the resources acquired by the runtime, both
    at the initialization and through the usage of it.
    The client should invoke this function once done with Eon3D.
    It is safe to call this function multiple times.
    After this function called, a new runtime intialization is needed.
    
    \see EON_startup
*/
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

void EON_logSetHandler(EON_LogHandler LogHandler, void *userData);

EON_LogHandler EON_logGetHandler(void);
void *EON_logGetUserData(void);

void EON_logDefaultHandler(void *userData,
                           const char *where, int level,
                           const char* fmt, va_list ap);


/*************************************************************************/
/* Materials                                                             */
/*************************************************************************/

/** \fn creates a new material.

    creates and returns a new generic empty material handle.
    The client can tune the characteristics of the material by modifying
    its public fields. Once done, the client must invoke EON_materialSeal
    to make the changes effective.

    \return a new generic empty material handle on success,
            NULL otherwise.

    \see EON_delMaterial
    \see EON_materialSeal
*/
EON_Material *EON_newMaterial(void);

/** \fn destroys a material

    relinquish a material created via EON_newMaterial.

    \param material handle to the material to dispose.

    \see EON_newMaterial
*/
void EON_delMaterial(EON_Material *material);


/** \fn seals a material, making a settings change effective.

    updates the internal representation of a material to reflect
    the changes made through the public fields.
    The client must call this function after very changeset of
    the public fields, to make them effective.
    The client can seal a given material an unlimited number of times.

    \param material handle to the material to seal.
    \return EON_OK on success,
            EON_ERROR otherwise.

    \see EON_newMaterial
*/
EON_Status EON_materialSeal(EON_Material *material);

/*************************************************************************/
/* Objects and primitives                                                */
/*************************************************************************/

EON_Object *EON_newObject(EON_UInt32 vertexes, EON_UInt32 faces,
                          EON_Material *material);
EON_Object *EON_delObject(EON_Object *object);

EON_Status EON_objectSetMaterial(EON_Object *object,
                                 EON_Material *material);
EON_Status EON_objectCalcNormals(EON_Object *object);
EON_Status EON_objectCenter(EON_Object *object);

EON_Object *EON_newBox(EON_Float w, EON_Float d, EON_Float h,
                       EON_Material *material);

/*************************************************************************/
/* Lights                                                                */
/*************************************************************************/

EON_Light *EON_newLight(EON_LightMode mode,
                        EON_Float x, EON_Float y, EON_Float z,
                        EON_Float intensity,
                        EON_Float halfDist);
void EON_delLight(EON_Light *light);

/*************************************************************************/
/* Frames                                                                */
/*************************************************************************/

EON_Frame *EON_newFrame(EON_Int width, EON_Int height);
void EON_delFrame(EON_Frame *frame);

void EON_frameClean(EON_Frame *frame);
EON_Status EON_framePutPixel(EON_Frame *frame,
                             EON_Int x, EON_Int y, EON_UInt32 color);


/*************************************************************************/
/* Camera                                                                */
/*************************************************************************/

EON_Camera *EON_newCamera(EON_Int width, EON_Int height,
                          EON_Float aspectRatio,
                          EON_Float fieldOfView);
void EON_delCamera(EON_Camera *camera);

/*************************************************************************/
/* Rendering                                                             */
/*************************************************************************/

EON_Renderer *EON_newRenderer(void);
void EON_delRenderer(EON_Renderer *rend);

EON_Status EON_rendererSetup(EON_Renderer *rend, EON_Camera *camera);
EON_Status EON_rendererLight(EON_Renderer *rend, EON_Light *light);
EON_Status EON_rendererObject(EON_Renderer *rend, EON_Object *object);
EON_Status EON_rendererProcess(EON_Renderer *rend, EON_Frame *frame);
EON_Status EON_rendererTeardown(EON_Renderer *rend);

/*************************************************************************/

#endif /* ! EON3D_H */

/* vim: set ts=4 sw=4 et */
/* EOF */

