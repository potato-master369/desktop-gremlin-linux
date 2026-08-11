// main file for the thing

static int requested_monitor = 0;
#include <gtk/gtk.h>
#include <gtk4-layer-shell.h>
#include <stdint.h>
#include <cairo.h>
// These are global for a pretty good reason
int32_t sprite_x = 400;
int32_t sprite_y = 400;
GtkWindow *w;
GtkWidget *sprite;
GtkWidget *fcontainer;

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
cairo_region_t *input_region = NULL;
cairo_rectangle_int_t crect = {0, 0, 0, 0};
GdkSurface *surface_cache;
static void degrli_input_region_init(void) {
	input_region = cairo_region_create(); // empty region

	crect.x = 0;
	crect.y = 0;
	crect.width = 0;
	crect.height = 0;

	surface_cache = gtk_native_get_surface(GTK_NATIVE(w));
}
// actual function to move input region. Params are self explanatory.
// Sets the input region to a rectangle of size wx by wy, positioned with the top left corner at
// x, y.
// w stands for width.
static void degrli_move_input_region(int32_t x, int32_t y, int32_t wx, int32_t wy) {
	if (crect.x == x && crect.y == y && crect.width == wx && crect.height ==wy) {
		//g_print("Unnecessary call of degrli_move_input_region. Please fix this shit.\n");
		return; // mis-call: Idiot called this function unnecessarily.
	}

	if (!surface_cache) {
		surface_cache = gtk_native_get_surface(GTK_NATIVE(w));
		return; // Surface not realized!
	}

	cairo_rectangle_int_t extents;
	cairo_region_get_extents(input_region, &extents);
	cairo_region_subtract_rectangle(input_region, &extents);

	crect.x = x;
	crect.y = y;
	crect.width = wx;
	crect.height = wy;

	cairo_region_union_rectangle(input_region, &crect);

	gdk_surface_set_input_region(surface_cache, input_region);
}

// Cleanup function. CALL ME WHEN DESTROYED.
static void degrli_input_region_cleanup(void) {
	if (input_region) {
		cairo_region_destroy(input_region);
		input_region = NULL;
	}
}

static int32_t ddsx, ddsy;
// Functions to move input region with drag
static void on_drag_begin(GtkGestureDrag *gesture, double sxp, double syp, gpointer user_data) {
	ddsx = sprite_x;
	ddsy = sprite_y;
}

static void on_drag_update(GtkGestureDrag *gesture, double offset_x, double offset_y, gpointer user_data) {
	int32_t x = ddsx + (int32_t)(offset_x);
	int32_t y = ddsy + (int32_t)(offset_y);

	gtk_fixed_move(GTK_FIXED(fcontainer), sprite, x, y);
	
	sprite_x = x;
	sprite_y = y;

	int width = gtk_widget_get_width(sprite);
	int height = gtk_widget_get_height(sprite);

	degrli_move_input_region(x, y, width, height);
}

static void on_r_click() {

}
// This function runs when program is started.
static void activate(GtkApplication *app, gpointer user_data) {
  // Basic settings for the window
  w = GTK_WINDOW(gtk_application_window_new(app));
  gtk_window_set_title(w, "desktop-gremlin-linux");
  gtk_window_set_decorated(w, FALSE);

  // NOTE: Set monitor
  GdkMonitor *mon = pick_monitor();
  if (mon != NULL) {
    GdkRectangle geometry;
    gdk_monitor_get_geometry(mon, &geometry);
    gtk_window_set_default_size(w, geometry.width, geometry.height);
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
  gtk_layer_init_for_window(w); // dont forget to init
  gtk_layer_set_layer(w, GTK_LAYER_SHELL_LAYER_OVERLAY);

  // Fixed container - is ok because we arent using text
  fcontainer = gtk_fixed_new();
  gtk_window_set_child(w, fcontainer);
  
  degrli_input_region_init();
  sprite = gtk_image_new_from_file("blanktexture.png");
  gtk_image_set_pixel_size(GTK_IMAGE(sprite), 325);
  gtk_fixed_put(GTK_FIXED(fcontainer), sprite, 400, 400);
  degrli_move_input_region(400, 400, 325, 325);

  // set up dragging
  GtkGesture *drag = gtk_gesture_drag_new();
  g_signal_connect(drag, "drag-begin", G_CALLBACK(on_drag_begin), NULL);
  g_signal_connect(drag, "drag-update", G_CALLBACK(on_drag_update), NULL);

  gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(drag), GDK_BUTTON_PRIMARY);

  gtk_widget_add_controller(fcontainer, GTK_EVENT_CONTROLLER(drag));
}

// This function is called before the Application is closed.
// All cleanup should go here.
static void cleanup() {
	g_print("Exiting...\n");
	degrli_input_region_cleanup();
}

int main(int argc, char **argv) {
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

  GtkApplication *app =
      gtk_application_new("io.github.potato-master369.desktop-gremlin-linux",
                          G_APPLICATION_DEFAULT_FLAGS | G_APPLICATION_NON_UNIQUE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  cleanup();
  g_object_unref(app);
  return status;
}
