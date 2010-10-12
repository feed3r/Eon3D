/*
 * ex01_cube.c: the very first eon3d example: rotates a flat shaded cube
 */

#include <float.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include <eon3d.h> 
#include <eon3dx.h>


enum {
    WIDTH  = 320,
    HEIGHT = 200
};



int main(int argc, char *argv[])
{
    int i, ret;

    EON_Light *light;       /* Our light */
    EON_Object *cube;       /* Our cube object */
    EON_Material *cubeMat;  /* The material for the cube */
    EON_Camera *camera;     /* Our camera */
    EON_Frame *frame;
    EON_Renderer *rend;

    EONx_Console *console;  /* for our viewing pleasure */

    EONx_setup();

    frame = EON_newFrame(WIDTH, HEIGHT);

    cubeMat = EON_newMaterial();    
    cubeMat->NumGradients = 100; /* Have it use 100 colors */
    cubeMat->Shade = EON_SHADE_FLAT; 
    EON_materialInit(cubeMat);   /* Don't forget this! */

    cube = EON_newBox(100.0, 100.0, 100.0, cubeMat); // Create the cube

    camera = EON_newCamera(WIDTH, HEIGHT,
                           WIDTH*3.0/(HEIGHT*4.0), /* Aspect ratio */
                           90.0);                  /* Field of view */
    camera->Position.Z = -300; /* Back the camera up from the origin */

    light = EON_newLight(EON_LIGHT_VECTOR,
                         0.0,0.0,0.0,      /* rotation angles */
                         1.0,              /* intensity */
                         1.0);             /* not used for vector lights */

    console = EONx_newConsole(camera);
    if (!console) {
        EONx_logError();
        EONx_exit();
    }

    rend = EON_newRenderer();
    if (!rend) {
        EONx_logError();
        EONx_exit();
    }

    while (!EONx_consoleNextEvent(console)) {
        cube->Rotation.X += 1.0; // Rotate by 1 degree on each axis
        cube->Rotation.Y += 1.0;
        cube->Rotation.Z += 1.0;

        EON_rendererSetup(rend, camera);     // Start rendering with the camera
        EON_rendererAddLight(rend, light);   // Render our light
        EON_rendererAddObject(rend, cube);   // Render our object
        EON_rendererProcess(rend, frame);    // Finish rendering

        EONx_consoleShow(console, frame);
    }

    EON_delRenderer(rend);
    EONx_delConsole(console);
    EON_delLight(light);
    EON_delCamera(camera);
    EON_delObject(cube);
    EON_delMaterial(cubeMat);
    EON_delFrame(frame);

    EONx_exit();

    return 0;
}

/* vim: set ts=4 sw=4 et */
/* EOF */

