# desktop-gremlin-linux

*NOTE 2026/04/18 We are currently working on the Wayland client. See `wayland.c` for more information. It is a work in progress, but no further work on the Xlib client will be done.*
A fork of Desktop_Gremlin made for linux, initially meant for learning Xlib with C. You will need to download the source code folder as it contains assets and installation. If you do not have access to the required libraries, by default the prebuilt ones will be included.

The JukeBox only works as intended on pure X11 environments (as shape emulation is flaky on XWayland) and will **NOT** be actively maintained, just like Tachyon. However, Tachyon is not animated so you can still use that!


## Prerequisites
* a C compiler supporting at least C99 (you probably already have this try typing `gcc` or `clang`)
* Xlib (you probably also have this preinstalled)
* X11 (it should in theory run fine under XWayland but I never checked)
* an X11 compositor with alpha support

## Uninstallation (v2.3.1+)

`make uninstall` (this depends on the folders and binaries being the same as the current release u are using. If it changes, download the Source Code (.zip) from the release of your previous version and use its `make uninstall`)

## Clean

*This step is for people who want to build their own version as the repo already contains prebuilt binaries for x86_64 Debian 13 Trixie/ArchLinux depending on which of my machines I feel like using*

`make clean`: removes binaries (`*.o`, `Manhattan_Cafe`, `desktop-gremlin-linux-manager`)

## Build
To build the thing, just run:
`git clone https://github.com/potato-master369/desktop-gremlin-linux`

~~`clang dynamic.c -lX11 -lXpm -lXext -lm -o 'Manhattan Cafe'`~~

`make` (as of 2.1.0)
*feel free to replace clang with a `cc` of your choice and change dynamic.c to any of the .c files (feel free to change the -o as well it doesnt have to be called that. ~~use `-o jukebox` on `jukebox.c` for it to work with v1~~)*

~~`cp ./desktop-gremlin-assets ~/Desktop`~~
~~Copy the assets folder to the desktop - required for the program to function properly, without which it is undefined behaviour.~~

## Install

(as of 2.3.0)
You can install `desktop-gremlin-linux` using `sudo make install` (starting v2.3.1). This will copy the assets and config as well.
## Configuration
`vim ~/Desktop/desktop-gremlin-assets/gremlin_config.ini` (For &lt;2.3.0)
`vim ~/.local/share/desktop-gremlin-linux/gremlin_config.ini` (For 2.3.0)
`vim /usr/share/desktop-gremlin-linux/gremlin_config.ini` (For 2.3.1+)
Several options are listed inside the file.

-----
## FAQ
> Are you gonna add more features?

yeah, I would love to follow `Desktop_Gremlin` in terms of general direction, but I'd also like to add my own features, and/or remove features if I deem them bad or just find them difficult to port over to Xlib + C.
> When is v3?

Idk bro
> Are you gonna be working on this forever?

no but it's a passion project so I'll check back on it once in a while

## Thanks to:
* [KirtValesco](https://github.com/KurtVelasco) for the [original](https://github.com/KurtVelasco/Desktop_Gremlin), inspiration, and assets (sowwy for stealing them I can't figure umaViewer out)
* ImageMagick team: ~~stealing~~ repurposing KurtValesco's assets would have been impossible without this umazing (pun intended) tool.
* Ben Hoyt for INIH
* and of course, YOU for being interested in this project of mine!

# future to-do
~~* mimic drag features~~
* add sound
* figure UmaViewer out so I can get assets for more characters (and/or just steal more from the [original](https://github.com/KurtVelasco/Desktop_Gremlin))
