#ifndef DEGRLI_PKG_H
#define DEGRLI_PKG_H
#include <gio/gio.h>
#include <gtk/gtk.h>

typedef struct {
  char *pkgname;
  int status;
} pkg_entry_t;

typedef struct {
  int len;
  pkg_entry_t *entries;
} pkg_list_t;

#define DEGRLI_PKG_PATH "/usr/share/desktop-gremlin-linux/dlpm/recipes"
#define DEGRLI_PKG_WORLDPATH "/usr/share/desktop-gremlin-linux/dlpm/world"
#define DEGRLI_PKG_INSTALLED 1
#define DEGRLI_PKG_N_INSTALLED 0

void pkg_populate_gui(GListStore *self);
void pkg_setup_box(GtkWidget *self);
void pkg_update_box(GListStore *dat, int sel);
int pkg_init(void);

#endif
