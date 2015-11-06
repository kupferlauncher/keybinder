AC_INIT(keybinder, 0.3.1,
    [])
AC_CONFIG_SRCDIR(libkeybinder/bind.c)
AC_CONFIG_MACRO_DIR([m4])

AC_PREREQ(2.62)
AM_INIT_AUTOMAKE([-Wno-portability])

# Libtool version
# For any change: increment revision
# If interfaces are added/changed/removed: increment current, reset revision
# If library changed but is backwards-compatible: increment age
# If library changed backwards-incompatibly: reset age
m4_define([keybinder_lt_current],  [1])
m4_define([keybinder_lt_revision], [0])
m4_define([keybinder_lt_age],      [1])
LT_CURRENT=keybinder_lt_current
LT_REVISION=keybinder_lt_revision
LT_AGE=keybinder_lt_age
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

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
PACKAGE=$PACKAGE_NAME
PACKAGE_TARNAME=$PACKAGE_NAME
AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_TARNAME)

AS_AC_EXPAND(DATADIR, $datarootdir)
AC_SUBST(DATADIR)

AS_AC_EXPAND(LIBDIR, $libdir)
AC_SUBST(LIBDIR)

dnl ****************************************************************************
dnl * GTK
dnl ****************************************************************************
gtk_version="gtk+-2.0"
PKG_CHECK_MODULES(GTK, [gtk+-2.0	>= 2.20])

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)


dnl **
dnl * Required gtk package (for keybinder.pc)
dnl **
AS_AC_EXPAND(GTKPACKAGEDEP, $gtk_version)
AC_SUBST(GTKPACKAGEDEP)

dnl * gobject-introspection
GOBJECT_INTROSPECTION_CHECK([0.6.7])

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
                            [disable python bindings [default=yes]]),
			    ,
                            [enable_python=yes])

AC_ARG_ENABLE(lua,
             AS_HELP_STRING([--disable-lua],
                            [disable lua bindings [default=if found]]),
			    [enable_lua=no],
                            [enable_lua=yes])

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
	AC_PATH_PROG(PYGOBJECT_CODEGEN, pygobject-codegen-2.0, no)
	if test "x$PYGOBJECT_CODEGEN" = xno; then
	  AC_MSG_ERROR(could not find pygobject-codegen-2.0 script)
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
dnl  Check for Lua 5.1
dnl ****************************************************************************

if test "x$enable_lua" != "xno"; then
  AX_LUA_HEADERS
  if test "x$ac_cv_header_lua_h" = "xyes" ; then
    AX_LUA_LIB_VERSION([501], [502])
    LUA_VERSION=[5.1]
    AC_SUBST(LUA_VERSION)
    AC_SUBST(LUA_INCLUDE)
    have_lua="yes"
  else
    have_lua="no"
  fi
else
  have_lua="no"
fi
AM_CONDITIONAL(HAVE_LUA, test "x$have_lua" = "xyes")

dnl ***
dnl * GTK DOC
dnl ***

GTK_DOC_CHECK([1.14], [--flavour no-tmpl])

dnl ****************************************************************************
dnl * --------------------------------------------------------------------------
dnl ****************************************************************************
AC_OUTPUT([
Makefile
libkeybinder/Makefile
libkeybinder/$PACKAGE_NAME.pc
lua-keybinder/Makefile
python-keybinder/__init__.py
python-keybinder/Makefile
docs/Makefile
docs/version.info
])

echo
echo $PACKAGE v$VERSION
echo

echo Prefix............... : $prefix
echo GTK version ......... : $gtk_version
echo libkeybinder ........ : yes
echo gobject-introspection : $found_introspection
echo gtk-doc ............. : $enable_gtk_doc
echo python-keybinder .... : $have_python
echo lua-keybinder ....... : $have_lua
echo 

echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
