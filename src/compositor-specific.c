// compositor-specific.c
// source file for degrli compositor specific
// functions.

#include <gtk-4.0/gtk/gtk.h>
#ifdef X11_SUPPORT
#include <gdk/x11/gdkx.h>
#endif

// our header
#include "compositor-specific.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
// for KWin
#ifndef DGL_NO_DBUS
#include <dbus/dbus.h>
#endif
// remove the following line to ignore X11
#define X11_SUPPORT
#ifdef X11_SUPPORT
#include <X11/X.h>
#include <unistd.h>
#endif
#define DGL_CS_X11 0x01
#define DGL_CS_MUTTER 0x02
#define DGL_CS_KWIN 0x03
#define DGL_CS_SWAY 0x04 // what i personally use, so most updated.
			 // I may or may not even test the rest.

int dgl_is_running(const char *process) {
	char cmd[256];
	snprintf(cmd, sizeof(cmd), "pgrep -x %s > /dev/null", process);
	return system(cmd) == 0;
}
unsigned char dgl_detect_session(void) {
	char *XDG_SESSION_TYPE = getenv("XDG_SESSION_TYPE");
	if (strcmp(XDG_SESSION_TYPE, "x11") == 0) {
		return DGL_CS_X11;
	} else if (strcmp(XDG_SESSION_TYPE, "wayland") == 0) {
		// wayland
		if (dgl_is_running("sway"))
			return DGL_CS_SWAY;
		if (dgl_is_running("gnome-shell"))
			return DGL_CS_MUTTER;
		if (dgl_is_running("kwin_wayland"))
			return DGL_CS_KWIN;

		// other thing
		return 255;
	} else {
		return 255;
	}
}

#ifdef X11_SUPPORT
Window dgl_get_x11_window(GtkWindow *window) {
	GtkNative *native = gtk_widget_get_native(GTK_WIDGET (window));
	GdkSurface *surface = gtk_native_get_surface(native);
	if (GDK_IS_X11_SURFACE (surface)) {
		// note: we will have to use a deprecated function
		// cos screw GTK4.
		Window xid = gdk_x11_surface_get_xid(surface);
		return xid;
	} else {
		perror("not on X11\n");
		exit(1);
	}
}
#endif

void send_sway_command(const char *command) {
    const char *socket_path = getenv("SWAYSOCK");
    if (!socket_path) {
        fprintf(stderr, "SWAYSOCK not set\n");
        return;
    }

    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return;
    }

    // IPC Header: Magic string + Payload Length + Payload Type
    char magic[] = "i3-ipc";
    uint32_t payload_len = strlen(command);
    uint32_t payload_type = 0; // RUN_COMMAND

    write(socket_fd, magic, 6);
    write(socket_fd, &payload_len, 4);
    write(socket_fd, &payload_type, 4);
    write(socket_fd, command, payload_len);

    // Read reply (optional but recommended)
    char reply[1024];
    read(socket_fd, reply, sizeof(reply));

    close(socket_fd);
}

void kwin_ipc_init() {

}

void kwin_move_window(int pid, int offset_x, int offset_y) {

}

void dgl_move_window(GtkWindow *window, unsigned char wmtype, int x_offset, int y_offset) {
	char command[256];
	Window w;
	Display *d;
	XWindowAttributes a;
	switch (wmtype) {
		case DGL_CS_SWAY:
			
			if (x_offset == 0) {
				if (y_offset > 0) {
					snprintf(command, sizeof(command), "[pid=%d] move up", getpid());
					send_sway_command(command);
				} else {
					snprintf(command, sizeof(command), "[pid=%d] move down", getpid());
					send_sway_command(command);
				}
			} else {
				if (x_offset > 0) {
					snprintf(command, sizeof(command), "[pid=%d] move right", getpid());
					send_sway_command(command);
				} else {
					snprintf(command, sizeof(command), "[pid=%d] move left", getpid());
					send_sway_command(command);
				}
			}
			break;
		case DGL_CS_X11:
			d = GDK_DISPLAY_XDISPLAY (gdk_display_get_default());
			w = dgl_get_x11_window(window);
			XGetWindowAttributes(d, w, &a);
			XMoveWindow(d, w, a.x + x_offset, a.y + y_offset);
			break;
		case DGL_CS_KWIN:

			break;
		default:
			printf("This compositor, code %02d, is not supported right now.", wmtype);
	}
}
