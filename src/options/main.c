#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include "meme.h"
// Made with GTK-4.0
// Options program, designed to emulate Options.exe from Desktop_Gremlin

static GtkStringList *char_list = NULL;

static void add_setting_row(GtkGrid *grid, int row, const char *name,
                            GtkWidget *control_widget, const char *desc_text) {
  GtkWidget *lbl_name = gtk_label_new(name);
  gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
  gtk_widget_set_hexpand(lbl_name, FALSE);
  gtk_widget_set_valign(lbl_name, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(lbl_name, FALSE);

  gtk_widget_set_halign(control_widget, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(control_widget, GTK_ALIGN_CENTER);
  gtk_widget_set_valign(control_widget, GTK_ALIGN_CENTER);

  GtkWidget *lbl_desc = gtk_label_new(desc_text);
  gtk_widget_set_halign(lbl_desc, GTK_ALIGN_START);
  gtk_widget_set_hexpand(lbl_desc, TRUE); // Fills remaining horizontal space
  gtk_label_set_wrap(GTK_LABEL(lbl_desc), TRUE);
  gtk_label_set_max_width_chars(GTK_LABEL(lbl_desc), 40);
  gtk_widget_set_hexpand(lbl_desc, TRUE);
  gtk_label_set_wrap(GTK_LABEL(lbl_desc), TRUE);

  gtk_grid_attach(grid, lbl_name, 0, row, 1, 1);
  gtk_grid_attach(grid, control_widget, 1, row, 1, 1);
  gtk_grid_attach(grid, lbl_desc, 2, row, 1, 1);
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *w = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(w), "Desktop-gremlin-linux Options");
  // basic layout
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_start(box, 20);
  gtk_widget_set_margin_end(box, 20);
  gtk_widget_set_margin_top(box, 20);
  gtk_widget_set_margin_bottom(box, 20);
  GtkWidget *menubox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_append(GTK_BOX(box), menubox);
  GtkWidget *b_revert = gtk_button_new_with_label("Revert Defaults");
  gtk_widget_set_margin_end(b_revert, 15);
  GtkWidget *b_save = gtk_button_new_with_label("Save Changes");
  GtkWidget *b_spawn = gtk_button_new_with_label("Release the Gremlin");
  GtkWidget *b_horde = gtk_button_new_with_label("Unleash the Horde");
  gtk_widget_add_css_class(GTK_WIDGET(b_horde), "destructive-action");
  gtk_box_append(GTK_BOX(menubox), b_revert);
  gtk_box_append(GTK_BOX(menubox), b_save);
  gtk_box_append(GTK_BOX(menubox), b_spawn);
  gtk_box_append(GTK_BOX(menubox), b_horde);
  GtkWidget *notebook = gtk_notebook_new();
  GtkWidget *l_tab_1 = gtk_label_new("General Settings");
  GtkWidget *g_tab_1 = gtk_grid_new();
  gtk_widget_set_margin_top(g_tab_1, 20);
  gtk_widget_set_margin_start(g_tab_1, 20);
  gtk_widget_set_margin_end(g_tab_1, 20);
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_1), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_1), 20);
  char_list = gtk_string_list_new(NULL);
  GtkWidget *t_tab_1_opt_1 = gtk_drop_down_new(G_LIST_MODEL(char_list), NULL);
  add_setting_row(GTK_GRID(g_tab_1), 0, "Starting Character", t_tab_1_opt_1, "Available Characters in SpriteSheet/Gremlins");
  GtkWidget *t_tab_1_opt_2 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 1, "Language Difference", t_tab_1_opt_2, "Windows Machines with non-English locale will break the 'config.txt'. Leave this on.");
  GtkWidget *t_tab_1_opt_3 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 2, "Enable Keyboard", t_tab_1_opt_3, "Allow Keyboard control for the gremlin");
  GtkWidget *t_tab_1_opt_4 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 3, "Show Taskbar Icon", t_tab_1_opt_4, "Show the Program in the taskbar (Windows)");
  GtkWidget *t_tab_1_opt_5 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 4, "Allow Error Messages", t_tab_1_opt_5, "Display Error Messages (just check stdout i dont really do error messages)");
  GtkWidget *t_tab_1_opt_6 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 5, "Use WPF Player", t_tab_1_opt_6, "Switch between WPF and SoundPlayer. Some systems cannot use WPF Players, unless manually enabled. (Windows-only, linux use miniaudio.h");
  GtkAdjustment *a_tab_1_opt_7 = gtk_adjustment_new(0.5, 0.00, 1.00, 0.05, 1.00, 0);
  GtkWidget *t_tab_1_opt_7 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *t_tab_1_opt_7_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_1_opt_7);
  gtk_widget_set_size_request(t_tab_1_opt_7_o1, 160, -1);
  GtkWidget *t_tab_1_opt_7_o2 = gtk_spin_button_new(a_tab_1_opt_7, 0.05, 2);
  gtk_box_append(GTK_BOX(t_tab_1_opt_7), t_tab_1_opt_7_o1);
  gtk_box_append(GTK_BOX(t_tab_1_opt_7), t_tab_1_opt_7_o2);
  add_setting_row(GTK_GRID(g_tab_1), 6, "Volume Level", t_tab_1_opt_7, "Volume can only be changed if using WPF Player (on Windows), but on Linux it works fine regardless.");
  GtkWidget *t_tab_1_opt_8 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 7, "Randomise Spawn", t_tab_1_opt_8, "[Disable force centre in Configuration first] Spawns the sprites in a random location upon initialisation. The 'start at bottom' in Sprite Settings, will be affected by this.");
  GtkWidget *t_tab_1_opt_9 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_1_opt_9), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_1), 8, "Spawn Distance", t_tab_1_opt_9, "The Random Distance variance from the centre. [Higher = more spread] [Lower = Closer to the centre]");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_1, l_tab_1);
  
  GtkWidget *l_tab_2 = gtk_label_new("Sprite Settings");
  GtkWidget *g_tab_2 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_2), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_2), 20);
  GtkWidget *t_tab_2_opt_1 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_2_opt_1), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_2), 0, "FrameRate:", t_tab_2_opt_1, "Frames per second for the sprite player");
  GtkAdjustment *a_tab_2_opt_2 = gtk_adjustment_new(10.0, 0.0, 30.0, 1.0, 1.0, 0);
  GtkWidget *t_tab_2_opt_2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *t_tab_2_opt_2_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_2_opt_2);
  gtk_widget_set_size_request(t_tab_2_opt_2_o1, 160, -1);
  GtkWidget *t_tab_2_opt_2_o2 = gtk_spin_button_new(a_tab_2_opt_2, 1.0, 1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_2), t_tab_2_opt_2_o1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_2), t_tab_2_opt_2_o2);
  add_setting_row(GTK_GRID(g_tab_2), 1, "Movement Speed", t_tab_2_opt_2, "The speed at which the Sprite follows your mouse");

  GtkAdjustment *a_tab_2_opt_3 = gtk_adjustment_new(150.0, 0.0, 300.0, 10.0, 50.0, 0); 
  GtkWidget *t_tab_2_opt_3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *t_tab_2_opt_3_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_2_opt_3);
  gtk_widget_set_size_request(t_tab_2_opt_3_o1, 160, -1);
  GtkWidget *t_tab_2_opt_3_o2 = gtk_spin_button_new(a_tab_2_opt_3, 10.0, 1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_3), t_tab_2_opt_3_o1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_3), t_tab_2_opt_3_o2);
  add_setting_row(GTK_GRID(g_tab_2), 2, "Follow Radius", t_tab_2_opt_3, "Area size at which the Sprite will stop following our mouse.");
  GtkWidget *t_tab_2_opt_4 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_2), 3, "Enable Gravity", t_tab_2_opt_4, "Allow the sprite to fall");
  GtkAdjustment *a_tab_2_opt_5 = gtk_adjustment_new(20.0, 0.0, 30.0, 5.0, 30.0, 0);
  GtkWidget *t_tab_2_opt_5 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *t_tab_2_opt_5_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_2_opt_5);
  gtk_widget_set_size_request(t_tab_2_opt_5_o1, 160, -1);
  GtkWidget *t_tab_2_opt_5_o2 = gtk_spin_button_new(a_tab_2_opt_5, 5.0, 1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_5), t_tab_2_opt_5_o1);
  gtk_box_append(GTK_BOX(t_tab_2_opt_5), t_tab_2_opt_5_o2);
  add_setting_row(GTK_GRID(g_tab_2), 4, "Gravity Strength", t_tab_2_opt_5, "How fast the sprite falls, Micmicking gravity");
  GtkWidget *t_tab_2_opt_6 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_2), 5, "Start at Bottom", t_tab_2_opt_6, "Spawn the sprite at the bottom of your window screen");
  GtkWidget *t_tab_2_opt_7 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_2_opt_7), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_2), 6, "Start Sleep", t_tab_2_opt_7, "Seconds before sprite sleeps");
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_2, l_tab_2);

  GtkWidget *l_tab_3 = gtk_label_new("Random Actions");
  GtkWidget *g_tab_3 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_3), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_3), 20);
  GtkWidget *t_tab_3_opt_1 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_3), 0, "Allow Random Actions", t_tab_3_opt_1, "Allow the sprite to perform random actions");
  GtkWidget *t_tab_3_opt_2 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_3_opt_2), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 1, "Minimum Interval", t_tab_3_opt_2, "Minimum random action interval (seconds) [It will crash if this is higher than Max, heh]");
  GtkWidget *t_tab_3_opt_3 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_3_opt_3), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 2, "Maximum Interval", t_tab_3_opt_3, "Maximum random action interval (seconds)");
  GtkWidget *t_tab_3_opt_4 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_3_opt_4), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 3, "Walk Distance", t_tab_3_opt_4, "Distance the sprite moves when walking"); 
  GtkWidget *t_tab_3_opt_5 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_3_opt_5), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 4, "Random Move Distance", t_tab_3_opt_5, "Distance for random movements [Walk speed]");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_3, l_tab_3);

  GtkWidget *l_tab_4 = gtk_label_new("Configuration");
  GtkWidget *g_tab_4 = gtk_grid_new();
  GtkWidget *t_tab_4_opt_1 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 0, "Allow Color Hotspot", t_tab_4_opt_1, "Enable color hotspots for debugging");
  GtkWidget *t_tab_4_opt_2 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 1, "Disable hotspots", t_tab_4_opt_2, "Disable all sprite hotspots around the sprites [Can be disabled if you use Keyboard Controls]");
  GtkWidget *t_tab_4_opt_3 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 2, "Enable Minmum Resize", t_tab_4_opt_3, "Allow sprite to be resized minimally");
  GtkWidget *t_tab_4_opt_4 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 3, "Force Centre", t_tab_4_opt_4, "Keep sprite centered in window. Turn this off since this will override every positional setting");
  GtkWidget *t_tab_4_opt_5 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 4, "Force Fake Transparent", t_tab_4_opt_5, "Use fake transparency if needed. (Windows) On linux, transparency depends on a compositor (X11), and works natively on wayland.");
  GtkWidget *t_tab_4_opt_6 = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 5, "Allow Cache", t_tab_4_opt_6, "Enable memory caching for performance [Experimental] (Windows)");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_4, l_tab_4);
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_4), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_4), 20);

  GtkWidget *g_tab_5 = gtk_grid_new();
  GtkWidget *l_tab_5 = gtk_label_new("Quirks");
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_5), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_5), 20);
  GtkWidget *t_tab_5_opt_1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkAdjustment *a_tab_5_opt_1 = gtk_adjustment_new(0.3, 0.0, 1.0, 0.1, 1.0, 0);
  GtkWidget *t_tab_5_opt_1_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_5_opt_1);
  gtk_widget_set_size_request(GTK_WIDGET(t_tab_5_opt_1_o1), 160, -1);
  GtkWidget *t_tab_5_opt_1_o2 = gtk_spin_button_new(a_tab_5_opt_1, 0.1, 1);
  gtk_box_append(GTK_BOX(t_tab_5_opt_1), t_tab_5_opt_1_o1);
  gtk_box_append(GTK_BOX(t_tab_5_opt_1), t_tab_5_opt_1_o2);
  add_setting_row(GTK_GRID(g_tab_5), 0, "Current Acceleration", t_tab_5_opt_1, "Acceleration when following food/item");
  GtkAdjustment *a_tab_5_opt_2 = gtk_adjustment_new(0.2, 0.0, 1.0, 0.1, 1.0, 0);
  GtkWidget *t_tab_5_opt_2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *t_tab_5_opt_2_o1 = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_tab_5_opt_2);
  GtkWidget *t_tab_5_opt_2_o2 = gtk_spin_button_new(a_tab_5_opt_2, 0.1, 1);
  gtk_box_append(GTK_BOX(t_tab_5_opt_2), t_tab_5_opt_2_o1);
  gtk_box_append(GTK_BOX(t_tab_5_opt_2), t_tab_5_opt_2_o2);
  add_setting_row(GTK_GRID(g_tab_5), 1, "Follow Acceleration", t_tab_5_opt_2, "Acceleration when following food/item");
  GtkWidget *t_tab_5_opt_3 = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(t_tab_5_opt_3), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_5), 2, "Max Acceleration", t_tab_5_opt_3, "Maxmimum allowed acceleration");
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_5, l_tab_5);

  gtk_box_append(GTK_BOX(box), notebook);
  GtkWidget *g_tab_6 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 40);
  GtkWidget *l_tab_6 = gtk_label_new("Cool Information");
  GtkWidget *meme_image = gtk_image_new_from_resource("/io/github/desktop-gremlin-linux/meme.png");
  gtk_image_set_pixel_size(GTK_IMAGE(meme_image), 300);
  GtkWidget *infolabel = gtk_label_new("desktop-gremlin-linux by potato-master369\nVersion 4.0.0 (Pre-release)");
  gtk_widget_set_valign(infolabel, GTK_ALIGN_START);
  GtkWidget *vbox_tab_6 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_box_append(GTK_BOX(g_tab_6), vbox_tab_6);
  gtk_box_append(GTK_BOX(vbox_tab_6), meme_image);
  gtk_box_append(GTK_BOX(vbox_tab_6), infolabel);
  GtkWidget *guthib = gtk_button_new_with_label("Github");
  GtkWidget *yootoob = gtk_button_new_with_label("YouTube");
  gtk_box_append(GTK_BOX(vbox_tab_6), guthib);
  gtk_box_append(GTK_BOX(vbox_tab_6), yootoob);
  GtkWidget *loglabel = gtk_label_new("This is where I put version history and memes\n\n4.0 (I skipped to 4.0)\nAdded the entire thing");
  gtk_widget_set_vexpand(loglabel, false);
  gtk_box_append(GTK_BOX(g_tab_6), loglabel);
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_6, l_tab_6);

  GtkWidget *copyright_label = gtk_label_new("desktop-gremlin-linux v4.x - potato-master369 (GitHub)");
  gtk_widget_set_halign(copyright_label, GTK_ALIGN_END);
  gtk_box_append(GTK_BOX(box), copyright_label);
  GtkWidget *gsw = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(gsw), box);
  gtk_window_set_child(GTK_WINDOW(w), gsw);
  gtk_window_present(GTK_WINDOW(w));
}

int main(int argc, char **argv) {
  g_resources_register(app_get_resource());
  GtkApplication *app = gtk_application_new(
      "io.github.potato-master369.degrli-options", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
