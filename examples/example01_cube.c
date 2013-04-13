/* Rotates a flat shaded cube */

#include <stdio.h>
#include <time.h>

#include <eon3d.h>
#include <eon3dx.h>
#include <eon3dx_console.h>


int main()
{
    int frames = 0;
    time_t start = 0, stop = 0;
    // Our variables
    EON_Light *TheLight;   // Our light
    EON_Obj *TheCube;      // Our cube object
    EON_Mat *CubeMat;      // The material for the cube
    EON_Cam *TheCamera; // Our camera
    EON_Rend *TheRend;
    EON_Frame *TheFrame;
    EONx_Console *TheConsole;
    double edge = 100.0;

    EONx_ConsoleStartup("Eon3D :: example 1", NULL);

    CubeMat = EON_MatCreate();    // Create the material for the cube
    CubeMat->ShadeType = EON_SHADE_FLAT; // Make the cube flat shaded

    CubeMat->Ambient[0] = 132; // Set red ambient component
    CubeMat->Ambient[1] = 0;   // Set green ambient component
    CubeMat->Ambient[2] = 0;   // Set blue ambient component

    EON_MatInit(CubeMat);    // Initialize the material

    TheConsole = EONx_ConsoleNew(800, // Screen width
                                 600, // Screen height
                                 90.0 // Field of view
                                 );

    TheCube = EON_MakeBox(edge,edge,edge,CubeMat); // Create the cube

    TheFrame = EONx_ConsoleGetFrame(TheConsole);
    TheCamera = EONx_ConsoleGetCamera(TheConsole);
    TheCamera->Z = -300; // Back the camera up from the origin

    TheLight = EON_LightNew(EON_LIGHT_VECTOR, // vector light
                            0.0,0.0,0.0, // rotation angles
                            1.0, // intensity
                            1.0); // falloff, not used for vector lights

    TheRend = EON_RendCreate(TheCamera);

    start = time(NULL);
    while (!EONx_ConsoleWaitKey(TheConsole)) { // While the keyboard hasn't been touched
        TheCube->Xa += 1.0; // Rotate by 1 degree on each axis
        TheCube->Ya += 1.0;
        TheCube->Za += 1.0;
        EONx_ConsoleClearFrame(TheConsole);
        EON_RenderBegin(TheRend);           // Start rendering with the camera
        EON_RenderLight(TheRend, TheLight); // Render our light
        EON_RenderObj(TheRend, TheCube);    // Render our object
        EON_RenderEnd(TheRend, TheFrame);   // Finish rendering
        EONx_ConsoleShowFrame(TheConsole);
        frames++;
    }
    stop = time(NULL);

    fprintf(stderr, "(%s) %i frames in %f seconds: %.3f FPS\n",
            __FILE__,
            frames, (double)stop-(double)start,
            (double)frames/((double)stop-(double)start));

    return EONx_ConsoleShutdown();
}

