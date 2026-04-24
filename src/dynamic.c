#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>
#include <X11/xpm.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* inih -- simple .INI file parser
SPDX-License-Identifier: BSD-3-Clause
Copyright (C) 2009-2025, Ben Hoyt
See licenses/BSD3-LICENSE for more info */
#include "ini.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string.h>
/* Animation   | Offset | size | comment
 * down        | 0      | 30   | \
 * right       | 30     | 30   |  } main motion
 * left        | 60     | 30   |  |
 * up          | 90     | 30   | /
 * idle        | 120    | 60   | The idle animation played when there is no user input up till 20s
 * (old) emote2| 180    | 35   | removed; used incorrectly
 * click       | 180    | 111  | newly implemented; to replace emote2
 * grab        | 291    | 50   | animation used while dragged
 * hover       | 341    | 89   | animation used when hovering over sprite.
 * intro       | 430    | 100  | animation for intro sequence - use in fork with audio
 * outro       | 530    | 140  | outro animation; trigger with kill()
 * total       | 0      | 670  | -
 */

#define NFRAMES 670 // change with new assets added
#define WIDTH 325
#define HEIGHT 325
#ifndef M_PI
#define M_PI 3.14
#endif

typedef struct
{
    int InitX;
    int InitY;
    int InitPtrState;
    int InitIdle;
    int ChaseIdleReq;
    int TickDelay;
} configuration;

#ifdef GREMLIN_DEBUG
#warning "You are using the debug. Please do not use -DGREMLIN_DEBUG for better performance."
#endif
// def globals
Display *d = NULL;
Window w = 0;
GC gc = 0;
configuration config;
XRenderPictFormat *fmt;
Picture frames[NFRAMES];
Pixmap masks[NFRAMES];
Picture dst;
// move to top-level to allow referencing in sigrthandler
int current = 0;
int handlerwait = 0;

static void
drawf (short fid)
{
    dst = XRenderCreatePicture (d, w, fmt, 0, NULL);
    if (!dst)
    {
        fprintf (stderr, "dst failed\n");
        return;
    }
    XRenderColor clear = { 0, 0, 0, 0 }; // transparent black
    Picture fill = XRenderCreateSolidFill (d, &clear);
    XRenderComposite (d, PictOpSrc, fill, None, dst,
                      0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
    XRenderFreePicture (d, fill);

    // 请用这个function来解决后来发生的某事比如：
    // 关于XWayland或X11有些超大的区别
    if (!frames[fid])
    {
        fprintf (stderr, "src failed\n");
        return;
    }

    XShapeCombineMask (d, w, ShapeBounding, 0, 0, masks[fid], ShapeSet);
    XShapeCombineMask (d, w, ShapeInput, 0, 0, masks[fid], ShapeSet);

    XRenderComposite (d, PictOpOver, frames[fid], None, dst, 0, 0, 0, 0, 0, 0, WIDTH, HEIGHT);

    // FREEEEEEEEEEEEEEEEEEEEEEEEEEEEBIRD
    XRenderFreePicture (d, dst);
}
static int
handler (void *user, const char *section, const char *name,
         const char *value)
{
    configuration *pconfig = (configuration *)user;

#define MATCH(s, n) strcmp (section, s) == 0 && strcmp (name, n) == 0
    if (MATCH ("Window", "InitX"))
    {
        pconfig->InitX = atoi (value);
    }
    else if (MATCH ("Window", "InitY"))
    {
        pconfig->InitY = atoi (value);
    }
    else if (MATCH ("Tweaks", "InitPtrState"))
    {
        pconfig->InitPtrState = atoi (value);
    }
    else if (MATCH ("Tweaks", "InitIdle"))
    {
        pconfig->InitIdle = atoi (value);
    }
    else if (MATCH ("Tweaks", "ChaseIdleReq"))
    {
        pconfig->ChaseIdleReq = atoi (value);
    }
    else if (MATCH ("Tweaks", "TickDelay"))
    {
        pconfig->TickDelay = atoi (value);
    }
    else
    {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

void
cleanup (int sig)
{
#ifdef GREMLIN_DEBUG
    printf ("Cleanup time!\n");
#endif

    // Goodbye, cruel world!
    //  - our outro animation goes here!
    //  - in future, a handler will be created using
    //    kill() to find the PID of this process and
    //    send SIGINT.
    for (int i = 530; i < 670; ++i)
    {
        drawf (i);
        XFlush (d);
        usleep (config.TickDelay);
    }

    // start true cleanup
    if (d)
    {
        for (int i = 0; i < NFRAMES; i++)
        {
            if (frames[i] != None)
                XRenderFreePicture (d, frames[i]);
        }
        XFreeGC (d, gc);
        XDestroyWindow (d, w);
        XCloseDisplay (d);
    }
    fflush (stdout);
    _exit (0);
}

void
sigrthandler (int sig)
{
    XWindowAttributes wa;
#ifdef GREMLIN_DEBUG
    printf ("Recieved signal: %d", sig);
#endif
    switch (sig - SIGRTMIN)
    {
    case 0:
        // w
        XGetWindowAttributes (d, w, &wa);
        XMoveWindow (d, w, wa.x, wa.y - 10);
        break;
    case 1:
        // a
        XGetWindowAttributes (d, w, &wa);
        XMoveWindow (d, w, wa.x - 10, wa.y);
        break;
    case 2:
        // s
        XGetWindowAttributes (d, w, &wa);
        XMoveWindow (d, w, wa.x, wa.y + 10);
        break;
    case 3:
        // d
        XGetWindowAttributes (d, w, &wa);
        XMoveWindow (d, w, wa.x + 10, wa.y);
        break;
    default:
        // gracefully exit without issues
        break;
    }
    short base = (sig - SIGRTMIN == 2)   ? 0
                 : (sig - SIGRTMIN == 3) ? 30
                 : (sig - SIGRTMIN == 1) ? 60
                 : (sig - SIGRTMIN == 0) ? 90
                                         : 0;
    short idx = base + (current % 30);
    ++current;
    drawf (idx);
    handlerwait = 10;
}

int
main ()
{
    signal (SIGINT, cleanup);
    signal (SIGTERM, cleanup);
    signal (SIGRTMIN, sigrthandler);
    signal (SIGRTMIN + 1, sigrthandler);
    signal (SIGRTMIN + 2, sigrthandler);
    signal (SIGRTMIN + 3, sigrthandler);

    const char *wayland = getenv ("WAYLAND_DISPLAY");
    const char *x11 = getenv ("DISPLAY");
    if (wayland)
    {
        fprintf (stdout, "WARNING: YOU ARE RUNNING UNDER WAYLAND. Some features may be unavailable.\n");
        x11 = 0;
    }
    else if (!x11)
    {
        printf ("Unknown compositor\n");
        exit (1);
    }

    // READ CONFIGURATION

    config.InitX = 200;
    config.InitY = 200;
    config.InitPtrState = 0;
    config.InitIdle = 600;
    config.ChaseIdleReq = 600;
    config.TickDelay = 100000;

    // Detect $HOME
    char filename[256];

    snprintf (filename, sizeof (filename),
              "/usr/share/desktop-gremlin-linux/desktop-gremlin-assets/gremlin_config.ini");

    if (ini_parse (filename, handler, &config) < 0)
    {
        fprintf (stderr, "Can't load %s\n", filename);
        return 1;
    }

    d = XOpenDisplay (NULL);
    if (!d)
    {
        fprintf (stderr, "Failed to open display\n");
        return 1;
    }

    int screen = DefaultScreen (d);
#ifdef GREMLIN_DEBUG
    printf ("Hello we loaded da screen things\n");
#endif

    // visual info

    XVisualInfo vinfo;
    XMatchVisualInfo (
        d,                 // display
        DefaultScreen (d), // default screen
        32,                // 32-bit depth
        TrueColor,         // class
        &vinfo             // a valid visual on success
    );

    Colormap colormap = XCreateColormap (d, RootWindow (d, vinfo.screen), vinfo.visual, AllocNone);
    XSetWindowAttributes swa;
    swa.override_redirect = True;
    swa.colormap = colormap;

    // don't forget to check the result!

    w = XCreateWindow (d, RootWindow (d, screen), config.InitX, config.InitY, WIDTH, HEIGHT, 0, vinfo.depth, InputOutput, vinfo.visual,
                       CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect, &swa);
    if (!w)
    {
        fprintf (stderr, "Failed to create window\n");
        cleanup (0);
    }
    XSelectInput (d, w,
                  ButtonPressMask | ButtonReleaseMask | PointerMotionHintMask);

    XMapWindow (d, w);
    gc = XCreateGC (d, w, 0, NULL);
    XSetGraphicsExposures (d, gc, False);

#ifdef GREMLIN_DEBUG
    printf ("Loading our frames i guess\n");
#endif
    int width, height, channels;
    unsigned char *data;
    XImage *tmp;
    Pixmap tmpp;

    fmt = XRenderFindStandardFormat (d, PictStandardARGB32);
    for (int i = 0; i < NFRAMES; ++i)
    {
#ifdef GREMLIN_DEBUG
        printf ("Loading from file: %s\n", filename);
#endif
        masks[i] = XCreatePixmap (d, w, WIDTH, HEIGHT, 1);
        GC gc_mask = XCreateGC (d, masks[i], 0, NULL);
        XSetForeground (d, gc_mask, 0);
        XFillRectangle (d, masks[i], gc_mask, 0, 0, WIDTH, HEIGHT);
        frames[i] = None;
#ifdef GREMLIN_DEBUG
        printf ("Loading frame %d\n", i);
#endif
        snprintf (filename, sizeof (filename),
                  "/usr/share/desktop-gremlin-linux/desktop-gremlin-assets/%d.png", i);

        data = stbi_load (filename, &width, &height, &channels, 4);
        if (!data)
        {
            fprintf (stderr, "Failed to load PNG\n");
            exit (1);
        }
        // pre-multiply alphas
        for (int i = 0; i < width * height; i++)
        {
            // swap ABGR -> RGBA
            unsigned char r = data[4 * i + 0];
            unsigned char g = data[4 * i + 1];
            unsigned char b = data[4 * i + 2];
            unsigned char a = data[4 * i + 3];
            data[4 * i + 0] = b;
            data[4 * i + 1] = g;
            data[4 * i + 2] = r;
            data[4 * i + 3] = a; // alpha
            r = data[4 * i + 0];
            g = data[4 * i + 1];
            b = data[4 * i + 2];
            a = data[4 * i + 3];
            data[4 * i + 0] = (data[4 * i + 0] * a) / 255; // red
            data[4 * i + 1] = (data[4 * i + 1] * a) / 255; // green
            data[4 * i + 2] = (data[4 * i + 2] * a) / 255; // blue
            // alpha stays as-is
        }
        tmpp = XCreatePixmap (d, w, 325, 325, vinfo.depth);
        tmp = XCreateImage (d, vinfo.visual, vinfo.depth,
                            ZPixmap, 0,
                            (char *)data,
                            WIDTH, HEIGHT,
                            32, 0);
        GC gc2 = XCreateGC (d, tmpp, 0, NULL);
        XPutImage (d, tmpp, gc2, tmp, 0, 0, 0, 0, WIDTH, HEIGHT);
        XFreeGC (d, gc2);

        frames[i] = XRenderCreatePicture (d, tmpp, fmt, 0, NULL);
        XImage *mask_img = XCreateImage (d, DefaultVisual (d, screen), 1,
                                         ZPixmap, 0,
                                         calloc (WIDTH * HEIGHT, 1),
                                         WIDTH, HEIGHT, 8, 0);

        for (int y = 0; y < HEIGHT; ++y)
        {
            for (int x = 0; x < WIDTH; ++x)
            {
                unsigned char a = data[4 * (y * WIDTH + x) + 3]; // alpha
                if (a > 128)
                {
                    XPutPixel (mask_img, x, y, 1);
                }
            }
        }
        XPutImage (d, masks[i], gc_mask, mask_img, 0, 0, 0, 0, WIDTH, HEIGHT);
        XDestroyImage (mask_img);
        XFreeGC (d, gc_mask);

        XDestroyImage (tmp);
        XFreePixmap (d, tmpp);

#ifdef GREMLIN_DEBUG
        printf ("Loaded frame %d successfully!\n", i);
#endif
    }

    drawf (0);
    XFlush (d);

    //    Cleaned up by clanker so idk if this is wrong
    short idx;
    // State tracking

    short idle = config.InitIdle;
    char PtrState = config.InitPtrState;
    short final_dir = 0;

    // Direction and motion
    double tmp_dir = 0.0;
    int dx = 0, dy = 0;
    int new_x = 0, new_y = 0;

    // Pointer and window info
    int root_x = 0, root_y = 0;
    int win_x = 0, win_y = 0;
    unsigned int mask = 0;

    // idle
    int cachePX, cachePY;

    // X11 handles
    Window root = DefaultRootWindow (d);
    Window ret_root = 0;
    Window ret_child = 0;
    XWindowAttributes wa; // NOTE: if ur stupid or blind (or both), wa stands for Window Attributes

    XGetWindowAttributes (d, w, &wa);
#ifdef GREMLIN_DEBUG
    printf ("Window mapped at %d,%d size %dx%d\n", wa.x, wa.y, wa.width, wa.height);
#endif
#ifdef GREMLIN_DEBUG
    printf ("Starting loop...");
#endif
// put our intro here
//  - Must be after the loading, yet also before the main loop, so the audio will sync up properly!!
//  - do note that here we must try not to use X11 things in the other branch of fork() (in future as
//    of time of writing so it will be added later)
// EDIT PER 1/2/2026: its been like 2 months and im still too lazy to add audio.
#ifdef GREMLIN_DEBUG
    printf ("HELLO WORLD!");
#endif
    for (int i = 430; i < 530; ++i)
    {
        drawf (i);
        XFlush (d);
        usleep (config.TickDelay);
    }

    while (1)
    {
        if (handlerwait > 0)
        {
            handlerwait--; // skip drawing this tick
            usleep (20000);
            continue;
        }
#ifdef GREMLIN_DEBUG
        XGetWindowAttributes (d, w, &wa);
        printf ("New tick: %d, XPending: %d, PtrState: %d, winx: %d, winy: %d, wa.x: %d, wa.y: %d, rootx: %d, rooty: %d, delay: %d\n", idle, XPending (d), PtrState, win_x, win_y, wa.x, wa.y, root_x, root_y, config.TickDelay);
#endif
        // new thingy
        if (XPending (d) > 0)
        {             // is something going on? - IMPT; as XNextEvent will
                      // stall if there is no event
            XEvent e; // NOTE: e is created multiple times; to allow for
                      // e to be reset, in case some attributes are not updated
                      // by XNextEvent
            // The XNextEvent() function copies the first event from the event
            // queue into the specified XEvent structure and then removes it
            // from the queue.
            XNextEvent (d, &e); // wtf is going on -> e
#ifdef GREMLIN_DEBUG
            printf ("Event type: %d\n", e.type);
#endif

            switch (e.type)
            {
            case Expose:
#ifdef GREMLIN_DEBUG
                printf ("Expose call!\n");
#endif
                // play idle anim
                idx = 120 + (current % 60);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 60;
                usleep (config.TickDelay);

                idle += 1;
                break;
            case ButtonPress:
#ifdef GREMLIN_DEBUG
                printf ("Button click: ");
#endif
                // le button click
                // is it RMB?
                if (e.xbutton.button == Button3)
                {
#ifdef GREMLIN_DEBUG
                    printf ("RMB\n");
#endif
                    idle = 0;
                    // do the emote
                    for (char i = 0; i < 111; ++i)
                    {
                        idx = 180 + i;
                        drawf (idx);
                        XFlush (d);
                        usleep (config.TickDelay);
                        XFlush (d);
                    }
                }
                else if (e.xbutton.button == Button1)
                {
                    // LMB
                    // start drag
                    PtrState = 1;
                    idle = 0;
                }
                break;
            case ButtonRelease:
                // reset drag state machine
                PtrState = 0;
                break;

            // else
            default:
                // there is an unknown input; can occur such as ~~a hover~~ etc
                // NOTE: now, due to the removal of PointerMotionMask, that is
                // no longer of concern.

                // play idle anim
                idx = 120 + (current % 60);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 60;
                usleep (config.TickDelay);
            }
        }
        else
        {
            if (PtrState == 3 && x11)
            {
                // Hover animation
                //  - Same as the idle animation but with different offset
                //  - PtrState for this is controlled by if (XPending(d) > 0)
                //    so this should not interrupt the main input loop.

                idx = 341 + (current % 89);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 50;

                if (XQueryPointer (d, w, &ret_root, &ret_child, &root_x,
                                   &root_y, &win_x, &win_y, &mask))
                {
                    XGetWindowAttributes (d, w, &wa);
                    if (!(root_x >= wa.x + 80 && root_x <= wa.x + 245 && root_y >= wa.y + 0 && root_y <= wa.y + 325))
                    {
                        PtrState = 0;
                    }
                }
                usleep (config.TickDelay);
            }
            else if (PtrState == 1)
            {
                // DRAG
                idx = 291 + (current % 50);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 50;

                if (XQueryPointer (d, root, &ret_root, &ret_child, &root_x,
                                   &root_y, &win_x, &win_y, &mask))
                {
#ifdef GREMLIN_DEBUG
                    printf ("Mouse at: %d,%d, dx %d, dy %d, current %d\n", root_x,
                            root_y, dx, dy, current);
#endif
                }
                XMoveWindow (d, w, root_x - 162, root_y - 162);
                usleep (config.TickDelay);
            }
            if (PtrState == 2)
            {
                if (XQueryPointer (d, root, &ret_root, &ret_child, &root_x,
                                   &root_y, &win_x, &win_y, &mask))
                {
#ifdef GREMLIN_DEBUG
                    printf ("Mouse at: %d,%d, dx %d, dy %d\n", cachePX,
                            cachePY, dx, dy);
#endif
                }
                // dir & distance
                XGetWindowAttributes (d, w, &wa);
                // delta from window center to mouse
                dx = cachePX - wa.x;
                dy = cachePY - wa.y;
                // angle in radians double
                tmp_dir = atan2 (-dy, dx);
                short tmp_dir2 = (int)(tmp_dir * 180.0 / M_PI);
                double dist = sqrt (dx * dx + dy * dy);
                if (tmp_dir2 < 0)
                    tmp_dir2 += 360;
                final_dir = ((tmp_dir2 + 45) / 90) * 90 % 360;
                if (dist > 40)
                {
                    new_x = wa.x + dx / 12;
                    new_y = wa.y + dy / 12;
                }
                else
                {
                    new_x = wa.x + dx;
                    new_y = wa.y + dy;
                    idle = 0;
                    PtrState = 0;
                }
                short base = (final_dir == 270)   ? 0
                             : (final_dir == 0)   ? 30
                             : (final_dir == 180) ? 60
                             : (final_dir == 90)  ? 90
                                                  : 0;
                idx = base + (current % 30);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 30;
                XMoveWindow (d, w, new_x, new_y);
                usleep (config.TickDelay);
            }
            else if (PtrState == 0)
            {
#ifdef GREMLIN_DEBUG
                printf ("Main loop!\n");
#endif
                // continue like nothing happened because nothing happened :p

                idx = 120 + (current % 60);
                drawf (idx);
                XFlush (d);
                current = (current + 1) % 60;

                idle += 1;

                usleep (config.TickDelay);

                // have we been idle too long?
                //  - creates the illusion of natural movement and reaction. After
                //    all, if you were bored doing nothing, wouldn't you want to
                //    pester someone?
                if (idle >= 600)
                {
                    // take cursor location
                    if (XQueryPointer (d, root, &ret_root, &ret_child, &root_x,
                                       &root_y, &win_x, &win_y, &mask))
                    {
#ifdef GREMLIN_DEBUG
                        printf ("Mouse at: %d,%d, dx %d, dy %d\n", root_x, root_y,
                                dx, dy);
#endif
                    }
                    cachePX = root_x;
                    cachePY = root_y;
                    PtrState = 2;
                }
                if (XQueryPointer (d, w, &ret_root, &ret_child, &root_x,
                                   &root_y, &win_x, &win_y, &mask))
                {
                    if (win_x >= 80 && win_x <= 245 && win_y >= 0 && win_y <= 325 && x11)
                    {
                        PtrState = 3;
                    }
                }
            }
        }
        XFlush (d);
    }

    // Not reached, but in case:
    cleanup (0);
    return 0;
}
//
