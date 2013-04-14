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
    EON_Mat *AllMaterials[3]; // Used for creating palette
    EON_Cam *TheCamera; // Our camera
    EON_Rend *TheRend;
    EON_Frame *TheFrame;
    EONx_Console *TheConsole;
    uint8_t ThePalette[3 * 256];

    EONx_ConsoleStartup("Eon3D :: example 2", NULL);

    CubeMat = EON_MatCreate();    // Create the material for the cube
    CubeMat->NumGradients = 100; // Have it use 100 colors
    CubeMat->ShadeType = EON_SHADE_FLAT; // Make the cube flat shaded

    CubeMat->Ambient[0] = 132; // Set red ambient component
    CubeMat->Ambient[1] = 0;  // Set green ambient component
    CubeMat->Ambient[2] = 16; // Set blue ambient component

    CubeMat->Diffuse[0] = 200; // Set red diffuse component
    CubeMat->Diffuse[1] = 100; // Set green diffuse component
    CubeMat->Diffuse[2] = 150; // Set blue diffuse component

    EON_MatInit(CubeMat);          // Initialize the material

    TorusMat = EON_MatCreate();    // Create the material for the torus
    TorusMat->NumGradients = 100; // Have it use 100 colors
    TorusMat->ShadeType = EON_SHADE_GOURAUD; // Make the torus gouraud shaded
    TorusMat->Shininess = 10; // Make the torus a bit more shiny

    TorusMat->Ambient[0] = 0; // Set red ambient component
    TorusMat->Ambient[1] = 132;  // Set green ambient component
    TorusMat->Ambient[2] = 4; // Set blue ambient component

    TorusMat->Diffuse[0] = 20; // Set red diffuse component
    TorusMat->Diffuse[1] = 60; // Set green diffuse component
    TorusMat->Diffuse[2] = 70; // Set blue diffuse component

    TorusMat->Specular[0] = 100; // Set red specular component
    TorusMat->Specular[1] = 200; // Set green specular component
    TorusMat->Specular[2] = 150; // Set blue specular component

    AllMaterials[0] = CubeMat; // Make list of materials
    AllMaterials[1] = TorusMat; // Make list of materials
    AllMaterials[2] = 0; // Null terminate list of materials
    EON_MatMakeOptPal(ThePalette,1,255,AllMaterials,3); // Create a nice palette

    ThePalette[0] = ThePalette[1] = ThePalette[2] = 0; // Color 0 is black

    EON_MatMapToPal(CubeMat,ThePalette,0,255); // Map the material to our palette
    EON_MatMapToPal(TorusMat,ThePalette,0,255); // Map the material to our palette

    TheConsole = EONx_ConsoleNew(800, // Screen width
                          600, // Screen height
                          90.0 // Field of view
                          );

    EONx_ConsoleSetPalette(TheConsole, ThePalette, 256);

    TheCube = EON_MakeBox(100.0,100.0,100.0,CubeMat); // Create the cube
    TheTorus = EON_MakeTorus(40.0,100.0,10,8,TorusMat); // Create the torus
    TheTorus->Xp = -70.0; // Shift the torus to the left a bit

    TheFrame = EONx_ConsoleGetFrame(TheConsole);
    TheCamera = EONx_ConsoleGetCamera(TheConsole);
    TheCamera->Z = -300; // Back the camera up from the origin

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

