#include <X11/Xlib.h>
#include <X11/Xutil.h>
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

Pixmap frames[NFRAMES];
Pixmap masks[NFRAMES];



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
    printf("Cleanup time!\n");
    #endif

    // Goodbye, cruel world!
    //  - our outro animation goes here!
    //  - in future, a handler will be created using
    //    kill() to find the PID of this process and
    //    send SIGINT.
    for (int i = 530; i < 670; ++i)
    {
        if (masks[i] != None)
            XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                               masks[i], ShapeSet);
        if (frames[i] != None)
            XCopyArea (d, frames[i], w, gc, 0, 0, WIDTH, HEIGHT,
                       0, 0);
        XFlush (d);
        usleep (config.TickDelay);
    }

    // start true cleanup
    if (d)
    {
        for (int i = 0; i < NFRAMES; i++)
        {
            if (frames[i] != None)
                XFreePixmap (d, frames[i]);
            if (masks[i] != None)
                XFreePixmap (d, masks[i]);
        }
        XFreeGC (d, gc);
        XDestroyWindow (d, w);
        XCloseDisplay (d);
    }
    fflush (stdout);
    _exit (0);
}

int
main ()
{
    signal (SIGINT, cleanup);
    signal (SIGTERM, cleanup);

    // READ CONFIGURATION
    
    config.InitX = 200;
    config.InitY = 200;
    config.InitPtrState = 0;
    config.InitIdle = 600;
    config.ChaseIdleReq = 600;
    config.TickDelay = 100000;

    // Detect $HOME
    const char *home = getenv ("HOME");
    char filename[256];
    if (!home)
    {
        fprintf (stderr, "HOME not set\n");
        cleanup (0);
    }

    snprintf (filename, sizeof (filename),
              "%s/Desktop/desktop-gremlin-assets/gremlin_config.ini", home);

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
    XSetWindowAttributes swa;
    swa.override_redirect = True;

    w = XCreateWindow (d, RootWindow (d, screen), config.InitX, config.InitY, WIDTH, HEIGHT, 0,
                       DefaultDepth (d, screen), InputOutput,
                       DefaultVisual (d, screen), CWOverrideRedirect, &swa);
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

    for (int i = 0; i < NFRAMES; ++i)
    {
        frames[i] = None;
        masks[i] = None;
#ifdef GREMLIN_DEBUG
        printf ("Loading frame %d\n", i);
#endif
        snprintf (filename, sizeof (filename),
                  "%s/Desktop/desktop-gremlin-assets/%d.xpm", home, i);
#ifdef GREMLIN_DEBUG
        printf ("Loading from file: %s\n", filename);
#endif

        XpmAttributes xpm_attrs;
        xpm_attrs.valuemask = 0; // default; not requesting extra data

        int status = XpmReadFileToPixmap (d, w, filename, &frames[i],
                                          &masks[i], &xpm_attrs);

        if (status != XpmSuccess || frames[i] == None)
        {
            fprintf (stderr, "Failed to load %s (status=%d)\n", filename,
                     status);
            frames[i] = None;
            masks[i] = None;
            continue;
        }
        #ifdef GREMLIN_DEBUG
        printf ("Loaded frame %d successfully!\n", i);
        #endif
    }

    if (masks[0] != None)
        XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                           masks[0], ShapeSet);
    if (frames[0] != None)
        XCopyArea (d, frames[0], w, gc, 0, 0, WIDTH, HEIGHT,
                   0, 0);
    XFlush (d);
    
    //    Cleaned up by clanker so idk if this is wrong
    short idx;
    // State tracking
    int current = 0;
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
    #ifdef GREMLIN_DEBUG
    printf("HELLO WORLD!");
    #endif
    for (int i = 430; i < 530; ++i)
    {
        if (masks[i] != None)
            XShapeCombineMask (d, w, ShapeBounding, 0, 0, masks[i],
                               ShapeSet);
        if (frames[i] != None)
            XCopyArea (d, frames[i], w, gc, 0, 0, WIDTH, HEIGHT, 0,
                       0);
        XFlush (d);
        usleep (config.TickDelay); 
    }

    while (1)
    {
        #ifdef GREMLIN_DEBUG
        printf ("New tick: %d, XPending: %d, PtrState: %d, winx: %d, winy: %d\n", idle, XPending (d), PtrState, win_x, win_y);
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
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0, masks[idx],
                                       ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0,
                               0);
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
                        if (masks[idx] != None)
                        {
                            XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                                               masks[idx], ShapeSet);
                        }
                        if (frames[idx] != None)
                        {
                            XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH,
                                       HEIGHT, 0, 0);
                        }
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
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0, masks[idx],
                                        ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0,
                                0);
                current = (current + 1) % 60;
                usleep (config.TickDelay); 
            }
        }
        else
        {
            if (PtrState == 3)
            {
                // Hover animation
                //  - Same as the idle animation but with different offset
                //  - PtrState for this is controlled by if (XPending(d) > 0)
                //    so this should not interrupt the main input loop.

                idx = 341 + (current % 89);
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                                       masks[idx], ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT,
                               0, 0);
                current = (current + 1) % 50;

                if (XQueryPointer (d, w, &ret_root, &ret_child, &root_x,
                                   &root_y, &win_x, &win_y, &mask))
                {
                    if (win_x < 80 || win_x > 245 || win_y < 5 || win_y > 325) {
                        PtrState = 0;
                    }
                }
                usleep (config.TickDelay);
            }
            else if (PtrState == 1)
            {
                // DRAG
                idx = 291 + (current % 50);
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                                       masks[idx], ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT,
                               0, 0);
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
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0,
                                       masks[idx], ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT,
                               0, 0);
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
                if (masks[idx] != None)
                    XShapeCombineMask (d, w, ShapeBounding, 0, 0, masks[idx],
                                       ShapeSet);
                if (frames[idx] != None)
                    XCopyArea (d, frames[idx], w, gc, 0, 0, WIDTH, HEIGHT, 0, 0);
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
                    if (win_x >= 80 && win_x <= 245 && win_y >= 0 && win_y <= 325)
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
