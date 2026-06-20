#ifndef DEGRLI_AUDIO_H
#define DEGRLI_AUDIO_H

int degrli_init_audio(void);

int degrli_destroy_audio(void);

int degrli_get_sound_assetpack(const char *assetpack, const char *sound);

int degrli_play_sound_assetpack(const char *assetpack, const char *sound);

#endif
