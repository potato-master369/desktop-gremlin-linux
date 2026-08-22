#include "animation.h"
#include "defines.h"
#include "config.h"
#include "gtk/gtk.h"
#include <glib.h>
degrli_conf_t *local_conf_animation;

void animation_init(void) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [  anim  ] Initialising animation stuff...\n");
#endif
  local_conf_animation = degrli_request_localconf(); // get local conf
}
