========================================
         desktop-gremlin-linux
========================================

SOURCE CODE OF DESKTOP-GREMLIN-LINUX

This source code is freely available
under the MIT license. It is free and
open source software, that anyone is
free to fork, redistribute, modify or
reuse without permission.

MIT LICENSE:

Copyright 2026- potato-master369

Permission is hereby granted,
free of charge, to any person
obtaining a copy of this software and
associated documentation files (the
“Software”), to deal in the Software
without restriction, including without
limitation the rights to use, copy,
modify, merge, publish, distribute,
sublicense, and/or sell copies of the
Software, and to permit persons to whom
the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this
permission notice shall be included in
all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED “AS IS”,
WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

END MIT LICENSE

BRIEF DESCRIPTION OF FILES
--------------------------

src/
  - The main folder for source.
  main.c -- main file for Gremlin
  sounds.c -- sound subsystem
  sounds.h -- sound subsystem header
  miniaudio.h -- MiniAudio (MIT-0
  license)
  config.c -- config handler
  config.h -- config handler header
  defines.h -- Defines -
  distributor/filepaths/etc
  options/
    - the folder for Options dialog
    main.c -- main C program (GTK4)
    meme.c -- Resources (and meme)
    meme.h -- header for the
    aforementioned
    app.resource.xml --
    GTK4 XML for resources. Use
    glib-compile-resources to generate
    meme.c and meme.h)

This folder structure is subject to
change across versions. Read updates
for the newest version in source.

MIRRORS
-------

You may find a mirror
of this on GitHub, at
potato-master369/desktop-gremlin-linux
(Branch: main)

Branches of the GitHub mirror:
  - main (current)
  - v3 (old v3 client (archival))

HIST/CHANGELOG
--------------
2026-08-15 - Start Changelog. On this
day I added the Options dialog.
