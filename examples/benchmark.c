/* simple Eon3D benchmark using the PLY model loader. */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <eon3d.h>
#include <eon3dx.h>
#include <eon3dx_reader.h>

typedef struct null_console_ {
    EON_Cam *cam;
    EON_Frame *fb;
} NullConsole;

static void *NullConsoleDelete(NullConsole *ctx)
{
    if (ctx->cam) {
        EON_CamDelete(ctx->cam);
        ctx->cam = NULL;
    }
    if (ctx->fb) {
        EON_FrameDelete(ctx->fb);
        ctx->fb = NULL;
    }
    free(ctx);
    return NULL;
}

NullConsole *NullConsoleCreate(EON_uInt sw, EON_uInt sh)
{
    int err = -1;
    NullConsole *ctx = calloc(1, sizeof(*ctx));
    if (ctx) {
        ctx->fb = EON_FrameCreate(sw, sh, 4/* XXX */);
        if (ctx->fb) {
            ctx->cam = EON_CamCreate(sw, sh, 1.0, 90.0);
            if (ctx->cam) {
                err = 0;
            }
        }
    }
    if (err) {
        ctx = NullConsoleDelete(ctx);
    }
    return ctx;
}

int NullConsoleClearFrame(NullConsole *ctx)
{
    int err = -1;
    if (ctx) {
        EON_FrameClear(ctx->fb);
        err = 0;
    }
    return err;
}


#define EXE "benchmark.c"


static void usage(void)
{
    fprintf(stderr, "Usage: %s [options] model\n", EXE);
    fprintf(stderr, "    -d dist           Render at the `dist' view distance. Affects performance.\n");
    fprintf(stderr, "    -v verbosity      Verbosity mode.\n");
    fprintf(stderr, "    -n frames         Renders `frames' frames.\n");
    fprintf(stderr, "\n");
}


int main(int argc, char *argv[])
{
    const char *filename;
    int frames = 0;
    int framenum = 100000;
    time_t start = 0, stop = 0;
    // Our variables
    EON_Light *TheLight;   // Our light
    EON_Obj *TheModel;      // Our cube object
    EON_Mat *ModelMat;      // The material for the cube
    EON_Cam *TheCamera;
    EON_Rend *TheRend;
    EON_Frame *TheFrame;
    NullConsole *TheConsole;
    double distance = 50;
    int ch = -1;
    int verbose = 1;

    while (1) {
        ch = getopt(argc, argv, "d:n:v:h?");
        if (ch == -1) {
            break;
        }

        switch (ch) {
          case 'd':
            distance = atof(optarg);
            break;
          case 'n':
            framenum = atoi(optarg);
            break;
          case 'v':
            verbose = atoi(optarg);
            break;
          case '?': /* fallthrough */
          case 'h': /* fallthrough */
          default:
            usage();
            return 0;
        }
    }

    /* XXX: watch out here */
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
        return 1;
    }
    filename = argv[0];

    if (verbose) {
        fprintf(stderr, "[%s] settings> rendering model = %s\n",
                EXE, filename);
        fprintf(stderr, "[%s] settings> frame limit     = %i\n",
                EXE, framenum);
    }

    ModelMat = EON_MatCreate(); 
    ModelMat->ShadeType = EON_SHADE_FLAT;

    ModelMat->Ambient[0] = 216; // Set red ambient component
    ModelMat->Ambient[1] = 216; // Set green ambient component
    ModelMat->Ambient[2] = 216; // Set blue ambient component

    EON_MatInit(ModelMat);          // Initialize the material

    TheConsole = NullConsoleCreate(800, // Screen width
                                   600 // Screen height
                                   );

    TheModel = EONx_ReadPLYObj(filename, ModelMat);
    if (TheModel == NULL) {
        fprintf(stderr, "[%s] failed to load model '%s', aborting\n",
                EXE, filename);
        return 1;
    }

    TheFrame = TheConsole->fb;
    TheCamera = TheConsole->cam;
    TheCamera->Z = -distance; // Back the camera up from the origin

    TheLight = EON_LightNew(EON_LIGHT_VECTOR, // vector light
                            0.0, 0.0, 0.0, // rotation angles
                            1.0, // intensity
                            1.0); // falloff, not used for vector lights

    TheRend = EON_RendCreate(TheCamera);

    start = time(NULL);
    while (frames < framenum) {
        // Rotate by 1 degree on each axis
        TheModel->Xa += 1.0;
        TheModel->Ya += 1.0;
        TheModel->Za += 1.0;
        NullConsoleClearFrame(TheConsole);
        EON_RenderBegin(TheRend);           // Start rendering with the camera
        EON_RenderLight(TheRend, TheLight); // Render our light
        EON_RenderObj(TheRend, TheModel);   // Render our object
        EON_RenderEnd(TheRend, TheFrame);   // Finish rendering
        frames++;
    }
    stop = time(NULL);

    fprintf(stderr, "[%s] %i frames in %f seconds: %.3f FPS\n",
            EXE,
            frames, (double)stop-(double)start,
            (double)frames/((double)stop-(double)start));

    return 0;
}

