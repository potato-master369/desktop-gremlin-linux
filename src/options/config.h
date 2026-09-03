#ifndef OPTIONS_CONFIG_H
#define OPTIONS_CONFIG_H
#include <stdbool.h>
typedef struct {
  char start_char[64];
  bool language_diff;
  bool enable_keyboard;
  bool allow_error_messages;
  bool show_taskbar;
  bool randomize_spawn;
  bool use_wpfplayer;
  int spawn_distance;
  double volume_level;
  int sprite_framerate;
  int sprite_speed;
  int follow_radius;
  bool start_buttom;
  bool enable_gravity;
  int sleep_time;
  int gravity_strength;
  bool allow_random_actions;
  int max_interval;
  int min_interval;
  int random_move_distance;
  int walk_distance;
  bool allow_color_hotspot;
  bool disable_hotspots;
  bool enable_min_resize;
  bool force_center;
  bool enable_manual_resize;
  bool force_fake_transparent;
  bool allow_cache;
  double current_acceleration;
  double follow_acceleration;
  int max_acceleration;
} config_t;

void load_conf(config_t *ret);
void conf_apply_default(config_t *ret);
void write_conf(config_t conf);
#endif
