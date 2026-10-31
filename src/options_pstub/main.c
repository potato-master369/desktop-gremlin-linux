// options-pstub is basically there to make
// pkg.c not a massive polkit soup

// run me as root!
#define _POSIX_C_SOURCE 200809L
#include "../defines.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

static int pkg_world_add(const char *pkgname) {
  char **lines = NULL;
  int n_lines = 0;
  _Bool already_installed = FALSE;
  FILE *f = fopen(DEGRLI_PKG_WORLD_PATH, "r");
  if (f != NULL) {
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, f) != -1) {
      line[strcspn(line, "\r\n")] = '\0';
      if (line[0] == '\0')
        continue;
      if (strcmp(line, pkgname) == 0) {
        already_installed = TRUE;
        continue;
      }

      char **tmp = realloc(lines, (n_lines + 1) * sizeof(lines[0]));
      if (tmp == NULL) {
        fprintf(stderr, "pkg: realloc failed while updating world file\n");
        free(line);
        for (int i = 0; i < n_lines; ++i)
          free(lines[i]);
        free(lines);
        fclose(f);
        return -1;
      }
      lines = tmp;
      lines[n_lines++] = strdup(line);
    }
    free(line);
    fclose(f);
  }

  if (already_installed) {
    for (int i = 0; i < n_lines; ++i)
      free(lines[i]);
    free(lines);
    return 0;
  }

  f = fopen(DEGRLI_PKG_WORLD_PATH, "w");
  if (f == NULL) {
    fprintf(stderr,
            "pkg: could not write world file " DEGRLI_PKG_WORLD_PATH ": %s\n",
            strerror(errno));
    for (int i = 0; i < n_lines; ++i)
      free(lines[i]);
    free(lines);
    return -1;
  }

  for (int i = 0; i < n_lines; ++i)
    if (fprintf(f, "%s\n", lines[i]) < 0) {
      fclose(f);
      fprintf(stderr, "pkg: write fail\n");
      return 1;
    }
  if (fprintf(f, "%s\n", pkgname) < 0) {
    fclose(f);
    fprintf(stderr, "pkg: write fail\n");
    return 1;
  }
  fclose(f);

  for (int i = 0; i < n_lines; ++i)
    free(lines[i]);
  free(lines);
  return 0;
}

static int pkg_world_remove(const char *pkgname) {
  printf("astoflo\n");
  FILE *worldfile, *tmp_file;
  char tmppath[] =
      "/usr/share/desktop-gremlin-linux/dlpm/degrli_options_XXXXXX"; // must end
                                                                     // in 6 Xs
  int tfd = mkstemp(tmppath);
  if (tfd == -1) {
    fprintf(stderr, "pkg: could not create tmp file fd: %s\n", strerror(errno));
    return -1;
  }
  worldfile = fopen(DEGRLI_PKG_WORLD_PATH, "r");
  if (worldfile == NULL) {
    close(tfd);
    unlink(tmppath);
    fprintf(stderr,
            "pkg: could not read world file " DEGRLI_PKG_WORLD_PATH ": %s\n",
            strerror(errno));
    return -1;
  }
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  tmp_file = fdopen(tfd, "w+");
  if (tmp_file == NULL) {
    fclose(worldfile);
    fprintf(stderr, "pkg: could not open tmp file: %s\n", strerror(errno));
    return -1;
  }

  while ((read = getline(&line, &len, worldfile)) != -1) {
    line[strcspn(line, "\r\n")] = '\0';
    if (strcmp(line, pkgname) != 0) {
      fputs(line, tmp_file);
      fputs("\n", tmp_file);
    }
  }

  free(line);

  if (ferror(tmp_file)) {
    fprintf(stderr, "pkg: cannot replace file. File has errors.\n");
    fclose(worldfile);
    fclose(tmp_file);
    unlink(tmppath);
    return -1;
  }
  fclose(worldfile);
  if (fclose(tmp_file) != 0) {
    fprintf(stderr, "pkg: fclose fail\n");
    close(tfd);
    unlink(tmppath);
    return -1;
  }
  unlink(tmppath);
  fchmod(fileno(tmp_file), 0644);
  if (rename(tmppath, DEGRLI_PKG_WORLD_PATH) != 0) {
    fprintf(stderr, "pkg: could not replace world file: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 3) {
  asofpihafsohoasfhosaf:
    fprintf(stderr,
            "Invalid usage. This program should not be run by the user.\n");
    return 1;
  }
#define MATCH(a, b) (strcmp(a, b) == 0)
  if MATCH (argv[1], "add") {
    return pkg_world_add(argv[2]);
  }
  if MATCH (argv[1], "remove") {
    return pkg_world_remove(argv[2]);
  }

  goto asofpihafsohoasfhosaf;
}
