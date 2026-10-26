#ifndef DEGRLI_PKG_H
#define DEGRLI_PKG_H

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

#endif
