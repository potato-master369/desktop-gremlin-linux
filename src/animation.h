#ifndef ANIMATION_H
#define ANIMATION_H
#include <gtk/gtk.h>
void animation_init(void);
void animation_cleanup(void);
// Defines for state
#define ANIM_STATE_IDLE 0
#define ANIM_STATE_DRAG 1
#define ANIM_STATE_EMOTE1 2
#define ANIM_STATE_EMOTE2 3
#define ANIM_STATE_EMOTE3 4
#define ANIM_STATE_EMOTE4 5
#ifndef DEGRLI_NO_ANIM_DEMO
void play_emote1(GtkWidget *a);
#endif

// Main loop
void anim_start_loop(GtkWidget *a);
#endif
