AUTOMAKE_OPTIONS = foreign
INCLUDES = -I$(top_srcdir)

lib_LTLIBRARIES = libambix.la

libambix_la_SOURCES = libambix.c \
	matrix.c \
	utils.c \
	uuid_chunk.c \
	private.h


if HAVE_SNDFILE
libambix_la_SOURCES += sndfile.c
else
libambix_la_SOURCES += dummy.c
endif


libambix_la_LDFLAGS = -version-info 0:0:0

libambix_la_CFLAGS  = -DAMBIX_INTERNAL
libambix_la_CFLAGS += -fvisibility=hidden
