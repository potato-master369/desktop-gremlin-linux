#include "config.h"
#include "../defines.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <gtk/gtk.h>

static int parse_bool_str(const char *s) {
  if (s == NULL) return 0;
  if (strcasecmp(s, "true") == 0) return 1;
  if (strcmp(s, "1") == 0) return 1;
  return 0;
}

void write_conf(config_t conf) { 
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
  if (res >= (int)sizeof(filepath)) {
    g_print(" [ config ] WARN: file path was truncated. We will probably not "
            "be able to read!\n");
  } else if (res < 0) {
    g_print(" [ config ] WARN: snprintf encoding error while generating "
            "filepath!!!\n");
  }
  FILE *configfile = fopen(filepath, "w");
  if (configfile == NULL) {
    g_print(" [ config ] WARN: cannot open config.txt Permission denied, or something idk\n");
    return;
  }
  g_print(" [ config] Writing config to %s\n", filepath);

  // General Settings
  fprintf(configfile, "//General Settings\n");
  fprintf(configfile, "START_CHAR = %s\n", conf.start_char);
  fprintf(configfile, "LANGUAGE_DIFF = %s\n", conf.language_diff ? "true" : "false");
  fprintf(configfile, "ENABLE_KEYBOARD = %s\n", conf.enable_keyboard ? "true" : "false");
  fprintf(configfile, "ALLOW_ERROR_MESSAGES = %s\n", conf.allow_error_messages ? "true" : "false");
  fprintf(configfile, "SHOW_TASKBAR = %s\n", conf.show_taskbar ? "true" : "false");
  fprintf(configfile, "RANDOMIZE_SPAWN = %s\n", conf.randomize_spawn ? "true" : "false");
  fprintf(configfile, "USE_WPFPLAYER = %s\n", conf.use_wpfplayer ? "true" : "false");
  fprintf(configfile, "SPAWN_DISTANCE = %d\n", conf.spawn_distance);
  fprintf(configfile, "VOLUME_LEVEL = %g\n\n", conf.volume_level);

  // Sprite Settings
  fprintf(configfile, "//Sprite Settings\n");
  fprintf(configfile, "SPRITE_FRAMERATE = %d\n", conf.sprite_framerate);
  fprintf(configfile, "SPRITE_SPEED = %d\n", conf.sprite_speed);
  fprintf(configfile, "FOLLOW_RADIUS = %d\n\n", conf.follow_radius);

  // Movement Settings
  fprintf(configfile, "//Movement Settings\n");
  fprintf(configfile, "START_BUTTOM = %s\n", conf.start_buttom ? "true" : "false");
  fprintf(configfile, "ENABLE_GRAVITY = %s\n", conf.enable_gravity ? "true" : "false");
  fprintf(configfile, "SLEEP_TIME = %d\n", conf.sleep_time);
  fprintf(configfile, "GRAVITY_STRENGTH = %d\n\n", conf.gravity_strength);

  // RANDOM ACTIONS
  fprintf(configfile, "//RANDOM ACTIONS\n");
  fprintf(configfile, "ALLOW_RANDOM_ACTIONS = %s\n", conf.allow_random_actions ? "true" : "false");
  fprintf(configfile, "MAX_INTERVAL = %d\n", conf.max_interval);
  fprintf(configfile, "MIN_INTERVAL = %d\n", conf.min_interval);
  fprintf(configfile, "RANDOM_MOVE_DISTANCE = %d\n", conf.random_move_distance);
  fprintf(configfile, "WALK_DISTANCE = %d\n\n", conf.walk_distance);

  // Configuration Settings
  fprintf(configfile, "//Configuration Settings\n");
  fprintf(configfile, "ALLOW_COLOR_HOTSPOT = %s\n", conf.allow_color_hotspot ? "true" : "false");
  fprintf(configfile, "DISABLE_HOTSPOTS = %s\n", conf.disable_hotspots ? "true" : "false");
  fprintf(configfile, "ENABLE_MIN_RESIZE = %s\n", conf.enable_min_resize ? "true" : "false");
  fprintf(configfile, "FORCE_CENTER = %s\n", conf.force_center ? "true" : "false");
  fprintf(configfile, "ENABLE_MANUAL_RESIZE = %s\n", conf.enable_manual_resize ? "true" : "false");
  fprintf(configfile, "FORCE_FAKE_TRANSPARENT = %s\n", conf.force_fake_transparent ? "true" : "false");
  fprintf(configfile, "ALLOW_CACHE = %s\n\n", conf.allow_cache ? "true" : "false");

  // Food and Item Settings
  fprintf(configfile, "//Food and Item Settings\n");
  fprintf(configfile, "CURRENT_ACCELERATION = %g\n", conf.current_acceleration);
  fprintf(configfile, "FOLLOW_ACCELERATION = %g\n", conf.follow_acceleration);
  fprintf(configfile, "MAX_ACCELERATION = %d\n", conf.max_acceleration);

  fclose(configfile);
}

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

  } else {
    char *buf = (char*)malloc(256 * sizeof(char)); // allocs 256 chars of space (1-line max width)
    if (buf == NULL) {
      g_print(" [ config ] ERROR: out of memory reading config\n");
      fclose(configfile);
      return;
    }
    while (fgets(buf, 256, configfile) != NULL) {
      char *line = buf;
      line[strcspn(line, "\r\n")] = '\0';

      // check if BOM (0xEFBBBF)
      if ((unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
        line += 3;
      }

      // strip trailing spaces
      int len = strlen(line);
      while (len > 0 && isspace((int)line[len - 1])) {
        line[len - 1] = '\0';
        len--;
      }
      if (line[0] == '/' && line[1] == '/') {
        // comment
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
        g_print(" [ config ] Comment at line: %s\n", line);
#endif
        continue;
      }
      if (strlen(line) == 0) {
        continue;
      }

      int offset = 0;
      char key[127], key_lwr[127], value[127];
      memset(key, 0, sizeof(key));
      memset(key_lwr, 0, sizeof(key_lwr));
      memset(value, 0, sizeof(value));
      sscanf(line, " %126[^ =] = %n", key, &offset);
      const char *vstart = line + offset;
      while (*vstart && isspace((unsigned char)*vstart)) vstart++;
      strncpy(value, vstart, sizeof(value) - 1);
      for (size_t i = 0; i < sizeof(key) && key[i]; ++i) {
        key_lwr[i] = (char)tolower((unsigned char)key[i]);
      }
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [ config ] Key: %s\n [ config ] Value: %s\n", key_lwr, value);
#endif
  // ADD to config (compare lowercase keys)
#define MATCH(a) (strcmp(key_lwr, a) == 0)
  if (MATCH("start_char")) {
        strncpy(conf->start_char, value, sizeof(conf->start_char) - 1);
        conf->start_char[sizeof(conf->start_char) - 1] = '\0';
      } else if (MATCH("language_diff")) {
        conf->language_diff = parse_bool_str(value);
      } else if (MATCH("enable_keyboard")) {
        conf->enable_keyboard = parse_bool_str(value);
      } else if (MATCH("allow_error_messages")) {
        conf->allow_error_messages = parse_bool_str(value);
      } else if (MATCH("show_taskbar")) {
        conf->show_taskbar = parse_bool_str(value);
      } else if (MATCH("randomize_spawn")) {
        conf->randomize_spawn = parse_bool_str(value);
      } else if (MATCH("use_wpfplayer")) {
        conf->use_wpfplayer = parse_bool_str(value);
      } else if (MATCH("spawn_distance")) {
        conf->spawn_distance = atoi(value);
      } else if (MATCH("volume_level")) {
        conf->volume_level = atof(value);
      } else if (MATCH("sprite_framerate")) {
        conf->sprite_framerate = atoi(value);
      } else if (MATCH("sprite_speed")) {
        conf->sprite_speed = atoi(value);
      } else if (MATCH("follow_radius")) {
        conf->follow_radius = atoi(value);
      } else if (MATCH("start_buttom")) {
        conf->start_buttom = parse_bool_str(value);
      } else if (MATCH("enable_gravity")) {
        conf->enable_gravity = parse_bool_str(value);
      } else if (MATCH("sleep_time")) {
        conf->sleep_time = atoi(value);
      } else if (MATCH("gravity_strength")) {
        conf->gravity_strength = atoi(value);
      } else if (MATCH("allow_random_actions")) {
        conf->allow_random_actions = parse_bool_str(value);
      } else if (MATCH("max_interval")) {
        conf->max_interval = atoi(value);
      } else if (MATCH("min_interval")) {
        conf->min_interval = parse_bool_str(value);
      } else if (MATCH("random_move_distance")) {
        conf->random_move_distance = atoi(value);
      } else if (MATCH("walk_distance")) {
        conf->walk_distance = atoi(value);
      } else if (MATCH("allow_color_hotspot")) {
        conf->allow_color_hotspot = parse_bool_str(value);
      } else if (MATCH("disable_hotspots")) {
        conf->disable_hotspots = parse_bool_str(value);
      } else if (MATCH("enable_min_resize")) {
        conf->enable_min_resize = parse_bool_str(value);
      } else if (MATCH("force_center")) {
        conf->force_center = parse_bool_str(value);
      } else if (MATCH("enable_manual_resize")) {
        conf->enable_manual_resize = parse_bool_str(value);
      } else if (MATCH("force_fake_transparent")) {
        conf->force_fake_transparent = parse_bool_str(value);
      } else if (MATCH("allow_cache")) {
        conf->allow_cache = parse_bool_str(value);
      } else if (MATCH("current_acceleration")) {
        conf->current_acceleration = atof(value);
      } else if (MATCH("follow_acceleration")) {
        conf->follow_acceleration = atof(value);
      } else if (MATCH("max_acceleration")) {
        conf->max_acceleration = atoi(value);
      }
    }
    free(buf);
  }

  if (configfile) fclose(configfile);
}

void conf_apply_default(config_t *conf) {
  // Apply default settings - sourced from Desktop_Gremlin v4.0 (kritzkingvoid/Desktop_Gremlin on GitHub)
  const char a[] = "Cafe";
  strncpy(conf->start_char, a, sizeof(conf->start_char));
  conf->language_diff = true;
  conf->enable_keyboard = true;
  conf->allow_error_messages = true;
  conf->show_taskbar = true;
  conf->randomize_spawn = false;
  conf->use_wpfplayer = false;
  conf->spawn_distance = 150;
  conf->volume_level = 0.5;

  // Sprite settings
  conf->sprite_framerate = 60;
  conf->sprite_speed = 10;
  conf->follow_radius = 150;

  // Movement settings
  conf->start_buttom = true;
  conf->enable_gravity = false;
  conf->sleep_time = 500;
  conf->gravity_strength = 20;

  // Random actions
  conf->allow_random_actions = true;
  conf->max_interval = 30;
  conf->min_interval = 25;
  conf->random_move_distance = 120;
  conf->walk_distance = 200;

  // Config settings
  conf->allow_color_hotspot = false;
  conf->disable_hotspots = false;
  conf->enable_min_resize = true;
  conf->force_center = true;
  conf->enable_manual_resize = true;
  conf->force_fake_transparent = true;
  conf->allow_cache = false;

  // Food and Item Settings
  conf->current_acceleration = 0.3;
  conf->follow_acceleration = 0.2;
  conf->max_acceleration = 200;
}
