AUTOMAKE_OPTIONS = foreign serial-tests

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
TESTS_ENVIRONMENT = TESTDIR=$(srcdir) ./runtest.sh

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

TESTS          += none_float32
check_PROGRAMS += none_float32
none_float32_SOURCES = none_float32.c ambix_none.c common.c

TESTS          += simple_float32 simple_pcm32 simple_pcm16
check_PROGRAMS += simple_float32 simple_pcm32 simple_pcm16
simple_float32_SOURCES = simple_float32.c ambix_simple.c common.c
simple_pcm32_SOURCES = simple_pcm32.c ambix_simple.c common.c
simple_pcm16_SOURCES = simple_pcm16.c ambix_simple.c common.c

TESTS          += extended_float32_0 extended_pcm32_0 extended_pcm16_0
check_PROGRAMS += extended_float32_0 extended_pcm32_0 extended_pcm16_0
extended_float32_0_SOURCES = extended_float32_0.c ambix_extended.c common.c
extended_pcm32_0_SOURCES   = extended_pcm32_0.c ambix_extended.c common.c
extended_pcm16_0_SOURCES   = extended_pcm16_0.c ambix_extended.c common.c

TESTS          += extended_float32_1024 extended_pcm32_1024 extended_pcm16_1024
check_PROGRAMS += extended_float32_1024 extended_pcm32_1024 extended_pcm16_1024
extended_float32_1024_SOURCES = extended_float32_1024.c ambix_extended.c common.c
extended_pcm32_1024_SOURCES   = extended_pcm32_1024.c ambix_extended.c common.c
extended_pcm16_1024_SOURCES   = extended_pcm16_1024.c ambix_extended.c common.c

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


.PHONY: buildtests
buildtests: $(check_PROGRAMS)
