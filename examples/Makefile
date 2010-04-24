GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LDFLAGS = $(shell pkg-config --libs gtk+-2.0)

LUA_CFLAGS = $(shell pkg-config --cflags lua5.1)
LUA_LDFLAGS = $(shell pkg-config --libs lua5.1)

KEYB_LDFLAGS = -lkeybinder

all: keybinder.so minigtk.so main

# Example C program with libkeybinder
main: main.c
	$(CC) $(GTK_CFLAGS) $(GTK_LDFLAGS) \
		$(KEYB_LDFLAGS) $< -o $@

# Lua keybinder library
keybinder.so: lkeybinder.c
	$(CC) $(LUA_CFLAGS) $(LUA_LDFLAGS) \
		$(GTK_CFLAGS) $(KEYB_LDFLAGS) $< -shared -o $@

# Lua "minigtk" library
minigtk.so: lminigtk.c
	$(CC) $(GTK_CFLAGS) $(GTK_LDFLAGS) \
		$(LUA_CFLAGS) $(LUA_LDFLAGS) $< -shared -o $@

clean:
	rm -f *.so *.lo main

.PHONY: clean all
