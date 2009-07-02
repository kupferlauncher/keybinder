AC_INIT(keybinder, 0.0.1,
    [])
AC_CONFIG_SRCDIR(keybinder/__init__.py)
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
dnl * Gnome Doc Utils
dnl ****************************************************************************
GNOME_DOC_INIT

dnl ****************************************************************************
dnl * Python 2.5
dnl ****************************************************************************
AM_PATH_PYTHON(2.5)
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
AM_CHECK_PYMOD(glib)
AM_CHECK_PYMOD(gobject)
AM_CHECK_PYMOD(gtk)
AM_CHECK_PYMOD(gtk.gdk)


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
dnl * --------------------------------------------------------------------------
dnl ****************************************************************************
AC_OUTPUT([
Makefile
keybinder/Makefile
])

echo
echo $PACKAGE v$VERSION
echo

echo Prefix............... : $prefix

echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
