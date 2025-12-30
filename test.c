#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#define NFRAMES 215
#define WIDTH 325
#define HEIGHT 325

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

    for (int i = 0; i < NFRAMES; ++i) {
        frames[i] = None;
        masks[i]  = None;

        printf("Loading frame %d\n", i);
        snprintf(filename, sizeof(filename),
                 "%s/Desktop/desktop-gremlin-assets/%d.xpm", home, i);
        printf("Loading from file: %s\n", filename);

        XpmAttributes xpm_attrs;
        xpm_attrs.valuemask = 0; // default; not requesting extra data

        int status = XpmReadFileToPixmap(d, w, filename,
                                         &frames[i], &masks[i], &xpm_attrs);

        if (status != XpmSuccess || frames[i] == None) {
            fprintf(stderr, "Failed to load %s (status=%d)\n", filename, status);
            frames[i] = None;
            masks[i]  = None;
            continue;
        }

        printf("Loaded frame %d successfully!\n", i);
    }
    int current = 0;
    Window root = DefaultRootWindow(d);
    Window ret_root, ret_child;
    int root_x, root_y, win_x, win_y;
    unsigned int mask;
    XWindowAttributes wa;
    int final_dir;
    double tmp_dir;
    int dx, dy, new_x, new_y;
    int idle = 0;

    printf("Starting loop...");
    while (1) {

        if (dx != 0 || dy != 0) {
            int base = (final_dir == 270) ? 0 :
            (final_dir == 0)   ? 30 :
            (final_dir == 180) ? 60 :
            (final_dir == 90)  ? 90 : 0;
            int idx = base + (current % 30);
            if (masks[idx]  != None) XShapeCombineMask(d, w, ShapeBounding, 0, 0, masks[idx], ShapeSet);
            if (frames[idx] != None) XCopyArea(d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
            current = (current + 1) % 30;
        } else {
            if (system("pgrep -x jukebox > /dev/null") == 0) {
                // dance?
                int idx = 180 + (current % 35);
                if (masks[idx]  != None) XShapeCombineMask(d, w, ShapeBounding, 0, 0, masks[idx], ShapeSet);
                if (frames[idx] != None) XCopyArea(d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
                current = (current + 1) % 35;
            } else {
            int idx = 120 + (current % 60);
            if (masks[idx]  != None) XShapeCombineMask(d, w, ShapeBounding, 0, 0, masks[idx], ShapeSet);
            if (frames[idx] != None) XCopyArea(d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
            current = (current + 1) % 60;
            }
        }


        // handle movement
        if (XQueryPointer(d, root, &ret_root, &ret_child, &root_x, &root_y, &win_x, &win_y, &mask)) {
            printf("Mouse at: %d,%d, dx %d, dy %d\n", root_x, root_y, dx, dy);
        }

        // dir & distance
        XGetWindowAttributes(d, w, &wa);
        // delta from window center to mouse
        dx = root_x - wa.x;
        dy = root_y - wa.y;
        // angle in radians double
        tmp_dir = atan2(-dy, dx);
        int tmp_dir2 = (int)(tmp_dir * 180.0 / M_PI);
        if (tmp_dir2 < 0) tmp_dir2 += 360;
        final_dir = ((tmp_dir2 + 45) / 90) * 90 % 360;
        if (dx + dy > 40) {
            new_x = wa.x + dx / 12; new_y = wa.y + dy / 12;
        } else {
            new_x = wa.x + dx; new_y = wa.y + dy;
        }



        XMoveWindow(d, w, new_x, new_y);
        usleep(100000); // ~30 fps
    }

    // Not reached, but in case:
    cleanup(0);
    return 0;
}
