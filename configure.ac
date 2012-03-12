AC_INIT(libambix/src/libambix.c)
AM_INIT_AUTOMAKE(libambix,0.0.1)
AM_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libambix/Makefile libambix/src/Makefile])
AC_CONFIG_FILES([utils/Makefile])

AC_PROG_INSTALL

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET

AC_HEADER_STDC

/* check for math */
AC_CHECK_LIB([m],[sqrt])

/* checks for libsndfile */
AC_CHECK_HEADERS(sndfile.h)
AC_SEARCH_LIBS([sf_open], [sndfile], have_sndfile="yes")
AM_CONDITIONAL(HAVE_SNDFILE, [test "x$have_sndfile" = "xyes"])

if test "x$have_sndfile" = "xyes"; then
AC_SEARCH_LIBS([sf_get_chunk_size], [sndfile], [AC_DEFINE([HAVE_SF_GET_CHUNK_SIZE], [1], [Define to 1 if libsndfile can get custom chunks using sf_get_chunk()])])
AC_CHECK_TYPES([SF_UUID_INFO], [], [], [[#include <sndfile.h>]])
AC_CHECK_TYPES([SF_CHUNK_INFO], [], [], [[#include <sndfile.h>]])
fi

AC_SUBST(SNDFILE_CFLAGS)
AC_SUBST(SNDFILE_LDFLAGS)



AC_OUTPUT
