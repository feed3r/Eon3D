#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include <eon3d.h>
#include <eon3dx.h>
#include <eon3dx_console.h>
#include <eon3dx_reader.h>

enum {
    LAND_SIZE = 65000, /* Physical size of land */
    LAND_DIV  = 128    /* Number of divisions of the land.
                          Higher number == more polygons */
};

void setup_materials(EONx_Console *con, EON_Mat **mat, uint8_t *pal)
{
    // create our 3 materials, make the fourth null so that EON_MatMakeOptPal2()
    // knows where to stop
    mat[0] = EON_MatCreate();
    mat[1] = EON_MatCreate();
    mat[2] = EON_MatCreate();
    mat[3] = 0;

    pal[0] = pal[1] = pal[2] = 0; // make color 0 black.

    // set up material 0 (the ground)
    mat[0]->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    mat[0]->Shininess = 1;
    mat[0]->NumGradients = 2500;
    mat[0]->Ambient[0] = pal[0]*2 - 255; // these calculations are to get the
    mat[0]->Ambient[1] = pal[1]*2 - 255; // distance shading to work right
    mat[0]->Ambient[2] = pal[2]*2 - 255;
    mat[0]->Diffuse[0] = 127-pal[0];
    mat[0]->Diffuse[1] = 127-pal[1];
    mat[0]->Diffuse[2] = 127-pal[2];
    mat[0]->Specular[0] = 127-pal[0];
    mat[0]->Specular[1] = 127-pal[1];
    mat[0]->Specular[2] = 127-pal[2];
    mat[0]->FadeDist = 10000.0;
    mat[0]->Texture = EONx_ReadPCXTex("ground.pcx",1,1);
    mat[0]->TexScaling = 40.0*LAND_SIZE/50000;
    mat[0]->PerspectiveCorrect = 16;

    // set up material 1 (the sky)
    mat[1]->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    mat[1]->Shininess = 1;
    mat[1]->NumGradients = 1500;
    mat[1]->Ambient[0] = pal[0]*2 - 255;
    mat[1]->Ambient[1] = pal[1]*2 - 255;
    mat[1]->Ambient[2] = pal[2]*2 - 255;
    mat[1]->Diffuse[0] = 127-pal[0];
    mat[1]->Diffuse[1] = 127-pal[1];
    mat[1]->Diffuse[2] = 127-pal[2];
    mat[1]->Specular[0] = 127-pal[0];
    mat[1]->Specular[1] = 127-pal[1];
    mat[1]->Specular[2] = 127-pal[2];
    mat[1]->FadeDist = 10000.0;
    mat[1]->Texture = EONx_ReadPCXTex("sky.pcx",1,1);
    mat[1]->TexScaling = 45.0*LAND_SIZE/50000;
    mat[1]->PerspectiveCorrect = 32;

    // set up material 2 (the second sky)
    mat[2]->ShadeType = EON_SHADE_NONE;
    mat[2]->Shininess = 1;
    mat[2]->NumGradients = 1500;
    mat[2]->Texture = EONx_ReadPCXTex("sky2.pcx",1,1);
    mat[2]->TexScaling = 10.0; //200.0*LAND_SIZE/50000;
    mat[2]->PerspectiveCorrect = 2;

    // intialize the materials
    EON_MatInit(mat[0]);
    EON_MatInit(mat[1]);
    EON_MatInit(mat[2]);

    // make a nice palette
    EON_MatMakeOptPal(pal,1,255,mat,3);

    // map the materials to this new palette
    EON_MatMapToPal(mat[0],pal,0,255);
    EON_MatMapToPal(mat[1],pal,0,255);
    EON_MatMapToPal(mat[2],pal,0,255);

    EONx_ConsoleSetPalette(con, pal, 256);
}


EON_Obj *setup_landscape(EON_Mat *m, EON_Mat *sm, EON_Mat *sm2)
{
    int i;
    // make our root object the land
    EON_Obj *o = EON_MakePlane(LAND_SIZE,LAND_SIZE,LAND_DIV-1,m);
    // give it a nice random bumpy effect
    for (i = 0; i < o->NumVertices; i ++)
        o->Vertices[i].y += (float) (rand()%1400)-700;
    // gotta recalculate normals for backface culling to work right
    EON_ObjCalcNormals(o);

    // Make our first child the first sky
    o->Children[0] = EON_MakePlane(LAND_SIZE,LAND_SIZE,1,sm);
    o->Children[0]->Yp = 2000;
    o->Children[0]->BackfaceCull = 0;

    // and the second the second sky
    o->Children[1] = EON_MakeSphere(LAND_SIZE,10,10,sm2);
    o->Children[1]->Yp = 2000;
    EON_ObjFlipNormals(o->Children[1]);

    return o;
}

// FIXME: move to/from SDL
uint64_t eon_gettime_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}  

int main(int argc, char *argv[])
{
    int frames = 0;
    EON_Mat *mat[3+1]; // our materials, we have 1 extra for null
                       // termination for EON_MatMakeOptPal()
    EON_Cam *cam;
    EON_Obj *land;
    EON_Obj *sky, *sky2;
    EONx_Console *con;
    EON_Rend *rend;
    EON_Frame *frame;
    EON_Font *font;
    uint64_t ts = 0;

    uint8_t pal[3 * 256]; // our palette

    srand(0); // initialize prng

    EONx_ConsoleStartup("Eon3D :: Fly v1.0", NULL);
    con = EONx_ConsoleNew(800, // Screen width
                          600, // Screen height
                          90.0 // Field of view
                          );

    frame = EONx_ConsoleGetFrame(con);
    cam = EONx_ConsoleGetCamera(con);
    rend = EON_RendCreate(cam);
    cam->Y = 800; // move the camera up from the ground

    font = EON_TextDefaultFont();

    setup_materials(con,mat,pal); // intialize materials and palette

    land = setup_landscape(mat[0],mat[1],mat[2]); // create landscape
    sky = land->Children[0]; // unhierarchicalize the sky from the land
    land->Children[0] = 0;
    sky2 = land->Children[1];
    land->Children[1] = 0;

    frames = 0;     // set up for framerate counter
    ts = eon_gettime_ms();
    while (!EONx_ConsoleWaitKey(con)) {
        // save time when the frame began, to be used later.
        uint64_t elapsed = 0;
        frames++;

        cam->Z += 1;

        EONx_ConsoleClearFrame(con);

        if (cam->Y > 2000) {
            // if above the sky, only render the skies, with no far clip plane
            cam->ClipBack = 0.0;
            EON_RenderBegin(rend);
            EON_RenderObj(rend, sky);
            EON_RenderObj(rend, sky2);
        } else {
            // otherwise, render the sky (but not the second sky),
            // and the land, with a far clip plane
            cam->ClipBack = 10000.0;
            EON_RenderBegin(rend);
            EON_RenderObj(rend, sky);
            EON_RenderObj(rend, land);
        }
        EON_RenderEnd(rend, frame);

        elapsed = (eon_gettime_ms() - ts) / 1000000;
        EON_TextPrintf(font, cam, frame,
                       cam->ClipLeft+5, cam->ClipTop, 0.0,
                      "%.3f FPS",
                      (frames/ (double) elapsed));
        fprintf(stderr, 
                      "Camera={%f,%f,%f) %i frames %li elapsed %.3f FPS\r",
                      cam->X, cam->Y, cam->Z,
                      frames, (long)elapsed,
                      (frames/ (double) elapsed));
        
        EONx_ConsoleShowFrame(con);

        // wraparound
        if (cam->X >  LAND_SIZE/2) cam->X = -LAND_SIZE/2;
        if (cam->X < -LAND_SIZE/2) cam->X =  LAND_SIZE/2;
        if (cam->Z >  LAND_SIZE/2) cam->Z = -LAND_SIZE/2;
        if (cam->Z < -LAND_SIZE/2) cam->Z =  LAND_SIZE/2;
        if (cam->Y <  0          ) cam->Y =  8999;
        if (cam->Y >  8999       ) cam->Y =  0;
    }

    EON_FontDelete(font);
    EON_ObjDelete(land);
    EON_ObjDelete(sky);
    EON_ObjDelete(sky2);
    EON_MatDelete(mat[0]);
    EON_MatDelete(mat[1]);
    EON_MatDelete(mat[2]);

    return EONx_ConsoleShutdown();
}

