#ifndef DEGRLI_CONFIG_H
#define DEGRLI_CONFIG_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {
	// TYPE  // NAME                // COMMENT
	char     start_char[64];
	bool     enable_keyboard;
	bool     randomize_spawn;
	int32_t  spawn_distance;
        float    volume_level;

	int32_t  sprite_framerate;
        int32_t  sprite_speed;
	int16_t  follow_radius;
	
	bool     start_bottom;           // MISSPELL: START_BUTTOM
	bool     enable_gravity;
	int32_t  sleep_time;             // in s
	int16_t  gravity_strength;

	bool     allow_random_actions;
	int32_t  max_interval;
	int32_t  min_interval;
	int32_t  random_move_distance;
	int32_t  walk_distance;

	bool     allow_col_hotspot;
	bool     disable_hotspots;
	bool     enable_min_resize;
	bool     force_centre;           // we aint american Kirt
	bool     enable_manual_resize;
	// Cache not needed because mmap exists
	
	float    current_acceleration;
	float    follow_acceleration;
	int32_t  max_acceleration;      // Integer limit for accel?
	char     companion_char[64];
	float    companion_scale;
	int32_t  companion_follow;
	char     food_spawn[64];
	bool     straight_line;
} degrli_conf_t;

void degrli_init_readconf(void);
#endif
