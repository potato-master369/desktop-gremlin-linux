# Makefile for desktop-gremlin-linux
# Copyright (C) potato-master369 2026

CC      = gcc
CFLAGS  = $(shell pkg-config --cflags gtk4) -O2 -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include/ $(shell pkg-config --cflags gtk4-layer-shell-0)
LDFLAGS = -Wl,--gc-sections
# MC_EFLAGS = $( pkg-config --cflags-only-I gtk4 )
LDLIBS  = $(shell pkg-config --libs gtk4) $(shell pkg-config --libs gtk4-layer-shell-0) -lm -lX11 -lXext -lXrender
NCURSES = -lncursesw

MANAGER = degrli-manager
TARGETS = degrli
OBJ     = dynamic.o config.o

# Default target
all: $(TARGETS) $(MANAGER)

# NEW WAYLAND CLIENT11111!!!
degrli: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS) -lX11

dynamic.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

config.o: src/config.c src/config.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(TARGETS)

install:
	mkdir -p /usr/share/desktop-gremlin-linux
	mkdir -p /usr/share/desktop-gremlin-linux/assets
	cp -r manhattancafe /usr/share/desktop-gremlin-linux/assets
	#cp desktop-gremlin-linux-manager /usr/local/bin
	cp blanktexture.png /usr/share/desktop-gremlin-linux/assets
	cp degrli /usr/local/bin
	cp degrli-manager /usr/local/bin
	cp desktop-gremlin-linux-manager.desktop /usr/share/applications

uninstall:
	rm -rf /usr/share/desktop-gremlin-linux
	rm -f /usr/local/bin/degrli-manager
	rm -f /usr/local/bin/degrli
	rm -f /usr/share/applications/desktop-gremlin-linux-manager.desktop
