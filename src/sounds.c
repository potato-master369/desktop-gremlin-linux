#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <unistd.h>

#include "sounds.h"
// degrli
#include "config.h"
#include "defines.h"
ma_result res;
ma_engine engine;
int engine_initialized = 0;
degrli_conf_t *local_config_sounds = NULL;
ma_sound cur_sound;
// cache - improve performance by not requesting
// filepath every time.
struct {
  char emote1[256];
  char emote2[256];
  char emote3[256];
  char emote4[256];
  char   grab[256];
  char  hover[256];
  char  intro[256];
  char  mambo[256];
  char    run[256];
  char    eat[256];
  char  outro[256];
  char  sleep[256];
  char   food[256];
} sounds_cache_paths;

struct {
  char grab[256];
  char hover[256];
  char intro[256];
} sounds_cache_paths_comp;

// Supported sounds:
//  * emote1-emote4
//  * grab
//  * hover
//  * intro
//
// Call this to initialise audio subsystem.
// Do note that config.h must be init-ed first.
int degrli_init_audio(void) {
  printf(" [ sounds ] Engine initialised.\n");
  res = ma_engine_init(NULL, &engine);

  if (res != MA_SUCCESS) {
    engine_initialized = 0;
    return 1;
  }
  engine_initialized = 1;
  local_config_sounds = degrli_request_localconf();
  ma_engine_set_volume(&engine, local_config_sounds->volume_level);
  
  if (local_config_sounds == NULL) {
    return 2;
  }
  // PREPARE CACHE!
  char filepath[256];
  snprintf(sounds_cache_paths.emote1, sizeof(sounds_cache_paths.emote1), "%sSounds/%s/emote1.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.emote2, sizeof(sounds_cache_paths.emote2), "%sSounds/%s/emote2.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.emote3, sizeof(sounds_cache_paths.emote3), "%sSounds/%s/emote3.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.emote4, sizeof(sounds_cache_paths.emote4), "%sSounds/%s/emote4.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.grab, sizeof(sounds_cache_paths.grab), "%sSounds/%s/grab.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.hover, sizeof(sounds_cache_paths.hover), "%sSounds/%s/hover.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.intro, sizeof(sounds_cache_paths.intro), "%sSounds/%s/intro.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.mambo, sizeof(sounds_cache_paths.mambo), "%sSounds/%s/mambo.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  // I was copying from Tamamo, but turns out that there are additional sounds. Assumption stands
  // that Cafe is full?
  snprintf(sounds_cache_paths.eat, sizeof(sounds_cache_paths.eat), "%sSounds/%s/eat.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.run, sizeof(sounds_cache_paths.run), "%sSounds/%s/run.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.outro, sizeof(sounds_cache_paths.outro), "%sSounds/%s/outro.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.run, sizeof(sounds_cache_paths.food), "%sSounds/%s/food.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  snprintf(sounds_cache_paths.sleep, sizeof(sounds_cache_paths.sleep), "%sSounds/%s/sleep.wav", DEGRLI_ASSET_DIR, local_config_sounds->start_char);
  // Companion cache
  snprintf(sounds_cache_paths_comp.grab, sizeof(sounds_cache_paths_comp.grab), "%sSounds/%s/grab.wav", DEGRLI_ASSET_DIR, local_config_sounds->companion_char);
  snprintf(sounds_cache_paths_comp.hover, sizeof(sounds_cache_paths_comp.hover), "%sSounds/%s/hover.wav", DEGRLI_ASSET_DIR, local_config_sounds->companion_char);
  snprintf(sounds_cache_paths_comp.intro, sizeof(sounds_cache_paths_comp.intro), "%sSounds/%s/intro.wav", DEGRLI_ASSET_DIR, local_config_sounds->companion_char);
  return 0;
}

// should be called on cleanup
int degrli_destroy_audio(void) {
  printf(" [ sounds ] Cleaning up...\n");
  ma_engine_uninit(&engine);
  engine_initialized = 0;
  return 0;
}

// helper for sound
bool acts = false;
ma_result degrli_play_exec(ma_engine *pEngine, const char *filepath) {
  if (acts) {
    ma_sound_stop(&cur_sound);
    ma_sound_uninit(&cur_sound);
    acts = false;
  }

  ma_result res = ma_sound_init_from_file(pEngine, filepath, 0, NULL, NULL, &cur_sound);
  if (res == MA_SUCCESS) {
    ma_sound_start(&cur_sound);
    acts = true;
  }
  return MA_SUCCESS;
}

// function for sound! Osu!
// メルト is a great song lowk listen to it
void degrli_play_sound(const char *sound) {
  printf(" [ sounds ] Playing sound: %s\n", sound);
  #define MATCH(a) (strncmp(sound, a, 8) == 0)
  if (MATCH("emote1")) {
    if (access(sounds_cache_paths.emote1, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.emote1);    
    }
  }
  if (MATCH("emote2")) {
    if (access(sounds_cache_paths.emote2, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.emote2);    
    }
  }
  if (MATCH("emote3")) {
    if (access(sounds_cache_paths.emote3, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.emote3);    
    }
  }
  if (MATCH("emote4")) {
    if (access(sounds_cache_paths.emote4, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.emote4);    
    }
  }
  if (MATCH("grab")) {
    if (access(sounds_cache_paths.grab, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.grab);    
    }
  }
  if (MATCH("hover")) {
    if (access(sounds_cache_paths.hover, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.hover);    
    }
  }
  if (MATCH("intro")) {
    if (access(sounds_cache_paths.intro, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.intro);    
    }
  }
  if (MATCH("mambo")) {
    if (access(sounds_cache_paths.mambo, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.mambo);    
    }
  }
  if (MATCH("run")) {
    if (access(sounds_cache_paths.run, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.run);
    }
  }
  // Same here, additional sounds
  if (MATCH("eat")) {
    if (access(sounds_cache_paths.eat, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.eat);
    }
  }
  if (MATCH("outro")) {
    if (access(sounds_cache_paths.outro, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.outro);
    }
  }
  if (MATCH("sleep")) {
    if (access(sounds_cache_paths.sleep, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.sleep);
    }
  }
  if (MATCH("run")) {
    if (access(sounds_cache_paths.run, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.run);
    }
  }
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  printf(" [ sounds ] MiniAudio result: %d\n", (int)res);
#endif
}

// Function for companion sounds
void degrli_play_sound_comp(const char *sound) {
  printf(" [ sounds ] Playing sound (companion): %s\n", sound);
  #define MATCH(a) (strncmp(sound, a, 8) == 0)
  if (MATCH("grab")) {
    if (access(sounds_cache_paths_comp.grab, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths_comp.grab);    
    }
  }
  if (MATCH("hover")) {
    if (access(sounds_cache_paths_comp.hover, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths_comp.hover);    
    }
  }
  if (MATCH("intro")) {
    if (access(sounds_cache_paths_comp.intro, F_OK) == 0) {
      res = degrli_play_exec(&engine, sounds_cache_paths.intro);    
    }
  }
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  printf(" [ sounds ] MiniAudio result: %d\n", (int)res);
#endif

}
