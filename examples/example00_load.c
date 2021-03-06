/* Loads and rotates a PLY model. */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <eon3d.h>
#include <eon3dx.h>
#include <eon3dx_console.h>
#include <eon3dx_reader.h>


int main(int argc, char *argv[])
{
    const char *filename;
    int frames = 0;
    time_t start = 0, stop = 0;
    // Our variables
    EON_Light *TheLight;   // Our light
    EON_Obj *TheModel;      // Our cube object
    EON_Mat *ModelMat;      // The material for the cube
    EON_Cam *TheCamera; // Our camera
    EON_Rend *TheRend;
    EON_Frame *TheFrame;
    EONx_Console *TheConsole;
    double distance = 50;

    if (argc != 3) {
        fprintf(stderr, "usage: %s plymodel distance\n",
                argv[0]);
        exit(1);
    } else {
        filename = argv[1];
        distance = atof(argv[2]);
    }

    EONx_ConsoleStartup("Eon3D :: example 0", NULL);

    ModelMat = EON_MatCreate(); 
    ModelMat->ShadeType = EON_SHADE_FLAT;

    ModelMat->Ambient.R = 200;
    ModelMat->Ambient.G = 200;
    ModelMat->Ambient.B = 200;

    EON_MatInit(ModelMat);          // Initialize the material

    TheConsole = EONx_ConsoleCreate(800, // Screen width
                                    600, // Screen height
                                    90.0 // Field of view
                                    );

    TheModel = EONx_ReadPLYObj(filename, ModelMat);

    TheFrame = EONx_ConsoleGetFrame(TheConsole);
    TheCamera = EONx_ConsoleGetCamera(TheConsole);
    TheCamera->Pos.Z = -distance; // Back the camera up from the origin

    TheLight = EON_LightNew(EON_LIGHT_VECTOR, // vector light
                            0.0, 0.0, 0.0, // rotation angles
                            1.0, // intensity
                            1.0); // falloff, not used for vector lights

    TheRend = EON_RendCreate(TheCamera);

    start = time(NULL);
    while (!EONx_ConsoleNextEvent(TheConsole)) {
        // Rotate by 1 degree on each axis
        TheModel->Xa += 1.0;
        TheModel->Ya += 1.0;
        TheModel->Za += 1.0;
        EONx_ConsoleClearFrame(TheConsole);
        EON_RenderBegin(TheRend);           // Start rendering with the camera
        EON_RenderLight(TheRend, TheLight); // Render our light
        EON_RenderObj(TheRend, TheModel);   // Render our object
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

