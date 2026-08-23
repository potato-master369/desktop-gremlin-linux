#include "animation.h"
#include "defines.h"
#include "config.h"
#include "asset.h"
#include "gtk/gtk.h"
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
  asset_conf_animation = asset_request_conf(); // again, for assets.
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
  g_print(" [  anim  ] WARN: Demo code. Please remove me. Define #define DEGRLI_NO_ANIM_DEMO\n");
    if (local_conf_animation->sprite_framerate <= 0) return;

    EmoteAnimationData *data = g_new0(EmoteAnimationData, 1);
    data->image = a;
    data->current_frame = 0;
    data->max_frames = asset_conf_animation->emote1;
    data->lru_id = DEGRLI_LRU_EMOTE_1;

    guint interval_ms = 1000 / local_conf_animation->sprite_framerate;
    g_timeout_add(interval_ms, on_emote_frame_tick, data);
}
#endif

typedef struct {
  GtkWidget *img;
  int state;
  int cur;
  int maxf;
  int lru_id;
} animation_data_t;

static gboolean animation_tick(gpointer user_data) {
  animation_data_t *data = (animation_data_t *)user_data;

  // Do something
  
  return G_SOURCE_CONTINUE; // we will most LIKELY never quit the loop.
}

void anim_start_loop(GtkWidget *a) {
  g_print(" [  anim  ] Starting main loop...\n");
  if (local_conf_animation->sprite_framerate <= 0) {
    g_print(" [  anim  ] ERROR! Invalid framerate (<=0). Open the Options, and set this in Sprite Settings > FrameRate\n[  anim  ] Quitting main loop...\n");
    return;
  }

  animation_data_t *data = g_new0(animation_data_t, 1);
  data->img = a;
  data->cur = 0;
  data->state = 0;
  data->maxf = asset_conf_animation->idle;
  data->lru_id = DEGRLI_LRU_IDLE;

  guint interval_ms = 1000 / local_conf_animation->sprite_framerate;
  g_timeout_add(interval_ms, animation_tick, data);
}
