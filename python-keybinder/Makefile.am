AM_CPPFLAGS = $(PYTHON_INCLUDES) $(PYGTK_CFLAGS) -I$(top_srcdir)/libkeybinder

AM_CFLAGS = -Wall

_keybinderdir = $(pyexecdir)/keybinder
_keybinder_LTLIBRARIES = _keybinder.la
_keybinder_la_CPPFLAGS = $(AM_CPPFLAGS) $(X_CFLAGS) $(PYEXTRAFLAGS)
_keybinder_la_LDFLAGS = -module -avoid-version -export-symbols-regex init_keybinder $(X_LDFLAGS)
_keybinder_la_LIBADD = ../libkeybinder/libkeybinder.la $(PYGTK_LIBS)
_keybinder_la_SOURCES = _keybindermodule.c

nodist__keybinder_la_SOURCES = _keybinder.c

_keybinder_PYTHON = \
	__init__.py

CLEANFILES = _keybinder.c
EXTRA_DIST = \
	_keybinder.defs \
	_keybinder.override

_keybinder.c: _keybinder.defs _keybinder.override

.defs.c:
	(cd $(srcdir) \
	 && $(PYGOBJECT_CODEGEN) \
	    --override $*.override \
	    --register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gdk-types.defs \
	    --prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c

