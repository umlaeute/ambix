AC_INIT([ambix],[0.0.1],[zmoelnig@iem.at],
                [libambix],[http://ambisonics.iem.at/xchange/format])
AM_INIT_AUTOMAKE($PACKAGE_NAME,$PACKAGE_VERSION)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libambix/Makefile libambix/src/Makefile libambix/tests/Makefile])
AC_CONFIG_FILES([utils/Makefile utils/jcommon/Makefile])
AC_CONFIG_FILES([doc/Makefile])

AC_CONFIG_FILES([pd/Makefile])

IEM_OPERATING_SYSTEM


AC_PROG_INSTALL

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET


#------------------------------------------------------------------------------------
# Rules for library version information:
#
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public release of
#     your software. More frequent updates are unnecessary, and only guarantee
#     that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, then
#     increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last update,
#     increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then increment
#     age.
#  6. If any interfaces have been removed since the last public release, then set age
#     to 0.

CLEAN_VERSION=`echo $PACKAGE_VERSION | $SED "s/p.*//"`
VERSION_MINOR=`echo $CLEAN_VERSION | $SED "s/.*\.//"`

SHARED_VERSION_INFO="0:$VERSION_MINOR:0"
AC_SUBST(SHARED_VERSION_INFO)

IEM_CHECK_RTE

AC_HEADER_STDC

/* check for math */
AC_CHECK_LIB([m],[sqrt])

/* checks for libsndfile */
tmp_sndfile_CFLAGS="$CFLAGS"
tmp_sndfile_LIBS="$LIBS"
PKG_CHECK_MODULES([SNDFILE], [sndfile], [have_sndfile="yes"], [])
AM_CONDITIONAL(HAVE_SNDFILE, [test "x$have_sndfile" = "xyes"])

AM_CONDITIONAL(DISABLED, [test "xno" = "xyes"])
AM_CONDITIONAL(ENABLED, [test "xyes" = "xyes"])

if test "x$have_sndfile" = "xyes"; then
 CFLAGS="$CFLAGS $SNDFILE_CFLAGS"
 LIBS="$LIBS $SNDFILE_LIBS"
 AC_CHECK_HEADERS(sndfile.h)
 AC_SEARCH_LIBS([sf_set_chunk], [sndfile], [AC_DEFINE([HAVE_SF_SET_CHUNK], [1], [Define to 1 if libsndfile can set custom chunks using sf_set_chunk()])])
 AC_SEARCH_LIBS([sf_get_chunk_iterator], [sndfile], [AC_DEFINE([HAVE_SF_GET_CHUNK_ITERATOR], [1], [Define to 1 if libsndfile can get custom chunks via an iterator])])
 AC_CHECK_TYPES([SF_CHUNK_INFO], [], [], [[#include <sndfile.h>]])

dnl a custom version of libsndfile used sf_command to write UUID-chunks
 AC_CHECK_TYPES([SF_UUID_INFO], [], [], [[#include <sndfile.h>]])
fi

CFLAGS="$tmp_sndfile_CFLAGS"
LIBS="$tmp_sndfile_LIBS"


/* checks for jack */
tmp_jack_CFLAGS="$CFLAGS"
tmp_jack_LIBS="$LIBS"

PKG_CHECK_MODULES([JACK], [jack], [have_jack="yes"], [])
AM_CONDITIONAL(HAVE_JACK, [test "x$have_jack" = "xyes"])

CFLAGS="$tmp_jack_CFLAGS"
LIBS="$tmp_jack_LIBS"



/* checks for samplerate */
tmp_samplerate_CFLAGS="$CFLAGS"
tmp_samplerate_LIBS="$LIBS"

PKG_CHECK_MODULES([SAMPLERATE], [samplerate], [have_samplerate="yes"], [])
AM_CONDITIONAL(HAVE_SAMPLERATE, [test "x$have_samplerate" = "xyes"])

CFLAGS="$tmp_samplerate_CFLAGS"
LIBS="$tmp_samplerate_LIBS"


AX_PTHREAD


AC_OUTPUT
