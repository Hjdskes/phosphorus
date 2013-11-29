dnl
dnl init phase
dnl
AC_INIT([Phosphorus], [0.1], [jthidskes@outlook.com])
AC_CONFIG_SRCDIR([src/phosphorus.c])
AM_INIT_AUTOMAKE([-Wall])
AM_SILENT_RULES([yes])

dnl
dnl shopping list
dnl
AC_PROG_CC
PKG_PROG_PKG_CONFIG

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

GDK_PIXBUF_VERSION=2.26.1
GTK3_VERSION=3.4.2

AC_SUBST(GDK_PIXBUF_VERSION)
AC_SUBST(GTK3_VERSION)

PKG_CHECK_MODULES(GTK3,[gtk+-3.0 >= $GTK3_VERSION])
PKG_CHECK_MODULES(GDK_PIXBUF,[gdk-pixbuf-xlib-2.0 >= $GDK_PIXBUF_VERSION])

PHOSPHORUS_LIBS="$GTK3_LIBS $GDK_PIXBUF_LIBS"
PHOSPHORUS_CFLAGS="$GTK3_CFLAGS $GDK_PIXBUF_CFLAGS"
AC_CHECK_LIB(X11, XOpenDisplay, [PHOSPHORUS_LIBS="$PHOSPHORUS_LIBS -lX11"])
AC_CHECK_LIB(m, ceil, [PHOSPHORUS_LIBS="$PHOSPHORUS_LIBS -lm"])

PHOSPHORUS_CFLAGS="-std=c99 -Wextra -Wno-unused-parameter $PHOSPHORUS_CFLAGS"

AC_SUBST(PHOSPHORUS_LIBS)
AC_SUBST(PHOSPHORUS_CFLAGS)
dnl
dnl output phase
dnl
AC_CONFIG_FILES([Makefile src/Makefile data/Makefile data/phosphorus.desktop data/icons/Makefile po/Makefile.in])
AC_OUTPUT

echo ""
echo "You can now type \"make\" to compile Phosphorus"
