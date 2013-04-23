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
    int width, height;
    double rx, ry, rz;
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
    fprintf(stderr, "    -g w,h            Frame size Width,Height in pixel.\n");
    fprintf(stderr, "    -r x,y,z          Rotates around axes of the specified degrees.\n");
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
          case 'g':
            if (sscanf(optarg, "%i,%i",
                       &opts->width, &opts->height) != 2
             || (opts->width  < 320 || opts->width > 1920)
             || (opts->height < 200 || opts->height > 1080)) { // XXX
                opts->width = 800;
                opts->height = 600;
            }
            break;
          case 'r':
            if (sscanf(optarg, "%lf,%lf,%lf",
                       &opts->rx, &opts->ry, &opts->rz) != 3) { // XXX
                opts->rx = 1.0;
                opts->ry = 1.0;
                opts->rz = 1.0;
            }
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

static int onkey_Screenshot(int key, EONx_Console *ctx, void *userdata)
{
    EONViewer *view = userdata;
    int err = 0;
    char name[1024] = { '\0' }; // XXX
    EONx_ConsoleMakeName(ctx, name, sizeof(name),
                         "screenshot", "png");
    err = EONx_ConsoleSaveFrame(ctx, name, "png");
    CX_log_trace(view->logger, CX_LOG_INFO, EXE,
                 "saving screenshot to [%s] -> %s (%i)\n",
                 name, (err) ?EONx_ConsoleGetError(ctx) :"OK", err);
    return err;
}

static int onkey_Quit(int key, EONx_Console *ctx, void *userdata)
{
    EONViewer *view = userdata;
    CX_log_trace(view->logger, CX_LOG_INFO, EXE, "Bye!");
    return 1;
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

    CX_log_trace(view.logger, CX_LOG_INFO, EXE,
                 "Frame Size: %ix%i",
                 view.opts.width, view.opts.height);
    CX_log_trace(view.logger, CX_LOG_INFO, EXE,
                 "Model rotation: X=%f Y=%f Z=%f",
                 view.opts.rx, view.opts.ry, view.opts.rz);

    EONx_ConsoleStartup("Eon3D Model Viewer", NULL);

    material = EON_MatCreate(); 
    material->ShadeType = EON_SHADE_FLAT;

    material->Ambient[0] = 200;
    material->Ambient[1] = 200;
    material->Ambient[2] = 200;

    EON_MatInit(material);

    console = EONx_ConsoleNew(view.opts.width, view.opts.height, 90);

    EONx_ConsoleBindEventKey(console, 's', onkey_Screenshot, &view); // XXX
    EONx_ConsoleBindEventKey(console, 'q', onkey_Quit, &view); // XXX

    model = EONx_ReadPLYObj(view.opts.filename, material);

    frame = EONx_ConsoleGetFrame(console);
    camera = EONx_ConsoleGetCamera(console);
    camera->Z = -view.opts.distance;

    light = EON_LightNew(EON_LIGHT_VECTOR, 0.0, 0.0, 0.0, 1.0, 1.0);

    rend = EON_RendCreate(camera);

    start = time(NULL);
    while (!EONx_ConsoleNextEvent(console)) {
        model->Xa += view.opts.rx;
        model->Ya += view.opts.ry;
        model->Za += view.opts.rz;
        EONx_ConsoleClearFrame(console);
        EON_RenderBegin(rend);
        EON_RenderLight(rend, light);
        EON_RenderObj(rend, model);
        EON_RenderEnd(rend, frame);
        EONx_ConsoleShowFrame(console);
        frames++;
    }
    stop = time(NULL);

    CX_log_trace(view.logger, CX_LOG_INFO, EXE,
                 "%i frames in %f seconds: %.3f FPS\n",
                 frames, (double)stop-(double)start,
                 (double)frames/((double)stop-(double)start));

    return EONx_ConsoleShutdown();
}

