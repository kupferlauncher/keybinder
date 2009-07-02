AC_INIT(deskbar-applet, 2.27.4,
    [http://bugzilla.gnome.org/browse.cgi?product=deskbar-applet])
AC_CONFIG_SRCDIR(deskbar/__init__.py)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
GNOME_COMMON_INIT

AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")
AC_PROG_CC

dnl ****************************************************************************
dnl * Pkg-Config
dnl ****************************************************************************
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

dnl ****************************************************************************
dnl * Translation & i18n
dnl ****************************************************************************

GETTEXT_PACKAGE="deskbar-applet"
IT_PROG_INTLTOOL([0.35.0])
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext package])
AM_GLIB_GNU_GETTEXT

dnl ****************************************************************************
dnl * Gnome Doc Utils
dnl ****************************************************************************
GNOME_DOC_INIT

dnl ****************************************************************************
dnl * GConf support
dnl ****************************************************************************
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AM_GCONF_SOURCE_2

dnl ****************************************************************************
dnl * Python 2.4
dnl ****************************************************************************
AM_PATH_PYTHON(2.4)
AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])

dnl ****************************************************************************
dnl * Write the values of various paths in defs.py
dnl ****************************************************************************
AC_SUBST(VERSION)
AC_SUBST(PACKAGE)

AS_AC_EXPAND(DATADIR, $datarootdir)
AC_SUBST(DATADIR)

AS_AC_EXPAND(LIBDIR, $libdir)
AC_SUBST(LIBDIR)

dnl ****************************************************************************
dnl * PyGTK & co for Gnome Streamer
dnl ****************************************************************************
PKG_CHECK_MODULES(DESKBAR,
	gtk+-2.0				>= 2.12
	pygtk-2.0				>= 2.12
	pygobject-2.0			>= 2.15.3
)
AC_SUBST(DESKBAR_CFLAGS)
AC_SUBST(DESKBAR_LIBS)

AC_MSG_CHECKING([for gnomeapplet module])
if AC_RUN_LOG([DISPLAY= $PYTHON -c '
import pygtk
pygtk.require("2.0")

try:
    import gnomeapplet
except ImportError, e:
    if str(e).find("gnomeapplet") >= 0:
          raise
except:
    pass
']); then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
  AC_MSG_ERROR([gnomeapplet Python module required to build deskbar])
fi

dnl ******************
dnl * Python modules
dnl ******************
AM_CHECK_PYMOD(dbus)
AM_CHECK_PYMOD(gconf)
AM_CHECK_PYMOD(gio)
AM_CHECK_PYMOD(glib)
AM_CHECK_PYMOD(gnomedesktop)
AM_CHECK_PYMOD(gnomekeyring)
AM_CHECK_PYMOD(gobject)
AM_CHECK_PYMOD(gtk)
AM_CHECK_PYMOD(gtk.gdk)
AM_CHECK_PYMOD(pango)
AM_CHECK_PYMOD(wnck)

dnl ****************************************************************************
dnl * Check for evolution's libebook to build the evolution handler
dnl ****************************************************************************
AC_ARG_ENABLE(evolution,
	AC_HELP_STRING([--disable-evolution], [Disable evolution's libebook support (default auto)]),
	enable_libebook=$enableval,
	enable_libebook=auto)

PKG_CHECK_MODULES(EVOLUTION,
	libebook-1.2			>= 1.7.92
	gtk+-2.0				>= 2.6
	pygtk-2.0				>= 2.6
	pygobject-2.0			>= 2.6,
	have_libebook_dependencies=yes,
	have_libebook_dependencies=no)

if test "x$have_libebook_dependencies" = "xno"; then
	dnl error out if user has explicitly requested libebook
	if test "x$enable_libebook" = "xyes"; then
		AC_MSG_ERROR([Could not find evolution's libebook])
	fi
	enable_libebook="no (missing dependencies)"
else
	if test "x$enable_libebook" != "xno"; then
		enable_libebook="yes"
	else
		enable_libebook="no (disabled)"
	fi
fi
AM_CONDITIONAL(ENABLE_EVOLUTION, test "x$enable_libebook" = "xyes")
AC_SUBST(EVOLUTION_LIBS)
AC_SUBST(EVOLUTION_CFLAGS)

dnl ****************************************************************************
dnl * PyGTK Codegen and defs files
dnl ****************************************************************************
AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)
if test "x$PYGTK_CODEGEN" = xno; then
  AC_MSG_ERROR(could not find pygtk-codegen-2.0 script)
fi

AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

dnl ****************************************************************************
dnl * Check for prctl()
dnl ****************************************************************************
AC_CHECK_FUNC(prctl, HAVE_PRCTL="yes", HAVE_PRCTL="yes")
if test "x$HAVE_PRCTL" = "xyes"; then
	AC_CHECK_DECL(PR_SET_NAME, STILL_HAVE_PRCTL="yes", STILL_HAVE_PRCTL="no", [#include <sys/prctl.h>])
	if test "x$STILL_HAVE_PRCTL" = "xyes"; then
		AC_DEFINE(HAVE_PRCTL, 1, [Defined as 1 if prctl and PR_SET_NAME are available])
	fi
fi

dnl ****************************************************************************
dnl * Check for DBUS Services directory
dnl ****************************************************************************
DBUS_SERVICES_DIR="$DATADIR/dbus-1/services"
AC_SUBST(DBUS_SERVICES_DIR)
AC_DEFINE_UNQUOTED(DBUS_SERVICES_DIR, "$DBUS_SERVICES_DIR", [DBus services directory])

dnl ****************************************************************************
dnl * --------------------------------------------------------------------------
dnl ****************************************************************************
AC_OUTPUT([
Makefile
deskbar/Makefile
deskbar/defs.py
deskbar/core/Makefile
deskbar/core/keybinder/Makefile
deskbar/core/updater/Makefile
deskbar/core/_userdirs/Makefile
deskbar/handlers/Makefile
deskbar/handlers/actions/Makefile
deskbar/osutils/Makefile
deskbar/interfaces/Makefile
deskbar/ui/Makefile
deskbar/ui/cuemiac/Makefile
deskbar/ui/iconentry/Makefile
deskbar/ui/preferences/Makefile
data/Makefile
data/deskbar-applet.pc
data/art/Makefile
po/Makefile.in
help/Makefile
])

echo
echo $PACKAGE v$VERSION
echo

echo Prefix............... : $prefix
echo Evolution Support.... : $enable_libebook
echo Dbus Services Dir.... : $DBUS_SERVICES_DIR

echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
