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
    EON_Mat *AllMaterials[2]; // Used for creating palette
    EON_Cam *TheCamera; // Our camera
    EON_Rend *TheRend;
    EONx_Console *TheConsole;
    uint8_t ThePalette[3 * 256];
    double edge = 100.0;

    EONx_ConsoleStartup("Eon3D :: example 1", NULL);

    CubeMat = EON_MatCreate();    // Create the material for the cube
    CubeMat->NumGradients = 100; // Have it use 100 colors
    CubeMat->ShadeType = EON_SHADE_FLAT; // Make the cube flat shaded

    CubeMat->Ambient[0] = 32; // Set red ambient component
    CubeMat->Ambient[1] = 0;  // Set green ambient component
    CubeMat->Ambient[2] = 16; // Set blue ambient component

    CubeMat->Diffuse[0] = 200; // Set red diffuse component
    CubeMat->Diffuse[1] = 100; // Set green diffuse component
    CubeMat->Diffuse[2] = 150; // Set blue diffuse component

    EON_MatInit(CubeMat);          // Initialize the material

    AllMaterials[0] = CubeMat; // Make list of materials
    AllMaterials[1] = 0; // Null terminate list of materials
    EON_MatMakeOptPal(ThePalette,1,255,AllMaterials,2); // Create a nice palette

    ThePalette[0] = ThePalette[1] = ThePalette[2] = 0; // Color 0 is black

    EON_MatMapToPal(CubeMat,ThePalette,0,255); // Map the material to our palette

    TheConsole = EONx_ConsoleNew(800, // Screen width
                          600, // Screen height
                          90.0 // Field of view
                          );

    EONx_ConsoleSetPalette(TheConsole, ThePalette, 256);

    TheCube = EON_MakeBox(edge,edge,edge,CubeMat); // Create the cube

    TheCamera = EONx_ConsoleGetCamera(TheConsole);
    TheCamera->Z = -300; // Back the camera up from the origin
    TheCamera->Sort = 0; // We don't need to sort since zbuffering takes care
                       // of it for us!

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
        EON_RenderEnd(TheRend);             // Finish rendering
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

