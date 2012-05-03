AUTOMAKE_OPTIONS = foreign
INCLUDES = -I$(top_srcdir)/libambix

lib_LTLIBRARIES = libambix.la

libambix_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ -no-undefined
libambix_la_CFLAGS  = -DAMBIX_INTERNAL
libambix_la_CFLAGS += -fvisibility=hidden
libambix_la_LIBADD  = $(LIBM)

libambix_la_SOURCES = libambix.c \
	adaptor.c \
	adaptor_acn.c \
	adaptor_fuma.c \
	matrix.c \
	utils.c \
	uuid_chunk.c \
	private.h

if HAVE_SNDFILE
libambix_la_SOURCES += sndfile.c
libambix_la_CFLAGS += @SNDFILE_CFLAGS@
libambix_la_LIBADD += @SNDFILE_LIBS@
else
libambix_la_SOURCES += null.c
endif




