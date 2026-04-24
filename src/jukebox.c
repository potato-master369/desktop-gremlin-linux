#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#define NFRAMES 2
#define WIDTH 300
#define HEIGHT 450

// def globals
Display *d = NULL;
Window w = 0;
GC gc = 0;
Pixmap frames[NFRAMES];
Pixmap masks[NFRAMES];

void cleanup(int sig) {
    if (d) {
        for (int i = 0; i < NFRAMES; i++) {
            if (frames[i] != None) XFreePixmap(d, frames[i]);
            if (masks[i]  != None) XFreePixmap(d, masks[i]);
        }
        XFreeGC(d, gc);
        XDestroyWindow(d, w);
        XCloseDisplay(d);
    }
    _exit(0);
}

int main() {

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    d = XOpenDisplay(NULL);
    if (!d) {
        fprintf(stderr, "Failed to open display\n");
        return 1;
    }

    int screen = DefaultScreen(d);
    printf("Hello we loaded da screen things\n");

    XSetWindowAttributes swa;
    swa.override_redirect = True;

    w = XCreateWindow(
        d, RootWindow(d, screen),
                      800, 300, WIDTH, HEIGHT, 0,
                      DefaultDepth(d, screen),
                      InputOutput,
                      DefaultVisual(d, screen),
                      CWOverrideRedirect, &swa
    );
    if (!w) {
        fprintf(stderr, "Failed to create window\n");
        cleanup(0);
    }


    XMapWindow(d, w);
    gc = XCreateGC(d, w, 0, NULL);

    printf("Loading our frames i guess\n");

    // load frames & masks
    char filename[256];
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME not set\n");
        cleanup(0);
    }

    int i = 0;
    frames[i] = None;
    masks[i]  = None;

    printf("Loading frame %d\n", i);
    snprintf(filename, sizeof(filename),
                "%s/Desktop/desktop-gremlin-assets/jukebox%d.xpm", home, i);
    printf("Loading from file: %s\n", filename);

    XpmAttributes xpm_attrs;
    xpm_attrs.valuemask = 0; // default; not requesting extra data

    int status = XpmReadFileToPixmap(d, w, filename,
                                        &frames[i], &masks[i], &xpm_attrs);

    if (status != XpmSuccess || frames[i] == None) {
        fprintf(stderr, "Failed to load %s (status=%d)\n", filename, status);
        frames[i] = None;
        masks[i]  = None;
    }

    printf("Loaded frame %d successfully!\n", i);

    int current = 0;
    Window root = DefaultRootWindow(d);
    Window ret_root, ret_child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XWindowAttributes wa;
    double tmp_dir;
    int tmp_dir2;
    int final_dir;
    int dx, dy, new_x, new_y;
    int idle = 0;

    printf("Starting loop...");
    pid_t f = fork();

    if (f == 0) {
        while (1) {
            snprintf(filename, sizeof(filename),
                     "paplay %s/Desktop/desktop-gremlin-assets/umapyoi.mp3", home);
            system(filename);
        }
    }

    while (1) {
            // idle
            if (idle == 0) {
                current = 0;
                idle = 1;
            }
            XClearWindow(d, w);


            if (masks[current] != None) {
                XShapeCombineMask(d, w, ShapeBounding, 0, 0, masks[current], ShapeSet);
            }

            if (frames[current] != None) {
                XCopyArea(d, frames[current], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
            }

        // handle movement
        if (XQueryPointer(d, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask)) {
            printf("Mouse at: %d,%d, dir %d, dx %d, dy %d\n", root_x, root_y, final_dir, dx, dy);
        }

        usleep(500000); // ~30 fps
    }

    // Not reached, but in case:
    cleanup(0);
    return 0;
}
