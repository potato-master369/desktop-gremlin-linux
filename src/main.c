// main file for the thing

static int requested_monitor = 0;
#include <gtk/gtk.h>

static GdkMonitor *pick_monitor(void) {
  GdkDisplay *display = gdk_display_get_default();
  GListModel *monitors = gdk_display_get_monitors(display);
  guint n = g_list_model_get_n_items(monitors);

  if (requested_monitor >= 0 && (guint)requested_monitor < n)
    return g_list_model_get_item(monitors, requested_monitor);

  return n > 0 ? g_list_model_get_item(monitors, 0) : NULL;
}

// This function runs when program is started.
static void activate(GtkApplication *app, gpointer user_data) {
  // Basic settings for the window
  GtkWindow *w = GTK_WINDOW(gtk_application_window_new(app));
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
  //gtk_widget_add_css_class(GTK_WIDGET(w), "overlay-window");
  gtk_css_provider_load_from_string(
      css, ".window { background-color: rgba(1, 0, 0, 0.01); border: none; }");
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(css),
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  gtk_widget_add_css_class(GTK_WIDGET(w), "window");
  g_print("uwu\n");
  g_object_unref(css);

  gtk_window_present(GTK_WINDOW(w));
  g_print("still alive!\n");
}

int main(int argc, char **argv) {
  // parse options
  for (int i = 0; i < argc; ++i) {
    if (g_strcmp0(argv[i], "--monitor") == 0 && (i + 1) < argc) {
      requested_monitor = atoi(argv[i + 1]);
    }
  }
  g_print("desktop-gremlin-linux v4.x\nCopyright (C) 2026- potato-master369 (GitHub). Source code available on request. Compile time: %s %s\n GCC version: %d\n", __DATE__, __TIME__, __GNUC__);

  GtkApplication *app =
      gtk_application_new("io.github.potato-master369.desktop-gremlin-linux",
                          G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
