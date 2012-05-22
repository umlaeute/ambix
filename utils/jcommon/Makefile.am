noinst_LTLIBRARIES = libjcommon.la

libjcommon_la_CFLAGS =
libjcommon_la_LIBADD =

libjcommon_la_CFLAGS += \
	@SAMPLERATE_CFLAGS@ @PTHREAD_CFLAGS@ \
	@JACK_CFLAGS@

libjcommon_la_LIBADD += $(top_builddir)/libambix/src/libambix.la \
	@SAMPLERATE_LIBS@ @PTHREAD_LIBS@ \
	@JACK_LIBS@

libjcommon_la_SOURCES = \
	common.c \
	common.h \
	jack-ringbuffer.c \
	jack-ringbuffer.h \
	observe-signal.c \
	observe-signal.h
