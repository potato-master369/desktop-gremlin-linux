#include <gtk/gtk.h>

static void
print_hello (GtkWidget *widget, gpointer data)
{
  g_print ("Hello World\n");
}

static void
activate (GtkApplication *app, gpointer user_data)
{

#define WIDTH 800
#define HEIGHT 600

  GtkWidget *window;
  GtkWidget *button;
  GtkWidget *fixed_container;
  GtkCssProvider *css_provider;

  window = gtk_application_window_new (app);

  // css stuff
  css_provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (
      css_provider,
      // rbga, `a` set to 0.0 makes the window background transparent
      ".window { background-color: rgba(0, 0, 0, 0.0); }");

  gtk_style_context_add_provider_for_display (
      gtk_widget_get_display (window),
      (GtkStyleProvider *) css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
  
  gtk_widget_add_css_class (window, "window");
  // end css stuff

  gtk_window_set_title (GTK_WINDOW (window), "Hello");
  gtk_window_set_default_size (GTK_WINDOW (window), WIDTH, HEIGHT);

  button = gtk_button_new_with_label ("Hello World");
  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

  fixed_container = gtk_fixed_new ();
  gtk_window_set_child (GTK_WINDOW (window), fixed_container);

  gtk_fixed_put (GTK_FIXED (fixed_container), button, 20, 20);
  gtk_widget_set_size_request (button, 400, 100);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int argc, char *argv[])
{

  GtkApplication *app;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);

  int retval = g_application_run (G_APPLICATION (app), argc, argv);

  g_object_unref (app);
  return retval;
}
