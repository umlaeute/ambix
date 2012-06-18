SUBDIRS =

bin_PROGRAMS = \
	ambix-info

noinst_PROGRAMS = \
	ambix-dump \
	ambix-matrix \
	ambix-test

if HAVE_SNDFILE
bin_PROGRAMS += \
	ambix-interleave \
	ambix-deinterleave
if HAVE_JACK
SUBDIRS += jcommon
bin_PROGRAMS += \
	ambix-jplay \
	ambix-jrecord
endif HAVE_JACK
endif HAVE_SNDFILE

AM_CPPFLAGS = -I$(top_srcdir)/libambix -I$(top_srcdir)
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

ambix_matrix_SOURCES = ambix-matrix.c


ambix_jplay_CFLAGS = @JACK_CFLAGS@ @SAMPLERATE_CFLAGS@ @PTHREAD_CFLAGS@
ambix_jplay_LDADD = $(top_builddir)/libambix/src/libambix.la \
	$(builddir)/jcommon/libjcommon.la \
	@JACK_LIBS@ @SAMPLERATE_LIBS@ @PTHREAD_LIBS@
ambix_jplay_SOURCES = ambix-jplay.c

ambix_jrecord_CFLAGS = @JACK_CFLAGS@ @SAMPLERATE_CFLAGS@ @PTHREAD_CFLAGS@ @SNDFILE_CFLAGS@
ambix_jrecord_LDADD = $(top_builddir)/libambix/src/libambix.la \
	$(builddir)/jcommon/libjcommon.la \
	@JACK_LIBS@ @SAMPLERATE_LIBS@ @PTHREAD_LIBS@ @SNDFILE_LIBS@
ambix_jrecord_SOURCES = ambix-jrecord.c
