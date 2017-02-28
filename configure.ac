AC_INIT(keybinder-3.0, 0.3.2,
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
m4_define([keybinder_lt_current],  [0])
m4_define([keybinder_lt_revision], [0])
m4_define([keybinder_lt_age],      [0])
LT_CURRENT=keybinder_lt_current
LT_REVISION=keybinder_lt_revision
LT_AGE=keybinder_lt_age
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

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
gtk_version="gtk+-3.0"
PKG_CHECK_MODULES(GTK, [gtk+-3.0	>= 3.0])

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
echo 

echo "Now type make to compile"
echo "Then su to root and type: make install"
echo
