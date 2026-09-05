// main file for the thing

#include <bits/types/locale_t.h>
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
#include "animation.h"
#include "asset.h"
#include "config.h"
#include "defines.h"
#include "sounds.h"
// External deps
#include "trace.h"
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

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))
// These are global for a pretty good reason
int32_t sprite_x = 400;
int32_t sprite_y = 400;
int32_t food_x = 0;
int32_t food_y = 0;
bool food_enabled = false;
bool is_hover = false;
bool is_gravity = true;
GtkWindow *w;
GtkWidget *sprite;
GtkWidget *foodsprite;
GtkWidget *fcontainer;
degrli_conf_t *local_config_main;
asset_conf_t *asset_config_main;

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
  trace_log(INFO, "[ main ] Init input regions.\n");
  input_region = cairo_region_create(); // empty region

  free(tracked_rects);
  tracked_rects = NULL;
  tc = 0;

  surface_cache = gtk_native_get_surface(GTK_NATIVE(w));
}
// actual function to move input region. Params are self explanatory.
// Sets the input region to a rectangle of size wx by wy, positioned with the
// top left corner at x, y. w stands for width.
static void degrli_move_input_region(int16_t num_rec, ...) {
  va_list args;
  va_start(args, num_rec);
  cairo_rectangle_int_t *new_rects =
      malloc(sizeof(cairo_rectangle_int_t) * num_rec);
  for (int16_t i = 0; i < num_rec; ++i) {
    int32_t x = va_arg(args, int32_t);
    int32_t y = va_arg(args, int32_t);
    int32_t wx = va_arg(args, int32_t);
    int32_t wy = va_arg(args, int32_t);
    new_rects[i] =
        (cairo_rectangle_int_t){.x = x, .y = y, .width = wx, .height = wy};
  }
  va_end(args);

  // check if repeat call
  if (tc == num_rec && tracked_rects != NULL) {
    bool s = true;
    for (int16_t i = 0; i < num_rec; ++i) {
      if (tracked_rects[i].x != new_rects[i].x ||
          tracked_rects[i].y != new_rects[i].y ||
          tracked_rects[i].width != new_rects[i].width ||
          tracked_rects[i].height != new_rects[i].height) {
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
      trace_log(ERROR, " [  main  ] Surface cache is not realised.\n");
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
  GtkWidget *picked =
      gtk_widget_pick(GTK_WIDGET(fcontainer), sxp, syp, GTK_PICK_DEFAULT);
  if ((picked == foodsprite || gtk_widget_is_ancestor(picked, foodsprite)) &&
      food_enabled) {
    target_w = foodsprite;
    ddsx = food_x;
    ddsy = food_y;
    trace_log(INFO, " [  main  ] Drag started on FOOD\n");
  } else if (picked == sprite || gtk_widget_is_ancestor(picked, sprite)) {
    target_w = sprite;
    ddsx = sprite_x;
    ddsy = sprite_y;
    trace_log(INFO, " [  main  ] Drag started on SPRITE\n");
  } else {
    target_w = NULL;
    trace_log(INFO, " [  main  ] Drag started on NULL/EMPTY\n");
  }
  anim_trigger_drag_start();
}

// function to move gremlin
static void degrli_mov(int32_t offset_x, int32_t offset_y) {
  // should make sure we don't move it out of bounds! GtkFixed will not stop us,
  // but it will be out of view and hard to get back.
  int target_x = sprite_x + offset_x;
  int target_y = sprite_y + offset_y;

  if (target_x >= 0 &&
      target_x + asset_config_main->width <=
          gtk_widget_get_width(GTK_WIDGET(w)) &&
      target_y >= 0 &&
      target_y + asset_config_main->height <=
          gtk_widget_get_height(GTK_WIDGET(w))) {
    trace_log(TRACE, " [  main  ] Move: %d %d\n", sprite_x, sprite_y);
    gtk_fixed_move(GTK_FIXED(fcontainer), sprite, sprite_x + offset_x,
                   sprite_y + offset_y);
    sprite_x += offset_x;
    sprite_y += offset_y;
    if (food_enabled) {
      degrli_move_input_region(
          2, food_x, food_y, gtk_widget_get_width(foodsprite),
          gtk_widget_get_height(foodsprite), sprite_x, sprite_y,
          gtk_widget_get_width(sprite), gtk_widget_get_height(sprite));
    } else {
      degrli_move_input_region(1, sprite_x, sprite_y,
                               gtk_widget_get_width(sprite),
                               gtk_widget_get_height(sprite));
    }
  }
}

static void on_drag_update(GtkGestureDrag *gesture, double offset_x,
                           double offset_y, gpointer user_data) {
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
    trace_log(WARN,
              " [  main  ] WHY ARE YOU CALLING DRAG UPDATE WITH NULL DRAG?!\n");
  }

  if (food_enabled)
    degrli_move_input_region(
        2, sprite_x, sprite_y, gtk_widget_get_width(sprite),
        gtk_widget_get_height(sprite), food_x, food_y,
        gtk_widget_get_width(foodsprite), gtk_widget_get_height(foodsprite));
  else
    degrli_move_input_region(1, sprite_x, sprite_y,
                             gtk_widget_get_width(sprite),
                             gtk_widget_get_height(sprite));
}

// for animation trigger
static void on_drag_end(GtkGestureDrag *gesture, double offset_x,
                        double offset_y, gpointer user_data) {
  anim_trigger_drag_end();
}

// This function is called before the Application is closed.
// All cleanup should go here.
static void cleanup() {
  trace_log(INFO, " [  main  ] Exiting...\n");
  degrli_input_region_cleanup();
  asset_cleanup();
  animation_cleanup();
  degrli_destroy_audio();
}

static void cleanup_anim_wrapper(void) {
  GtkRoot *root = gtk_widget_get_root(sprite);
  GtkApplication *app =
      root ? gtk_window_get_application(GTK_WINDOW(root)) : NULL;

  if (app != NULL) {
    g_application_quit(G_APPLICATION(app));
  }
}

struct {
  bool w;
  bool a;
  bool s;
  bool d;
} keydata;

typedef struct {
  double dx;
  double dy;
  int step;
  double subpix_mov_x;
  double subpix_mov_y;
} random_move_t;

static gboolean random_move_event(gpointer user_data);

static gboolean on_key_pressed(GtkEventControllerKey *controller, guint keyval,
                               guint keycode, GdkModifierType state,
                               gpointer user_data) {
  trace_log(INFO, " [  main  ] Key pressed!\n");
  // neutralise key
  keyval = gdk_keyval_to_lower(keyval);
  if (keyval == GDK_KEY_x) {
    trace_log(INFO, " [  main  ] X key pressed! Exiting...\n");
    anim_trigger_quit(cleanup_anim_wrapper);
    return TRUE;
  } else if (keyval == GDK_KEY_1) {
    anim_trigger_emote_1();
    return TRUE;
  } else if (keyval == GDK_KEY_2) {
    anim_trigger_emote_2();
    return TRUE;
  } else if (keyval == GDK_KEY_3) {
    anim_trigger_emote_3();
    return TRUE;
  } else if (keyval == GDK_KEY_4) {
    anim_trigger_emote_4();
    return TRUE;
  }
  // walking
  else if (keyval == GDK_KEY_w) {
    keydata.w = true;
    is_gravity = false;
    return TRUE;
  } else if (keyval == GDK_KEY_a) {
    keydata.a = true;
    return TRUE;
  } else if (keyval == GDK_KEY_s) {
    keydata.s = true;
    return TRUE;
  } else if (keyval == GDK_KEY_d) {
    keydata.d = true;
    return TRUE;
  }
  // other stuff
  else if (keyval == GDK_KEY_c) {
    local_config_main->enable_gravity = !local_config_main->enable_gravity;
  } else if (keyval == GDK_KEY_t) {
    anim_trigger_sleep();
  } else if (keyval == GDK_KEY_r) {
    int move_x = rand() % (local_config_main->random_move_distance * 2) -
                 local_config_main->random_move_distance;
    int move_y = rand() % (local_config_main->random_move_distance * 2) -
                 local_config_main->random_move_distance;

    int target_x = max(0, min(gtk_widget_get_width(GTK_WIDGET(w)) -
                                  gtk_widget_get_width(sprite),
                              sprite_x + move_x));
    int target_y = min(gtk_widget_get_height(GTK_WIDGET(w)),
                       max(gtk_widget_get_height(sprite), sprite_y + move_y));

    // Dynamically allocate on heap
    random_move_t *r = g_new0(random_move_t, 1);
    r->dx =
        (double)(target_x - sprite_x) / local_config_main->random_move_distance;
    r->dy =
        (double)(target_y - sprite_y) / local_config_main->random_move_distance;
    r->step = 0;
    r->subpix_mov_x = 0;
    r->subpix_mov_y = 0;

    g_timeout_add(1000 / local_config_main->sprite_framerate, random_move_event,
                  r);
  }
  return false;
}

// remove the states applied by key-pressed.
// This allows us to detect when multiple keys are pressed, and also
// detec keypresses more smoothly.
// For example, if "a" and "w" are true, we move up-left.
static void on_key_release(GtkEventControllerKey *controller, guint keyval,
                           guint keycode, GdkModifierType state,
                           gpointer user_data) {
  keyval = gdk_keyval_to_lower(keyval);
  if (keyval == GDK_KEY_w) {
    keydata.w = false;
    is_gravity = local_config_main->enable_gravity ? true : false;
  }
  if (keyval == GDK_KEY_a)
    keydata.a = false;
  if (keyval == GDK_KEY_s)
    keydata.s = false;
  if (keyval == GDK_KEY_d)
    keydata.d = false;
}

static gboolean key_update_timer(gpointer user_data) {
  if (keydata.w || keydata.a || keydata.s || keydata.d) {
    int32_t offset_x = keydata.a * -local_config_main->sprite_speed +
                       keydata.d * local_config_main->sprite_speed;
    int32_t offset_y = keydata.w * -local_config_main->sprite_speed +
                       keydata.s * local_config_main->sprite_speed;
    degrli_mov(offset_x, offset_y);
    // Play the animation
    if (offset_y < 0 && offset_x == 0) {
      anim_trigger_run_up();
    } else if (offset_y > 0 && offset_x == 0) {
      anim_trigger_run_down();
    } else if (offset_y == 0 && offset_x < 0) {
      anim_trigger_run_left();
    } else if (offset_y == 0 && offset_x > 0) {
      anim_trigger_run_right();
    } else if (offset_y < 0 && offset_x < 0) {
      anim_trigger_up_left();
    } else if (offset_y < 0 && offset_x > 0) {
      anim_trigger_up_right();
    } else if (offset_y > 0 && offset_x < 0) {
      anim_trigger_down_left();
    } else if (offset_y > 0 && offset_x > 0) {
      anim_trigger_down_right();
    }
  }
  return G_SOURCE_CONTINUE;
}

static void on_r_click() { anim_trigger_rclick(); }

static gboolean gravity_callback(gpointer user_data) {
  if (is_gravity && local_config_main->enable_gravity) {
    degrli_mov(0, local_config_main->gravity_strength);
  }
  return G_SOURCE_CONTINUE;
}

static void on_enter(GtkEventControllerMotion *controller, double x, double y,
                     gpointer user_data) {
  if ((int32_t)x > sprite_x &&
      (int32_t)x < sprite_x + gtk_widget_get_width(sprite) &&
      (int32_t)y > sprite_y &&
      (int32_t)y < sprite_y + gtk_widget_get_height(sprite)) {
    is_hover = true;
    trace_log(INFO, " [  main  ] Hover enter\n");
    anim_trigger_hover_start();
  }
}

static void on_leave(GtkEventController *controller, gpointer user_data) {
  if (is_hover) {
    trace_log(INFO, " [  main  ] Hover leave\n");
    anim_trigger_hover_end();
  }
}

static gboolean on_close_request(GtkWindow *w, gpointer user_data) {
  return TRUE;
}

static gboolean random_move_event(gpointer user_data) {
  trace_log(TRACE, " [  main  ] Random move tick\n");
  if (anim_request_state() == ANIM_STATE_SLEEP) {
    trace_log(INFO, " [  main  ] Move event interrupted by sleep!\n");
    free(user_data);
    return G_SOURCE_REMOVE;
  }
  random_move_t *r = (random_move_t *)user_data;

  degrli_mov(r->dx, r->dy);
  r->step++;

  if (local_config_main->enable_gravity) {
    r->dy = 0;
  }

  r->subpix_mov_x += r->dx;
  r->subpix_mov_y += r->dy;
  int offset_x = 0;
  int offset_y = 0;
  if (r->subpix_mov_x >= 1) {
    offset_x = round(r->subpix_mov_x);
    r->subpix_mov_x -= (double)round(r->subpix_mov_x);
  }
  if (r->subpix_mov_y >= 1) {
    offset_y = round(r->subpix_mov_y);
    r->subpix_mov_y -= (double)round(r->subpix_mov_y);
  }
  trace_log(TRACE,
            " [  main  ] Subpix_mov: x: %f, y: %f offset_x: %d offset_y: %d\n",
            r->subpix_mov_x, r->subpix_mov_y, offset_x, offset_y);

  if (r->dy < 0 && r->dx == 0) {
    anim_trigger_run_up();
  } else if (r->dy > 0 && r->dx == 0) {
    anim_trigger_run_down();
  } else if (r->dy == 0 && r->dx < 0) {
    anim_trigger_run_left();
  } else if (r->dy == 0 && r->dx > 0) {
    anim_trigger_run_right();
  } else if (r->dy < 0 && r->dx < 0) {
    anim_trigger_up_left();
  } else if (r->dy < 0 && r->dx > 0) {
    anim_trigger_up_right();
  } else if (r->dy > 0 && r->dx < 0) {
    anim_trigger_down_left();
  } else if (r->dy > 0 && r->dx > 0) {
    anim_trigger_down_right();
  }
  degrli_mov(offset_x, offset_y);

  if (r->step >= local_config_main->random_move_distance || anim_request_state() == ANIM_STATE_SLEEP) {
    g_free(r); // Free heap memory when steps complete
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static gboolean schedule_random_event(gpointer user_data) {
  trace_log(INFO, " [  main  ] RANDOM ACTION\n");

#ifndef DEGRLI_RANDOM_OVERRIDE
  int state = rand() % 4;
#else
  int state = DEGRLI_RANDOM_OVERRIDE;
#endif

  trace_log(TRACE, " [  main  ] Random Action state: %d\n", state);
  if (anim_request_state() != ANIM_STATE_SLEEP) {
    switch (state) {
    case 0:
      break;
    case 1:
      anim_trigger_rclick();
      break;
    case 2:
      break;
    case 3: {
      int move_x = rand() % (local_config_main->random_move_distance * 2) -
                   local_config_main->random_move_distance;
      int move_y = rand() % (local_config_main->random_move_distance * 2) -
                   local_config_main->random_move_distance;

      int target_x = max(0, min(gtk_widget_get_width(GTK_WIDGET(w)) -
                                    gtk_widget_get_width(sprite),
                                sprite_x + move_x));
      int target_y = min(gtk_widget_get_height(GTK_WIDGET(w)),
                         max(gtk_widget_get_height(sprite), sprite_y + move_y));

      // Dynamically allocate on heap
      random_move_t *r = g_new0(random_move_t, 1);
      r->dx = (double)(target_x - sprite_x) /
              local_config_main->random_move_distance;
      r->dy = (double)(target_y - sprite_y) /
              local_config_main->random_move_distance;
      r->step = 0;
      r->subpix_mov_x = 0;
      r->subpix_mov_y = 0;

      g_timeout_add(1000 / local_config_main->sprite_framerate,
                    random_move_event, r);
      break;
    }
    }
  } else {
    trace_log(INFO, " [  main  ] Random Action suppressed by sleep.\n");
  }

  guint next_interval = (rand() % (local_config_main->max_interval -
                                   local_config_main->min_interval)) +
                        local_config_main->min_interval;
  g_timeout_add(next_interval, schedule_random_event, NULL);

  return G_SOURCE_REMOVE;
}

// This function runs when program is started.
static void activate(GtkApplication *app, gpointer user_data) {
  // Basic settings for the window
  w = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(w, "desktop-gremlin-linux");
  gtk_window_set_decorated(w, FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(w), true);
  g_signal_connect(w, "close-request", G_CALLBACK(on_close_request), NULL);

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
    trace_log(WARN, "WARN: Monitor was NULL! Check code if developer.\n");
  }
  gtk_window_set_resizable(w, FALSE);
  GtkCssProvider *css = gtk_css_provider_new();
  // Compositor security should not let us make a transparent window.
  gtk_css_provider_load_from_string(
      css,
      ".window { background-color: rgba(0, 0, 0, 0); border: none; }");
  gtk_style_context_add_provider_for_display(gdk_display_get_default(),
                                             GTK_STYLE_PROVIDER(css),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  gtk_widget_add_css_class(GTK_WIDGET(w), "window");
  g_object_unref(css);

  gtk_window_present(GTK_WINDOW(w));

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
      trace_log(
          WARN,
          "Warning: Could not resolve native for Window. Skipping staying "
          "on top.\n");
      goto skiptop;
    }

    GdkSurface *surface = gtk_native_get_surface(native);
    if (!surface) {
      trace_log(
          WARN,
          "Warning: Could not resolve surface for native. Skipping staying "
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
  sprite = gtk_image_new();
  char foodpath[256];
  snprintf(foodpath, sizeof(foodpath), "%sSpriteSheet/Misc/%s",
           DEGRLI_ASSET_DIR, local_config_main->food_spawn);
  // Integer copy: parses string to int using atoi
  trace_log(INFO, " [  main  ] Food spawn path: %s\n", foodpath);
  foodsprite = gtk_image_new_from_file(foodpath);
  // FIXME: lowk set this to do something properly
  gtk_image_set_pixel_size(GTK_IMAGE(foodsprite), 184);
  gtk_fixed_put(GTK_FIXED(fcontainer), foodsprite, 0, 0);
  // Set relative scale
  int32_t scaled_h =
      (int32_t)(asset_config_main->height * asset_config_main->scale);
  int32_t scaled_w =
      (int32_t)(asset_config_main->width * asset_config_main->scale);
  gtk_image_set_pixel_size(GTK_IMAGE(sprite), scaled_h);
  if (local_config_main->randomize_spawn == false) {
    gtk_fixed_put(GTK_FIXED(fcontainer), sprite, mon_w / 2, mon_h / 2);
    degrli_move_input_region(1, mon_w / 2, mon_h / 2, scaled_w, scaled_h);
    sprite_x = mon_w / 2;
    sprite_y = mon_h / 2;
  } else {
    trace_log(INFO, " [  main  ] Randomising position...\n");
    srand((unsigned)time(0)); // reset seed
    trace_log(TRACE, " [  main  ] spawn distance: %d\n",
              local_config_main->spawn_distance);
    sprite_x =
        mon_w / 2 + (0 - local_config_main->spawn_distance) +
        (2 * local_config_main->spawn_distance) * (double)rand() / RAND_MAX;
    sprite_y =
        mon_h / 2 + (0 - local_config_main->spawn_distance) +
        (2 * local_config_main->spawn_distance) * (double)rand() / RAND_MAX;
    gtk_fixed_put(GTK_FIXED(fcontainer), sprite, sprite_x, sprite_y);
    degrli_move_input_region(1, sprite_x, sprite_y, scaled_w, scaled_h);
  }
  degrli_move_input_region(2, sprite_x, sprite_y, scaled_w, scaled_h, food_x,
                           food_y, gtk_widget_get_width(foodsprite),
                           gtk_widget_get_height(foodsprite));

  // Audio
  degrli_init_audio();
  // set up dragging
  GtkGesture *drag = gtk_gesture_drag_new();
  g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), NULL);
  g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), NULL);
  g_signal_connect(drag, "drag-end", G_CALLBACK(on_drag_end), NULL);

  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);

  gtk_widget_add_controller(fcontainer, GTK_EVENT_CONTROLLER(drag));

  if (local_config_main->enable_keyboard == true) {
    trace_log(INFO, " [  main  ] NOTE: Keyboard has been enabled.\n");
    GtkEventController *kbp = gtk_event_controller_key_new();
    g_signal_connect(kbp, "key-pressed", G_CALLBACK(on_key_pressed), app);
    g_signal_connect(kbp, "key-released", G_CALLBACK(on_key_release), app);
    gtk_widget_add_controller(GTK_WIDGET(w), kbp);
  }
  trace_log(TRACE, " [  main  ] Window size: %d by %d\n",
            gtk_widget_get_width(GTK_WIDGET(w)),
            gtk_widget_get_height(GTK_WIDGET(w)));
  // Disable food
  if (!food_enabled)
    gtk_widget_set_visible(foodsprite, false);
  // set up right click
  GtkGesture *click = gtk_gesture_click_new();
  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 3);
  g_signal_connect(click, "pressed", G_CALLBACK(on_r_click), NULL);
  gtk_widget_add_controller(fcontainer, GTK_EVENT_CONTROLLER(click));
  // Set up hover
  // Hacky anti-oguri technique (she ate her hover spritesheet)
  if (asset_config_main->hover != 0) {
    GtkEventController *hover = gtk_event_controller_motion_new();
    g_signal_connect(hover, "enter", G_CALLBACK(on_enter), NULL);
    g_signal_connect(hover, "leave", G_CALLBACK(on_leave), NULL);
    gtk_widget_add_controller(GTK_WIDGET(w), hover);
  }
  // This is a test, initially used to test our player.
  // This identified some non-null issues. Thanks to @potato-master369
  // (me) for finding that out.
#ifndef DEGRLI_NO_ANIM_DEMO
  play_emote1(sprite);
#endif
  // Random Actions - does stuff from interval min_interval to max_interval
  if (local_config_main->allow_random_actions) {
    trace_log(INFO, " [  main  ] NOTE: Random Actions have been enabled.\n");
    g_timeout_add(local_config_main->min_interval, schedule_random_event, NULL);
  }
  // gravity
  if (local_config_main->enable_gravity) {
    trace_log(INFO, " [  main  ] NOTE: Gravity has been enabled.\n");
    is_gravity = true;
  }
  g_timeout_add(1000 / local_config_main->sprite_framerate, gravity_callback,
                NULL);
  // Loop for controlling the more complicated keyboard inputs
  g_timeout_add(1000 / local_config_main->sprite_framerate, key_update_timer,
                NULL);
  anim_start_loop(sprite);
}

static void cleanup_sig(int sig) {
  cleanup();
  exit(0);
}
int main(int argc, char **argv) {
  signal(SIGINT, cleanup_sig);
  char startchar_override[64];
  bool override_st = false;
  // parse options
  trace_set_loglevel(WARN);
  for (int i = 0; i < argc; ++i) {
    if (g_strcmp0(argv[i], "--monitor") == 0 && (i + 1) < argc) {
      requested_monitor = atoi(argv[i + 1]);
    } else if (g_strcmp0(argv[i], "--char") == 0 && (i + 1) < argc) {
      strcpy(startchar_override, argv[i + 1]);
      override_st = true;
    } else if (g_strcmp0(argv[i], "--loglevel") == 0 && (i + 1) < argc) {
      int level = atoi(argv[i + 1]);
      if (level < -1 || level > 4) {
        trace_log(ERROR, "Invalid log level. Must be between -1 and 4.\n");
        return 1;
      }
      trace_set_loglevel(level);
    } else if (g_strcmp0(argv[i], "--help") == 0) {
      g_print(
          "desktop-gremlin-linux v4.x\nCopyright (C) 2026 - potato-master369\n"
          "OPTIONS:\n"
          "  --monitor n         Sets the monitor to use, to n, using GDK. By "
          "defualt uses whatever is your primary monitor. Trial and error I "
          "guess\n"
          "  --char char         Overrides config.txt to use character char. \n"
          "  --loglevel n        Sets the log level to show all ABOVE n.\n"
          "                      TRACE = 0, DEBUG = 1, INFO = 2, WARN = 3, "
          "ERROR = 4,\n"
          "                      FATAL = 5.\n"
          "E.g., --char Agnes will use Tachyon instead of whatever is in your "
          "config.\n");
      return 0;
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
  local_config_main = degrli_request_localconf();
  if (override_st) {
    g_strlcpy(local_config_main->start_char, startchar_override,
              sizeof(startchar_override));
  }
  if (local_config_main->min_interval > local_config_main->max_interval &&
      local_config_main->allow_random_actions) {
    g_print(" [  main  ] CRASH: min_interval > max_interval. Fix this!\n");
    cleanup();
    exit(1);
  }
  // convert intervals from s to ms
  local_config_main->min_interval *= 1000;
  local_config_main->max_interval *= 1000;

  asset_init();
  srand(time(NULL));
  asset_config_main = asset_request_conf();
  animation_init();
  GtkApplication *app = gtk_application_new(
      "io.github.potato-master369.desktop-gremlin-linux",
      G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), 0, NULL);
  cleanup();
  g_object_unref(app);
  return status;
}
