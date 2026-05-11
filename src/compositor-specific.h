// compositor-specific.h
// header file for degrli compositor specific
// functions.

#ifndef DGL_COMPOSITOR_SPECIFIC
#define DGL_COMPOSITOR_SPECIFIC
#include <gtk-4.0/gtk/gtk.h>
#include <gdk/x11/gdkx.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#define X11_SUPPORT
#ifdef X11_SUPPORT
#include <X11/X.h>
#include <unistd.h>
#endif
unsigned char dgl_detect_session(void);
void dgl_move_window(GtkWindow *window, unsigned char wmtype, int x_offset, int y_offset);

#endif