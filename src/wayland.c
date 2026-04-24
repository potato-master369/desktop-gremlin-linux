#include <gtk-4.0/gtk/gtk.h>
#include <cairo.h>
#include "ini.h"

struct
{
  char *assetpack;
  int ipcid;
  bool validated;
  GPtrArray *textures;
} app_cdata;

struct
{
  int ssleep, esleep, sidle, eidle, sclick, eclick, sgrab, egrab, shover,
    ehover, sintro, eintro, soutro, eoutro, InitX, InitY, InitIdle,
    ChaseIdleReq, TickDelay, total, current;
} app_adata;

static int
tick_cb (gpointer user_data) {
	g_print("I AM ALIVE!\n");
	GtkImage *image = (GtkImage *)user_data;
	gtk_image_set_from_paintable(image, g_ptr_array_index(app_cdata.textures, app_adata.current));
	if (!(app_adata.current < app_adata.total)) {
		app_adata.current = 0;
	} else {
		app_adata.current++;
	}
	return 1;
}
// scary loading thing
static void
loadf (void)
{
  char filename[256];
  app_cdata.textures = g_ptr_array_new_with_free_func(g_object_unref);

  // load all from 0-total in an array.
  for (int i = 0; i < app_adata.total; ++i) {
	  snprintf(filename, sizeof (filename), "/usr/share/desktop-gremlin-linux/assets/%s/%d.png", app_cdata.assetpack, i);
	  GdkTexture *tex = gdk_texture_new_from_filename(filename, NULL);
	   
	  if (tex) {
		g_print("Loaded frame: %d\n", i);
		g_ptr_array_add(app_cdata.textures, g_object_ref(tex));
		g_object_unref(tex);
          }
  }
}

// free frames
static void
freef (void)
{
	g_ptr_array_unref(app_cdata.textures);
}

static int
handler (void *user, const char *section, const char *name, const char *value)
{
#define INI_MATCH(s, n) strcmp (section, s) == 0 && strcmp (name, n) == 0
  if (INI_MATCH ("Window", "InitX"))
    {
      app_adata.InitX = atoi (value);
    }
  else if (INI_MATCH ("Window", "InitY"))
    {
      app_adata.InitY = atoi (value);
    }
  else if (INI_MATCH ("Tweaks", "InitIdle"))
    {
      app_adata.InitIdle = atoi (value);
    }
  else if (INI_MATCH ("Tweaks", "ChaseIdleReq"))
    {
      app_adata.ChaseIdleReq = atoi (value);
    }
  else if (INI_MATCH ("Tweaks", "TickDelay"))
    {
      app_adata.TickDelay = atoi (value);
    }				// -- all the slow asset copy paste --
  else if (INI_MATCH ("TextureBounds", "ssleep"))
    {
      app_adata.ssleep = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "ssleep"))
    {
      app_adata.ssleep = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "esleep"))
    {
      app_adata.esleep = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "sidle"))
    {
      app_adata.sidle = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "eidle"))
    {
      app_adata.eidle = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "sclick"))
    {
      app_adata.sclick = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "eclick"))
    {
      app_adata.eclick = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "sgrab"))
    {
      app_adata.sgrab = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "egrab"))
    {
      app_adata.egrab = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "shover"))
    {
      app_adata.shover = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "ehover"))
    {
      app_adata.ehover = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "sintro"))
    {
      app_adata.sintro = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "eintro"))
    {
      app_adata.eintro = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "soutro"))
    {
      app_adata.soutro = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "eoutro"))
    {
      app_adata.eoutro = atoi (value);
    }
  else if (INI_MATCH ("TextureBounds", "total"))
    {
      app_adata.total = atoi (value);
    }

  return 1;
}

static int
command_line (GApplication *app, GApplicationCommandLine *cmdline)
{
  gchar **argv;
  gint argc;
  gint i;

  argv = g_application_command_line_get_arguments (cmdline, &argc);

  if (argc != 3)
    {
      g_print
	(" [error] Please launch the application with the following syntax: [PROGRAM] [ASSETPACK] [IPC ID]\n");
      exit (1);
    }

  app_cdata.assetpack = argv[1];
  gchar *endptr;
  app_cdata.ipcid = g_ascii_strtoll (argv[2], &endptr, 10);

  if (*endptr != '\0')
    {
      g_print (" [error] invalid IPC ID; conversion to int failed\n");
      exit (1);
    }

  app_cdata.validated = true;

  // activate or smth idk
  g_application_activate (app);
  return 0;
}

static void
activate (GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  GtkCssProvider *css__;


  /* Set up the fun stuff XD */
  window = gtk_application_window_new (app);

  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
  gtk_window_set_titlebar (GTK_WINDOW (window), NULL);
  gtk_window_set_decorated (GTK_WINDOW (window), FALSE);
  gtk_window_set_deletable (GTK_WINDOW (window), FALSE);
  gtk_window_set_title (GTK_WINDOW (window), "desktop-gremlin-linux v3.x");
  gtk_window_set_default_size (GTK_WINDOW (window), 320, 320);
  gtk_widget_set_opacity (window, 1.0);
  // style
  // css stuff

  css__ = gtk_css_provider_new ();
  gtk_css_provider_load_from_string (css__,
				     // rbga, `a` set to 0.0 makes the window background transparent
				     ".window { background-color: rgba(0, 0, 0, 0); background: none; border: none; }");

  gtk_style_context_add_provider_for_display (gtk_widget_get_display (window),
					      (GtkStyleProvider *) css__,
					      GTK_STYLE_PROVIDER_PRIORITY_USER);

  gtk_widget_add_css_class (window, "window");
  gtk_widget_set_overflow (window, GTK_OVERFLOW_HIDDEN);
  // end css stuff

  GtkWidget *image =
    gtk_image_new_from_file
    ("/usr/share/desktop-gremlin-linux/desktop-gremlin-assets/0.png");

  // more css stuff
  gtk_widget_add_css_class (image, "window");
  //gtk_widget_set_opacity(window, 0.0);
  gtk_widget_set_opacity (image, 1.0);

  // funky block
  if (app_cdata.validated == false)
    {
      g_print (" [error] command-line did not run\n");
      exit (1);
    }

  char filename[256];

  snprintf (filename, sizeof (filename),
	    "/usr/share/desktop-gremlin-linux/assets/%s/config.ini",
	    app_cdata.assetpack);
  ini_parse (filename, handler, NULL);	// really jank, but we want to reduce typedefs so it's a BIT cleaner this way.
  //printf("DEBUG: InitX: %d", app_adata.InitX); 
  loadf();
  g_print("I am alive!\n");
  app_adata.current = 0; // reset stupid overengineered state machine
			 // cos u have to

  gtk_window_set_child (GTK_WINDOW (window), image);

  gtk_window_present (GTK_WINDOW (window));

  g_print("I am alive?\n");
  g_print("TickDelay: %d\n", app_adata.TickDelay);
  g_timeout_add (app_adata.TickDelay, tick_cb, image);
}

int
main (int argc, char **argv)
{
  app_cdata.validated = false;
  GtkApplication *app;
  int status;

  app =
    gtk_application_new ("io.github.potato-master369.desktop-gremlin-linux",
			 G_APPLICATION_HANDLES_COMMAND_LINE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  g_signal_connect (app, "command-line", G_CALLBACK (command_line), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);

  return status;
}
