#AUTOMAKE_OPTIONS = foreign serial-tests
AUTOMAKE_OPTIONS = foreign
SUBDIRS=data

#TESTS_ENVIRONMENT = RUNTESTS_NOLOG=yes TESTDIR=$(srcdir)
#AM_TESTS_ENVIRONMENT = TESTDIR=$(srcdir) ./runtest.sh

@VALGRIND_CHECK_RULES@

AM_CPPFLAGS = -I$(top_srcdir)/libambix
if DEBUG
AM_CPPFLAGS += -I$(top_srcdir)/libambix/src -DAMBIX_INTERNAL
endif
LDADD = $(top_builddir)/libambix/src/libambix.la

check_PROGRAMS =
TESTS = \
	pass skip fail

XFAIL_TESTS = fail

SOURCES=common.c common.h data.h common_basic2extended.h
noinst_HEADERS = common.h data.h common_basic2extended.h

pass_SOURCES=pass.c
skip_SOURCES=skip.c
fail_SOURCES=fail.c

TESTS += pass_if0 pass_if1 skip_if0 skip_if1 fail_if0 fail_if1
XFAIL_TESTS += fail_if1
pass_if0_SOURCES=pass_if0.c
pass_if1_SOURCES=pass_if1.c
skip_if0_SOURCES=skip_if0.c
skip_if1_SOURCES=skip_if1.c
fail_if0_SOURCES=fail_if0.c
fail_if1_SOURCES=fail_if1.c


TESTS += datatest
datatest_SOURCES = datatest.c common.c

TESTS += matrix
matrix_SOURCES = matrixtests.c common.c

TESTS += matrices
matrices_SOURCES = matrices.c common.c

TESTS += const_matrix
const_matrix_SOURCES = const_matrix.c common.c

if DEBUG
TESTS += debug_utils
debug_utils_SOURCES = debug_utils.c common.c
endif

TESTS          += markers_regions
markers_regions_SOURCES = markers_regions.c common.c

TESTS          += none_float32
none_float32_SOURCES = none_float32.c common_none.c common.c
TESTS          += none_float64
none_float64_SOURCES = none_float64.c common_none.c common.c

TESTS          += basic_float32 basic_float64 basic_pcm32 basic_pcm16
basic_float32_SOURCES = basic_float32.c common_basic.c common.c
basic_float64_SOURCES = basic_float64.c common_basic.c common.c
basic_pcm32_SOURCES = basic_pcm32.c common_basic.c common.c
basic_pcm16_SOURCES = basic_pcm16.c common_basic.c common.c

TESTS          += extended_float32_0 extended_float64_0 extended_pcm32_0 extended_pcm16_0
extended_float32_0_SOURCES = extended_float32_0.c common_extended.c common.c
extended_float64_0_SOURCES = extended_float64_0.c common_extended.c common.c
extended_pcm32_0_SOURCES   = extended_pcm32_0.c common_extended.c common.c
extended_pcm16_0_SOURCES   = extended_pcm16_0.c common_extended.c common.c

TESTS          += extended_float32_1024 extended_float64_1024 extended_pcm32_1024 extended_pcm16_1024
extended_float32_1024_SOURCES = extended_float32_1024.c common_extended.c common.c
extended_float64_1024_SOURCES = extended_float64_1024.c common_extended.c common.c
extended_pcm32_1024_SOURCES   = extended_pcm32_1024.c common_extended.c common.c
extended_pcm16_1024_SOURCES   = extended_pcm16_1024.c common_extended.c common.c

TESTS += ambix_open
ambix_open_SOURCES = ambix_open.c

TESTS += ambix_close
ambix_close_SOURCES = ambix_close.c

TESTS += ambix_get_sndfile
ambix_get_sndfile_SOURCES = ambix_get_sndfile.c

TESTS += ambix_seek
ambix_seek_SOURCES = ambix_seek.c

TESTS += ambix_seek_read
ambix_seek_read_SOURCES = ambix_seek_read.c

TESTS += ambix_readf_int16
ambix_readf_int16_SOURCES = ambix_readf_int16.c

TESTS += ambix_writef_int16
ambix_writef_int16_SOURCES = ambix_writef_int16.c

common_b2x=common_basic2extended.c common.c
## float32
TESTS          += \
	basic2extended_identity4x4_float32 \
	basic2extended_N3D4x4_float32 \
	basic2extended_SID4x4_float32 \
	basic2extended_FUMA4x4_float32 \
	basic2extended_rand4x9_float32 \
	basic2extended_rand4x7_float32 \
	basic2extended_identity4x4_3extra_float32 \
	basic2extended_N3D4x4_3extra_float32 \
	basic2extended_SID4x4_3extra_float32 \
	basic2extended_FUMA4x4_3extra_float32 \
	basic2extended_rand4x9_3extra_float32 \
	basic2extended_rand4x7_3extra_float32
basic2extended_identity4x4_float32_SOURCES = basic2extended_identity4x4_float32.c $(common_b2x)
basic2extended_N3D4x4_float32_SOURCES = basic2extended_N3D4x4_float32.c $(common_b2x)
basic2extended_SID4x4_float32_SOURCES = basic2extended_SID4x4_float32.c $(common_b2x)
basic2extended_FUMA4x4_float32_SOURCES = basic2extended_FUMA4x4_float32.c $(common_b2x)
basic2extended_rand4x9_float32_SOURCES = basic2extended_rand4x9_float32.c $(common_b2x)
basic2extended_rand4x7_float32_SOURCES = basic2extended_rand4x7_float32.c $(common_b2x)
basic2extended_identity4x4_3extra_float32_SOURCES = basic2extended_identity4x4_3extra_float32.c $(common_b2x)
basic2extended_N3D4x4_3extra_float32_SOURCES = basic2extended_N3D4x4_3extra_float32.c $(common_b2x)
basic2extended_SID4x4_3extra_float32_SOURCES = basic2extended_SID4x4_3extra_float32.c $(common_b2x)
basic2extended_FUMA4x4_3extra_float32_SOURCES = basic2extended_FUMA4x4_3extra_float32.c $(common_b2x)
basic2extended_rand4x9_3extra_float32_SOURCES = basic2extended_rand4x9_3extra_float32.c $(common_b2x)
basic2extended_rand4x7_3extra_float32_SOURCES = basic2extended_rand4x7_3extra_float32.c $(common_b2x)

TESTS          += \
	basic2extended_identity4x4_float32__f64 \
	basic2extended_identity4x4_float32__i16 \
	basic2extended_identity4x4_float32__i32
basic2extended_identity4x4_float32__f64_SOURCES = basic2extended_identity4x4_float32__f64.c $(common_b2x)
basic2extended_identity4x4_float32__i16_SOURCES = basic2extended_identity4x4_float32__i16.c $(common_b2x)
basic2extended_identity4x4_float32__i32_SOURCES = basic2extended_identity4x4_float32__i32.c $(common_b2x)

## float64
TESTS          += \
	basic2extended_identity4x4_float64 \
	basic2extended_N3D4x4_float64 \
	basic2extended_SID4x4_float64 \
	basic2extended_FUMA4x4_float64 \
	basic2extended_rand4x9_float64 \
	basic2extended_rand4x7_float64 \
	basic2extended_identity4x4_3extra_float64 \
	basic2extended_N3D4x4_3extra_float64 \
	basic2extended_SID4x4_3extra_float64 \
	basic2extended_FUMA4x4_3extra_float64 \
	basic2extended_rand4x9_3extra_float64 \
	basic2extended_rand4x7_3extra_float64
basic2extended_identity4x4_float64_SOURCES = basic2extended_identity4x4_float64.c $(common_b2x)
basic2extended_N3D4x4_float64_SOURCES = basic2extended_N3D4x4_float64.c $(common_b2x)
basic2extended_SID4x4_float64_SOURCES = basic2extended_SID4x4_float64.c $(common_b2x)
basic2extended_FUMA4x4_float64_SOURCES = basic2extended_FUMA4x4_float64.c $(common_b2x)
basic2extended_rand4x9_float64_SOURCES = basic2extended_rand4x9_float64.c $(common_b2x)
basic2extended_rand4x7_float64_SOURCES = basic2extended_rand4x7_float64.c $(common_b2x)
basic2extended_identity4x4_3extra_float64_SOURCES = basic2extended_identity4x4_3extra_float64.c $(common_b2x)
basic2extended_N3D4x4_3extra_float64_SOURCES = basic2extended_N3D4x4_3extra_float64.c $(common_b2x)
basic2extended_SID4x4_3extra_float64_SOURCES = basic2extended_SID4x4_3extra_float64.c $(common_b2x)
basic2extended_FUMA4x4_3extra_float64_SOURCES = basic2extended_FUMA4x4_3extra_float64.c $(common_b2x)
basic2extended_rand4x9_3extra_float64_SOURCES = basic2extended_rand4x9_3extra_float64.c $(common_b2x)
basic2extended_rand4x7_3extra_float64_SOURCES = basic2extended_rand4x7_3extra_float64.c $(common_b2x)

## pcm32
TESTS          += \
	basic2extended_identity4x4_pcm32 \
	basic2extended_N3D4x4_pcm32 \
	basic2extended_SID4x4_pcm32 \
	basic2extended_FUMA4x4_pcm32 \
	basic2extended_rand4x9_pcm32 \
	basic2extended_rand4x7_pcm32 \
	basic2extended_identity4x4_3extra_pcm32 \
	basic2extended_N3D4x4_3extra_pcm32 \
	basic2extended_SID4x4_3extra_pcm32 \
	basic2extended_FUMA4x4_3extra_pcm32 \
	basic2extended_rand4x9_3extra_pcm32 \
	basic2extended_rand4x7_3extra_pcm32
basic2extended_identity4x4_pcm32_SOURCES = basic2extended_identity4x4_pcm32.c $(common_b2x)
basic2extended_N3D4x4_pcm32_SOURCES = basic2extended_N3D4x4_pcm32.c $(common_b2x)
basic2extended_SID4x4_pcm32_SOURCES = basic2extended_SID4x4_pcm32.c $(common_b2x)
basic2extended_FUMA4x4_pcm32_SOURCES = basic2extended_FUMA4x4_pcm32.c $(common_b2x)
basic2extended_rand4x9_pcm32_SOURCES = basic2extended_rand4x9_pcm32.c $(common_b2x)
basic2extended_rand4x7_pcm32_SOURCES = basic2extended_rand4x7_pcm32.c $(common_b2x)
basic2extended_identity4x4_3extra_pcm32_SOURCES = basic2extended_identity4x4_3extra_pcm32.c $(common_b2x)
basic2extended_N3D4x4_3extra_pcm32_SOURCES = basic2extended_N3D4x4_3extra_pcm32.c $(common_b2x)
basic2extended_SID4x4_3extra_pcm32_SOURCES = basic2extended_SID4x4_3extra_pcm32.c $(common_b2x)
basic2extended_FUMA4x4_3extra_pcm32_SOURCES = basic2extended_FUMA4x4_3extra_pcm32.c $(common_b2x)
basic2extended_rand4x9_3extra_pcm32_SOURCES = basic2extended_rand4x9_3extra_pcm32.c $(common_b2x)
basic2extended_rand4x7_3extra_pcm32_SOURCES = basic2extended_rand4x7_3extra_pcm32.c $(common_b2x)

## pcm16
TESTS          += \
	basic2extended_identity4x4_pcm16 \
	basic2extended_N3D4x4_pcm16 \
	basic2extended_SID4x4_pcm16 \
	basic2extended_FUMA4x4_pcm16 \
	basic2extended_rand4x9_pcm16 \
	basic2extended_rand4x7_pcm16 \
	basic2extended_identity4x4_3extra_pcm16 \
	basic2extended_N3D4x4_3extra_pcm16 \
	basic2extended_SID4x4_3extra_pcm16 \
	basic2extended_FUMA4x4_3extra_pcm16 \
	basic2extended_rand4x9_3extra_pcm16 \
	basic2extended_rand4x7_3extra_pcm16
basic2extended_identity4x4_pcm16_SOURCES = basic2extended_identity4x4_pcm16.c $(common_b2x)
basic2extended_N3D4x4_pcm16_SOURCES = basic2extended_N3D4x4_pcm16.c $(common_b2x)
basic2extended_SID4x4_pcm16_SOURCES = basic2extended_SID4x4_pcm16.c $(common_b2x)
basic2extended_FUMA4x4_pcm16_SOURCES = basic2extended_FUMA4x4_pcm16.c $(common_b2x)
basic2extended_rand4x9_pcm16_SOURCES = basic2extended_rand4x9_pcm16.c $(common_b2x)
basic2extended_rand4x7_pcm16_SOURCES = basic2extended_rand4x7_pcm16.c $(common_b2x)
basic2extended_identity4x4_3extra_pcm16_SOURCES = basic2extended_identity4x4_3extra_pcm16.c $(common_b2x)
basic2extended_N3D4x4_3extra_pcm16_SOURCES = basic2extended_N3D4x4_3extra_pcm16.c $(common_b2x)
basic2extended_SID4x4_3extra_pcm16_SOURCES = basic2extended_SID4x4_3extra_pcm16.c $(common_b2x)
basic2extended_FUMA4x4_3extra_pcm16_SOURCES = basic2extended_FUMA4x4_3extra_pcm16.c $(common_b2x)
basic2extended_rand4x9_3extra_pcm16_SOURCES = basic2extended_rand4x9_3extra_pcm16.c $(common_b2x)
basic2extended_rand4x7_3extra_pcm16_SOURCES = basic2extended_rand4x7_3extra_pcm16.c $(common_b2x)


# ##################################################
check_PROGRAMS += $(TESTS)

.PHONY: buildtests
buildtests: $(check_PROGRAMS)
