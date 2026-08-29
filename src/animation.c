#include "animation.h"
#include "asset.h"
#include "config.h"
#include "defines.h"
#include "gtk/gtk.h"
#include "sounds.h"
#include <glib.h>
degrli_conf_t *local_conf_animation;
asset_conf_t *asset_conf_animation;

// INIT Dependencies:
//  - CONFIG
//  - ASSET
void animation_init(void) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [  anim  ] Initialising animation stuff...\n");
#endif
  local_conf_animation = degrli_request_localconf(); // get local conf
  asset_conf_animation = asset_request_conf();       // again, for assets.
}
// This block is the demo. Provides a naive example of
// the Asset LRU Engine. It does its job pretty well.
#ifndef DEGRLI_NO_ANIM_DEMO
typedef struct {
  GtkWidget *image;
  int current_frame;
  int max_frames;
  int lru_id;
} EmoteAnimationData;

static gboolean on_emote_frame_tick(gpointer user_data) {
  EmoteAnimationData *data = (EmoteAnimationData *)user_data;

  if (data->current_frame >= data->max_frames) {
    g_free(data);
    return G_SOURCE_REMOVE; // Stop the timer
  }

  asset_apply(data->lru_id, data->current_frame, data->image);
  data->current_frame++;

  return G_SOURCE_CONTINUE; // Run again on next interval
}

void play_emote1(GtkWidget *a) {
  g_print(" [  anim  ] WARN: Demo code. Please remove me. Define #define "
          "DEGRLI_NO_ANIM_DEMO\n");
  if (local_conf_animation->sprite_framerate <= 0)
    return;

  EmoteAnimationData *data = g_new0(EmoteAnimationData, 1);
  data->image = a;
  data->current_frame = 0;
  data->max_frames = asset_conf_animation->emote1;
  data->lru_id = DEGRLI_LRU_EMOTE_1;

  guint interval_ms = 1000 / local_conf_animation->sprite_framerate;
  g_timeout_add(interval_ms, on_emote_frame_tick, data);
}
#endif
typedef void (*cleanup_func)(void);
cleanup_func cleanup;
typedef struct {
  GtkWidget *img;
  int state;
  int cur;
  int maxf;
  int walkticks;
} animation_data_t;

animation_data_t *data;
static gboolean animation_tick(gpointer user_data) {

  // state machine
  switch (data->state) {
  case ANIM_STATE_IDLE:
    data->cur = ++data->cur % asset_conf_animation->idle;
    asset_apply(DEGRLI_LRU_IDLE, data->cur, data->img);
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_EMOTE1:
    data->cur = ++data->cur % asset_conf_animation->emote1;
    asset_apply(DEGRLI_LRU_EMOTE_1, data->cur, data->img);
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_EMOTE2:
    data->cur = ++data->cur % asset_conf_animation->emote3;
    asset_apply(DEGRLI_LRU_EMOTE_3, data->cur, data->img);
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_EMOTE3:
    asset_apply(DEGRLI_LRU_EMOTE_2, data->cur, data->img);
    ++data->cur;
    if (data->cur == asset_conf_animation->emote2) {
      data->cur = 0;
      data->state = 0;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_EMOTE4:
    asset_apply(DEGRLI_LRU_EMOTE_4, data->cur, data->img);
    ++data->cur;
    if (data->cur == asset_conf_animation->emote4) {
      data->cur = 0;
      data->state = 0;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_DRAG:
    data->cur =
        ++data->cur % asset_conf_animation->grab; // drag is called grab by kurt
    asset_apply(DEGRLI_LRU_GRAB, data->cur, data->img);
    return G_SOURCE_CONTINUE;
    break;
  case ANIM_STATE_INTRO:
    asset_apply(DEGRLI_LRU_INTRO, data->cur, data->img);
    ++data->cur;
    if (data->cur == asset_conf_animation->intro) {
      data->cur = 0;
      data->state = 0;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_CLICK:
    asset_apply(DEGRLI_LRU_CLICK, data->cur, data->img);
    ++data->cur;
    if (data->cur == asset_conf_animation->click) {
      data->cur = 0;
      data->state = 0;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_OUTRO:
    asset_apply(DEGRLI_LRU_OUTRO, data->cur, data->img);
    ++data->cur;
    if (data->cur == asset_conf_animation->outro) {
      cleanup();
      return G_SOURCE_REMOVE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_HOVER:
    asset_apply(DEGRLI_LRU_HOVER, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->hover;
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_RUN_UP:
    asset_apply(DEGRLI_LRU_RUN_UP, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->runup;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_RUN_DOWN:
    asset_apply(DEGRLI_LRU_RUN_DOWN, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->rundown;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_RUN_LEFT:
    asset_apply(DEGRLI_LRU_RUN_LEFT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->runleft;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_RUN_RIGHT:
    asset_apply(DEGRLI_LRU_RUN_RIGHT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->runright;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_UP_LEFT:
    asset_apply(DEGRLI_LRU_UP_LEFT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->upleft;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_UP_RIGHT:
    asset_apply(DEGRLI_LRU_UP_RIGHT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->upright;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_DOWN_LEFT:
    asset_apply(DEGRLI_LRU_DOWN_LEFT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->downleft;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  case ANIM_STATE_DOWN_RIGHT:
    asset_apply(DEGRLI_LRU_DOWN_RIGHT, data->cur, data->img);
    data->cur = ++data->cur % asset_conf_animation->downright;
    ++data->walkticks;
    if (data->walkticks == 2) {
      data->walkticks = 0;
      data->cur = 0;
      data->state = ANIM_STATE_IDLE;
    }
    return G_SOURCE_CONTINUE;
  default:
    g_print(" [  anim  ] Warn: invalid state! Resetting to idle...\n");
    data->state = ANIM_STATE_IDLE;
    data->cur = 0;
    break;
  }

  return G_SOURCE_CONTINUE; // we will most LIKELY never quit the loop.
}

void anim_trigger_drag_start(void) {
  degrli_play_sound("grab");
  data->state = ANIM_STATE_DRAG;
  data->cur = 0;
}

void anim_trigger_quit(cleanup_func a) {
  cleanup = a;
  degrli_play_sound("outro");
  data->state = ANIM_STATE_OUTRO;
  data->cur = 0;
}

void anim_trigger_rclick(void) {
  degrli_play_sound("mambo"); // this is the sound played in original
  data->state = ANIM_STATE_CLICK;
  data->cur = 0;
}

void anim_trigger_drag_end(void) {
  data->state = ANIM_STATE_IDLE;
  data->cur = 0;
}
// NOTE:
// Finding as of 26/08/25:
//   Emotes 3 and 2 are swapped.
void anim_trigger_emote_1(void) {
  degrli_play_sound("emote1");
  data->state = ANIM_STATE_EMOTE1;
  data->cur = 0;
}
void anim_trigger_emote_2(void) {
  degrli_play_sound("emote3");
  data->state = ANIM_STATE_EMOTE2;
  data->cur = 0;
}
void anim_trigger_emote_3(void) {
  degrli_play_sound("emote2");
  data->state = ANIM_STATE_EMOTE3;
  data->cur = 0;
}
void anim_trigger_emote_4(void) {
  degrli_play_sound("emote4");
  data->state = ANIM_STATE_EMOTE4;
  data->cur = 0;
}

// Hover triggers
void anim_trigger_hover_start(void) {
  if (data->state == ANIM_STATE_IDLE) {
    degrli_play_sound("hover");
    data->state = ANIM_STATE_HOVER;
    data->cur = 0;
  }
}
void anim_trigger_hover_end(void) {
  if (data->state == ANIM_STATE_HOVER) {
    data->state = ANIM_STATE_IDLE;
    data->cur = 0;
  }
}

// Run triggers
void anim_trigger_run_up(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->runup)
    data->cur = 0;
  data->state = ANIM_STATE_RUN_UP;
}
void anim_trigger_run_down(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->rundown)
    data->cur = 0;
  data->state = ANIM_STATE_RUN_DOWN;
}
void anim_trigger_run_left(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->runleft)
    data->cur = 0;
  data->state = ANIM_STATE_RUN_LEFT;
}
void anim_trigger_run_right(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->runright)
    data->cur = 0;
  data->state = ANIM_STATE_RUN_RIGHT;
}
void anim_trigger_up_left(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->upleft) {
#ifdef DEGRLI_DEBUG_HIGH
    g_print(" [  anim  ] HI: Resetting cur for walk\n");
#endif
    data->cur = 0;
  }
  data->state = ANIM_STATE_UP_LEFT;
}
void anim_trigger_up_right(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->upright) {
#ifdef DEGRLI_DEBUG_HIGH
    g_print(" [  anim  ] HI: Resetting cur for walk\n");
#endif
    data->cur = 0;
  }
  data->state = ANIM_STATE_UP_RIGHT;
}
void anim_trigger_down_left(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->downleft){
#ifdef DEGRLI_DEBUG_HIGH
    g_print(" [  anim  ] HI: Resetting cur for walk\n");
#endif
    data->cur = 0;
  }
  data->state = ANIM_STATE_DOWN_LEFT;
}
void anim_trigger_down_right(void) {
  data->walkticks = 0;
  if (data->cur >= asset_conf_animation->downright){
#ifdef DEGRLI_DEBUG_HIGH
    g_print(" [  anim  ] HI: Resetting cur for walk\n");
#endif
    data->cur = 0;
  }
  data->state = ANIM_STATE_DOWN_RIGHT;
}

void anim_start_loop(GtkWidget *a) {
  data = malloc(sizeof(animation_data_t));
  g_print(" [  anim  ] Starting main loop...\n");
  if (local_conf_animation->sprite_framerate <= 0) {
    g_print(" [  anim  ] ERROR! Invalid framerate (<=0). Open the Options, and "
            "set this in Sprite Settings > FrameRate\n[  anim  ] Quitting main "
            "loop...\n");
    return;
  }

  data->img = a;
  data->cur = 0;
  data->state = ANIM_STATE_INTRO;
  data->maxf = asset_conf_animation->idle;
  data->walkticks = 0;

  guint interval_ms = 1000 / local_conf_animation->sprite_framerate;
  // play intro sound
  degrli_play_sound("intro");
  g_timeout_add(interval_ms, animation_tick, data);
}

// cleanup function
void animation_cleanup(void) {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [  anim  ] Cleaning up animation data...\n");
#endif
  free(data);
}
