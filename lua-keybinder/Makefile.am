AM_CPPFLAGS = $(LUA_INCLUDE) $(GTK_CFLAGS) -I../libkeybinder

AM_CFLAGS = -Wall

luakeybinderdir = $(libdir)/lua/$(LUA_VERSION)
luakeybinder_LTLIBRARIES = keybinder.la
keybinder_la_LDFLAGS = -module -avoid-version
keybinder_la_LIBADD = ../libkeybinder/libkeybinder.la $(LUA_LIB)
keybinder_la_SOURCES = lkeybinder.c
