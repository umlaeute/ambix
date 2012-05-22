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
