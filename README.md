# desktop-gremlin-linux

A fork of Desktop_Gremlin made for linux, initially meant for learning Xlib with C
*I honestly just gave up on the 20 subs promise you know what guys here you go but still check me out and/or subscribe on (YouTube)[https://www.youtube.com/@WompWomp1234-j6c]*

---
## Prerequisites
* a C compiler supporting at least C99 (you probably already have this try typing `gcc` or `clang`)
* Xlib (you probably also have this preinstalled)
* X11 (it should in theory run fine under XWayland but I never checked)
* an X11 compositor with alpha support

## build
To build the thing, just run:
`clang dynamic.c -lX11 -lXpm -lXext -lm -o 'Manhattan Cafe'`
*(feel free to replace clang with a `cc` of your choice and change dynamic.c to any of the .c files (feel free to change the -o as well it doesnt have to be called that. ~~use `-o jukebox` on `jukebox.c` for it to work with v1~~)*

## FAQ
> Are you gonna add more features?
yeah, I would love to follow `Desktop_Gremlin` in terms of general direction, but I'd also like to add my own features, and/or remove features if I deem them bad or just find them difficult to port over to Xlib + C.
> When is v3?
Idk bro
> Are you gonna be working on this forever?
no but it's a passion project so I'll check back on it once in a while

## Thanks to:
* [KirtVakesco](https://github.com/KurtVelasco) for the [original](https://github.com/KurtVelasco/Desktop_Gremlin), inspiration, and assets (sowwy for stealing them I can't figure umaViewer out)
* ImageMagick team: ~~stealing~~ repurposing KurtValesco's assets would have been impossible without this umazing (pun intended) tool.
* and of course, YOU for being interested in this project of mine!

# future to-do
~~* mimic drag features~~
* add sound
* figure UmaViewer out so I can get assets for more characters (and/or just steal more from the [original](https://github.com/KurtVelasco/Desktop_Gremlin))
