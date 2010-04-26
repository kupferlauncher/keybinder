INCLUDES = $(GTK_CFLAGS)

AM_CFLAGS = -Wall

LIBKEYBINDER_VERSION = $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

# C library libkeybinder
#
keybinderdir = $(libdir)
keybinder_LTLIBRARIES = libkeybinder.la
libkeybinder_la_SOURCES = bind.c
libkeybinder_la_LDFLAGS = $(GTK_LIBS)  -version-info $(LIBKEYBINDER_VERSION)
include_HEADERS = keybinder.h

