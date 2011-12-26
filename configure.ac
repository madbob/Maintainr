m4_define([maintainr_major_version], [0])
m4_define([maintainr_minor_version], [1])
m4_define([maintainr_micro_version], [0])

m4_define([maintainr_version],
          [maintainr_major_version.maintainr_minor_version.maintainr_micro_version])

m4_define([maintainr_interface_age], [0])
m4_define([maintainr_binary_age],
          [m4_eval(100 * maintainr_minor_version + maintainr_micro_version)])

m4_define([lt_current],
          [m4_eval(100 * maintainr_minor_version + maintainr_micro_version - maintainr_interface_age)])
m4_define([lt_revision], [maintainr_interface_age])
m4_define([lt_age],
          [m4_eval(maintainr_binary_age - maintainr_interface_age)])

m4_define([gtk_req_version], [3.1.0])
m4_define([xml_req_version], [2.7.4])
m4_define([grss_req_version], [0.5.0])
m4_define([opengdesktop_req_version], [0.5.0])

AC_PREREQ([2.59])
AC_INIT([maintainr], [maintainr_version], [], [maintainr])

AM_INIT_AUTOMAKE([1.10])

GNOME_COMMON_INIT
GNOME_COMPILE_WARNINGS
GNOME_DEBUG_CHECK

IT_PROG_INTLTOOL([0.35.0])

AM_DISABLE_STATIC
AM_PATH_GLIB_2_0
AM_PROG_CC_C_O
AM_PROG_LIBTOOL

AC_HEADER_STDC
AC_C_CONST

MAINTAINR_MAJOR_VERSION=maintainr_major_version
MAINTAINR_MINOR_VERSION=maintainr_minor_version
MAINTAINR_MICRO_VERSION=maintainr_micro_version
MAINTAINR_VERSION=maintainr_version
AC_SUBST(MAINTAINR_MAJOR_VERSION)
AC_SUBST(MAINTAINR_MICRO_VERSION)
AC_SUBST(MAINTAINR_MINOR_VERSION)
AC_SUBST(MAINTAINR_VERSION)

dnl maintainr checks
PKG_CHECK_MODULES(MAINTAINR,
                  gtk+-3.0 >= gtk_req_version dnl
                  libxml-2.0 >= xml_req_version dnl
                  libgrss-0 >= grss_req_version dnl
                  libopengdesktop-0 >= opengdesktop_req_version)
AC_SUBST(MAINTAINR_CFLAGS)
AC_SUBST(MAINTAINR_LIBS)

dnl = Enable debug level ===================================================

m4_define([debug_default],
          m4_if(m4_eval(maintainr_minor_version % 2), [1], [yes], [minimum]))

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug=@<:@no/minimum/yes@:>@],
                             [turn on debugging @<:@default=debug_default@:>@]),
,
              enable_debug=debug_default)

if test "x$enable_debug" = "xyes"; then
  test "$cflags_set" = set || CFLAGS="$CFLAGS -g"
  MAINTAINR_DEBUG_CFLAGS="-DMAINTAINR_ENABLE_DEBUG"
else
  if test "x$enable_debug" = "xno"; then
    MAINTAINR_DEBUG_CFLAGS="-DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
  else # minimum
    MAINTAINR_DEBUG_CFLAGS="-DMAINTAINR_ENABLE_DEBUG -DG_DISABLE_CAST_CHECKS"
  fi
fi

AC_SUBST(MAINTAINR_DEBUG_CFLAGS)

dnl = Enable strict compiler flags =========================================

# use strict compiler flags only on development releases
#m4_define([maintainer_flags_default],
#          m4_if(m4_eval(maintainr_minor_version % 2), [1], [yes], [no]))
m4_define([maintainer_flags_default], [no])
AC_ARG_ENABLE([maintainer-flags],
              AC_HELP_STRING([--enable-maintainer-flags=@<:@no/yes@:>@],
                             [Use strict compiler flags @<:@default=maintainer_flags_default@:>@]),,
              enable_maintainer_flags=maintainer_flags_default)

if test "x$enable_maintainer_flags" = "xyes"; then
  CPPFLAGS="$CPPFLAGS -g -Wall -Wshadow -Wcast-align -Wno-uninitialized -Werror"
else
  CPPFLAGS="$CPPFLAGS -g -Wall"
fi

dnl Internationalization
GETTEXT_PACKAGE=maintainr
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE", [Define the gettext package to use])
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        po/Makefile.in
])

AC_OUTPUT

echo ""
echo "              Maintainr: $VERSION"
echo ""
echo "                 Prefix: ${prefix}"
echo "         Compiler flags: ${CPPFLAGS}"
echo ""
