/* Rotates a flat shaded cube AND a gouraud shaded torus */

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
    EON_Float TheLight_Xa = 0.0, TheLight_Ya = 0.0, TheLight_Za = 0.0;
                     // The rotation angles of our light
    EON_Obj *TheCube;      // Our cube object
    EON_Obj *TheTorus;     // Our torus object
    EON_Mat *CubeMat;      // The material for the cube
    EON_Mat *TorusMat;     // The material for the torus
    EON_Cam *TheCamera; // Our camera
    EON_Rend *TheRend;
    EON_Frame *TheFrame;
    EONx_Console *TheConsole;

    EONx_ConsoleStartup("Eon3D :: example 2", NULL);

    CubeMat = EON_MatCreate();    // Create the material for the cube
    CubeMat->ShadeType = EON_SHADE_FLAT; // Make the cube flat shaded

    CubeMat->Ambient.R = 132;
    CubeMat->Ambient.G = 0;
    CubeMat->Ambient.B = 16;

    CubeMat->Diffuse.R = 200;
    CubeMat->Diffuse.G = 100;
    CubeMat->Diffuse.B = 150;

    EON_MatInit(CubeMat);          // Initialize the material

    TorusMat = EON_MatCreate();    // Create the material for the torus
    TorusMat->ShadeType = EON_SHADE_GOURAUD; // Make the torus gouraud shaded
    TorusMat->Shininess = 10; // Make the torus a bit more shiny

    TorusMat->Ambient.R = 0;
    TorusMat->Ambient.G = 132;
    TorusMat->Ambient.B = 4;

    TorusMat->Diffuse.R = 20;
    TorusMat->Diffuse.G = 60;
    TorusMat->Diffuse.B = 70;

    TorusMat->Specular.R = 100;
    TorusMat->Specular.G = 200;
    TorusMat->Specular.B = 150;

    EON_MatInit(TorusMat);

    TheConsole = EONx_ConsoleCreate(800, // Screen width
                                    600, // Screen height
                                    90.0 // Field of view
                                    );

    TheCube = EON_MakeBox(100.0,100.0,100.0,CubeMat); // Create the cube
    TheTorus = EON_MakeTorus(40.0,100.0,10,8,TorusMat); // Create the torus
    TheTorus->Xp = -70.0; // Shift the torus to the left a bit

    TheFrame = EONx_ConsoleGetFrame(TheConsole);
    TheCamera = EONx_ConsoleGetCamera(TheConsole);
    TheCamera->Pos.Z = -300; // Back the camera up from the origin

    TheLight = EON_LightCreate(); // Create the light. Will be set up every frame

    TheRend = EON_RendCreate(TheCamera);

    start = time(NULL);
    while (!EONx_ConsoleNextEvent(TheConsole)) { // While the keyboard hasn't been touched
        TheCube->Xa += 1.0; // Rotate cube by 1 degree on each axis
        TheCube->Ya += 1.0;
        TheCube->Za += 1.0;

        TheTorus->Xa += 1.9;  // Rotate the torus
        TheTorus->Ya -= 1.0;
        TheTorus->Za += 0.3;

        TheLight_Za += 1.0; // Rotate the light
        TheLight_Xa = 50.0;

        EON_LightSet(TheLight, EON_LIGHT_VECTOR, // Set the newly rotated light
                     TheLight_Xa, TheLight_Ya, TheLight_Za, // angles
                     1.0, // intensity
                     1.0); // falloff, not used for vector lights

        EONx_ConsoleClearFrame(TheConsole);
        EON_RenderBegin(TheRend);           // Start rendering with the camera
        EON_RenderLight(TheRend, TheLight); // Render our light
        EON_RenderObj(TheRend, TheCube);    // Render our object
        EON_RenderObj(TheRend, TheTorus);   // Render our torus
        EON_RenderEnd(TheRend, TheFrame);   // Finish rendering
        EONx_ConsoleShowFrame(TheConsole);
        frames++;
    }
    stop = time(NULL);

    fprintf(stderr, "(%s) %i frames in %f seconds: %.3f FPS\n",
            __FILE__,
            frames, (double)stop-(double)start,
            (double)frames/((double)stop-(double)start));

    EONx_ConsoleDelete(TheConsole);
    EON_LightDelete(TheLight);
    EON_ObjDelete(TheCube);
    EON_ObjDelete(TheTorus);
    EON_MatDelete(CubeMat);
    EON_MatDelete(TorusMat);

    return EONx_ConsoleShutdown();
}

