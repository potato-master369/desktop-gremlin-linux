#include <cairo.h>
#include <gtk-4.0/gtk/gtk.h>

// function to draw things onto window on tick
static gboolean tick_cb(gpointer user_data) {
  g_print("Test\n");
  return TRUE;
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkCssProvider *css_provider;

  /* Set up the fun stuff XD */
  window = gtk_application_window_new(app);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  gtk_window_set_titlebar(GTK_WINDOW(window), NULL);
  gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  gtk_window_set_deletable(GTK_WINDOW(window), FALSE);
  gtk_window_set_title(GTK_WINDOW(window), "desktop-gremlin-linux v3.x");
  gtk_window_set_default_size(GTK_WINDOW(window), 320, 320);

  // style
  // css stuff
  css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_string(
      css_provider,
      // rbga, `a` set to 0.0 makes the window background transparent
      ".window { background-color: transparent; background-image: none; border: none; }");

  gtk_style_context_add_provider_for_display(gtk_widget_get_display(window),
                                             (GtkStyleProvider *)css_provider,
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_widget_add_css_class(window, "window"); 
  // end css stuff

  GtkWidget *image = gtk_image_new_from_file(
      "/usr/share/desktop-gremlin-linux/desktop-gremlin-assets/0.png");

  // more css stuff
  gtk_widget_add_css_class (image, "window");

  gtk_window_set_child(GTK_WINDOW(window), image);

  gtk_window_present(GTK_WINDOW(window));

  g_timeout_add(1000, tick_cb, NULL);
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);

  return status;
}
