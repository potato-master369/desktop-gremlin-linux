# Makefile for desktop-gremlin-linux
# Copyright (C) potato-master369 2026

CC      = gcc
CFLAGS  = $(shell pkg-config --cflags gtk4) -O2 
LDFLAGS = -Wl,--gc-sections
# MC_EFLAGS = $( pkg-config --cflags-only-I gtk4 )
LDLIBS  = $(shell pkg-config --libs gtk4)
NCURSES = -lncursesw

TARGETS = Manhattan_Cafe desktop-gremlin-linux-manager
OBJ     = ini.o dynamic.o

# Default target
all: $(TARGETS)

# Manhattan_Cafe build (X11-based)
Manhattan_Cafe: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

# desktop-gremlin-linux-manager build (ncurses-based)
desktop-gremlin-linux-manager: ncurses-manager.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ ncurses-manager.c $(NCURSES)

# Object rules
dynamic.o: src/wayland.c src/ini.h
	$(CC) $(CFLAGS) -c $< -o $@

ini.o: src/ini.c src/ini.h
	$(CC) $(CFLAGS) -c $< -o $@ -DINI_MAX_LINE=83 -DINI_ALLOW_INLINE_COMMENTS=0

# Clean
clean:
	rm -f $(OBJ) $(TARGETS)

install:
	mkdir /usr/share/desktop-gremlin-linux
	cp desktop-gremlin-assets /usr/share/desktop-gremlin-linux -r
	cp icon.xpm /usr/share/desktop-gremlin-linux
	cp desktop-gremlin-linux-manager /usr/local/bin
	cp Manhattan_Cafe /usr/local/bin
	cp desktop-gremlin-linux-manager.desktop /usr/share/applications

uninstall:
	rm -rf /usr/share/desktop-gremlin-linux
	rm -f /usr/local/bin/desktop-gremlin-linux-manager
	rm -f /usr/local/bin/Manhattan_Cafe
	rm -f /usr/share/applications/desktop-gremlin-linux-manager.desktop
