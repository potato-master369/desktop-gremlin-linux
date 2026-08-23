# Configuration Options for desktop_gremlin_linux
# Desktop Gremlin Configuration Reference

`START_CHAR` (type `string`): Sets the name of the default character folder/sprite loaded at startup.
### Examples
`START_CHAR = Tamamo` - Loads the Tamamo sprite character asset set on launch.

`LANGUAGE_DIFF` (type `bool`): Toggles localized language settings or alternative text formatting. Ignored on Linux.
### Examples
`LANGUAGE_DIFF = true` - Enables regional/alternative language handling.

`ENABLE_KEYBOARD` (type `bool`): Enables or disables global keyboard listener shortcuts.
### Examples
`ENABLE_KEYBOARD = true` - Allows the pet to react to or listen for user keyboard inputs.

`ALLOW_ERROR_MESSAGES` (type `bool`): Controls whether runtime error popups and warning dialogs are shown. Ignored on Linux. Check STDOUT instead.
### Examples
`ALLOW_ERROR_MESSAGES = true` - Displays error dialogs if missing assets or config issues occur.

`SHOW_TASKBAR` (type `bool`): Determines whether the pet application displays an icon in the desktop taskbar. Ignored on Linux. Not able to standardise.
### Examples
`SHOW_TASKBAR = true` - Shows the application icon on the taskbar.

`RANDOMIZE_SPAWN` (type `bool`): Randomizes the initial $(X, Y)$ screen coordinates when the pet spawns.
### Examples
`RANDOMIZE_SPAWN = true` - Spawns the pet at a random location on your desktop upon launch.

`USE_WPFPLAYER` (type `bool`): Toggles legacy WPF media playback features. Ignored on Linux.
### Examples
`USE_WPFPLAYER = false` - Bypasses WPF media player initialization.

`SPAWN_DISTANCE` (type `int`): Defines the offset distance in pixels for companion or initial spawning relative to edges/objects.
### Examples
`SPAWN_DISTANCE = 150` - Sets a 150-pixel spawn offset gap.

`VOLUME_LEVEL` (type `float`): Sets the audio output volume level for sound effects and voice lines (range `0.0` to `1.0`).
### Examples
`VOLUME_LEVEL = 0.5` - Sets sound playback volume to 50%.

`SPRITE_FRAMERATE` (type `int`): Specifies the target frames-per-second (FPS) rendering rate for sprite animations.
### Examples
`SPRITE_FRAMERATE = 60` - Renders the character animations at 60 FPS.

`SPRITE_SPEED` (type `int`): Base movement speed multiplier for character locomotion across the screen.
### Examples
`SPRITE_SPEED = 10` - Sets the default movement step speed to 10.

`FOLLOW_RADIUS` (type `int`): Distance threshold in pixels where the pet begins tracking or following the cursor.
### Examples
`FOLLOW_RADIUS = 300` - The pet starts reacting to cursor movement within 300 pixels.

`START_BUTTOM` (type `bool`): Anchors the pet's initial spawn location to the bottom edge of the primary monitor. (Linux extension: can be spelled as `START_BOTTOM`)
### Examples
`START_BUTTOM = false` - Allows spawning anywhere rather than locking to the bottom screen border.

`ENABLE_GRAVITY` (type `bool`): Toggles gravity simulation, pulling the pet down toward the desktop floor when dragged or dropped.
### Examples
`ENABLE_GRAVITY = false` - Disables gravity, allowing the pet to float or stay anywhere on screen.

`SLEEP_TIME` (type `int`): Duration in seconds before the pet enters an idle/sleeping state.
### Examples
`SLEEP_TIME = 500` - Waits 500 units of inactivity before switching to the sleep animation.

`GRAVITY_STRENGTH` (type `int`): Acceleration force applied to the pet when falling under gravity.
### Examples
`GRAVITY_STRENGTH = 20` - Applies a downward gravity pull rate of 20 units.

`ALLOW_RANDOM_ACTIONS` (type `bool`): Enables or disables autonomous random idle behaviors, wandering, and emotes.
### Examples
`ALLOW_RANDOM_ACTIONS = true` - Allows the pet to perform random actions when left unattended.

`MAX_INTERVAL` (type `int`): Upper bound timer limit (in seconds) between random autonomous triggers.
### Examples
`MAX_INTERVAL = 40` - The pet will wait at most 40 seconds before picking a new random action.

`MIN_INTERVAL` (type `int`): Lower bound timer limit (in seconds) between random autonomous triggers.
### Examples
`MIN_INTERVAL = 30` - The pet will wait at least 30 seconds between random actions.

`RANDOM_MOVE_DISTANCE` (type `int`): Maximum distance in pixels the pet can travel during a random wander event.
### Examples
`RANDOM_MOVE_DISTANCE = 120` - Restricts autonomous wandering distance to 120 pixels per move.

`WALK_DISTANCE` (type `int`): Target travel distance in pixels for designated walking state sequences.
### Examples
`WALK_DISTANCE = 200` - Sets standard walking distance paths to 200 pixels.

`ALLOW_COLOR_HOTSPOT` (type `bool`): Enables hit-testing for defined directional interactive body regions (hotspots).
### Examples
`ALLOW_COLOR_HOTSPOT = true` - Enables regional clicking triggers on the character.

`DISABLE_HOTSPOTS` (type `bool`): Master toggle to completely turn off interactive click-zone detection.
### Examples
`DISABLE_HOTSPOTS = false` - Keeps interactive click hotspots active.

`ENABLE_MIN_RESIZE` (type `bool`): Restricts window downsizing below a minimum safe pixel threshold.
### Examples
`ENABLE_MIN_RESIZE = true` - Prevents resizing the pet window so small that it becomes invisible or broken.

`FORCE_CENTER` (type `bool`): Forces the window and sprite to remain centered on the primary display.
### Examples
`FORCE_CENTER = false` - Disables forced center locking, allowing manual positioning.

`ENABLE_MANUAL_RESIZE` (type `bool`): Allows users to resize the pet window bounds manually.
### Examples
`ENABLE_MANUAL_RESIZE = true` - Enables dragging window borders to resize the sprite canvas.

`FORCE_FAKE_TRANSPARENT` (type `bool`): Forces chroma-key or simulated window transparency fallback for unsupported compositors. Ignored on Linux.
### Examples
`FORCE_FAKE_TRANSPARENT = true` - Enables fake transparency modes for legacy or non-composited window managers.

`ALLOW_CACHE` (type `bool`): Toggles memory caching of pre-loaded sprite frames and audio files into RAM. Ignored on Linux.
### Examples
`ALLOW_CACHE = false` - Reads sprite frames directly from disk rather than holding them in memory cache.

`CURRENT_ACCELERATION` (type `float`): Active physics acceleration rate for smooth movement transitions.
### Examples
`CURRENT_ACCELERATION = 0.3` - Sets movement acceleration smoothing factor to 0.3.

`FOLLOW_ACCELERATION` (type `float`): Acceleration curve applied specifically when following the mouse cursor or food items.
### Examples
`FOLLOW_ACCELERATION = 0.2` - Applies a 0.2 acceleration factor during tracking states.

`MAX_ACCELERATION` (type `int`): Maximum velocity cap for physics-driven movement.
### Examples
`MAX_ACCELERATION = 200` - Caps movement velocity at 200 units.

`COMPANION_CHAR` (type `string`): Sets the folder name of a secondary companion sprite character to spawn alongside the primary pet.
### Examples
`COMPANION_CHAR = OguriComp` - Spawns OguriComp as a secondary companion pet.

`COMPANION_SCALE` (type `float`): Size multiplier scale factor applied to the secondary companion sprite (e.g. `0.5` = 50% size).
### Examples
`COMPANION_SCALE = 0.5` - Scales the companion pet down to half size.

`COMPANION_FOLLOW` (type `int`): Target distance offset in pixels that the companion maintains relative to the primary pet.
### Examples
`COMPANION_FOLLOW = 90` - Tells the companion pet to trail 90 pixels behind the primary pet.

`FOOD_SPAWN` (type `string`): File name of the item/food sprite image used when spawning interactive food objects.
### Examples
`FOOD_SPAWN = food1.png` - Uses `food1.png` as the item image when dropping food.

`STRAIGHT_LINE` (type `bool`): Forces movement pathfinding to use direct straight lines rather than curved or path-finding trajectories.
### Examples
`STRAIGHT_LINE = false` - Allows natural curved/interpolated movement paths.

## Format

### Int

Declared as `OPTION = 1234` (OPTION is not a real option.)

### Bool

Declared as `OPTION = true` or `OPTION = false` (OPTION is not a real option.)

### Str

Declared as `Option = string` (no quotes) (OPTION is not a real option.)

## Linux extensions

By default, `desktop-gremlin-linux` adds some format extensions to the original `Desktop_Gremlin`.

* Bool can now be declared as true (Case-insensitive, so true, True, TRUE, etc), 1. Applies to False and 0.
* Spacing can be added for padding to look pretty.
* DOS and UNIX newline support.
* The misspelling for `START_BOTTOM` as `START_BUTTOM` can be fixed. Something I really hate.

## Source options

These changes require a recompile, and cannot be changed after install (will require a re-install).
Change the following in the following places:

* Debug/Release, demo options, etc `src/defines.h`
* LRU Cache size `src/asset.h`

Comments in source contain what altering these does.

## Mistakes

If there are any mistakes in this document, open an issue, or email `random.nonprivate.email@gmail.com`. I will probably answer issues first.

## Document license

This document does not fall under any license. Further derivative works do not require credit (though appreciated), and modifications are allowed.

