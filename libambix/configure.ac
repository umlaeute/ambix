AC_INIT(libambix.c)
AM_INIT_AUTOMAKE(libambix,0.0.1)
AM_PROG_LIBTOOL

AC_PROG_INSTALL

AC_LANG_C
AC_PROG_CC
AC_PROG_MAKE_SET

AC_HEADER_STDC

/* checks for libsndfile */
AC_CHECK_HEADERS(sndfile.h)
AC_SEARCH_LIBS([sf_open], [sndfile], have_sndfile="yes")
AM_CONDITIONAL(HAVE_SNDFILE, [test "x$have_sndfile" = "xyes"])

AC_OUTPUT(Makefile utils/Makefile)
