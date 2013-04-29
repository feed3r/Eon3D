/*
 * landgen.c -- 3D world generator/explorer using Eon3.
 * (C)2013 Francesco Romani <fromani at gmail dot com>
 */

#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

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

typedef struct _World {
    EON_Obj *land;
    EON_Mat *landMat;
    EON_Obj *sky;
    EON_Mat *skyMat;
} World;

typedef struct _Options {
    double distance;
    int verbose;
    int frames;
    int width, height;
    int seed;
} Options;

typedef struct _EONLandGen {
    Options opts;
    World world;
    CX_LogContext *logger;
} EONLandGen;

#define EXE "landgen"

static void usage(void)
{
    fprintf(stderr, "Usage: %s [options]\n", EXE);
    fprintf(stderr, "    -d dist           Render at the `dist' lgen distance. Affects performance.\n");
    fprintf(stderr, "    -v verbosity      Verbosity mode.\n");
    fprintf(stderr, "    -n frames         Renders `frames' frames.\n");
    fprintf(stderr, "    -g w,h            Frame size Width,Height in pixel.\n");
    fprintf(stderr, "    -s seed           Generation seed.\n");
    fprintf(stderr, "\n");
}

static void opts_Defaults(Options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->distance = 50.0;
    opts->verbose = 1;
    opts->frames = -1;
    opts->width = 800;
    opts->height = 600;
    opts->seed = 0;
    return;
}

static int opts_Parse(Options *opts, int argc, char *argv[])
{
    int ch = -1;

    while (1) {
        ch = getopt(argc, argv, "d:g:n:R:S:v:h?");
        if (ch == -1) {
            break;
        }

        switch (ch) {
          case 'd':
            opts->distance = atof(optarg);
            break;
          case 'n':
            opts->frames = atoi(optarg);
            break;
          case 'v':
            opts->verbose = atoi(optarg);
            break;
          case 'g':
            if (sscanf(optarg, "%i,%i",
                       &opts->width, &opts->height) != 2
             || (opts->width  < 320 || opts->width > 1920)
             || (opts->height < 200 || opts->height > 1080)) { // XXX
                opts->width = 800;
                opts->height = 600;
            }
            break;
          case 's':
            opts->seed = atoi(optarg);
            break;
          case '?': /* fallthrough */
          case 'h': /* fallthrough */
          default:
            usage();
            return 1;
        }
    }

    return 0;
}

static int opts_ErrCode(int err)
{
    int ret = 0;
    if (err < 0) {
        ret = -err;
    } // else coalesce on zero
    return ret;
}


void world_setupLandscape(World *W,
                          CX_LogContext *Logger)
{
    int i;

    W->landMat = EON_MatCreate();
    W->landMat->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    W->landMat->Shininess = 1;
    W->landMat->Ambient.R = 12;//255;
    W->landMat->Ambient.G = 200;//255;
    W->landMat->Ambient.B = 5;//255;
    W->landMat->Diffuse.R = 127;
    W->landMat->Diffuse.G = 127;
    W->landMat->Diffuse.B = 127;
    W->landMat->Specular.R = 127;
    W->landMat->Specular.G = 127;
    W->landMat->Specular.B = 127;
    W->landMat->FadeDist = 10000.0;
    W->landMat->PerspectiveCorrect = 16;
    EON_MatInit(W->landMat);
    EON_MatInfo(W->landMat, Logger);

    W->skyMat = EON_MatCreate();
    W->skyMat->ShadeType = EON_SHADE_GOURAUD_DISTANCE;
    W->skyMat->Shininess = 1;
    W->skyMat->Ambient.R = 5;//255;
    W->skyMat->Ambient.G = 12;//255;
    W->skyMat->Ambient.B = 60;//255;
    W->skyMat->Diffuse.R = 127;
    W->skyMat->Diffuse.G = 127;
    W->skyMat->Diffuse.B = 127;
    W->skyMat->Specular.R = 127;
    W->skyMat->Specular.G = 127;
    W->skyMat->Specular.B = 127;
    W->skyMat->FadeDist = 10000.0;
    W->skyMat->PerspectiveCorrect = 32;
    EON_MatInit(W->skyMat);
    EON_MatInfo(W->skyMat, Logger);

    // make our root object the land
    W->land = EON_MakePlane(LAND_SIZE, LAND_SIZE, LAND_DIV-1,
                            W->landMat);
    // give it a nice random bumpy effect
    for (i = 0; i < W->land->NumVertices; i ++) {
        W->land->Vertices[i].y += (float) (rand()%1400)-700;
/*        if (i % 2 == 0) {
            W->land->Vertices[i].y += (float) (rand()%400)-200;
        }
        if (i % 4 == 0) {
            W->land->Vertices[i].y += (float) (rand()%400)-200;
        }
        if (i % 6 == 0) {
            W->land->Vertices[i].y += (float) (rand()%400)-200;
        }
        if (i % 8 == 0) {
            W->land->Vertices[i].y += (float) (rand()%400)-200;
        }
*/
    }
    for (i = 1; i < W->land->NumVertices-1; i ++) {
        W->land->Vertices[i].y = (W->land->Vertices[i-1].y
                                + W->land->Vertices[i  ].y
                                + W->land->Vertices[i+1].y)
                                /3;
    }
    // gotta recalculate normals for backface culling to work right
    EON_ObjCalcNormals(W->land);

    // Make our first child the first sky
    W->sky = EON_MakePlane(LAND_SIZE, LAND_SIZE, 1, W->skyMat);
    W->sky->Yp = 2000;
    W->sky->BackfaceCull = 0;

    EON_ObjInfo(W->land, Logger);
    EON_ObjInfo(W->sky,  Logger);

    return;
}

void world_cleanLandscape(World *W)
{
    EON_ObjDelete(W->land);
    EON_ObjDelete(W->sky);
    EON_MatDelete(W->landMat);
    EON_MatDelete(W->skyMat);
    return;
}


// FIXME: move to/from SDL
uint64_t eon_gettime_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}  


static int onkey_Screenshot(int key, EONx_Console *ctx, void *userdata)
{
    EONLandGen *lgen = userdata;
    int err = 0;
    char name[1024] = { '\0' }; // XXX
    EONx_ConsoleMakeName(ctx, name, sizeof(name),
                         "screenshot", "png");
    err = EONx_ConsoleSaveFrame(ctx, name, "png");
    CX_log_trace(lgen->logger, CX_LOG_INFO, EXE,
                 "saving screenshot to [%s] -> %s (%i)\n",
                 name, (err) ?EONx_ConsoleGetError(ctx) :"OK", err);
    return err;
}

static int onkey_Quit(int key, EONx_Console *ctx, void *userdata)
{
    EONLandGen *lgen = userdata;
    CX_log_trace(lgen->logger, CX_LOG_INFO, EXE, "Bye!");
    return 1;
}


int main(int argc, char *argv[])
{
    int err = 0;
    int frames = 0;
    time_t start = 0;
    time_t stop = 0;

    EONLandGen lgen;
    EONx_Console *con;
    EON_Cam *cam;
    EON_Rend *rend;
    EON_Frame *frame;
    EON_Font *font;
    uint64_t ts = 0;

    lgen.logger = CX_log_open_console(CX_LOG_MARK, stderr);

    opts_Defaults(&lgen.opts);
    err = opts_Parse(&lgen.opts, argc, argv);
    if (err) {
        return opts_ErrCode(err);
    }

    if (lgen.opts.seed == 0) {
        lgen.opts.seed = time(0);
    }

    CX_log_trace(lgen.logger, CX_LOG_INFO, EXE,
                 "Frame Size: %ix%i",
                 lgen.opts.width, lgen.opts.height);
    CX_log_trace(lgen.logger, CX_LOG_INFO, EXE,
                 "Generation Seed: %i",
                 lgen.opts.seed);

    srand(lgen.opts.seed);

    EONx_ConsoleStartup("Eon3D Land Generator", NULL);

    con = EONx_ConsoleCreate(lgen.opts.width, lgen.opts.height, 90);

    EONx_ConsoleBindEventKey(con, 's', onkey_Screenshot, &lgen); // XXX
    EONx_ConsoleBindEventKey(con, 'q', onkey_Quit, &lgen); // XXX

    frame = EONx_ConsoleGetFrame(con);
    cam = EONx_ConsoleGetCamera(con);
    rend = EON_RendCreate(cam);
    cam->Pos.Y = 800; // move the camera up from the ground

    font = EON_TextDefaultFont();

    world_setupLandscape(&lgen.world, lgen.logger); // create landscape

    ts = eon_gettime_ms();
    while (!EONx_ConsoleNextEvent(con)) {
        // save time when the frame began, to be used later.
        uint64_t elapsed = 0;
        frames++;

        cam->Pos.Z += 1;

        EONx_ConsoleClearFrame(con);

        // otherwise, render the sky (but not the second sky),
        // and the land, with a far clip plane
        cam->ClipBack = 10000.0;
        EON_RenderBegin(rend);
        EON_RenderObj(rend, lgen.world.sky);
        EON_RenderObj(rend, lgen.world.land);
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

    CX_log_trace(lgen.logger, CX_LOG_INFO, EXE,
                 "%i frames in %f seconds: %.3f FPS\n",
                 frames, (double)stop-(double)start,
                 (double)frames/((double)stop-(double)start));

    world_cleanLandscape(&lgen.world);
    EON_FontDelete(font);

    EONx_ConsoleShutdown();

    return CX_log_close(lgen.logger);
}

