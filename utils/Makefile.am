bin_PROGRAMS = \
	ambix-info \
	ambix-interleave \
	ambix-deinterleave \
	ambix-dump \
	ambix-test


INCLUDES = -I$(top_srcdir)/libambix
LDADD = $(top_builddir)/libambix/src/libambix.la

ambix_info_SOURCES = ambix-info.c

ambix_interleave_SOURCES = ambix-interleave.c
ambix_interleave_CFLAGS = @SNDFILE_CFLAGS@
ambix_interleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix_deinterleave_SOURCES = ambix-deinterleave.c
ambix_deinterleave_CFLAGS = @SNDFILE_CFLAGS@
ambix_deinterleave_LDADD = $(top_builddir)/libambix/src/libambix.la @SNDFILE_LIBS@

ambix_test_SOURCES = ambix-test.c

ambix_dump_SOURCES = ambix-dump.c

