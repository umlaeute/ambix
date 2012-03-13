AUTOMAKE_OPTIONS = foreign

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
TESTS_ENVIRONMENT = TESTDIR=$(srcdir)

INCLUDES = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

TESTS = \
	pass skip fail

XFAIL_TESTS = fail

check_PROGRAMS= \
	pass skip fail


pass_SOURCES=pass.c
skip_SOURCES=skip.c
fail_SOURCES=fail.c

TESTS += matrix
check_PROGRAMS += matrix
matrix_SOURCES=matrix.c




