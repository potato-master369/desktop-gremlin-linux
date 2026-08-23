#ifndef ASSET_H
#define ASSET_H
// Change this to set the LRU maximum size.
// Explanation:
//  [ ] [ ] [ ] [ ] [ ] <- This is the LRU. We store spritesheet
//                         Cache in here. (~19MiB per each for Cafe)
//  Reducing -> Reduces RAM footprint, but will thrash disk if set too low.
//              Cannot be 1 because LRU gets a lot of confusion if so.
//  Increasing -> Increases RAM footprint, but reduces disk thrash, especially
//                if doing a lot of diagonal walking.
#define DEGRLI_LRU_SIZE    5
#define DEGRLI_LRU_NULL    0
#define DEGRLI_LRU_EMOTE_1 1
#define DEGRLI_LRU_EMOTE_2 2
#define DEGRLI_LRU_EMOTE_3 3
#define DEGRLI_LRU_EMOTE_4 4
#define DEGRLI_LRU_IDLE    1
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
  float scale;
} asset_conf_t;

void asset_init(void);
asset_conf_t *asset_request_conf(void);
#endif

void asset_cleanup(void);
