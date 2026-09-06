#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtk/gtkshortcut.h>
#include "meme.h"
#include "config.h"
#include "../defines.h"
#include "../trace.h"
// Made with GTK-4.0
// Options program, designed to emulate Options.exe from Desktop_Gremlin

static GtkStringList *char_list = NULL;
static config_t conf;

// Moved these to toplevel so we can mess with them
static GtkWidget *w_start_char = NULL;
static GtkWidget *w_language_diff = NULL;
static GtkWidget *w_enable_keyboard = NULL;
static GtkWidget *w_show_taskbar = NULL;
static GtkWidget *w_allow_error_messages = NULL;
static GtkWidget *w_use_wpfplayer = NULL;
static GtkWidget *w_volume_level = NULL;
static GtkWidget *w_randomize_spawn = NULL;
static GtkWidget *w_spawn_distance = NULL;
static GtkWidget *w_sprite_framerate = NULL;
static GtkWidget *w_sprite_speed = NULL;
static GtkWidget *w_follow_radius = NULL;
static GtkWidget *w_enable_gravity = NULL;
static GtkWidget *w_gravity_strength = NULL;
static GtkWidget *w_start_buttom = NULL;
static GtkWidget *w_sleep_time = NULL;
static GtkWidget *w_allow_random_actions = NULL;
static GtkWidget *w_min_interval = NULL;
static GtkWidget *w_max_interval = NULL;
static GtkWidget *w_walk_distance = NULL;
static GtkWidget *w_random_move_distance = NULL;
static GtkWidget *w_allow_color_hotspot = NULL;
static GtkWidget *w_disable_hotspots = NULL;
static GtkWidget *w_enable_min_resize = NULL;
static GtkWidget *w_force_center = NULL;
static GtkWidget *w_enable_manual_resize = NULL;
static GtkWidget *w_force_fake_transparent = NULL;
static GtkWidget *w_allow_cache = NULL;
static GtkWidget *w_current_acceleration = NULL;
static GtkWidget *w_follow_acceleration = NULL;
static GtkWidget *w_max_acceleration = NULL;

static GtkAdjustment *a_volume_level = NULL;
static GtkAdjustment *a_sprite_speed = NULL;
static GtkAdjustment *a_follow_radius = NULL;
static GtkAdjustment *a_gravity_strength = NULL;
static GtkAdjustment *a_current_acceleration = NULL;
static GtkAdjustment *a_follow_acceleration = NULL;

static void find_characters(void) {
  char path[256];
  snprintf(path, sizeof(path), "%sSpriteSheet/Gremlins", DEGRLI_ASSET_DIR);
  GDir *directory = g_dir_open(path, 0, NULL);
  if (directory == NULL) {
    trace_log(ERROR, "Could not open character directory: %s\n", path);
    return;
  }

  const char *name;
  while ((name = g_dir_read_name(directory)) != NULL) {
    char character_path[512];
    snprintf(character_path, sizeof(character_path), "%s/%s", path, name);
    if (g_file_test(character_path, G_FILE_TEST_IS_DIR)) {
      gtk_string_list_append(char_list, name);
    }
  }
  g_dir_close(directory);
}

static int get_entry_int(GtkWidget *widget) {
  return atoi(gtk_editable_get_text(GTK_EDITABLE(widget)));
}

static void set_entry_int(GtkWidget *widget, int value) {
  char text[32];
  snprintf(text, sizeof(text), "%d", value);
  gtk_editable_set_text(GTK_EDITABLE(widget), text);
}

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

// function to start the child process
static void unleash_gremlin(gpointer user_data) {
  // requires GNU coreutils.
  // only works if degrli is in path.
  if (system("nohup degrli > /dev/null 2>&1 &") == 127) {
    trace_log(WARN, " WARNING: Either nohup or degrli was not found. Nohup is part of the coreutils, so it's probably degrli. Check if it's in PATH.\n");
  }
}

int horde_i = 0;
static int horde_callback(gpointer user_data) {
  if (horde_i == g_list_model_get_n_items(G_LIST_MODEL(char_list))) {
    return G_SOURCE_REMOVE;
  } else {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "nohup degrli --char %s > /dev/null 2>&1 &", gtk_string_list_get_string(char_list, horde_i));
    if (system(cmd) == 127) {
      trace_log(WARN, "WARNING: Either nohup or degrli was not found. Nohup is part of the coreutils, so it's probably degrli. Check if it's in PATH.\n");
    }
    horde_i++;
  }
  return G_SOURCE_CONTINUE;
}

static void unleash_horde(gpointer user_data) {
  horde_i = 0;
  g_timeout_add(1000, horde_callback, NULL);
}

static void apply_to_conf(void) {
  if (!conf.start_char[0]) {
    strncpy(conf.start_char, "Cafe", sizeof(conf.start_char));
    conf.start_char[sizeof(conf.start_char) - 1] = '\0';
  }

  conf.language_diff = gtk_switch_get_active(GTK_SWITCH(w_language_diff));
  conf.enable_keyboard = gtk_switch_get_active(GTK_SWITCH(w_enable_keyboard));
  conf.allow_error_messages = gtk_switch_get_active(GTK_SWITCH(w_allow_error_messages));
  conf.show_taskbar = gtk_switch_get_active(GTK_SWITCH(w_show_taskbar));
  conf.randomize_spawn = gtk_switch_get_active(GTK_SWITCH(w_randomize_spawn));
  conf.use_wpfplayer = gtk_switch_get_active(GTK_SWITCH(w_use_wpfplayer));
  conf.volume_level = gtk_adjustment_get_value(a_volume_level);
  conf.spawn_distance = get_entry_int(w_spawn_distance);
  conf.sprite_framerate = get_entry_int(w_sprite_framerate);
  conf.sprite_speed = (int)gtk_adjustment_get_value(a_sprite_speed);
  conf.follow_radius = (int)gtk_adjustment_get_value(a_follow_radius);
  conf.enable_gravity = gtk_switch_get_active(GTK_SWITCH(w_enable_gravity));
  conf.gravity_strength = (int)gtk_adjustment_get_value(a_gravity_strength);
  conf.start_buttom = gtk_switch_get_active(GTK_SWITCH(w_start_buttom));
  conf.sleep_time = get_entry_int(w_sleep_time);
  conf.allow_random_actions = gtk_switch_get_active(GTK_SWITCH(w_allow_random_actions));
  conf.min_interval = get_entry_int(w_min_interval);
  conf.max_interval = get_entry_int(w_max_interval);
  conf.walk_distance = get_entry_int(w_walk_distance);
  conf.random_move_distance = get_entry_int(w_random_move_distance);
  conf.allow_color_hotspot = gtk_switch_get_active(GTK_SWITCH(w_allow_color_hotspot));
  conf.disable_hotspots = gtk_switch_get_active(GTK_SWITCH(w_disable_hotspots));
  conf.enable_min_resize = gtk_switch_get_active(GTK_SWITCH(w_enable_min_resize));
  conf.force_center = gtk_switch_get_active(GTK_SWITCH(w_force_center));
  conf.enable_manual_resize = gtk_switch_get_active(GTK_SWITCH(w_enable_manual_resize));
  conf.force_fake_transparent = gtk_switch_get_active(GTK_SWITCH(w_force_fake_transparent));
  conf.allow_cache = gtk_switch_get_active(GTK_SWITCH(w_allow_cache));
  conf.current_acceleration = gtk_adjustment_get_value(a_current_acceleration);
  conf.follow_acceleration = gtk_adjustment_get_value(a_follow_acceleration);
  conf.max_acceleration = get_entry_int(w_max_acceleration);

  if (w_start_char != NULL && GTK_IS_DROP_DOWN(w_start_char)) {
    guint selected = gtk_drop_down_get_selected(GTK_DROP_DOWN(w_start_char));
    GtkStringObject *item = GTK_STRING_OBJECT(g_list_model_get_item(G_LIST_MODEL(gtk_drop_down_get_model(GTK_DROP_DOWN(w_start_char))), selected));
    if (item != NULL) {
      const char *name = gtk_string_object_get_string(item);
      if (name != NULL && name[0] != '\0') {
        strncpy(conf.start_char, name, sizeof(conf.start_char) - 1);
        conf.start_char[sizeof(conf.start_char) - 1] = '\0';
      }
    }
  }
}

static void apply_from_conf(void) {
  guint character_count = g_list_model_get_n_items(G_LIST_MODEL(char_list));
  for (guint i = 0; i < character_count; ++i) {
    const char *name = gtk_string_list_get_string(char_list, i);
    if (g_strcmp0(name, conf.start_char) == 0) {
      gtk_drop_down_set_selected(GTK_DROP_DOWN(w_start_char), i);
      break;
    }
  }
  gtk_switch_set_active(GTK_SWITCH(w_language_diff), conf.language_diff);
  gtk_switch_set_active(GTK_SWITCH(w_enable_keyboard), conf.enable_keyboard);
  gtk_switch_set_active(GTK_SWITCH(w_allow_error_messages), conf.allow_error_messages);
  gtk_switch_set_active(GTK_SWITCH(w_show_taskbar), conf.show_taskbar);
  gtk_switch_set_active(GTK_SWITCH(w_randomize_spawn), conf.randomize_spawn);
  gtk_switch_set_active(GTK_SWITCH(w_use_wpfplayer), conf.use_wpfplayer);
  gtk_adjustment_set_value(a_volume_level, conf.volume_level);
  set_entry_int(w_spawn_distance, conf.spawn_distance);
  set_entry_int(w_sprite_framerate, conf.sprite_framerate);
  gtk_adjustment_set_value(a_sprite_speed, conf.sprite_speed);
  gtk_adjustment_set_value(a_follow_radius, conf.follow_radius);
  gtk_switch_set_active(GTK_SWITCH(w_enable_gravity), conf.enable_gravity);
  gtk_adjustment_set_value(a_gravity_strength, conf.gravity_strength);
  gtk_switch_set_active(GTK_SWITCH(w_start_buttom), conf.start_buttom);
  set_entry_int(w_sleep_time, conf.sleep_time);
  gtk_switch_set_active(GTK_SWITCH(w_allow_random_actions), conf.allow_random_actions);
  set_entry_int(w_min_interval, conf.min_interval);
  set_entry_int(w_max_interval, conf.max_interval);
  set_entry_int(w_walk_distance, conf.walk_distance);
  set_entry_int(w_random_move_distance, conf.random_move_distance);
  gtk_switch_set_active(GTK_SWITCH(w_allow_color_hotspot), conf.allow_color_hotspot);
  gtk_switch_set_active(GTK_SWITCH(w_disable_hotspots), conf.disable_hotspots);
  gtk_switch_set_active(GTK_SWITCH(w_enable_min_resize), conf.enable_min_resize);
  gtk_switch_set_active(GTK_SWITCH(w_force_center), conf.force_center);
  gtk_switch_set_active(GTK_SWITCH(w_enable_manual_resize), conf.enable_manual_resize);
  gtk_switch_set_active(GTK_SWITCH(w_force_fake_transparent), conf.force_fake_transparent);
  gtk_switch_set_active(GTK_SWITCH(w_allow_cache), conf.allow_cache);
  gtk_adjustment_set_value(a_current_acceleration, conf.current_acceleration);
  gtk_adjustment_set_value(a_follow_acceleration, conf.follow_acceleration);
  set_entry_int(w_max_acceleration, conf.max_acceleration);

}

static void revert_settings(GtkWidget *widget, gpointer data) {
  trace_log(INFO, "Revert settings button was pressed\n");
  // Reset to default... means application defaults? Sure i guess kurt
  // tho setting to original before edits would make more sense;
  conf_apply_default(&conf);
  apply_from_conf();
}

static void save_defaults(GtkWidget *widget, gpointer data) {
  trace_log(INFO, "Saving settings!\n");
  // Save them to the file. Because we dont give a shit about kurt
  // We will just save them in whatever format the C library wants
  apply_to_conf();
  write_conf(conf);
}

static void activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *w = gtk_application_window_new(app);
  gtk_window_set_default_size(GTK_WINDOW(w), 800, 600);
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
  g_signal_connect(b_spawn, "clicked", G_CALLBACK(unleash_gremlin), NULL);
  g_signal_connect(b_horde, "clicked", G_CALLBACK(unleash_horde), NULL);
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
  find_characters();
  w_start_char = gtk_drop_down_new(G_LIST_MODEL(char_list), NULL);
  add_setting_row(GTK_GRID(g_tab_1), 0, "Starting Character", w_start_char, "Available Characters in SpriteSheet/Gremlins");
  w_language_diff = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 1, "Language Difference", w_language_diff, "Windows Machines with non-English locale will break the 'config.txt'. Leave this on.");
  w_enable_keyboard = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 2, "Enable Keyboard", w_enable_keyboard, "Allow Keyboard control for the gremlin");
  w_show_taskbar = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 3, "Show Taskbar Icon", w_show_taskbar, "Show the Program in the taskbar (Windows)");
  w_allow_error_messages = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 4, "Allow Error Messages", w_allow_error_messages, "Display Error Messages (just check stdout i dont really do error messages)");
  w_use_wpfplayer = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 5, "Use WPF Player", w_use_wpfplayer, "Switch between WPF and SoundPlayer. Some systems cannot use WPF Players, unless manually enabled. (Windows-only, linux use miniaudio.h");
  a_volume_level = gtk_adjustment_new(0.5, 0.00, 1.00, 0.05, 1.00, 0);
  GtkWidget *volume_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *volume_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_volume_level);
  gtk_widget_set_size_request(volume_scale, 160, -1);
  GtkWidget *volume_spin = gtk_spin_button_new(a_volume_level, 0.05, 2);
  gtk_box_append(GTK_BOX(volume_box), volume_scale);
  gtk_box_append(GTK_BOX(volume_box), volume_spin);
  w_volume_level = volume_box;
  add_setting_row(GTK_GRID(g_tab_1), 6, "Volume Level", w_volume_level, "Volume can only be changed if using WPF Player (on Windows), but on Linux it works fine regardless.");
  w_randomize_spawn = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_1), 7, "Randomise Spawn", w_randomize_spawn, "[Disable force centre in Configuration first] Spawns the sprites in a random location upon initialisation. The 'start at bottom' in Sprite Settings, will be affected by this.");
  w_spawn_distance = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_spawn_distance), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_1), 8, "Spawn Distance", w_spawn_distance, "The Random Distance variance from the centre. [Higher = more spread] [Lower = Closer to the centre]");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_1, l_tab_1);
  
  GtkWidget *l_tab_2 = gtk_label_new("Sprite Settings");
  GtkWidget *g_tab_2 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_2), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_2), 20);
  w_sprite_framerate = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_sprite_framerate), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_2), 0, "FrameRate:", w_sprite_framerate, "Frames per second for the sprite player");
  a_sprite_speed = gtk_adjustment_new(10.0, 0.0, 30.0, 1.0, 1.0, 0);
  GtkWidget *sprite_speed_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *sprite_speed_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_sprite_speed);
  gtk_widget_set_size_request(sprite_speed_scale, 160, -1);
  GtkWidget *sprite_speed_spin = gtk_spin_button_new(a_sprite_speed, 1.0, 1);
  gtk_box_append(GTK_BOX(sprite_speed_box), sprite_speed_scale);
  gtk_box_append(GTK_BOX(sprite_speed_box), sprite_speed_spin);
  w_sprite_speed = sprite_speed_box;
  add_setting_row(GTK_GRID(g_tab_2), 1, "Movement Speed", w_sprite_speed, "The speed at which the Sprite follows your mouse");

  a_follow_radius = gtk_adjustment_new(150.0, 0.0, 300.0, 10.0, 50.0, 0);
  GtkWidget *follow_radius_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *follow_radius_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_follow_radius);
  gtk_widget_set_size_request(follow_radius_scale, 160, -1);
  GtkWidget *follow_radius_spin = gtk_spin_button_new(a_follow_radius, 10.0, 1);
  gtk_box_append(GTK_BOX(follow_radius_box), follow_radius_scale);
  gtk_box_append(GTK_BOX(follow_radius_box), follow_radius_spin);
  w_follow_radius = follow_radius_box;
  add_setting_row(GTK_GRID(g_tab_2), 2, "Follow Radius", w_follow_radius, "Area size at which the Sprite will stop following our mouse.");
  w_enable_gravity = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_2), 3, "Enable Gravity", w_enable_gravity, "Allow the sprite to fall");
  a_gravity_strength = gtk_adjustment_new(20.0, 0.0, 30.0, 5.0, 30.0, 0);
  GtkWidget *gravity_strength_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  GtkWidget *gravity_strength_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_gravity_strength);
  gtk_widget_set_size_request(gravity_strength_scale, 160, -1);
  GtkWidget *gravity_strength_spin = gtk_spin_button_new(a_gravity_strength, 5.0, 1);
  gtk_box_append(GTK_BOX(gravity_strength_box), gravity_strength_scale);
  gtk_box_append(GTK_BOX(gravity_strength_box), gravity_strength_spin);
  w_gravity_strength = gravity_strength_box;
  add_setting_row(GTK_GRID(g_tab_2), 4, "Gravity Strength", w_gravity_strength, "How fast the sprite falls, Micmicking gravity");
  w_start_buttom = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_2), 5, "Start at Bottom", w_start_buttom, "Spawn the sprite at the bottom of your window screen");
  w_sleep_time = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_sleep_time), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_2), 6, "Start Sleep", w_sleep_time, "Seconds before sprite sleeps");
  
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_2, l_tab_2);

  GtkWidget *l_tab_3 = gtk_label_new("Random Actions");
  GtkWidget *g_tab_3 = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_3), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_3), 20);
  w_allow_random_actions = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_3), 0, "Allow Random Actions", w_allow_random_actions, "Allow the sprite to perform random actions");
  w_min_interval = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_min_interval), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 1, "Minimum Interval", w_min_interval, "Minimum random action interval (seconds) [It will crash if this is higher than Max, heh]");
  w_max_interval = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_max_interval), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 2, "Maximum Interval", w_max_interval, "Maximum random action interval (seconds)");
  w_walk_distance = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_walk_distance), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 3, "Walk Distance", w_walk_distance, "Distance the sprite moves when walking");
  w_random_move_distance = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_random_move_distance), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_3), 4, "Random Move Distance", w_random_move_distance, "Distance for random movements [Walk speed]");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_3, l_tab_3);

  GtkWidget *l_tab_4 = gtk_label_new("Configuration");
  GtkWidget *g_tab_4 = gtk_grid_new();
  w_allow_color_hotspot = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 0, "Allow Color Hotspot", w_allow_color_hotspot, "Enable color hotspots for debugging");
  w_disable_hotspots = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 1, "Disable hotspots", w_disable_hotspots, "Disable all sprite hotspots around the sprites [Can be disabled if you use Keyboard Controls]");
  w_enable_min_resize = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 2, "Enable Minmum Resize", w_enable_min_resize, "Allow sprite to be resized minimally");
  w_force_center = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 3, "Force Centre", w_force_center, "Keep sprite centered in window. Turn this off since this will override every positional setting");
  w_enable_manual_resize = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 4, "Enable Manual Resize", w_enable_manual_resize, "Allow manual resizing of the sprite window.");
  w_force_fake_transparent = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 5, "Force Fake Transparent", w_force_fake_transparent, "Use fake transparency if needed. (Windows) On linux, transparency depends on a compositor (X11), and works natively on wayland.");
  w_allow_cache = gtk_switch_new();
  add_setting_row(GTK_GRID(g_tab_4), 6, "Allow Cache", w_allow_cache, "Enable memory caching for performance [Experimental] (Windows)");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), g_tab_4, l_tab_4);
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_4), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_4), 20);

  GtkWidget *g_tab_5 = gtk_grid_new();
  GtkWidget *l_tab_5 = gtk_label_new("Quirks");
  gtk_grid_set_column_spacing(GTK_GRID(g_tab_5), 20);
  gtk_grid_set_row_spacing(GTK_GRID(g_tab_5), 20);
  GtkWidget *current_accel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  a_current_acceleration = gtk_adjustment_new(0.3, 0.0, 1.0, 0.1, 1.0, 0);
  GtkWidget *current_accel_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_current_acceleration);
  gtk_widget_set_size_request(GTK_WIDGET(current_accel_scale), 160, -1);
  GtkWidget *current_accel_spin = gtk_spin_button_new(a_current_acceleration, 0.1, 1);
  gtk_box_append(GTK_BOX(current_accel_box), current_accel_scale);
  gtk_box_append(GTK_BOX(current_accel_box), current_accel_spin);
  w_current_acceleration = current_accel_box;
  add_setting_row(GTK_GRID(g_tab_5), 0, "Current Acceleration", w_current_acceleration, "Acceleration when following food/item");
  GtkWidget *follow_accel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  a_follow_acceleration = gtk_adjustment_new(0.2, 0.0, 1.0, 0.1, 1.0, 0);
  GtkWidget *follow_accel_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, a_follow_acceleration);
  GtkWidget *follow_accel_spin = gtk_spin_button_new(a_follow_acceleration, 0.1, 1);
  gtk_widget_set_size_request(GTK_WIDGET(follow_accel_scale), 160, -1);
  gtk_box_append(GTK_BOX(follow_accel_box), follow_accel_scale);
  gtk_box_append(GTK_BOX(follow_accel_box), follow_accel_spin);
  w_follow_acceleration = follow_accel_box;
  add_setting_row(GTK_GRID(g_tab_5), 1, "Follow Acceleration", w_follow_acceleration, "Acceleration when following food/item");
  w_max_acceleration = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(w_max_acceleration), GTK_INPUT_PURPOSE_DIGITS);
  add_setting_row(GTK_GRID(g_tab_5), 2, "Max Acceleration", w_max_acceleration, "Maxmimum allowed acceleration");
  
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
  apply_from_conf();
  gtk_window_set_child(GTK_WINDOW(w), gsw);
  g_signal_connect(b_revert, "clicked", G_CALLBACK(revert_settings), NULL);
  g_signal_connect(b_save, "clicked", G_CALLBACK(save_defaults), NULL);
  gtk_window_present(GTK_WINDOW(w));
}

int main(int argc, char **argv) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  trace_log(WARN, "WARNING: This is a pre-release version!\nCompiled on %s %s\n", __DATE__, __TIME__);
#endif
  conf_apply_default(&conf);
  load_conf(&conf);
  g_resources_register(app_get_resource());
  GtkApplication *app = gtk_application_new(
      "io.github.potato-master369.degrli-options", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}
