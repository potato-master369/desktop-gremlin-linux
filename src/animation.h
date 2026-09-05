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
#define ANIM_STATE_INTRO 6
#define ANIM_STATE_CLICK 7
#define ANIM_STATE_OUTRO 8
#define ANIM_STATE_HOVER 9
#define ANIM_STATE_RUN_UP 10
#define ANIM_STATE_RUN_DOWN 11
#define ANIM_STATE_RUN_LEFT 12
#define ANIM_STATE_RUN_RIGHT 13
#define ANIM_STATE_UP_RIGHT 14
#define ANIM_STATE_UP_LEFT 15
#define ANIM_STATE_DOWN_RIGHT 16
#define ANIM_STATE_DOWN_LEFT 17
#define ANIM_STATE_SLEEP 18

#ifndef DEGRLI_NO_ANIM_DEMO
void play_emote1(GtkWidget *a);
#endif

// Main loop
void anim_start_loop(GtkWidget *a);

// triggers
void anim_trigger_drag_start(void);
void anim_trigger_drag_end(void);
void anim_trigger_emote_1(void);
void anim_trigger_emote_2(void);
void anim_trigger_emote_3(void);
void anim_trigger_emote_4(void);
void anim_trigger_rclick(void);
void anim_trigger_hover_start(void);
void anim_trigger_hover_end(void);
void anim_trigger_quit(void (*a)(void));
void anim_trigger_run_up(void);
void anim_trigger_run_down(void);
void anim_trigger_run_left(void);
void anim_trigger_run_right(void);
void anim_trigger_up_left(void);
void anim_trigger_up_right(void);
void anim_trigger_down_left(void);
void anim_trigger_down_right(void);
void anim_trigger_sleep(void);

// Request state
int anim_request_state(void);
#endif
