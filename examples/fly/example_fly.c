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

#include <logkit.h>


enum {
    LAND_SIZE = 65000, /* Physical size of land */
    LAND_DIV  = 128    /* Number of divisions of the land.
                          Higher number == more polygons */
};

void setup_materials(EONx_Console *con, EON_Mat **mat,
                     CX_LogContext *Logger)
{
    // create our 3 materials, make the fourth null so that EON_MatMakeOptPal2()
    // knows where to stop
    mat[0] = EON_MatCreate();
    mat[1] = EON_MatCreate();
    mat[2] = EON_MatCreate();
    mat[3] = 0;

    // set up material 0 (the ground)
    mat[0]->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    mat[0]->Shininess = 1;
    mat[0]->Ambient.R = 255;
    mat[0]->Ambient.G = 255;
    mat[0]->Ambient.B = 255;
    mat[0]->Diffuse.R = 127;
    mat[0]->Diffuse.G = 127;
    mat[0]->Diffuse.B = 127;
    mat[0]->Specular.R = 127;
    mat[0]->Specular.G = 127;
    mat[0]->Specular.B = 127;
    mat[0]->FadeDist = 10000.0;
    mat[0]->Texture = EONx_ReadPCXTex("ground.pcx");
    mat[0]->TexScaling = 40.0*LAND_SIZE/50000;
    mat[0]->PerspectiveCorrect = 16;
    EON_TexInfo(mat[0]->Texture, Logger);

    // set up material 1 (the sky)
    mat[1]->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    mat[1]->Shininess = 1;
    mat[1]->Ambient.R = 255;
    mat[1]->Ambient.G = 255;
    mat[1]->Ambient.B = 255;
    mat[1]->Diffuse.R = 127;
    mat[1]->Diffuse.G = 127;
    mat[1]->Diffuse.B = 127;
    mat[1]->Specular.R = 127;
    mat[1]->Specular.G = 127;
    mat[1]->Specular.B = 127;
    mat[1]->FadeDist = 10000.0;
    mat[1]->Texture = EONx_ReadPCXTex("sky.pcx");
    mat[1]->TexScaling = 45.0*LAND_SIZE/50000;
    mat[1]->PerspectiveCorrect = 32;
    EON_TexInfo(mat[1]->Texture, Logger);

    // set up material 2 (the second sky)
    mat[2]->ShadeType = EON_SHADE_NONE;
    mat[2]->Shininess = 1;
    mat[2]->Texture = EONx_ReadPCXTex("sky2.pcx");
    mat[2]->TexScaling = 10.0; //200.0*LAND_SIZE/50000;
    mat[2]->PerspectiveCorrect = 2;
    EON_TexInfo(mat[2]->Texture, Logger);

    // intialize the materials
    EON_MatInit(mat[0]);
    EON_MatInfo(mat[0], Logger);
    EON_MatInit(mat[1]);
    EON_MatInfo(mat[1], Logger);
    EON_MatInit(mat[2]);
    EON_MatInfo(mat[2], Logger);

    return;
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
    EON_Mat *mat[3];
    EON_Cam *cam;
    EON_Obj *land;
    EON_Obj *sky, *sky2;
    EONx_Console *con;
    EON_Rend *rend;
    EON_Frame *frame;
    EON_Font *font;
    uint64_t ts = 0;

    CX_LogContext *Logger = CX_log_open_console(CX_LOG_MARK, stderr);

    srand(0); // initialize prng

    EONx_ConsoleStartup("Eon3D :: Fly v1.1", NULL);
    con = EONx_ConsoleCreate(800, // Screen width
                             600, // Screen height
                             90.0 // Field of view
                             );

    frame = EONx_ConsoleGetFrame(con);
    cam = EONx_ConsoleGetCamera(con);
    rend = EON_RendCreate(cam);
    cam->Pos.Y = 800; // move the camera up from the ground

    font = EON_TextDefaultFont();

    setup_materials(con, mat, Logger); // intialize materials and palette

    land = setup_landscape(mat[0],mat[1],mat[2]); // create landscape
    sky = land->Children[0]; // unhierarchicalize the sky from the land
    land->Children[0] = 0;
    sky2 = land->Children[1];
    land->Children[1] = 0;

    EON_ObjInfo(land, Logger);
    EON_ObjInfo(sky,  Logger);
    EON_ObjInfo(sky2, Logger);

    frames = 0;     // set up for framerate counter
    ts = eon_gettime_ms();
    while (!EONx_ConsoleNextEvent(con)) {
        // save time when the frame began, to be used later.
        uint64_t elapsed = 0;
        frames++;

        cam->Pos.Z += 1;

        EONx_ConsoleClearFrame(con);

        if (cam->Pos.Y > 2000) {
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

        EONx_ConsoleShowFrame(con);

        // wraparound
        if (cam->Pos.X >  LAND_SIZE/2) cam->Pos.X = -LAND_SIZE/2;
        if (cam->Pos.X < -LAND_SIZE/2) cam->Pos.X =  LAND_SIZE/2;
        if (cam->Pos.Z >  LAND_SIZE/2) cam->Pos.Z = -LAND_SIZE/2;
        if (cam->Pos.Z < -LAND_SIZE/2) cam->Pos.Z =  LAND_SIZE/2;
        if (cam->Pos.Y <  0          ) cam->Pos.Y =  8999;
        if (cam->Pos.Y >  8999       ) cam->Pos.Y =  0;
    }

    EON_FontDelete(font);
    EON_ObjDelete(land);
    EON_ObjDelete(sky);
    EON_ObjDelete(sky2);
    EON_MatDelete(mat[0]);
    EON_MatDelete(mat[1]);
    EON_MatDelete(mat[2]);

    EONx_ConsoleShutdown();

    return CX_log_close(Logger);
}

