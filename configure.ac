AC_PREREQ([2.59])

# Making releases:
#   set the new version number:
#     odd minor -> development series
#     even minor -> stable series
#     increment micro for each release within a series
#   set nano_version to 0
#   make the release, tag it
#   set nano_version to 1

m4_define([tp_ytstenut_major_version], [0])
m4_define([tp_ytstenut_minor_version], [2])
m4_define([tp_ytstenut_micro_version], [0])
m4_define([tp_ytstenut_nano_version], [1])

# If library source has changed since last release, increment revision
# If interfaces have been added, removed or changed since last release,
#  increment current and set revision to 0
# If interfaces have been added since last release, increment age
# If interfaces have been removed since last release, set age to 0
#
# If interfaces have been added, release builds will fail unless you add a new
# file like TelepathyQt4/versions/0.1.1.abi to add them to the official ABI.
# This also forces binaries built against devel versions to be rebuilt
# (we don't guarantee that we won't add ABI then remove it again, if it was
# never seen in a release).

m4_define([tp_ytstenut_lt_current], [0])
m4_define([tp_ytstenut_lt_revision], [0])
m4_define([tp_ytstenut_lt_age], [0])

# Some magic
m4_define([tp_ytstenut_base_version],
          [tp_ytstenut_major_version.tp_ytstenut_minor_version.tp_ytstenut_micro_version])
m4_define([tp_ytstenut_version],
          [m4_if(tp_ytstenut_nano_version, 0, [tp_ytstenut_base_version],
              [tp_ytstenut_base_version].[tp_ytstenut_nano_version])])

AC_INIT([Telepathy-Ytstenut], [tp_ytstenut_version], [ytstenut@lists.collabora.co.uk])
AC_LANG([C++])

AC_CONFIG_MACRO_DIR([m4])

# Enable silent build rules by default, requires at least
# Automake-1.11. Disable by either passing --disable-silent-rules to
# configure or passing V=1 to make
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE([1.9 -Wno-portability tar-ustar foreign])
AM_CONFIG_HEADER(config.h)

dnl check for tools
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AM_PROG_LIBTOOL
AM_PROG_MKDIR_P
# pkg-config 0.21 adds CFLAGS from Requires.private, which we need
PKG_PROG_PKG_CONFIG([0.21])

dnl check for a C compiler too, for code taken from telepathy-glib
AC_LANG_PUSH([C])
AC_PROG_CC
AC_PROG_CC_STDC
AM_PROG_CC_C_O
AC_LANG_POP([C])

LT_CURRENT=tp_ytstenut_lt_current
LT_REVISION=tp_ytstenut_lt_revision
LT_AGE=tp_ytstenut_lt_age
AC_SUBST([LT_CURRENT])
AC_SUBST([LT_REVISION])
AC_SUBST([LT_AGE])

dnl optimizations, etc.
COMPILER_OPTIMISATIONS
COMPILER_COVERAGE
LINKER_OPTIMISATIONS
LINKER_VERSION_SCRIPT

ifelse(tp_ytstenut_nano_version, 0,
    [ official_release=yes ],
    [ official_release=no ])
AM_CONDITIONAL([OFFICIAL_RELEASE], [test "x$official_release" = xyes])

TP_COMPILER_WARNINGS([ERROR_CXXFLAGS], [test "x$official_release" = xno],
  [all \
   extra \
   sign-compare \
   pointer-arith \
   format-security \
   init-self],
   [missing-field-initializers \
    unused-parameter])
AC_SUBST([ERROR_CXXFLAGS])

# For C code taken from telepathy-glib, we want different warnings
AC_LANG_PUSH([C])
TP_COMPILER_WARNINGS([ERROR_CFLAGS], [test "x$official_release" = xno],
  [all \
   extra \
   declaration-after-statement \
   shadow \
   strict-prototypes \
   missing-prototypes \
   sign-compare \
   nested-externs \
   pointer-arith \
   format-security \
   init-self],
   [missing-field-initializers \
    unused-parameter])
AC_SUBST([ERROR_CFLAGS])
AC_LANG_POP([C])

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--disable-debug],[compile without debug code]),
    enable_debug=$enableval, enable_debug=yes )

if test x$enable_debug = xyes; then
  AC_DEFINE(ENABLE_DEBUG, [], [Enable debug code])
fi

GTK_DOC_CHECK([1.15])

dnl docs/Makefile.am needs to know whether it's an out of tree build
dnl (srcdir != builddir)
AM_CONDITIONAL([OUT_OF_TREE_BUILD], [test "z$ac_srcdir" != z.])

AC_ARG_ENABLE(glib,
  AC_HELP_STRING([--disable-glib],[compile without telepathy-ytstenut GLib library]),
    enable_glib=$enableval, enable_glib=yes )

AM_CONDITIONAL(ENABLE_GLIB, test x$enable_glib = xyes)

AC_ARG_ENABLE(qt4,
  AC_HELP_STRING([--disable-qt4],[compile without telepathy-ytstenut-qt4 library]),
    enable_qt4=$enableval, enable_qt4=yes )

AM_CONDITIONAL(ENABLE_QT4, test x$enable_qt4 = xyes)

if test x$enable_qt4 = xyes; then
  dnl Check for QtCore
  PKG_CHECK_MODULES(QTCORE, [QtCore >= 4.5])
  AC_SUBST(QTCORE_CFLAGS)
  AC_SUBST(QTCORE_LIBS)

  MOC=`$PKG_CONFIG --variable=moc_location QtCore`
  AC_SUBST(MOC)

  dnl Check for QtDBus
  PKG_CHECK_MODULES(QTDBUS, [QtDBus >= 4.5])
  AC_SUBST(QTDBUS_CFLAGS)
  AC_SUBST(QTDBUS_LIBS)

  dnl Check for TelepathyQt4
  PKG_CHECK_MODULES(TP_QT4, [TelepathyQt4 >= 0.5])
  AC_SUBST(TP_QT4_CFLAGS)
  AC_SUBST(TP_QT4_LIBS)

  dnl We want examples to be able to use TP_YTSTENUT_QT4_CFLAGS as though they
  dnl were independent programs that got them from pkgconfig; AC_SUBST'ing
  dnl TP_YTSTENUT_QT4_LIBS would interfere with dependency tracking, so don't do that
  TP_YTSTENUT_QT4_CFLAGS='-I${top_builddir}/TelepathyYtstenutQt4 -I${top_srcdir}'
  AC_SUBST(TP_YTSTENUT_QT4_CFLAGS)
fi

if test x$enable_glib = xyes; then
   dnl Check for Glib
   PKG_CHECK_MODULES(GLIB, [
	glib-2.0 >= 2.16,
	gobject-2.0 >= 2.16,
	gthread-2.0 >= 2.16,
	gio-2.0 >= 2.16])

  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)

  GLIB_GENMARSHAL=`$PKG_CONFIG --variable=glib_genmarshal glib-2.0`
  AC_SUBST(GLIB_GENMARSHAL)

  dnl Check for D-Bus
  PKG_CHECK_MODULES(DBUS, [dbus-1 >= 1.1.0, dbus-glib-1 >= 0.78])

  AC_SUBST(DBUS_CFLAGS)
  AC_SUBST(DBUS_LIBS)

  dnl Check for telepathy-glib
  PKG_CHECK_MODULES(TP_GLIB, [telepathy-glib >= 0.17.1])

  AC_SUBST(TP_GLIB_CFLAGS)
  AC_SUBST(TP_GLIB_LIBS)
fi

dnl Check for code generation tools
AM_PATH_PYTHON([2.5])

AC_ARG_ENABLE([spec-documentation],
  AC_HELP_STRING([--disable-spec-documentation],
      [do not build HTML documentation of specification]),
  [spec_documentation="$enableval"],
  [spec_documentation=yes])

AS_IF([test "x$spec_documentation" = xyes],
  [AC_CHECK_PROG([CHEETAH], [cheetah], [cheetah])
   if test -z $CHEETAH; then
     AC_MSG_FAILURE(
       [spec documentation enabled, but cheetah was not found]
     )
   fi
  ]
  )

AM_CONDITIONAL(SPEC_DOCUMENTATION, test x$spec_documentation = xyes)

AC_OUTPUT([
    Makefile
    docs/Makefile
    docs/reference/Makefile
    docs/reference/version.xml
    tp-qt4-tools/Makefile
    tp-glib-tools/Makefile
    TelepathyYtstenutQt4/Makefile
    TelepathyYtstenutQt4/TelepathyYtstenutQt4.pc
    TelepathyYtstenutQt4/TelepathyYtstenutQt4-uninstalled.pc
    telepathy-ytstenut-glib/Makefile
    telepathy-ytstenut-glib/telepathy-ytstenut-glib.pc
    telepathy-ytstenut-glib/telepathy-ytstenut-glib-uninstalled.pc
    telepathy-ytstenut-glib/extensions/Makefile
    telepathy-ytstenut-glib/tests/Makefile
])
