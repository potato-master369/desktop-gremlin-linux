#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#define NFRAMES 1
#define WIDTH   500
#define HEIGHT  500

// globals
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
        if (gc) XFreeGC(d, gc);
        if (w)  XDestroyWindow(d, w);
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
    XSetWindowAttributes swa;
    swa.override_redirect = True;

    w = XCreateWindow(
        d, RootWindow(d, screen),
                      200, 200, WIDTH, HEIGHT, 0,
                      DefaultDepth(d, screen),
                      InputOutput,
                      DefaultVisual(d, screen),
                      CWOverrideRedirect, &swa
    );
    if (!w) {
        fprintf(stderr, "Failed to create window\n");
        cleanup(0);
    }

    // select input events (mouse clicks, exposure)
    XSelectInput(d, w, ExposureMask | ButtonPressMask);

    XMapWindow(d, w);
    gc = XCreateGC(d, w, 0, NULL);

    // load frame
    char filename[256];
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "HOME not set\n");
        cleanup(0);
    }

    XpmAttributes xpm_attrs;
    snprintf(filename, sizeof(filename),
             "%s/Desktop/desktop-gremlin-assets/tachyon.xpm", home);
    xpm_attrs.valuemask = 0;
    int status = XpmReadFileToPixmap(d, w, filename,
                                     &frames[0], &masks[0], &xpm_attrs);
    if (status != XpmSuccess || frames[0] == None) {
        fprintf(stderr, "Failed to load %s (status=%d)\n", filename, status);
        frames[0] = None;
        masks[0]  = None;
    }

    Window root = DefaultRootWindow(d);
    Window ret_root, ret_child;
    int root_x = 0, root_y = 0, win_x, win_y;
    unsigned int mask;
    XWindowAttributes wa;
    int dx = 0, dy = 0, new_x = 200, new_y = 200;
    char prompt[64];

    printf("Starting loop...\n");
    while (1) {
        // check for events
        while (XPending(d)) {
            XEvent e;
            XNextEvent(d, &e);

            if (e.type == Expose) {
                if (frames[0] != None) {
                    XCopyArea(d, frames[0], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
                }
                if (masks[0] != None) {
                    XShapeCombineMask(d, w, ShapeBounding, 0, 0, masks[0], ShapeSet);
                }
            }

            if (e.type == ButtonPress) {
                snprintf(prompt, sizeof(prompt), "paplay $HOME/Desktop/desktop-gremlin-assets/aud%d.mp3", (rand() % 4) + 1);
                system(prompt);
            }
        }


            usleep(1000000 / 30); // ~30 fps
    }

    cleanup(0);
    return 0;
}
