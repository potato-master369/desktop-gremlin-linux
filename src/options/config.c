#include "config.h"
#include "../defines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <gtk/gtk.h>

void load_conf(config_t *conf) {
  int res;
  char filepath[256];
  if (DEGRLI_LOCALCONFPREFIX[0] != '~') {
    res = snprintf(filepath, sizeof(filepath), "%sconfig.txt", DEGRLI_LOCALCONFPREFIX);
  }
  else {
    const char *home = getenv("HOME");
    if (home != NULL) {
      char *localpref = DEGRLI_LOCALCONFPREFIX;
      res = snprintf(filepath, sizeof(filepath), "%s%sconfig.txt", home, localpref + 1);
    }
  }
  if (res > sizeof(filepath)) {
    g_print(" [ config ] WARN: file path was truncated. We will probably not "
            "be able to read!\n");
  } else if (res < 0) {
    g_print(" [ config ] WARN: snprintf encoding error while generating "
            "filepath!!!\n");
  }
  FILE *configfile = fopen(filepath, "r");
  if (configfile == NULL) {
    g_print(" [ config ] WARN: config.txt is missing. Some items may appear messed up.\n");

  }
}
