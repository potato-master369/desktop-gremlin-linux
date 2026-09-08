#ifndef HOTSPOT_H
#define HOTSPOT_H
#include <gtk/gtk.h>

void hotspot_play(int x, int y);
void hotspot_init(GtkFixed *fcontainer, double sprite_x, double sprite_y);
void hotspot_update(GtkFixed *fcontainer, double sprite_x, double sprite_y);
#endif
