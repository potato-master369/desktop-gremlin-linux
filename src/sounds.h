#ifndef DEGRLI_SOUNDS_H
#define DEGRLI_SOUNDS_H
// Sounds are the names of the files, without .wab
// e.g. emote1
int degrli_init_audio(void);
int degrli_destroy_audio(void);
void degrli_play_sound(const char *sound);
void degrli_play_sound_comp(const char *sound);
#endif
