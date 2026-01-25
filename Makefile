# Build ini.o and dynamic.o, then link into final binary with X11 libs

CC      = gcc
CFLAGS  = -Wall -O2 -s -ffunction-sections -fdata-sections -Wl,--gc-sections
LDLIBS  = -lX11 -lXpm -lXext -lm -lXrender
TARGET  = "Manhattan_Cafe"
OBJ     = ini.o dynamic.o

# Default target
all: $(TARGET)

# Final executable (linked with X11-related libraries)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

# Compile dynamic.c into dynamic.o (depends on ini.h for headers)
dynamic.o: dynamic.c ini.h
	$(CC) $(CFLAGS) -c dynamic.c -o dynamic.o

# Compile ini.c into ini.o (typical for inih; adjust if header-only)
ini.o: ini.c ini.h
	$(CC) $(CFLAGS) -c ini.c -o ini.o -DINI_MAX_LINE=83 -DINI_ALLOW_INLINE_COMMENTS=0

# Clean
clean:
	rm -f $(OBJ) $(TARGET)

