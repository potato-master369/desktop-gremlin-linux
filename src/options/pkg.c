// pkg.c
// ---------------------------
// Handle download manager

#define _XOPEN_SOURCE 700
#define _DEFAULT_SOURCE

#include <ctype.h>
#include "pkg.h"
#include "../trace.h"
#include <gio/gio.h>
#include <linux/limits.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

// configures line length
#define PKG_BUFSZ_X 4096

static pkg_list_t pkg_entries;
static GtkWidget *thefuckingbox;
static GtkWidget *header;
static GtkWidget *owndesc;
static GtkWidget *install;
static GtkWidget *desc;

static void pkg_init_list(void) {
  pkg_entries.len = 0;
  pkg_entries.entries = NULL;
}

static inline void pkg_ini_parse(FILE *fstream, void (*callback)(char *section, char *key, char *val)) {
  char *buf = (char *)malloc(PKG_BUFSZ_X * sizeof(char));
  bool isbom = false;
  char section[PKG_BUFSZ_X - 3];
  while (fgets(buf, PKG_BUFSZ_X, fstream) != NULL) {
    buf[strcspn(buf, "\r\n")] = '\0';

    // check for BOM
    if ((unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) {
      buf += 3;
      isbom = true;
    }

    // strip trailing spaces
    int len = strlen(buf);
    while (len > 0 && isspace((int)buf[len - 1])) {
      buf[len - 1] = '\0';
      --len;
    }
    // check if comment
    if (buf[0] == ';' || buf[0] == '#') {
      trace_log(DEBUG, "ini: comment: %s\n", buf);
      continue;
    }
    if (strlen(buf) == 0) {
      continue;
    }
    if (buf[0] == '[') {
      if (buf[strlen(buf) - 1] == ']') { // copy section
        strncpy(section, buf + 1, strlen(buf) - 2);
	section[strlen(buf) - 2] = '\0';
      } else
	      trace_log(WARN, "ini: malformed sec header\n");
      continue;
    }


    int offset = 0;
    char key[(PKG_BUFSZ_X - 1)/2];
    char value[(PKG_BUFSZ_X - 1)/2];
    sscanf(buf, " %2046[^ =] = %n", key, &offset);
    if (offset == 0) {
      trace_log(WARN, " ini: malformed kvp: %s\n", buf);
      continue;
    }
    strncpy(value, buf + offset, sizeof(value) - 1);
    trace_log(DEBUG, "ini: k \"%s\" v \"%s\"\n", key, value);
    callback(section, key, value); // call user provided parser
  }
}

static inline void pkg_list_append(char *name, int status) {
  if (!name)
	  return;
  void *tmp = realloc(pkg_entries.entries, (pkg_entries.len + 1) * sizeof(pkg_entries.entries[0]));
  if (tmp == NULL) {
    trace_log(ERROR, "malloc fail");
    return;
  }
  pkg_entries.entries = tmp;
  pkg_entries.entries[pkg_entries.len].pkgname = strdup(name);
  if (pkg_entries.entries[pkg_entries.len].pkgname == NULL) {
    trace_log(ERROR, "strdup fail");
    return;
  }
  pkg_entries.entries[pkg_entries.len].status = status;
  pkg_entries.len++;
}

static inline void pkg_list_free_all(void) {
  for (int i = 0; i < pkg_entries.len; ++i) {
      if (pkg_entries.entries[i].pkgname != NULL)
        free(pkg_entries.entries[i].pkgname);
  }
  free(pkg_entries.entries);
}

static inline int pkg_list_ssbn_safe(char *name, int status) {
  int found = -1;
  for (int i = 0; i < pkg_entries.len; ++i) {
    if (strcmp(pkg_entries.entries[i].pkgname, name) == 0) {
      found = i;
      break;
    }
  }
  if (found == -1)
	  return 1; // not found
  pkg_entries.entries[found].status = status;
  return 0;
}

int pkg_populate_list(FILE *wrldf) {
  DIR *pkgdir = opendir("/usr/share/desktop-gremlin-linux/dlpm/recipes");
  struct dirent *entry;

  if (!pkgdir) {
    trace_log(ERROR, "could not open the recipe folder");
    return 1;
  }

  while ((entry = readdir(pkgdir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      struct stat buf;
      char strbuf[PATH_MAX];
      snprintf(strbuf, PATH_MAX, DEGRLI_PKG_PATH "/%s/metadata.ini", entry->d_name);
      if (stat(strbuf, &buf) == 0) {
	      pkg_list_append(entry->d_name, DEGRLI_PKG_N_INSTALLED);
      }
      // is not a package
    }
  }

  // parse world file
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, wrldf)) != -1) {
    line[strcspn(line, "\r\n")] = '\0';
    int res = pkg_list_ssbn_safe(line, DEGRLI_PKG_INSTALLED);
    if (res != 0)
	    trace_log(WARN, "Package %s in world not found", line);
  }

  free(line);

  return 0;
}

int pkg_install_gui(char *name) {
  return 0;
}

int pkg_remove_gui(char *name) {
  return 0;
}

void pkg_populate_gui(GListStore *self) {
  for (int i = 0; i < pkg_entries.len; ++i) {
    g_list_store_append(self, gtk_string_object_new(pkg_entries.entries[i].pkgname));
  }
}

static void pkg_install_hook(GtkButton *button, gpointer user_data) {

}

#define MATCH(a, b) (strcmp(a, b) == 0)
static void pkg_ini_callback(char *section, char *key, char *value) {
  if MATCH(section, "Info") {
    if MATCH(key, "Name") {
      gtk_label_set_text(GTK_LABEL(header), value);
    }
    if MATCH(key, "Description") {
      gtk_label_set_text(GTK_LABEL(desc), value);
    }
  }
}
#undef MATCH

void pkg_update_box(GListStore *dat, int sel) {
  gpointer item = g_list_model_get_object(G_LIST_MODEL(dat), sel);
  char fpathbuf[PATH_MAX];
  if (item) {
    GtkStringObject *so = GTK_STRING_OBJECT(item);
    const char *s_sel = gtk_string_object_get_string(so);
    snprintf(fpathbuf, sizeof(fpathbuf), "/usr/share/desktop-gremlin-linux/dlpm/recipes/%s/metadata.ini", s_sel);
    trace_log(DEBUG, "sel=%d s_sel=%s path=%s\n", sel, s_sel, fpathbuf);
    FILE *metadata = fopen(fpathbuf, "r");
    if (!metadata)
	    return;
    pkg_ini_parse(metadata, pkg_ini_callback);
    g_object_unref(item);
    fclose(metadata);
  }
}

void pkg_setup_box(GtkWidget *self) {
  thefuckingbox = self;
  header = gtk_label_new("PACKAGE NAME");
  gtk_widget_set_halign(header, GTK_ALIGN_START);
  gtk_widget_add_css_class(header, "title-1");
  owndesc = gtk_label_new("OWNERNAME");
  gtk_widget_set_halign(owndesc, GTK_ALIGN_START);
  gtk_widget_add_css_class(owndesc, "dim-label"); // give it that soft feeling
  install = gtk_button_new_with_label("Install");
  gtk_widget_set_halign(install, GTK_ALIGN_START);
  gtk_widget_set_hexpand(install, FALSE);
  g_signal_connect(install, "clicked", G_CALLBACK(pkg_install_hook), NULL);
  gtk_widget_set_margin_start(install, 20);
  GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  desc = gtk_label_new("DESCRIPTION TEXT BLABLABLA I LOVE UMAUMSUME");
  gtk_widget_set_halign(desc, GTK_ALIGN_START);
  gtk_label_set_wrap(GTK_LABEL(desc), TRUE);
  gtk_box_append(GTK_BOX(thefuckingbox), header);
  gtk_box_append(GTK_BOX(thefuckingbox), owndesc);
  gtk_box_append(GTK_BOX(thefuckingbox), install);
  gtk_box_append(GTK_BOX(thefuckingbox), separator);
  gtk_box_append(GTK_BOX(thefuckingbox), desc);
}

int pkg_init(void) {
  pkg_init_list();
  FILE *wrldf = fopen(DEGRLI_PKG_WORLDPATH, "r");
  if (pkg_populate_list(wrldf) != 0) {
	  trace_log(ERROR, "pkg_populate_list fail");
	  return 1;
  }
  fclose(wrldf);
  return 0;
}
