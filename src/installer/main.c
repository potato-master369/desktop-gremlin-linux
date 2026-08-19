#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Desktop Gremlin Linux Installer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_top(box, 20);
    gtk_widget_set_margin_start(box, 20);
    gtk_widget_set_margin_end(box, 20);
    GtkWidget *label = gtk_label_new("Welcome to the desktop-gremlin-linux-installer. This will guide you through installing desktop-gremlin-linux on your Workstation. To continue, press \"Next\". If not, close this window.\n\ndesktop-gremlin-linux is a free and open source software, made by potato-master369. GitHub is the ONLY place to get it. If you downloaded me off of another site, I may have been tampered with.");
    GtkWidget *button = gtk_button_new_with_label("Next");
    gtk_widget_set_hexpand(button, false);
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(label, false);
    gtk_label_set_wrap(GTK_LABEL(label), true);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), label);
    gtk_box_append(GTK_BOX(box), button);
    gtk_window_set_child(GTK_WINDOW(window), box);
    gtk_window_present(GTK_WINDOW(window));
}

int main(void) {
    GtkApplication *app = gtk_application_new(
        "io.github.potato-master369.degrli-installer", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
    return status;
}
