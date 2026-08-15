# desktop-gremlin-linux

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

## License

Under MIT license. Source freely available and do what the hell you want with it.
