#include "defines.h"
#include <gdk/gdk.h>
#include <graphene.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include "asset.h"
#include "config.h"
// Change this based on memory usage
degrli_conf_t *localconf_asset;

typedef struct {
  int16_t id;
  GdkTexture *tex;
} asset_lru_t;
asset_lru_t asset_texture_lru[DEGRLI_LRU_SIZE];

void asset_lru_del_first(void) {
  g_object_unref(asset_texture_lru[0].tex);
  for (int i = 0; i < DEGRLI_LRU_SIZE - 1; ++i) {
    asset_texture_lru[i] = asset_texture_lru[i+1];
  }
  // prevent stupid access
  asset_texture_lru[DEGRLI_LRU_SIZE - 1].id = DEGRLI_LRU_NULL;
  asset_texture_lru[DEGRLI_LRU_SIZE - 1].tex = NULL;
}

GdkTexture *asset_lru_load(int16_t id) {
  if (id == DEGRLI_LRU_NULL) {
    return NULL;
  }
  char filename[256];
  switch (id) {
    case DEGRLI_LRU_EMOTE_1:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/emote1.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_2:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/emote2.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_3:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/emote3.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_4:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/emote1.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    default:
      strncpy(filename, "/dev/null", 256);
  }
  return gdk_texture_new_from_filename(filename, NULL);
}

int asset_lru_req_asset(int16_t id) {
  int found = -1;
  for (int i = DEGRLI_LRU_SIZE - 1; i >= 0; --i) {
    if (asset_texture_lru[i].id == id) {
      found = i;
      break;
    }
  }
  // free LRU train start
  if (found == -1) {
    asset_lru_del_first();
    found = DEGRLI_LRU_NULL - 1; // So now the last slot should be empty
    // Now alloc it
    asset_texture_lru[DEGRLI_LRU_SIZE - 1].id = id;
    asset_texture_lru[DEGRLI_LRU_SIZE - 1].tex = asset_lru_load(id);
    return DEGRLI_LRU_SIZE - 1;
  } else {
    if (found == DEGRLI_LRU_SIZE - 1) {
      return found;
    }
    else {
      asset_lru_t tmp = asset_texture_lru[found];
      for (int i = found; i < DEGRLI_LRU_SIZE - 1; ++i) {
        asset_texture_lru[i] = asset_texture_lru[i + 1];
      }
      asset_texture_lru[DEGRLI_LRU_SIZE - 1] = tmp;
      // It is now at front!
      return DEGRLI_LRU_SIZE - 1;
    }
  }
}

void asset_init(void) {
  localconf_asset = degrli_request_localconf();
}

void asset_apply(id, fid) {
  asset_lru_t a = asset_texture_lru[asset_lru_req_asset(id)];
  GdkPaintable *s = GDK_PAINTABLE(a);
}
