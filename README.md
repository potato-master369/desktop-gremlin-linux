

[![YouTube - WompWomp1234-j6c](https://img.shields.io/badge/YouTube-WompWomp1234--j6c-ff0033?logo=youtube)](https://www.youtube.com/@wompwomp1234-j6c)

[![potato-master369 - desktop-gremlin-linux](https://img.shields.io/static/v1?label=potato-master369&message=desktop-gremlin-linux&color=blue&logo=github)](https://github.com/potato-master369/desktop-gremlin-linux "Go to GitHub repo")
[![stars - desktop-gremlin-linux](https://img.shields.io/github/stars/potato-master369/desktop-gremlin-linux?style=social)](https://github.com/potato-master369/desktop-gremlin-linux)
[![forks - desktop-gremlin-linux](https://img.shields.io/github/forks/potato-master369/desktop-gremlin-linux?style=social)](https://github.com/potato-master369/desktop-gremlin-linux)

[![GitHub tag](https://img.shields.io/github/tag/potato-master369/desktop-gremlin-linux?include_prereleases=&sort=semver&color=blue)](https://github.com/potato-master369/desktop-gremlin-linux/releases/)
[![License](https://img.shields.io/badge/License-MIT-blue)](#license)
[![issues - desktop-gremlin-linux](https://img.shields.io/github/issues/potato-master369/desktop-gremlin-linux)](https://github.com/potato-master369/desktop-gremlin-linux/issues)


# desktop-gremlin-linux

## About

Who wouldn't want to have their favorite Umas running around on your desktop? That shouldn't be a Windows only privelege.

This is a rewrite of `desktop-gremlin-linux`. Similar to the V3.x branch, it is also made with GTK4. There are however, major changes. For example, the X11 client has been removed because we can now fix all the initial problems with X11.

See `OPTIONS.md`, for how to use the config.txt options.

## Dependencies

* gtk4-layer-shell (`libgtk4-layer-shell`)
* GTK4 (`libgtk-4.0`/`gtk-4.0`)
* GCC
* GNU Make
* Your own assetpacks (or take the ones from `Kritzkingvoid/Desktop_Gremlin`)

## Building

```bash
make
```

## Installing

```bash
make install
```

## Troubleshooting

Sometimes, `degrli` may segfault, and exit with a warning that `config.txt` is missing. In this case, run the following command:
```bash
mkdir -p ~/.config/desktop-gremlin-linux
```

After that, open the `desktop-gremlin-linux Options` application and press "Save Changes". This will create the file. It segfaults because it can't find the folder to use for the assets.

### Compositor compatibility matrix

| Compositor             | Status                | degrli version |
| ---------------------- | --------------------- | -------------- |
| KWin (wayland)         | Works                 | v4.0.0-beta0.4 |
| GNOME (mutter)         | Works (except for keep-above | v4.0.0-beta0.4
| XFCE (with compositing) | Works                | v4.0.0-beta0.4 |
| LXDE                   | Doesn't work          | v4.0.0-beta0.4 |

This is not complete, and LXDE is without compositing cos im too dumb to get picom working in a VM.

### Bug Reporting

Try updating first, before you make an issue. Follow the following format:
```Markdown
# bug-report: <ISSUE>

## System Information

Linux/BSD Distribution: <DISTRO>
Compositor: <COMPOSITOR e.g. kwin/compton(for X11)/etc>
DE (if relevant): <DE e.g. KDE Plasma/GNOME/NULL (if not applicable; you're running a separate WM)>
GPU driver: <run lspci -k | grep -EA3 'VGA|3D|Display' and copy the output from "Kernel driver in use">
X11/Wayland: <X11/Wayland>

## Issue

### Expected Behaviour

<What is supposed to happen. MAKE SURE YOU READ the README first before you think something should happen when it shouldn't. I would happily clarify though. e.g. "Sprite shows normally with an image of my favourite character">

### Actual Behaviour

<What actually happened instead of the above. e.g. "sprite shows up as a unknown bunch of colors and segfaults somehow">

### Recreation instructions

<A step-by-step instruction to repeat your issue. If I can't fix it, I'll just do random shit with your setup until it works

e.g.

1. Download Rice Shower assetpack from https://github.com/Kritzkingvoid/Desktop_Gremlin/Releases (this is a known issue. Run tool/fixup.py from my repo on
   rice to fix her but this is an example)

2. Set assetpack to Rice through desktop-gremlin-linux options.

3. Open desktop-gremlin-linux>

## Log

<Run `degrli --loglevel -1 in a terminal`, attempt to recreate issue  and paste output here>

```

### Feature Requests

Doesn't have to be as specific. Just whatever you think this project needs, as long as it's not absurd (or impossible).

## Optimization

For some reason, GTK4's defualt renderer is really heavy on RAM. Try `GSK_RENDERER=cairo` as an environment variable. Drops about 10MB of RAM usage.

## Getting more Umas

For whatever reason, I have only included Tamtam, but if you want to ~~steal~~ get some more umas, see [`Desktop_Gremlin` releases](https://github.com/Kritzkingvoid/Desktop_Gremlin/releases).

Copy them into `/usr/share/desktop-gremlin-linux/SpriteSheet/Gremlins` (for the SpriteSheet folder, e.g. `SpriteSheet/Gremlins/Cafe`) and `/usr/share/desktop-gremlin-linux/Sounds/` (for the Sounds folder: e.g. `Sounds/Cafe`).

## License

Under MIT license. Source freely available and do what the hell you want with it.
