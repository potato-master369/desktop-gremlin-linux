// Desktop-gremlin-linux defines
// These are meant for configuring the compiler. Most things are self-explanatory.
// If you plan to redistribute your binary, please change DEGRLI_DIST to how
// you identify yourself. I don't want people coming to me for problems from your
// binary if you messed too hard with these config options

// Most defaults should work fine, but some tweaks may be needed if your system
// is special or something.
// 
// DEFINITIONS:
//   - DEGRLI_DIST: the distributor of the binary.
//   - DEGRLI_LOCALCONFPREFIX: The prefix to find the local config files, (config.txt etc.)
//   - DEGRLI_ASSET_DIR: Where Sounds and Spritesheets are found. (The folders)
//   - DEGRLI_RELEASE_STATE: whether this version is Debug or Release. This may change
//     things like how messages are printed, etc. PLEASE only set to DEGRLI_DEBUG or
//     DEGRLI_RELEASE
//   - DEGRLI_NO_X11: Disables X11. Useful if gdk-x11 is deprecated (in future versions of
//     GTK4, or if you want to build, and are too lazy to install the X11 libs.)
#ifndef DEFINES_H
#define DEFINES_H

// standard
// FOR PATHS, remember to add "/" to the BACK!!!
#define DEGRLI_DIST "potato-master369"
// if ~ is put here, it will resolve to $HOME. However, only at the start
#define DEGRLI_LOCALCONFPREFIX "~/.config/desktop-gremlin-linux/"
#define DEGRLI_ASSET_DIR       "/usr/share/desktop-gremlin-linux/"
#define DEGRLI_CHECKOUT_VERSION "v4.0.0-beta0.4"
#define DEGRLI_DEBUG 0
#define DEGRLI_RELEASE 1
#define DEGRLI_RELEASE_STATE DEGRLI_DEBUG

// Uncomment to enable HIGH debug.
// This prints small things like fids and ids on refresh.
// #define DEGRLI_DEBUG_HIGH

// Options stuff
#define DEGRLI_VER_STRING "desktop-gremlin-linux by potato-master369\nVersion 4.0.0 (Pre-release 2)"
// uncomment to disable X11
// #define DEGRLI_NO_X11
// uncomment to disable the Emote1 demo.
#define DEGRLI_NO_ANIM_DEMO
#endif

// Uncomment to override the random action state, for testing
//  0 - NIL
//  1 - trigger rclick emo
//  2 - NIL
//  3 - random move
// #define DEGRLI_RANDOM_OVERRIDE 3

// I sincerely hope you enjoy my product, and don't forget. Credits for the orignal go to
// @KritzKingVoid. I HAVE NO IDEA HOW HIS ASSETS ARE LICENSED, but probably distributable.
// It's fine.
