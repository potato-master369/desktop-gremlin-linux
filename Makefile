# Makefile for desktop-gremlin-linux
# Copyright (C) potato-master369 2026

CC      = gcc
CFLAGS  = $(shell pkg-config --cflags gtk4) -O2 
LDFLAGS = -Wl,--gc-sections
# MC_EFLAGS = $( pkg-config --cflags-only-I gtk4 )
LDLIBS  = $(shell pkg-config --libs gtk4)
NCURSES = -lncursesw

TARGETS = degrli
OBJ     = ini.o cs.o dynamic.o

# Default target
all: $(TARGETS)

# NEW WAYLAND CLIENT11111!!!
degrli: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Object rules
dynamic.o: src/wayland.c src/ini.h
	$(CC) $(CFLAGS) -c $< -o $@

ini.o: src/ini.c src/ini.h
	$(CC) $(CFLAGS) -c $< -o $@ -DINI_MAX_LINE=83 -DINI_ALLOW_INLINE_COMMENTS=0

cs.o: src/compositor-specific.c src/compositor-specific.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(TARGETS)

install:
	mkdir -p /usr/share/desktop-gremlin-linux
	mkdir -p /usr/share/desktop-gremlin-linux/assets
	cp -r manhattancafe /usr/share/desktop-gremlin-linux/assets
	#cp desktop-gremlin-linux-manager /usr/local/bin
	cp degrli /usr/local/bin
	cp desktop-gremlin-linux-manager.desktop /usr/share/applications

uninstall:
	rm -rf /usr/share/desktop-gremlin-linux
	rm -f /usr/local/bin/desktop-gremlin-linux-manager
	rm -f /usr/local/bin/degrli
	rm -f /usr/share/applications/desktop-gremlin-linux-manager.desktop
