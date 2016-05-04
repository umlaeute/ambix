AUTOMAKE_OPTIONS = foreign serial-tests
SUBDIRS=data

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
AM_TESTS_ENVIRONMENT = TESTDIR=$(srcdir) ./runtest.sh

AM_CPPFLAGS = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

TESTS = \
	pass skip fail

XFAIL_TESTS = fail

check_PROGRAMS= \
	pass skip fail

SOURCES=common.c common.h data.h
noinst_HEADERS = common.h data.h

pass_SOURCES=pass.c
skip_SOURCES=skip.c
fail_SOURCES=fail.c

TESTS += matrix
check_PROGRAMS += matrix
matrix_SOURCES = matrix.c common.c

TESTS += matrices
check_PROGRAMS += matrices
matrices_SOURCES = matrices.c common.c

TESTS += const_matrix
check_PROGRAMS += const_matrix
const_matrix_SOURCES = const_matrix.c common.c

TESTS          += none_float32
check_PROGRAMS += none_float32
none_float32_SOURCES = none_float32.c common_none.c common.c

TESTS          += basic_float32 basic_pcm32 basic_pcm16
check_PROGRAMS += basic_float32 basic_pcm32 basic_pcm16
basic_float32_SOURCES = basic_float32.c common_basic.c common.c
basic_pcm32_SOURCES = basic_pcm32.c common_basic.c common.c
basic_pcm16_SOURCES = basic_pcm16.c common_basic.c common.c

TESTS          += extended_float32_0 extended_pcm32_0 extended_pcm16_0
check_PROGRAMS += extended_float32_0 extended_pcm32_0 extended_pcm16_0
extended_float32_0_SOURCES = extended_float32_0.c common_extended.c common.c
extended_pcm32_0_SOURCES   = extended_pcm32_0.c common_extended.c common.c
extended_pcm16_0_SOURCES   = extended_pcm16_0.c common_extended.c common.c

TESTS          += extended_float32_1024 extended_pcm32_1024 extended_pcm16_1024
check_PROGRAMS += extended_float32_1024 extended_pcm32_1024 extended_pcm16_1024
extended_float32_1024_SOURCES = extended_float32_1024.c common_extended.c common.c
extended_pcm32_1024_SOURCES   = extended_pcm32_1024.c common_extended.c common.c
extended_pcm16_1024_SOURCES   = extended_pcm16_1024.c common_extended.c common.c

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

#basic2extended_float32_1024.c    common_basic2extended.c
TESTS          += basic2extended_float32_1024
check_PROGRAMS += basic2extended_float32_1024
basic2extended_float32_1024_SOURCES = basic2extended_float32_1024.c common_basic2extended.c common.c


.PHONY: buildtests
buildtests: $(check_PROGRAMS)
