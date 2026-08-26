# Makefile for desktop-gremlin-linux
# Copyright (C) potato-master369 2026

CC      = gcc
CFLAGS_BASE  = $(shell pkg-config --cflags gtk4) -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include/ $(shell pkg-config --cflags gtk4-layer-shell-0)
LDFLAGS = -Wl,--gc-sections
# MC_EFLAGS = $( pkg-config --cflags-only-I gtk4 )
LDLIBS  = $(shell pkg-config --libs gtk4) $(shell pkg-config --libs gtk4-layer-shell-0) -lm -lX11 -lXext -lXrender
NCURSES = -lncursesw

TARGETS = degrli degrli_options degrli_installer
OBJ     = dynamic.o config.o sounds.o asset.o animation.o
OPTOBJ  = options.o meme.o config_opt.o
INSTOBJ = installer.o
CFLAGS = $(CFLAGS_BASE) -Os -ffast-math -fomit-frame-pointer -march=native -flto -fno-exceptions -fno-unroll-loops

debug: CFLAGS=$(CFLAGS_BASE) -g -O0 -DDEBUG
debug: clean $(TARGETS)

# Default target
all: $(TARGETS)

# NEW WAYLAND CLIENT11111!!!
degrli: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS) -lX11

dynamic.o: src/main.c
	$(CC) $(CFLAGS) -c $< -o $@

config.o: src/config.c src/config.h
	$(CC) $(CFLAGS) -c $< -o $@

sounds.o: src/sounds.c src/sounds.h
	$(CC) $(CFLAGS) -c $< -o $@

asset.o: src/asset.c src/asset.h
	$(CC) $(CFLAGS) -c $< -o $@

animation.o: src/animation.c src/animation.h
	$(CC) $(CFLAGS) -c $< -o $@

# Options program
degrli_options: $(OPTOBJ)
	$(CC) $(CFLAGS) -o $@ $(OPTOBJ) $(LDLIBS)

meme.o: src/options/meme.c src/options/meme.h
	$(CC) $(CFLAGS) -c $< -o $@

config_opt.o: src/options/config.c src/options/config.h
	$(CC) $(CFLAGS) -c $< -o $@

options.o: src/options/main.c
	$(CC) $(CFLAGS) -c $< -o $@

degrli_installer: $(INSTOBJ)
	$(CC) $(CFLAGS) -o $@ $(INSTOBJ) $(LDLIBS)

installer.o: src/installer/main.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -f $(OBJ) $(TARGETS) $(OPTOBJ)

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
