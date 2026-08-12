// config.c: stuff to deal with the Desktop_Gremlin CONFIG.TXT format
// Ensures full compat with Desktop_Gremlin.
#include "config.h"
#include "defines.h"
#include <ctype.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
// variables

// declare defaults
degrli_conf_t local_config_default = {
	.start_char          = "Cafe",
	.enable_keyboard       = true,
	.randomize_spawn      = false,
	.spawn_distance           = 0,
	.volume_level           = 0.5,
	.sprite_framerate        = 60,
	.sprite_speed            = 10,
	.follow_radius          = 300,
	.start_bottom         = false,
	.enable_gravity       = false,
	.sleep_time             = 500,
	.gravity_strength        = 20,
	.allow_random_actions  = true,
	.max_interval            = 40,
	.min_interval            = 30,
	.random_move_distance   = 120,
	.walk_distance          = 200,
	.allow_col_hotspot    = false,
	.disable_hotspots     = false,
	.enable_min_resize     = true,
	.force_centre         = false,
	.enable_manual_resize  = true,
	.current_acceleration   = 0.3,
	.follow_acceleration    = 0.2,
	.max_acceleration       = 200,
	.companion_char = "TachyonComp",
	.companion_scale        = 0.5,
	.companion_follow        = 90,
	.food_spawn     = "food1.png",
	.straight_line        = false
};

degrli_conf_t *local_config = &local_config_default;

void degrli_init_readconf(void) {
  int res;
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [ config ] Started config init...\n");
#endif
  g_print(" [ config ] Looking for file in %sconfig.txt\n",
          DEGRLI_LOCALCONFPREFIX);
  char filepath[256];
  // Looks for PATH/config.txt. Assume that the last letter is a /
  if (DEGRLI_LOCALCONFPREFIX[0] != '~') {
    res = snprintf(filepath, sizeof(filepath), "%sconfig.txt",
                   DEGRLI_LOCALCONFPREFIX);
  } else {
    const char *home = getenv("HOME");
    if (home != NULL) {
      char *localpref = DEGRLI_LOCALCONFPREFIX;
      snprintf(filepath, sizeof(filepath), "%s%sconfig.txt", home,
               localpref + 1);
    }
  }
  if (res > sizeof(filepath)) {
    g_print(" [ config ] WARN: file path was truncated. We will probably not "
            "be able to read!\n");
  } else if (res < 0) {
    g_print(" [ config ] WARN: snprintf encoding error while generating "
            "filepath!!!\n");
  }
  FILE *conf = fopen(filepath, "r");
  if (conf == NULL) {
    g_print(" [ config ] WARN: could not read filepath. Does %sconfig.txt "
            "exist? We will fall back to defaults.\n",
            DEGRLI_LOCALCONFPREFIX);
  } else {
    // PARSE
    char *buf = (char *)malloc(256 * sizeof(char));
    bool isbom = false;
    while (fgets(buf, 256, conf) != NULL) {

      // buf now contains our line.
      buf[strcspn(buf, "\r\n")] = '\0';

      // BOM check
      if ((unsigned char)buf[0] == 0xEF && (unsigned char)buf[1] == 0xBB &&
          (unsigned char)buf[2] == 0xBF) {
        buf += 3;
        isbom = true;
      }

      // strip trailing spaces
      int len = strlen(buf);
      while (len > 0 && isspace((int)buf[len - 1])) {
        buf[len - 1] = '\0';
        len--;
      }
      if (buf[0] == '/' && buf[1] == '/') {
        // comment
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
        g_print(" [ config ] Comment at line: %s\n", buf);
#endif
        continue;
      }
      if (strlen(buf) == 0) {
        continue;
      }

      int offset = 0;
      char key[127], value[127];
      sscanf(buf, " %49[^ =] = %n", key, &offset);
      strncpy(value, buf + offset, sizeof(value) - 1);
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
      g_print(" [ config ] Key: %s\n [ config ] Value: %s\n", key, value);
#endif
      // ADD to config
#define MATCH(a) (strcmp(key, a) == 0)
#define COPYKEY(a)                                                             \
  do {                                                                         \
    strncpy(local_config->a, value, sizeof(local_config->a) - 1);              \
    local_config->a[sizeof(local_config->a) - 1] = '\0';                       \
  } while (0)
#define COPYKEYBOOL(a)                                                         \
  do {                                                                         \
    local_config->a =                                                          \
        (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0);           \
  } while (0)

#define COPYKEYFLOAT(a)                                                        \
  do {                                                                         \
    local_config->a = (float)atof(value);                                      \
  } while (0)

#define COPYKEYINT(a)                                                          \
  do {                                                                         \
    local_config->a = atoi(value);                                             \
  } while (0)
      if (MATCH("START_CHAR"))
        COPYKEY(start_char);
      if (MATCH ("ENABLE_KEYBOARD"))
	COPYKEYBOOL(enable_keyboard);
      if (MATCH ("RANDOMIZE_SPAWN"))
	      COPYKEYBOOL(randomize_spawn);
      if (MATCH ("SPAWN_DISTANCE"))
	      COPYKEYINT(spawn_distance);
      if (MATCH("VOLUME_LEVEL"))
	      COPYKEYFLOAT(volume_level);
      if (MATCH("SPRITE_FRAMERATE"))
	      COPYKEYINT(sprite_framerate);
      if (MATCH("FOLLOW_RADIUS"))
	      COPYKEYINT(follow_radius);
      if (MATCH("START_BUTTOM") || MATCH("START_BOTTOM"))
	      COPYKEYBOOL(start_bottom);
      if (MATCH("ENABLE_GRAVITY"))
	      COPYKEYBOOL(enable_gravity);
      if (MATCH("SLEEP_TIME"))
	      COPYKEYINT(sleep_time);
      if (MATCH("GRAVITY_STRENGTH"))
	      COPYKEYINT(gravity_strength);
      if (MATCH("ALLOW_RANDOM_ACTIONS"))
	      COPYKEYBOOL(allow_random_actions);
      if (MATCH("MAX_INTERVAL"))
	      COPYKEYINT(max_interval);
      if (MATCH("MIN_INTERVAL"))
	      COPYKEYINT(min_interval);
      if (MATCH("RANDOM_MOVE_DISTANCE"))
              COPYKEYINT(random_move_distance);
      if (MATCH("WALK_DISTANCE"))
	      COPYKEYINT(walk_distance);
      if (MATCH("ALLOW_COLOR_HOTSPOT"))
	      COPYKEYBOOL(allow_col_hotspot);
      if (MATCH("DISABLE_HOTSPOTS"))
	      COPYKEYBOOL(disable_hotspots);
      if (MATCH("ENABLE_MIN_RESIZE"))
	      COPYKEYBOOL(enable_min_resize);
      if (MATCH("FORCE_CENTER"))
	      COPYKEYBOOL(force_centre);
      if (MATCH("ENABLE_MANUAL_RESIZE"))
	      COPYKEYBOOL(enable_manual_resize);
      if (MATCH("CURRENT_ACCELERATION"))
	      COPYKEYFLOAT(current_acceleration);
      if (MATCH("FOLLOW_ACCELERATION"))
	      COPYKEYFLOAT(follow_acceleration);
      if (MATCH("COMPANION_CHAR"))
	      COPYKEY(companion_char);
      if (MATCH("COMPANION_SCALE"))
	      COPYKEYFLOAT(companion_scale);
      if (MATCH("COMPANION_FOLLOW"))
	      COPYKEYINT(companion_follow);
      if (MATCH("FOOD_SPAWN"))
	      COPYKEY(food_spawn);
      if (MATCH("STRAIGHT_LINE"))
	      COPYKEYBOOL(straight_line);
    }
    if (isbom)
      buf -= 3;
    free(buf);
  }

  fclose(conf);
}
