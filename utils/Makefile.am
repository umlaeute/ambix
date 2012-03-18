bin_PROGRAMS = \
	ambix-info \
	ambix-interleave \
	ambix-deinterleave \
	ambix-test


INCLUDES = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

ambix-info_SOURCES = ambix-info.c

ambix-interleave_SOURCES = ambix-interleave.c
ambix-interleave_CFLAGS = @SNDFILE_CFLAGS@
ambix-interleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix-deinterleave_SOURCES = ambix-deinterleave.c
ambix-deinterleave_CFLAGS = @SNDFILE_CFLAGS@
ambix-deinterleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix-test_SOURCES = ambix-test.c

