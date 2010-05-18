GTK_CFLAGS = $(shell pkg-config --cflags gtk+-2.0)
GTK_LDFLAGS = $(shell pkg-config --libs gtk+-2.0)

LUA_CFLAGS = $(shell pkg-config --cflags lua5.1)
LUA_LDFLAGS = $(shell pkg-config --libs lua5.1)

KEYB_LDFLAGS = -lkeybinder

VALAC = valac

all: main lua vala

lua: minigtk.so

# Example C program with libkeybinder
main: main.c
	$(CC) $(GTK_CFLAGS) $(GTK_LDFLAGS) \
		$(KEYB_LDFLAGS) $< -o $@

# Lua "minigtk" library
minigtk.so: lminigtk.c
	$(CC) $(GTK_CFLAGS) $(GTK_LDFLAGS) \
		$(LUA_CFLAGS) $(LUA_LDFLAGS) $< -shared -o $@

vala: vexample

vexample: vexample.vala keybinder.vapi
	$(VALAC) --pkg gtk+-2.0 --pkg keybinder --vapidir=. $<

clean:
	rm -f *.so *.lo main vexample

.PHONY: clean all lua vala
