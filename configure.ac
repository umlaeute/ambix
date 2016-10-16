AC_INIT([ambix],[0.1.1],[zmoelnig@iem.at],
                [libambix],[http://ambisonics.iem.at/xchange/format])
AM_INIT_AUTOMAKE([foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libambix/Makefile libambix/src/Makefile libambix/tests/Makefile libambix/tests/data/Makefile])
AC_CONFIG_FILES([libambix/libambix.pc])
AC_CONFIG_FILES([utils/Makefile utils/jcommon/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([replacement/Makefile])

AC_CONFIG_FILES([samples/Makefile])
AC_CONFIG_FILES([samples/pd/Makefile samples/pd/ambix-meta.pd])

AC_CONFIG_FILES([build/Makefile build/w32-vs2008/Makefile])

IEM_OPERATING_SYSTEM


AC_PROG_INSTALL

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

m4_ifdef([AC_PROG_OBJC], [AC_PROG_OBJC])

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

have_pd="no"
IEM_CHECK_RTE
AM_CONDITIONAL(HAVE_PUREDATA, [test "x$have_pd" = "xyes"])

AC_HEADER_STDC

AM_CONDITIONAL(DISABLED, [test "xno" = "xyes"])
AM_CONDITIONAL(ENABLED, [test "xyes" = "xyes"])


AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging build, default: no]),
	       AS_CASE(["$enableval"],
			yes, debug=true,
			no,  debug=false,
			AC_MSG_ERROR([bad value ${enableval} for --enable-debug])),
               [debug=false])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")


## check for math
AC_CHECK_LIB([m],[sqrt])

## checks for libsndfile
AC_ARG_WITH([sndfile],
            [AS_HELP_STRING([--with-sndfile],
              [use libsndfile as backend for reading ambix-files])],
            [],
            [with_sndfile=yes])
have_sndfile="no"
AS_IF([test "x$with_sndfile" != xno], [
tmp_sndfile_CFLAGS="$CFLAGS"
tmp_sndfile_LIBS="$LIBS"
PKG_CHECK_MODULES([SNDFILE], [sndfile], [have_sndfile="yes"], [have_sndfile="no"])

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
])
AM_CONDITIONAL(HAVE_SNDFILE, [test "x$have_sndfile" = "xyes"])

## checks for CoreAudio frameworks
AC_ARG_WITH([CoreAudio],
            [AS_HELP_STRING([--with-CoreAudio],
              [use CoreAudio as backend for reading ambix-files])],
            [],
            [with_CoreAudio=check])
have_audiotoolbox="no"
AS_IF([test "x$with_CoreAudio" != xno],
      [
       IEM_CHECK_FRAMEWORK([Foundation],
		       	   IEM_CHECK_FRAMEWORK([AudioToolbox],
                                               [have_audiotoolbox="yes"])
                          )
      ]
     )
AM_CONDITIONAL([HAVE_FRAMEWORK_AUDIOTOOLBOX], [test "x$have_audiotoolbox" = "xyes"])

## checks for jack
AC_ARG_WITH([jack],
            [AS_HELP_STRING([--with-jack],
              [use JACK for playback/recording of live sound])],
            [],
            [with_jack=yes])
have_jack="no"
AS_IF([test "x$with_jack" != xno], [
tmp_jack_CFLAGS="$CFLAGS"
tmp_jack_LIBS="$LIBS"

PKG_CHECK_MODULES([JACK], [jack], [have_jack="yes"], [have_jack="no"])

CFLAGS="$tmp_jack_CFLAGS"
LIBS="$tmp_jack_LIBS"
])
AM_CONDITIONAL(HAVE_JACK, [test "x$have_jack" = "xyes"])

## checks for samplerate
AC_ARG_WITH([samplerate],
            [AS_HELP_STRING([--with-samplerate],
              [use libsamplerate for samplerate-conversion])],
            [],
            [with_samplerate=yes])
have_samplerate="no"
AS_IF([test "x$with_samplerate" != xno], [
tmp_samplerate_CFLAGS="$CFLAGS"
tmp_samplerate_LIBS="$LIBS"

PKG_CHECK_MODULES([SAMPLERATE], [samplerate], [have_samplerate="yes"], [have_samplerate="no"])

CFLAGS="$tmp_samplerate_CFLAGS"
LIBS="$tmp_samplerate_LIBS"
])
AM_CONDITIONAL(HAVE_SAMPLERATE, [test "x$have_samplerate" = "xyes"])



AC_CHECK_PROGS([DOXYGEN], [doxygen], [true])
AM_CONDITIONAL(HAVE_DOXYGEN, [test "x${DOXYGEN}" != "xtrue"])
AC_SUBST(DOXYGEN)

AC_CHECK_FUNCS([strndup])

AX_PTHREAD

# run unitttests in valgrind
AC_SUBST(VALGRIND_CHECK_RULES)
m4_ifdef([AX_VALGRIND_CHECK], [AX_VALGRIND_CHECK])

AC_OUTPUT
