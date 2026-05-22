#include <gtk/gtk.h>
#include <signal.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

GtkStringList *list;
GPtrArray *pids;
GtkStringList *model;
uint16_t currenti;
GtkStringList *assetpacks;
int ipcid;

static void
update_reg (GtkWidget *widget, gpointer data)
{
  g_print ("Updating registry...\n");
  FILE *fptr = fopen("/tmp/degrli-ipc.txt", "r");
  char buffer[128];
  char itembuffer[128];
  gtk_string_list_splice(model, 0, g_list_model_get_n_items(G_LIST_MODEL(model)), NULL);
  g_ptr_array_set_size(pids, 0);
  //g_ptr_array_set_size(ipcidlist, 0);
  char *token;

  if (fptr == NULL)
	  return;
  while (fgets(buffer, 127, fptr)) {
	g_print("Found line!\n");
	token = strtok(buffer, ":");
	snprintf(itembuffer, sizeof(itembuffer), "IPC ID: %s", token);
 // g_ptr_array_add(ipcidlist, GINT_TO_POINTER(atoi(token)));
        gtk_string_list_append(model, itembuffer);
	token = strtok(NULL, ":");
	/* stupid assumption that there will not be excess items */
	g_ptr_array_add(pids, GINT_TO_POINTER(atoi(token)));
  }

  //for (guint j = 0; j < ipcidlist->len; j++) {
    //g_print("Stored IPC ID: %d\n", GPOINTER_TO_INT(g_ptr_array_index(ipcidlist, j)));
  //}

  fclose(fptr);
}

static void spawn_degrli(GtkButton *btn, gpointer user_data) {
	GtkDropDown *dd = GTK_DROP_DOWN (user_data);
	int selected_idx = gtk_drop_down_get_selected(dd);

	GtkStringList *model = GTK_STRING_LIST (gtk_drop_down_get_model (dd));
	const char *selected_str = gtk_string_list_get_string (model, selected_idx);

	int i = 0;
	int position;
  char buf[48];
  	ipcid++;

	/* spawn child process */
	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "Fork failed!\n");
		exit(1);
	}
	else if (pid == 0) {
		char abuf[20];
		sprintf(abuf, "%d", ipcid);	
		execlp("/usr/local/bin/degrli", "degrli", selected_str, abuf, NULL);
		perror("Exec failed");
		exit(1);
	}

}
static void get_assetpacks() {
  struct dirent *entry;
  assetpacks = gtk_string_list_new(NULL);
  DIR *dp = opendir("/usr/share/desktop-gremlin-linux/assets/");

  if (dp == NULL) {
  	fprintf(stderr, "Opendir failed!\n");
	exit(1);
  }

  while ((entry = readdir(dp))) {
  	if (entry->d_type == DT_DIR) {
		/* assume that if it is a folder, it is an assetpack. */
			gtk_string_list_append(assetpacks, entry->d_name);
    
	}
  }
}

static void on_dropdown_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data) {
    /* Retrieve the selected item */
    GObject *selected_item = gtk_drop_down_get_selected_item(dropdown);
    
    if (selected_item != NULL) {
        /* Do something with the item (e.g., cast if it's a GtkStringObject) */
        const char *text = gtk_string_object_get_string(GTK_STRING_OBJECT(selected_item));
        g_print("Selected: %s\n", text);
	currenti = gtk_string_list_find(list, text);
	g_print("Currenti: %d\n", currenti);
    }
}

static void
go_left (GtkWidget *widget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGRTMIN + 1);
}

static void
go_right (GtkWidget *widget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGRTMIN + 3);
}

static void
go_up (GtkWidget *widget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGRTMIN);
}

static void
go_down (GtkWidget *widget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGRTMIN + 2);
}

static void
interrupt_degrli(GtkWidget *widgget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGINT);
}

static void
kill_degrli(GtkWidget *widget, gpointer data)
{
  int pid = GPOINTER_TO_INT(g_ptr_array_index(pids, currenti));
  kill(pid, SIGTERM);
}

static void do_blink(GtkWidget *widget, gpointer data) {
  int pid = GPOINTER_TO_INT (g_ptr_array_index(pids, currenti));
  kill(pid, SIGRTMIN + 4);
}

static gboolean
on_key_pressed (GtkEventControllerKey *controller,
                guint                  keyval,
                guint                  keycode,
                GdkModifierType        state,
                gpointer               user_data)
{
    switch (keyval) {
    	case GDK_KEY_w:
		go_up(NULL, NULL);
		return TRUE;
		break;
	case GDK_KEY_a:
		go_left(NULL, NULL);
		return TRUE;
		break;
	case GDK_KEY_s:
		go_down(NULL, NULL);
		return TRUE;
		break;
	case GDK_KEY_d:
		go_right(NULL, NULL);
		return TRUE;
		break;
    }

    return FALSE; /* Key not handled, let it propagate */
}

static void
activate (GtkApplication *app, gpointer user_data)
{
  get_assetpacks();
  GtkWidget *window;
  GtkWidget *button;
  pids = g_ptr_array_new();
  //ipcidlist = g_ptr_array_new();
  ipcid = 0;
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "desktop-gremlin-linux manager");
  gtk_window_set_default_size (GTK_WINDOW (window), 300, 400);
  button = gtk_button_new_with_label ("Refresh Registry");
  gtk_widget_set_valign (button, GTK_ALIGN_START);
  gtk_widget_set_halign (button, GTK_ALIGN_FILL);
  g_signal_connect (button, "clicked", G_CALLBACK (update_reg), NULL);
  
  GtkEventController *controller = gtk_event_controller_key_new ();
  g_signal_connect (controller, "key-pressed", G_CALLBACK (on_key_pressed), NULL);
  gtk_widget_add_controller (window, controller);

  GtkWidget *grid = gtk_grid_new ();
  gtk_grid_set_row_spacing (GTK_GRID (grid), 10);
  list = gtk_string_list_new(NULL);
  GtkWidget *btn_up = gtk_button_new_with_label ("W");
  GtkWidget *btn_left = gtk_button_new_with_label ("A");
  GtkWidget *btn_down = gtk_button_new_with_label ("S");
  GtkWidget *btn_right = gtk_button_new_with_label ("D");
  GtkWidget *btn_kill = gtk_button_new_with_label ("Kill (end)"); 
  GtkWidget *btn_term = gtk_button_new_with_label ("Terminate (may corrupt registry)");
  GtkWidget *btn_blink = gtk_button_new_with_label("Identify");
  get_assetpacks();
  GtkWidget *menudropdown = gtk_drop_down_new(G_LIST_MODEL (assetpacks), NULL);
  GtkWidget *menubutton = gtk_button_new_with_label("Spawn new gremlin");
  GtkWidget *menubox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start(menubox, 15);
  gtk_widget_set_margin_end(menubox, 15);
  gtk_widget_set_margin_top(menubox, 15);
  gtk_widget_set_margin_bottom(menubox, 15);
  gtk_box_append(GTK_BOX(menubox), menudropdown);
  gtk_box_append(GTK_BOX(menubox), menubutton);
  
  GtkWidget *popover = gtk_popover_new();
  gtk_popover_set_child(GTK_POPOVER (popover), menubox);
  
  GtkWidget *btn_new = gtk_menu_button_new();
  gtk_menu_button_set_label(GTK_MENU_BUTTON(btn_new), "New...");
  gtk_menu_button_set_popover (GTK_MENU_BUTTON(btn_new), popover);
  GtkWidget *regchooser = gtk_drop_down_new(G_LIST_MODEL( list ), NULL);
  model = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(regchooser)));
  
  g_signal_connect(menubutton, "clicked", G_CALLBACK(spawn_degrli), menudropdown); 
  g_signal_connect(btn_up, "clicked", G_CALLBACK (go_up), NULL);
  g_signal_connect(btn_down, "clicked", G_CALLBACK (go_down), NULL);
  g_signal_connect(btn_left, "clicked", G_CALLBACK(go_left), NULL);
  g_signal_connect(btn_right, "clicked", G_CALLBACK(go_right), NULL);
  g_signal_connect(btn_kill, "clicked", G_CALLBACK(interrupt_degrli), NULL);
  g_signal_connect(btn_term, "clicked", G_CALLBACK(kill_degrli), NULL);
  g_signal_connect(btn_blink, "clicked", G_CALLBACK (do_blink), NULL);
  g_signal_connect(regchooser, "notify::selected-item", G_CALLBACK(on_dropdown_changed), NULL);
  gtk_grid_attach(GTK_GRID (grid), button, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID (grid), regchooser, 2, 0, 3, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_up, 1, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_left, 0, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_down, 1, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_right, 2, 3, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_kill, 0, 4, 2, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_term, 2, 4, 4, 1);
  gtk_grid_attach(GTK_GRID (grid), btn_blink, 3, 2, 1, 1);
  gtk_grid_attach (GTK_GRID (grid), btn_new, 0, 5, 1, 1);
  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  gtk_box_append (GTK_BOX (hbox), grid);
  gtk_widget_set_halign (grid, GTK_ALIGN_START);	/* left-align within hbox */
  gtk_widget_set_valign (grid, GTK_ALIGN_END);	/* push to bottom */

  gtk_widget_set_vexpand (vbox, TRUE);
  gtk_box_append (GTK_BOX (vbox), hbox);
  gtk_window_set_child (GTK_WINDOW (window), vbox);

  gtk_window_present (GTK_WINDOW (window));
}

int
main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app =
    gtk_application_new ("io.github.potato-master369.desktop-gremlin-linux",
			 G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
