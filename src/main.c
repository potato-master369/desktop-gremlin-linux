// main file for the thing

#include <stdio.h>
#include <sys/types.h>
static int requested_monitor = 0;
#include <cairo.h>
#include <gdk/wayland/gdkwayland.h>
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
// Degrli stuff
#include "config.h"
#include "defines.h"
#include "sounds.h"
#include "asset.h"
#include "animation.h"
// X11-specific
// If you encounter problems building because
//   - a) GTK devs fully deprecated GDK X11
//   - b) system does not have X libraries
//
// use -DDEGRLI_NO_X11.

#ifndef DEGRLI_NO_X11
#include <X11/X.h>
#include <gdk/x11/gdkx.h>
#endif
// These are global for a pretty good reason
int32_t sprite_x  =   400;
int32_t sprite_y  =   400;
int32_t food_x    =     0;
int32_t food_y    =     0;
bool food_enabled = false;
GtkWindow           *w;
GtkWidget      *sprite;
GtkWidget  *foodsprite;
GtkWidget  *fcontainer;
degrli_conf_t *local_config_main;

static GdkMonitor *pick_monitor(void) {
  GdkDisplay *display = gdk_display_get_default();
  GListModel *monitors = gdk_display_get_monitors(display);
  guint n = g_list_model_get_n_items(monitors);

  if (requested_monitor >= 0 && (guint)requested_monitor < n)
    return g_list_model_get_item(monitors, requested_monitor);

  return n > 0 ? g_list_model_get_item(monitors, 0) : NULL;
}

// the magic that makes our overlay work
//   set a "bounding box" of sorts around our sprite, such that when mouse ISNT
//   on our sprite, clicks and stuff pass through to the windows behind.
static cairo_region_t *input_region = NULL;
static cairo_rectangle_int_t *tracked_rects = NULL;
static int16_t tc = 0;
static GdkSurface *surface_cache;
static void degrli_input_region_init(void) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
	g_print(" [  main  ] Init input regions.\n");
#endif
  input_region = cairo_region_create(); // empty region

  free(tracked_rects);
  tracked_rects = NULL;
  tc =0;

  surface_cache = gtk_native_get_surface(GTK_NATIVE(w));
}
// actual function to move input region. Params are self explanatory.
// Sets the input region to a rectangle of size wx by wy, positioned with the
// top left corner at x, y. w stands for width.
static void degrli_move_input_region(int16_t num_rec, ...) {
  va_list args;
  va_start(args, num_rec);
  cairo_rectangle_int_t *new_rects = malloc(sizeof(cairo_rectangle_int_t) * num_rec);
  for (int16_t i = 0; i < num_rec; ++i) {
    int32_t x  = va_arg(args, int32_t);
    int32_t y  = va_arg(args, int32_t);
    int32_t wx = va_arg(args, int32_t);
    int32_t wy = va_arg(args, int32_t);
    new_rects[i] = (cairo_rectangle_int_t){ .x = x, .y = y, .width = wx, .height = wy};
  }
  va_end(args);

  // check if repeat call
  if (tc == num_rec && tracked_rects != NULL) {
     bool s = true;
     for (int16_t i = 0; i < num_rec; ++i) {
      if (tracked_rects[i].x != new_rects[i].x || tracked_rects[i].y != new_rects[i].y || tracked_rects[i].width != new_rects[i].width || tracked_rects[i].height != new_rects[i].height) {
        s = false;
	break;
      }
     }
     // Then don't continue
     if (s) {
       free(new_rects);
       return;
     }
  }

  if (!surface_cache) {
    surface_cache = gtk_native_get_surface(GTK_NATIVE(w));
    if (!surface_cache) {
      free(new_rects);
      g_print(" [  main  ] ERR (non-fatal) Surface cache is not realised.\n");
      return;
    }
  }

  // Rebuild entire region
  if (input_region)
	  cairo_region_destroy(input_region);
  input_region = cairo_region_create();
  for (int16_t i = 0; i < num_rec; ++i) {
    cairo_region_union_rectangle(input_region, &new_rects[i]);
  }
  gdk_surface_set_input_region(surface_cache, input_region);
  free(tracked_rects);
  tracked_rects = new_rects;
  tc = num_rec;
}

// Cleanup function. CALL ME WHEN DESTROYED.
static void degrli_input_region_cleanup(void) {
  if (input_region) {
    cairo_region_destroy(input_region);
    input_region = NULL;
  }
  free(tracked_rects);
  tracked_rects = NULL;
  tc = 0;
}

static int32_t ddsx, ddsy;
static GtkWidget *target_w = NULL;
// Functions to move input region with drag
static void on_drag_begin(GtkGestureDrag *gesture, double sxp, double syp,
                          gpointer user_data) {
  GtkWidget *picked = gtk_widget_pick(GTK_WIDGET(fcontainer), sxp, syp, GTK_PICK_DEFAULT);
  if (picked == foodsprite || gtk_widget_is_ancestor(picked, foodsprite)) {
    target_w = foodsprite;
    ddsx = food_x;
    ddsy = food_y;
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
    g_print(" [  main  ] Drag started on FOOD\n");
#endif
  } 
  else if (picked == sprite || gtk_widget_is_ancestor(picked, sprite)) {
    target_w = sprite;
    ddsx = sprite_x;
    ddsy = sprite_y;
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
    g_print(" [  main  ] Drag started on SPRITE\n");
#endif
  } 
  else {
    target_w = NULL;
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
    g_print(" [  main  ] Drag started on NULL/EMPTY\n");
#endif
  }
}

static void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
  if (target_w == foodsprite) {
    int32_t x = ddsx + (int32_t)offset_x;
    int32_t y = ddsy + (int32_t)offset_y;
    gtk_fixed_move(GTK_FIXED(fcontainer), target_w, x, y);
    food_x = x;
    food_y = y;
  } else if (target_w == sprite) {
    int32_t x = ddsx + (int32_t)offset_x;
    int32_t y = ddsy + (int32_t)offset_y;
    gtk_fixed_move(GTK_FIXED(fcontainer), target_w, x, y);
    sprite_x = x;
    sprite_y = y;
  } else {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
	  g_print(" [  main  ] WHY ARE YOU CALLING DRAG UPDATE WITH NULL DRAG?!\n");
#endif
  }

  degrli_move_input_region(2, sprite_x, sprite_y, gtk_widget_get_width(sprite), gtk_widget_get_height(sprite), food_x, food_y, gtk_widget_get_width(foodsprite), gtk_widget_get_height(foodsprite));
}

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state,
                               gpointer user_data) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [  main  ] Key pressed!\n");
#endif
  if (keyval == GDK_KEY_x) {
    g_print(" [  main  ] X key pressed! Exiting...\n");
    g_application_quit(G_APPLICATION(user_data));
    return TRUE;
  }
  return false;
}
static void on_r_click() { g_print("Right mouse button clicked!\n"); }
// This function runs when program is started.
static void activate(GtkApplication *app, gpointer user_data) {
  // Basic settings for the window
  w = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(w, "desktop-gremlin-linux");
  gtk_window_set_decorated(w, FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(w), true);

  // NOTE: Set monitor
  GdkMonitor *mon = pick_monitor();
  int mon_w, mon_h;
  if (mon != NULL) {
    GdkRectangle geometry;
    gdk_monitor_get_geometry(mon, &geometry);
    gtk_window_set_default_size(w, geometry.width, geometry.height);
    mon_w = geometry.width;
    mon_h = geometry.height;
    g_object_unref(mon);
  } else {
    g_print("WARN: Monitor was NULL! Check code if developer.\n");
  }
  gtk_window_set_resizable(w, FALSE);
  GtkCssProvider *css = gtk_css_provider_new();
  // Compositor security should not let us make a transparent window.
  gtk_css_provider_load_from_string(
      css, ".window { background-color: rgba(1, 0, 0, 0.01); border: none; }");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(css),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  gtk_widget_add_css_class(GTK_WIDGET(w), "window");
  g_object_unref(css);

  gtk_window_present(GTK_WINDOW(w));
  g_print("still alive!\n");

  // Make it stay on top (Wayland)
  if (GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default())) {
    gtk_layer_init_for_window(w); // dont forget to init
    gtk_layer_set_layer(w, GTK_LAYER_SHELL_LAYER_OVERLAY);
    gtk_layer_set_keyboard_mode(w, GTK_LAYER_SHELL_KEYBOARD_MODE_ON_DEMAND);
  }
  // Make it stay on top (X11)
#ifndef DEGRLI_NO_X11
  else if (GDK_IS_X11_DISPLAY(gdk_display_get_default())) {
    GtkNative *native = gtk_widget_get_native(GTK_WIDGET(w));
    if (!native) {
      g_print("Warning: Could not resolve native for Window. Skipping staying "
              "on top.\n");
      goto skiptop;
    }

    GdkSurface *surface = gtk_native_get_surface(native);
    if (!surface) {
      g_print("Warning: Could not resolve surface for native. Skipping staying "
              "on top.\n");
      goto skiptop;
    }

    Display *xd = gdk_x11_display_get_xdisplay(gdk_display_get_default());
    Window xw = gdk_x11_surface_get_xid(surface);

    XEvent ev;
    Atom wm_state = XInternAtom(xd, "_NET_WM_STATE", false);
    Atom state_above = XInternAtom(xd, "_NET_WM_STATE_ABOVE", false);

    memset(&ev, 0, sizeof(ev));
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.serial = 0;
    ev.xclient.send_event = true;
    ev.xclient.display = xd;
    ev.xclient.window = xw;
    ev.xclient.message_type = wm_state;
    ev.xclient.format = 32;

    // Action payload data rules: 1 = Add property, 0 = Remove property
    ev.xclient.data.l[0] = 1;
    ev.xclient.data.l[1] = state_above;
    ev.xclient.data.l[2] = 0;
    ev.xclient.data.l[3] = 1; // Application source indication
    ev.xclient.data.l[4] = 0;

    XSendEvent(xd, DefaultRootWindow(xd), False,
               SubstructureNotifyMask | SubstructureRedirectMask, &ev);

    XFlush(xd);
  }
#endif
skiptop:

  // Fixed container - is ok because we arent using text
  fcontainer = gtk_fixed_new();
  gtk_window_set_child(w, fcontainer);

  degrli_input_region_init();
  sprite = gtk_image_new_from_file("blanktexture.png");
  char foodpath[256];
  snprintf(foodpath, sizeof(foodpath), "%sSpriteSheet/Misc/%s",
           DEGRLI_ASSET_DIR, local_config_main->food_spawn);
// Integer copy: parses string to int using atoi
#if (DEGRLI_RELEASE_STAGE) == (DEGRLI_DEBUG)
  g_print(" [  main  ] Food spawn path: %s\n", foodpath);
#endif
  foodsprite = gtk_image_new_from_file(foodpath);
  // FIXME: lowk set this to do something properly
  gtk_image_set_pixel_size(GTK_IMAGE(foodsprite), 184);
  gtk_fixed_put(GTK_FIXED(fcontainer), foodsprite, 0, 0);
  gtk_image_set_pixel_size(GTK_IMAGE(sprite), 325);
  if (local_config_main->randomize_spawn == false) {
    gtk_fixed_put(GTK_FIXED(fcontainer), sprite, mon_w / 2, mon_h / 2);
    degrli_move_input_region(1, mon_w / 2, mon_h / 2, 325, 325);
    sprite_x = mon_w / 2;
    sprite_y = mon_h / 2;
  } else {
    g_print(" [  main  ] Randomising position...\n");
    srand((unsigned)time(0)); // reset seed
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
    g_print(" [  main  ] spawn distance: %d\n",
            local_config_main->spawn_distance);
#endif
    sprite_x =
        mon_w / 2 + (0 - local_config_main->spawn_distance) +
        (2 * local_config_main->spawn_distance) * (double)rand() / RAND_MAX;
    sprite_y =
        mon_h / 2 + (0 - local_config_main->spawn_distance) +
        (2 * local_config_main->spawn_distance) * (double)rand() / RAND_MAX;
    gtk_fixed_put(GTK_FIXED(fcontainer), sprite, sprite_x, sprite_y);
    degrli_move_input_region(1, sprite_x, sprite_y, 325, 325);
  }
  degrli_move_input_region(2, sprite_x, sprite_y, 325, 325, food_x, food_y, gtk_widget_get_width(foodsprite), gtk_widget_get_height(foodsprite));

  // Audio
  degrli_init_audio();
  // set up dragging
  GtkGesture *drag = gtk_gesture_drag_new();
  g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), NULL);
  g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), NULL);

  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);

  gtk_widget_add_controller(fcontainer, GTK_EVENT_CONTROLLER(drag));

  if (local_config_main->enable_keyboard == true) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
	  g_print(" [  main  ] NOTE: Keyboard has been enabled.\n");
#endif
    GtkEventController *kbp = gtk_event_controller_key_new();
    g_signal_connect(kbp, "key-pressed", G_CALLBACK(on_key_pressed), app);
    gtk_widget_add_controller(GTK_WIDGET(w), kbp);
  }

  // FIXME: remove after test
  food_enabled = true;
  // set up right click
  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 3);
  g_signal_connect(click, "pressed", G_CALLBACK(on_r_click), NULL);
  gtk_widget_add_controller(fcontainer, GTK_EVENT_CONTROLLER(click));
  // This is a test, initially used to test our player.
  // This identified some non-null issues. Thanks to @potato-master369
  // (me) for finding that out.
#ifndef DEGRLI_NO_ANIM_DEMO
  play_emote1(sprite);
#endif
}

// This function is called before the Application is closed.
// All cleanup should go here.
static void cleanup() {
  g_print(" [  main  ] Exiting...\n");
  degrli_input_region_cleanup();
  asset_cleanup();
  degrli_destroy_audio();
}

static void cleanup_sig(int sig) {
  cleanup();
  exit(0);
}
int main(int argc, char **argv) {
  signal(SIGINT, cleanup_sig);
  // parse options
  for (int i = 0; i < argc; ++i) {
    if (g_strcmp0(argv[i], "--monitor") == 0 && (i + 1) < argc) {
      requested_monitor = atoi(argv[i + 1]);
    }
  }
  g_print("desktop-gremlin-linux v4.x\nCopyright (C) 2026- potato-master369 "
          "(GitHub). Source code available on request. Compile time: %s %s\n "
          "GCC version: %d\n",
          __DATE__, __TIME__, __GNUC__);
  g_print("Distributed by: %s\n", DEGRLI_DIST);
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print("WARNING: This is a debug version of Desktop-gremlin-linux. USE AT "
          "YOUR OWN RISK.\n");
#endif
  degrli_init_readconf();
  asset_init();
  animation_init();
  local_config_main = degrli_request_localconf();
  GtkApplication *app = gtk_application_new(
      "io.github.potato-master369.desktop-gremlin-linux",
      G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  cleanup();
  g_object_unref(app);
  return status;
}
