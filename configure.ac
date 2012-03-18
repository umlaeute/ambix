AC_INIT(libambix/src/libambix.c)
AM_INIT_AUTOMAKE(libambix,0.0.1)
AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libambix/Makefile libambix/src/Makefile libambix/tests/Makefile])
AC_CONFIG_FILES([utils/Makefile])

AC_PROG_INSTALL

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET

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



AC_OUTPUT
