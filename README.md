

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

## Optimization

For some reason, GTK4's defualt renderer is really heavy on RAM. Try `GSK_RENDERER=cairo` as an environment variable. Drops about 10MB of RAM usage.

## Getting more Umas

For whatever reason, I have only included Tamtam, but if you want to ~~steal~~ get some more umas, see [`Desktop_Gremlin` releases](https://github.com/Kritzkingvoid/Desktop_Gremlin/releases).

## License

Under MIT license. Source freely available and do what the hell you want with it.
