noinst_LTLIBRARIES = libjcommon.la

INCLUDES = -I$(top_srcdir)/libambix
CFLAGS =
LDADD =

CFLAGS += \
	@SAMPLERATE_CFLAGS@ @PTHREAD_CFLAGS@ \
	@JACK_CFLAGS@

LDADD += $(top_builddir)/libambix/src/libambix.la \
	@SAMPLERATE_LIBS@ @PTHREAD_LIBS@ \
	@JACK_LIBS@

libjcommon_la_SOURCES = \
	common.c \
	common.h \
	jack-ringbuffer.c \
	jack-ringbuffer.h \
	observe-signal.c \
	observe-signal.h
