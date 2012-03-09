AUTOMAKE_OPTIONS = foreign
lib_LTLIBRARIES = libambix.la

libambix_la_SOURCES = libambix.c \
	matrix.c \
	uuid_chunk.c

include_HEADERS = ambix/ambix.h \
	ambix/exportdefs.h

libambix_la_LDFLAGS = -version-info 0:0:0

libambix_la_CFLAGS = -DAMBIX_INTERNAL
