bin_PROGRAMS = \
	ambix_info \
	ambix_interleave \
	ambix_deinterleave \
	ambix_test


INCLUDES = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

ambix_info_SOURCES = ambix_info.c

ambix_interleave_SOURCES = ambix_interleave.c
ambix_interleave_CFLAGS = @SNDFILE_CFLAGS@
ambix_interleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix_deinterleave_SOURCES = ambix_deinterleave.c
ambix_deinterleave_CFLAGS = @SNDFILE_CFLAGS@
ambix_deinterleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix_test_SOURCES = ambix_test.c

