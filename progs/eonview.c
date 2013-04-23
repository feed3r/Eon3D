/*
 * eonview.c -- 3D model inspector/viewer using Eon3.
 * (C)2013 Francesco Romani <fromani at gmail dot com>
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <eon3d.h>
#include <eon3dx.h>
#include <eon3dx_console.h>
#include <eon3dx_reader.h>

#include <logkit.h>

typedef struct _Options {
    const char *filename;
    double distance;
    int verbose;
    int frames;
} Options;

typedef struct _EONViewer {
    Options opts;
    CX_LogContext *logger;
} EONViewer;

#define EXE "eonview"

static void usage(void)
{
    fprintf(stderr, "Usage: %s [options] model\n", EXE);
    fprintf(stderr, "    -d dist           Render at the `dist' view distance. Affects performance.\n");
    fprintf(stderr, "    -v verbosity      Verbosity mode.\n");
    fprintf(stderr, "    -n frames         Renders `frames' frames.\n");
    fprintf(stderr, "\n");
}

static void opts_Defaults(Options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->filename = NULL;
    opts->distance = 50.0;
    opts->verbose = 1;
    opts->frames = -1;
    return;
}

static int opts_Parse(Options *opts, int argc, char *argv[])
{
    int ch = -1;

    while (1) {
        ch = getopt(argc, argv, "d:n:v:h?");
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
          case '?': /* fallthrough */
          case 'h': /* fallthrough */
          default:
            usage();
            return 1;
        }
    }

    /* XXX: watch out here */
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        usage();
        return -1;
    }
    opts->filename = argv[0];

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


int main(int argc, char *argv[])
{
    int err = 0;
    int frames = 0;
    time_t start = 0;
    time_t stop = 0;

    EON_Light *light;
    EON_Obj *model;
    EON_Mat *material;
    EON_Cam *camera;
    EON_Rend *rend;
    EON_Frame *frame;
    EONx_Console *console;
    EONViewer view;

    view.logger = CX_log_open_console(CX_LOG_MARK, stderr);

    opts_Defaults(&view.opts);
    err = opts_Parse(&view.opts, argc, argv);
    if (err) {
        return opts_ErrCode(err);
    }

    EONx_ConsoleStartup("Eon3D Model Viewer", NULL);

    material = EON_MatCreate(); 
    material->ShadeType = EON_SHADE_FLAT;

    material->Ambient[0] = 200;
    material->Ambient[1] = 200;
    material->Ambient[2] = 200;

    EON_MatInit(material);

    console = EONx_ConsoleNew(800, // Screen width
                                 600, // Screen height
                                 90.0 // Field of view
                                 );

    model = EONx_ReadPLYObj(view.opts.filename, material);

    frame = EONx_ConsoleGetFrame(console);
    camera = EONx_ConsoleGetCamera(console);
    camera->Z = -view.opts.distance;

    light = EON_LightNew(EON_LIGHT_VECTOR,
                            0.0, 0.0, 0.0, // rotation angles
                            1.0, // intensity
                            1.0); // falloff, not used for vector lights

    rend = EON_RendCreate(camera);

    start = time(NULL);
    while (!EONx_ConsoleNextEvent(console)) {
        // Rotate by 1 degree on each axis
        model->Xa += 1.0;
        model->Ya += 1.0;
        model->Za += 1.0;
        EONx_ConsoleClearFrame(console);
        EON_RenderBegin(rend);
        EON_RenderLight(rend, light);
        EON_RenderObj(rend, model);
        EON_RenderEnd(rend, frame);
        EONx_ConsoleShowFrame(console);
        frames++;
    }
    stop = time(NULL);

    fprintf(stderr, "(%s) %i frames in %f seconds: %.3f FPS\n",
            __FILE__,
            frames, (double)stop-(double)start,
            (double)frames/((double)stop-(double)start));

    return EONx_ConsoleShutdown();
}

