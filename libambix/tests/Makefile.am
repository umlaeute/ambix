AUTOMAKE_OPTIONS = foreign

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
TESTS_ENVIRONMENT = TESTDIR=$(srcdir)

INCLUDES = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

TESTS=success skip fail 

XFAIL_TESTS = fail

check_PROGRAMS=success skip fail

success_SOURCES=success.c
skip_SOURCES=skip.c
fail_SOURCES=fail.c
