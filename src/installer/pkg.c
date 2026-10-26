// pkg.c
// ---------------------------
// Handle download manager

#include "pkg.h"
#include "../trace.h"
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

pkg_list_t pkg_entries;

static void pkg_init_list(void) {
  pkg_entries.len = 0;
  pkg_entries.entries = NULL;
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
