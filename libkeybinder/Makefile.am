AM_CPPFLAGS = $(GTK_CFLAGS)

AM_CFLAGS = -Wall -Wextra -ansi

LIBKEYBINDER_VERSION = $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

# C library libkeybinder
#
keybinderdir = $(libdir)
keybinder_LTLIBRARIES = libkeybinder-3.0.la
libkeybinder_3_0_la_SOURCES = bind.c
libkeybinder_3_0_la_LDFLAGS = $(GTK_LIBS)  -version-info $(LIBKEYBINDER_VERSION)
keybinder_includedir = $(pkgincludedir)
keybinder_include_HEADERS = keybinder.h

pkgconffile = $(PACKAGE).pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pkgconffile)

EXTRA_DIST = $(pkgconffile).in
DISTCLEANFILES = $(pkgconffile)

CLEANFILES =


# gobject-introspection for keybinder
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libkeybinder_3_0_la_SOURCES) $(keybinder_include_HEADERS)

Keybinder-3.0.gir: libkeybinder-3.0.la
Keybinder_3_0_gir_INCLUDES = GObject-2.0
Keybinder_3_0_gir_CFLAGS = $(INCLUDES)
Keybinder_3_0_gir_LIBS = libkeybinder-3.0.la
Keybinder_3_0_gir_FILES = $(introspection_sources)
Keybinder_3_0_gir_SCANNERFLAGS = --warn-all
INTROSPECTION_GIRS += Keybinder-3.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
