AUTOMAKE_OPTIONS = foreign

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
TESTS_ENVIRONMENT = TESTDIR=$(srcdir)

AM_CPPFLAGS = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

TESTS = \
	pass skip fail

XFAIL_TESTS = fail

check_PROGRAMS= \
	pass skip fail

SOURCES=common.c common.h
noinst_HEADERS = common.h 

pass_SOURCES=pass.c
skip_SOURCES=skip.c
fail_SOURCES=fail.c

TESTS += matrix
check_PROGRAMS += matrix
matrix_SOURCES = matrix.c common.c

TESTS += matrices
check_PROGRAMS += matrices
matrices_SOURCES = matrices.c common.c

TESTS += ambix_none
check_PROGRAMS += ambix_none
ambix_none_SOURCES = ambix_none.c common.c

TESTS += ambix_simple
check_PROGRAMS += ambix_simple
ambix_simple_SOURCES = ambix_simple.c common.c

TESTS += ambix_extended
check_PROGRAMS += ambix_extended
ambix_extended_SOURCES = ambix_extended.c common.c

TESTS += ambix_open
check_PROGRAMS += ambix_open
ambix_open_SOURCES = ambix_open.c


TESTS += ambix_close
check_PROGRAMS += ambix_close
ambix_close_SOURCES = ambix_close.c

TESTS += ambix_seek
check_PROGRAMS += ambix_seek
ambix_seek_SOURCES = ambix_seek.c

TESTS += ambix_seek_read
check_PROGRAMS += ambix_seek_read
ambix_seek_read_SOURCES = ambix_seek_read.c

TESTS += ambix_readf_int16
check_PROGRAMS += ambix_readf_int16
ambix_readf_int16_SOURCES = ambix_readf_int16.c

TESTS += ambix_writef_int16
check_PROGRAMS += ambix_writef_int16
ambix_writef_int16_SOURCES = ambix_writef_int16.c

