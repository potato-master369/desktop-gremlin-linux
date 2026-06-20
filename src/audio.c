/* Audio thing for desktop-gremlin-linux because 
 * the file is getting too big */
#include "audio.h"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <unistd.h>
#include <stdio.h>

ma_result res;
ma_engine engine;
int engine_initialized = 0;

int degrli_init_audio(void) {
	printf("Engine initialised.\n");
	res = ma_engine_init(NULL, &engine);

	if (res != MA_SUCCESS) {
		engine_initialized = 0;
		return 1;
	}
	engine_initialized = 1;
	return 0;
}

int degrli_destroy_audio(void) {
	ma_engine_uninit(&engine);
	engine_initialized = 0;
	return 0;
}

/* Supported sounds:
 * emote1 through emote4
 * grab
 * hover
 * intro
 * outro
 * sleep 
 * mambo
 * NOTE: only returns if it exists or not */
int degrli_get_sound_assetpack(const char *assetpack, const char *sound) {
	char filename[256];

	/* build path safely */
	 snprintf(filename, sizeof(filename), "/usr/share/desktop-gremlin-linux/assets/%s/Sounds/%s.wav", assetpack, sound);
	return access(filename, F_OK) == 0 ? 0 : -1;
}

/* the one for actually playing the sound */
int degrli_play_sound_assetpack(const char *assetpack, const char *sound) {
	printf("Playing sound %s from assetpack %s\n", sound, assetpack);

	/* ensure engine initialized */
	if (!engine_initialized) {
		if (degrli_init_audio() != 0) {
			return -3; /* failed to init audio */
		}
	}

	if (degrli_get_sound_assetpack(assetpack, sound) == -1) {
		return -1;
	}

	char filename[256];
	 snprintf(filename, sizeof(filename), "/usr/share/desktop-gremlin-linux/assets/%s/Sounds/%s.wav", assetpack, sound);

	res = ma_engine_play_sound(&engine, filename, NULL);
	if (res != MA_SUCCESS) {
		return -2;
	}

	return 0;
}
