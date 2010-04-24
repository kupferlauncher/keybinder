AC_INIT(keybinder, 0.2.0,
    [])
AC_CONFIG_SRCDIR(libkeybinder/bind.c)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
GNOME_COMMON_INIT

AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_SUBST(ACLOCAL_AMFLAGS, "$ACLOCAL_FLAGS -I m4")
AC_PROG_CC

dnl ****************************************************************************
dnl * Enable Automake 1.11 silent rules
dnl ****************************************************************************
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl ****************************************************************************
dnl * Pkg-Config
dnl ****************************************************************************
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, yes, no)
if test "x$HAVE_PKGCONFIG" = "xno"; then
  AC_MSG_ERROR(you need to have pkgconfig installed !)
fi

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
dnl * GTK
dnl ****************************************************************************
PKG_CHECK_MODULES(GTK,
	gtk+-2.0				>= 2.20
)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

dnl ***********************************
dnl * Require XKB
dnl ***********************************
X_LIBS="`$PKG_CONFIG --libs x11 xext xrender`"
X_CFLAGS="`$PKG_CONFIG --cflags x11 xext xrender`"
old_cflags="$CFLAGS"
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$X_LIBS $LIBS"

AC_CHECK_FUNC(XkbQueryExtension, [], AC_MSG_ERROR(Could not find XKB))

CFLAGS="$old_cflags"

X_LDFLAGS="$X_LIBS"

AC_SUBST(X_CFLAGS)
AC_SUBST(X_LDFLAGS)

dnl ****************************************************************************
dnl * Python
dnl ****************************************************************************
AC_ARG_ENABLE(python,
             AS_HELP_STRING([--disable-python],
                            [disable python bindings [[default=yes]]]),
			    ,
                            [enable_python=yes])

if test "x$enable_python" != "xno"; then
	dnl ****************************************************************************
	dnl * Python 2.5
	dnl ****************************************************************************
	have_python="yes"
	AM_PATH_PYTHON(2.5)
	AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(could not find Python headers)])

	PKG_CHECK_MODULES(PYGTK,
		pygtk-2.0				>= 2.12
		pygobject-2.0			>= 2.15.3
	)
	AC_SUBST(PYGTK_CFLAGS)
	AC_SUBST(PYGTK_LIBS)

	dnl ******************
	dnl * Python modules
	dnl ******************
	AM_CHECK_PYMOD(glib)
	AM_CHECK_PYMOD(gobject)
	AM_CHECK_PYMOD(gtk)
	AM_CHECK_PYMOD(gtk.gdk)

	dnl Python violates C99 rules, by casting between incompatible
	dnl pointer types. GCC may generate bad code as a result of that,
	dnl so use -fno-strict-aliasing if supported.
	AC_MSG_CHECKING(whether $CC accepts -fno-strict-aliasing)
	ac_save_cc="$CC"
	CC="$CC -fno-strict-aliasing"
	AC_CACHE_VAL(ac_cv_no_strict_aliasing_ok,
	  AC_TRY_COMPILE([],[int main() { return 0; }],
	  ac_cv_no_strict_aliasing_ok=yes,
	  ac_cv_no_strict_aliasing_ok=no))
	CC="$ac_save_cc"
	AC_MSG_RESULT($ac_cv_no_strict_aliasing_ok)
	if test $ac_cv_no_strict_aliasing_ok = yes
	then
	  PYEXTRAFLAGS="-fno-strict-aliasing"
	fi

	AC_SUBST(PYEXTRAFLAGS)


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
else
	have_python="no"
fi

AM_CONDITIONAL(HAVE_PYTHON, test "x$enable_python" != "xno")

dnl ****************************************************************************
dnl * --------------------------------------------------------------------------
dnl ****************************************************************************
AC_OUTPUT([
Makefile
libkeybinder/Makefile
python-keybinder/__init__.py
python-keybinder/Makefile
])

echo
echo $PACKAGE v$VERSION
echo

echo Prefix............... : $prefix
echo libkeybinder ........ : yes
echo python-keybinder .... : $have_python
echo 

echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
