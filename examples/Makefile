GTKPKG = gtk+-2.0
KEYBPKG = keybinder
GTK_CFLAGS = $(shell pkg-config --cflags $(GTKPKG))
GTK_LDFLAGS = $(shell pkg-config --libs $(GTKPKG))

LUA_CFLAGS = $(shell pkg-config --cflags lua5.1)
LUA_LDFLAGS = $(shell pkg-config --libs lua5.1)

KEYB_FLAGS = $(shell pkg-config --cflags --libs $(KEYBPKG))

CFLAGS ?= -g -O2

VALAC = valac

all: main lua vala

lua: minigtk.so

# Example C program with libkeybinder
main: main.c
	$(CC) $(KEYB_FLAGS) $(CFLAGS) $< -o $@

# Lua "minigtk" library
minigtk.so: lminigtk.c
	$(CC) $(GTK_CFLAGS) $(GTK_LDFLAGS) \
		$(LUA_CFLAGS) $(LUA_LDFLAGS) $< -shared -o $@

vala: vexample

vexample: vexample.vala keybinder.vapi
	$(VALAC) --pkg $(GTKPKG) --pkg keybinder --vapidir=. $<

clean:
	rm -f *.so *.lo main vexample

.PHONY: clean all lua vala
