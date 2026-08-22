#ifndef ASSET_H
#define ASSET_H
#define DEGRLI_LRU_SIZE 5
#define DEGRLI_LRU_NULL 0
#define DEGRLI_LRU_EMOTE_1 1
#define DEGRLI_LRU_EMOTE_2 2
#define DEGRLI_LRU_EMOTE_3 3
#define DEGRLI_LRU_EMOTE_4 4
typedef struct {
  int runup;
  int rundown;
  int runleft;
  int runright;
  int upleft;
  int upright;
  int downleft;
  int downright;
  int emote1;
  int emote2;
  int emote3;
  int emote4;
  int walkdown;
  int walkleft;
  int walkright;
  int walkup;
  int grab;
  int hover;
  int idle;
  int intro;
  int click;
  int outro;
  int pat;
  int runidle;
  int sleep;
  int width;
  int height;
  int column;
  int scale;
} asset_conf_t;
#endif
