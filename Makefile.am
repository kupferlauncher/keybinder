SUBDIRS = libkeybinder

if HAVE_PYTHON
SUBDIRS += python-keybinder
endif

if HAVE_LUA
SUBDIRS += lua-keybinder
endif

EXTRA_DIST = \
	examples/Makefile  \
	examples/README    \
	examples/example.py \
	examples/example.lua \
	examples/lkeybinder.c \
	examples/lminigtk.c \
	examples/main.c


DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	gnome-doc-utils.make

