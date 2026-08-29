#include <glib-object.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include "payload.h"

GtkWidget *window;
GtkWidget *label;
GtkWidget *button;

typedef struct {
  char log[2048];
  int step;  // 0=clone, 1=cd, 2=build, 3=install, 4=done
} StageData;

static void stage_1(gpointer user_data);

static void finish_callback(GtkWidget *widget, gpointer user_data) {
  gtk_window_close(GTK_WINDOW(window));
}

static gboolean update_ui_callback(gpointer user_data) {
  StageData *data = (StageData *)user_data;
  gtk_label_set_markup(GTK_LABEL(label), data->log);
  if (data->step == 4) {  // done
    gtk_button_set_label(GTK_BUTTON(button), "Finish");
    gtk_widget_set_sensitive(button, TRUE);
    g_signal_handlers_disconnect_by_func(button, G_CALLBACK(stage_3), NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(finish_callback), NULL);
    g_free(data);
    return FALSE;
  }
  return FALSE;
}

static gpointer stage_3_thread(gpointer user_data) {
  StageData *data = (StageData *)user_data;
  int result;

  strcpy(data->log, "<b>Cloning git...</b>\n");
  data->step = 0;
  g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
  
  result = clonegit();
  if (result != 0) {
    strncat(data->log, "STOP: something went wrong while performing the above action.\n", sizeof(data->log) - strlen(data->log) - 1);
    data->step = 4;
    g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
    g_free(data);
    return NULL;
  }
  g_usleep(100000);  // slight delay to allow UI update
  
  strncat(data->log, "<b>Changing directory...</b>\n", sizeof(data->log) - strlen(data->log) - 1);
  data->step = 1;
  g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
  
  result = cd_src();
  if (result != 0) {
    strncat(data->log, "STOP: something went wrong while performing the above action.\n", sizeof(data->log) - strlen(data->log) - 1);
    data->step = 4;
    g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
    g_free(data);
    return NULL;
  }
  g_usleep(100000);
  
  strncat(data->log, "<b>Building...</b>\n", sizeof(data->log) - strlen(data->log) - 1);
  data->step = 2;
  g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
  
  result = build_src();
  if (result != 0) {
    strncat(data->log, "STOP: something went wrong while performing the above action.\n", sizeof(data->log) - strlen(data->log) - 1);
    data->step = 4;
    g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
    g_free(data);
    return NULL;
  }
  g_usleep(100000);
  
  strncat(data->log, "<b>Installing...</b>\n", sizeof(data->log) - strlen(data->log) - 1);
  data->step = 3;
  g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
  
  result = install_src();
  if (result != 0) {
    strncat(data->log, "STOP: something went wrong while performing the above action.\n", sizeof(data->log) - strlen(data->log) - 1);
    data->step = 4;
    g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
    g_free(data);
    return NULL;
  }
  g_usleep(100000);
  
  strncat(data->log, "Done! Installation complete.\n", sizeof(data->log) - strlen(data->log) - 1);
  data->step = 4;
  g_idle_add(update_ui_callback, g_memdup2(data, sizeof(StageData)));
  g_free(data);
  return NULL;
}

static void stage_3(gpointer user_data) {
  StageData *data = g_malloc(sizeof(StageData));
  memset(data->log, 0, sizeof(data->log));
  data->step = 0;
  
  gtk_widget_set_sensitive(button, FALSE);
  g_thread_new("stage_3", stage_3_thread, data);
}

static void stage_2(gpointer user_data) {
  char log[512] = "<b>Checking for dependencies...</b>\n";
  int result = 0;
  bool fail = false;
  gtk_label_set_markup(GTK_LABEL(label), log);
  result = depcheck(DEP_GCC);
  if (result != 0) {
    strncat(log, "GCC not found!\n", sizeof(log) - strlen(log) - 1);
    fail = true;
  }
  result = depcheck(DEP_PKGCONFIG);
  if (result != 0) {
    strncat(log, "pkg-config not found!\n", sizeof(log) - strlen(log) - 1);
    fail = true;
  }
  result = depcheck(DEP_GIT);
  if (result != 0) {
    strncat(log, "git not found!\n", sizeof(log) - strlen(log) - 1);
    fail = true;
  }
  result = depcheck(DEP_GTK4);
  if (result != 0) {
    strncat(log, "GTK4 not found!\n", sizeof(log) - strlen(log) - 1);
    fail = true;
  }
  result = depcheck(DEP_GTK4_LAYERSHELL);
  if (result != 0) {
    strncat(log, "gtk4-layer-shell not found!\n", sizeof(log) - strlen(log) - 1);
    fail = true;
  }
  strncat(log, "Done! If any dependencies were not found, (e.g. \"... not found!\"), install them using your OS's package manager and try again. If it's all good, press \"Next\" to continue!\n", sizeof(log) - strlen(log) - 1);
  gtk_label_set_markup(GTK_LABEL(label), log);
  if (fail) {
    gtk_widget_set_sensitive(button, FALSE);
  } else {
    g_signal_handlers_disconnect_by_func(button, G_CALLBACK(stage_1), NULL);
    g_signal_connect(button, "clicked", G_CALLBACK(stage_3), NULL);
  }
}

static void stage_1(gpointer user_data) {
  gtk_label_set_markup(
      GTK_LABEL(label),
      "<b>We will now continue to checking for dependencies</b>\n\n"
      "desktop-gremlin-linux requires, for installation: \n"
      " - GCC\n - GNU Make or equivalent that provides make\n - pkg-config\n - "
      "GTK4 development headers and library\n - gtk4-layer-shell development "
      "headers and library\n - git\nEnsure that these are installed.");
  g_signal_connect(button, "clicked", G_CALLBACK(stage_2), NULL);
}

static void activate(GtkApplication *app, gpointer user_data) {
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Desktop Gremlin Linux Installer");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_margin_top(box, 20);
  gtk_widget_set_margin_start(box, 20);
  gtk_widget_set_margin_end(box, 20);
  label = gtk_label_new(
      "Welcome to the desktop-gremlin-linux-installer. This will guide you "
      "through installing desktop-gremlin-linux on your Workstation. To "
      "continue, press \"Next\". If not, close this "
      "window.\n\ndesktop-gremlin-linux is a free and open source software, "
      "made by potato-master369. GitHub is the ONLY place to get it. If you "
      "downloaded me off of another site, I may have been tampered with.");
  gtk_label_set_use_markup(GTK_LABEL(label), true);
  button = gtk_button_new_with_label("Next");
  gtk_widget_set_hexpand(button, false);
  gtk_widget_set_halign(button, GTK_ALIGN_END);
  gtk_widget_set_hexpand(label, false);
  gtk_label_set_wrap(GTK_LABEL(label), true);
  gtk_widget_set_halign(label, GTK_ALIGN_START);
  gtk_box_append(GTK_BOX(box), label);
  gtk_box_append(GTK_BOX(box), button);
  gtk_window_set_child(GTK_WINDOW(window), box);
  gtk_window_present(GTK_WINDOW(window));
  g_signal_connect(button, "clicked", G_CALLBACK(stage_1), NULL);
}

int main(void) {
  GtkApplication *app =
      gtk_application_new("io.github.potato-master369.degrli-installer",
                          G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  int status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);
  return status;
}
