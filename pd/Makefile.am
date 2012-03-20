AUTOMAKE_OPTIONS = foreign

pkglib_LTLIBRARIES=

AM_CFLAGS =
AM_LDFLAGS= -module -avoid-version -shared -shrext .@RTE_EXTENSION@

if WINDOWS
AM_LDFLAGS+= -no-undefined
endif

AM_LDFLAGS+= @EXTRA_LTFLAGS@

# RTE
AM_CFLAGS += 
AM_LDFLAGS+= 

pkglib_LTLIBRARIES+= \
        ambix_read~.la \
        ambix_readX~.la

LDADD = $(top_builddir)/libambix/src/libambix.la
INCLUDES = -I$(top_srcdir)/libambix

ambix_read__la_SOURCES		= ambix_read~.c
ambix_readX__la_SOURCES		= ambix_read~.c

