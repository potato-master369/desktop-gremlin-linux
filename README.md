# desktop-gremlin-linux


A fork of Desktop_Gremlin made for linux, initially meant for learning Xlib with C. You will need to download the source code folder as it contains assets and installation. If you do not have access to the required libraries, by default the prebuilt ones will be included.

## Prerequisites
* a C compiler supporting at least C99 (you probably already have this try typing `gcc` or `clang`)
* libgtk4-dev
* A resolution of at LEAST 640x480

## Compositor support (Wayland)
| Compositor | Supported? | Extra information                             |
|------------|------------|-----------------------------------------------|
| Sway[FX]   | Yes        | Sway & Derivatives                            |
| KWin       | WIP        | Compositor for KDE Plasma                     |
| Mutter     | No         | Compositor for GNOME                          |
| Hyprland   | WIP        | Wlroots tiling window manager                 |

It is quite impossible, for us to support Mutter without extensions, as GNOME devs are let's just say, quite opinionated, so it does not have a call to move windows that are not focused. If it is really popular, then I might as well just write a shell extension I guess...

## Uninstallation (v2.3.1+)

`make uninstall` (this depends on the folders and binaries being the same as the current release u are using. If it changes, download the Source Code (.zip) from the release of your previous version and use its `make uninstall`)

## Clean

*This step is for people who want to build their own version as the repo already contains prebuilt binaries for x86_64 Debian 13 Trixie/ArchLinux depending on which of my machines I feel like using*

`make clean`: removes binaries (`*.o`, `Manhattan_Cafe`)

## Build
To build the thing, just run:
`git clone https://github.com/potato-master369/desktop-gremlin-linux`

`make` (as of 2.1.0)

## Install

(as of 2.3.0)
You can install `desktop-gremlin-linux` using `sudo make install` (starting v2.3.1). This will copy the assets and config as well.

For some reason there's also `sudo make install32` but that installs a 32-bit version (if you use 32-bit linux for whatever reason) but it will not be updated, just please use 64-bit...

## Configuration
`vim ~/Desktop/desktop-gremlin-assets/gremlin_config.ini` (For &lt;2.3.0)
`vim ~/.local/share/desktop-gremlin-linux/gremlin_config.ini` (For 2.3.0)
`vim /usr/share/desktop-gremlin-linux/gremlin_config.ini` (For 2.3.1+)
Several options are listed inside the file.
Configuration is not needed on the Wayland client as packs are preconfigured and there aren't X11 quirks anymore yay

-----
## FAQ
> Are you gonna add more features?

yeah, I would love to follow `Desktop_Gremlin` in terms of general direction, but I'd also like to add my own features, and/or remove features if I deem them bad or just find them difficult to port over to Xlib + C.

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
