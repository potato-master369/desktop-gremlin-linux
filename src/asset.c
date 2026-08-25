#include "defines.h"
#include <gdk/gdk.h>
#include <glib.h>
#include <graphene.h>
#include <gtk/gtk.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asset.h"
#include "config.h"
// Change this based on memory usage
degrli_conf_t *localconf_asset;
asset_conf_t local_asset_conf = {
  .runup = 18, .rundown = 18, .runleft = 18, .runright = 18,
  .upleft = 18, .upright = 18, .downleft = 18, .downright = 18,
  .emote1 = 41, .emote2 = 35, .emote3 = 15, .emote4 = 42,
  .walkdown = 30, .walkleft = 30, .walkright = 30, .walkup = 30,
  .grab = 50, .hover = 89, .idle = 60, .intro = 100, .click = 111,
  .outro = 140, .pat = 0, .runidle = 60, .sleep = 50,
  .width = 325, .height = 325, .column = 10, .scale = 0.9f
};

typedef struct {
  int16_t id;
  GdkTexture *tex;
} asset_lru_t;
asset_lru_t asset_texture_lru[DEGRLI_LRU_SIZE];

void asset_lru_del_first(void) {
  // Make sure we don't attempt to free nothing!
  if (asset_texture_lru[0].tex != NULL) {
    g_object_unref(asset_texture_lru[0].tex);
  }
  for (int i = 0; i < DEGRLI_LRU_SIZE - 1; ++i) {
    asset_texture_lru[i] = asset_texture_lru[i+1];
  }
  // prevent stupid access
  asset_texture_lru[DEGRLI_LRU_SIZE - 1].id = DEGRLI_LRU_NULL;
  asset_texture_lru[DEGRLI_LRU_SIZE - 1].tex = NULL;
}

// Cleanup process - not NECESSARY, since GTK unrefs all assets with our app,
// but good practice
void asset_cleanup(void) {
  #if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
  g_print(" [ asset  ] Cleaning up...\n");
  #endif
  for (int i = 0; i < DEGRLI_LRU_SIZE; ++i) {
    if (asset_texture_lru[i].tex != NULL) {
      // Free it
      g_object_unref(asset_texture_lru[i].tex);
      asset_texture_lru[i].tex = NULL;
      #if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
      g_print(" [ asset  ] Cleaned up: i = %d\n", i);
      #endif
    }
  }
}

GdkTexture *asset_lru_load(int16_t id) {
  if (id == DEGRLI_LRU_NULL) {
    return NULL;
  }
  char filename[256];
  switch (id) {
    case DEGRLI_LRU_EMOTE_1:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Emotes/emote1.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_2:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Emotes/emote2.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_3:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Emotes/emote3.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_EMOTE_4:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Emotes/emote4.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_IDLE:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Actions/idle.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_INTRO:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Actions/intro.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_GRAB:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Actions/grab.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_CLICK:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Actions/click.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
      break;
    case DEGRLI_LRU_OUTRO:
      snprintf(filename, 256, "%sSpriteSheet/Gremlins/%s/Actions/outro.png", DEGRLI_ASSET_DIR, localconf_asset->start_char);
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

  char filepath[512];
  int path_length = snprintf(filepath, sizeof(filepath),
                             "%sSpriteSheet/Gremlins/%s/config.txt",
                             DEGRLI_ASSET_DIR, localconf_asset->start_char);
  if (path_length < 0 || (size_t)path_length >= sizeof(filepath)) {
    g_print(" [ asset  ] WARN: asset config path is too long.\n");
    return;
  }

  FILE *conf = fopen(filepath, "r");
  if (conf == NULL) {
    g_print(" [ asset  ] WARN: could not read %s. Using defaults.\n", filepath);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), conf) != NULL) {
    line[strcspn(line, "\r\n")] = '\0';
    char *start = line;
    while (isspace((unsigned char)*start))
      ++start;
    if ((unsigned char)start[0] == 0xEF && (unsigned char)start[1] == 0xBB &&
        (unsigned char)start[2] == 0xBF)
      start += 3;
    if (*start == '/' && start[1] == '/') {
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
      g_print(" [ asset  ] Comment at line: %s\n", start);
#endif
      continue;
    }

    char *separator = strchr(start, '=');
    if (separator == NULL)
      continue;
    *separator = '\0';
    char *key = start;
    char *value = separator + 1;
    char *end = key + strlen(key);
    while (end > key && isspace((unsigned char)end[-1]))
      *--end = '\0';
    while (isspace((unsigned char)*value))
      ++value;
    end = value + strlen(value);
    while (end > value && isspace((unsigned char)end[-1]))
      *--end = '\0';
#if (DEGRLI_RELEASE_STATE) == (DEGRLI_DEBUG)
    g_print(" [ asset  ] Key: %s\n [ asset  ] Value: %s\n", key, value);
#endif

    errno = 0;
    char *value_end;
    long parsed = strtol(value, &value_end, 10);
    while (isspace((unsigned char)*value_end))
      ++value_end;
    if (errno != 0 || value_end == value || *value_end != '\0')
      continue;

#define ASSET_INT(key_name, field) \
    if (strcmp(key, key_name) == 0) local_asset_conf.field = (int)parsed
    ASSET_INT("RUNUP", runup);
    ASSET_INT("RUNDOWN", rundown);
    ASSET_INT("RUNLEFT", runleft);
    ASSET_INT("RUNRIGHT", runright);
    ASSET_INT("UPLEFT", upleft);
    ASSET_INT("UPRIGHT", upright);
    ASSET_INT("DOWNLEFT", downleft);
    ASSET_INT("DOWNRIGHT", downright);
    ASSET_INT("EMOTE1", emote1);
    ASSET_INT("EMOTE2", emote2);
    ASSET_INT("EMOTE3", emote3);
    ASSET_INT("EMOTE4", emote4);
    ASSET_INT("WALKDOWN", walkdown);
    ASSET_INT("WALKLEFT", walkleft);
    ASSET_INT("WALKRIGHT", walkright);
    ASSET_INT("WALKUP", walkup);
    ASSET_INT("GRAB", grab);
    ASSET_INT("HOVER", hover);
    ASSET_INT("IDLE", idle);
    ASSET_INT("INTRO", intro);
    ASSET_INT("CLICK", click);
    ASSET_INT("OUTRO", outro);
    ASSET_INT("PAT", pat);
    ASSET_INT("RUNIDLE", runidle);
    ASSET_INT("SLEEP", sleep);
    ASSET_INT("WIDTH", width);
    ASSET_INT("HEIGHT", height);
    ASSET_INT("COLUMN", column);
#undef ASSET_INT

    if (strcmp(key, "SCALE") == 0) {
      errno = 0;
      char *scale_end;
      float scale = strtof(value, &scale_end);
      while (isspace((unsigned char)*scale_end))
        ++scale_end;
      if (errno == 0 && scale_end != value && *scale_end == '\0')
        local_asset_conf.scale = scale;
    }
  }
  fclose(conf);
  for (int i = 0; i < DEGRLI_LRU_SIZE; ++i) {
    asset_texture_lru[i].id = DEGRLI_LRU_NULL;
    asset_texture_lru[i].tex = NULL;
  }
}

asset_conf_t *asset_request_conf(void) {
  return &local_asset_conf;
}

void asset_apply(int id, int fid, GtkWidget *image) {
    asset_lru_t a = asset_texture_lru[asset_lru_req_asset(id)];

    int width = local_asset_conf.width;
    int height = local_asset_conf.height;
    int start_x = (fid % local_asset_conf.column) * width;
    int start_y = (fid / local_asset_conf.column) * height;

    gsize dest_stride = width * 4;
    gsize buffer_size = dest_stride * height;
    guchar *buf = g_malloc(buffer_size);

    GdkTextureDownloader *d = gdk_texture_downloader_new(a.tex);
    gdk_texture_downloader_set_format(d, GDK_MEMORY_R8G8B8A8);

    gsize src_stride = 0;
    GBytes *src_bytes = gdk_texture_downloader_download_bytes(d, &src_stride);
    gdk_texture_downloader_free(d);

    const guchar *src_data = g_bytes_get_data(src_bytes, NULL);

    for (int y = 0; y < height; y++) {
        const guchar *src_row = src_data + ((start_y + y) * src_stride) + (start_x * 4);
        guchar *dest_row = buf + (y * dest_stride);
        memcpy(dest_row, src_row, dest_stride);
    }

    g_bytes_unref(src_bytes);

    // 3. Create cropped memory texture & assign to GtkImage
    GBytes *bytes = g_bytes_new_take(buf, buffer_size);
    GdkTexture *crop = gdk_memory_texture_new(width, height, GDK_MEMORY_R8G8B8A8, bytes, dest_stride);

    gtk_image_set_from_paintable(GTK_IMAGE(image), GDK_PAINTABLE(crop));

    g_bytes_unref(bytes);
    g_object_unref(crop);
}
