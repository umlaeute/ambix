AUTOMAKE_OPTIONS = foreign

rteambixdir = @rtedir@/ambix

rteambix_LTLIBRARIES=
dist_rteambix_DATA=

AM_CFLAGS =
AM_LDFLAGS= -module -avoid-version -shared -shrext .@RTE_EXTENSION@

if WINDOWS
AM_LDFLAGS+= -no-undefined
endif

AM_CPPFLAGS = -I$(top_srcdir)/libambix -I$(top_srcdir)

# RTE
AM_CFLAGS += @RTE_CFLAGS@
AM_LDFLAGS+= @RTE_LIBS@
AM_LDFLAGS+=$(top_builddir)/libambix/src/libambix.la

ambix_read__la_CFLAGS	 = $(AM_CFLAGS) @PTHREAD_CFLAGS@
ambix_read__la_LDFLAGS	 = $(AM_LDFLAGS) @PTHREAD_LIBS@

ambix_readX__la_CFLAGS	 = $(AM_CFLAGS) @PTHREAD_CFLAGS@
ambix_readX__la_LDFLAGS	 = $(AM_LDFLAGS) @PTHREAD_LIBS@

ambix_write__la_CFLAGS	 = $(AM_CFLAGS) @PTHREAD_CFLAGS@
ambix_write__la_LDFLAGS	 = $(AM_LDFLAGS) @PTHREAD_LIBS@

rteambix_LTLIBRARIES+= \
        ambix_info.la \
        ambix_read~.la \
        ambix_readX~.la \
        ambix_write~.la

dist_rteambix_DATA+= \
	ambix_read~-help.pd \
	ambix_readX~-help.pd \
	ambix_write~-help.pd \
	ambix_info-help.pd \
	ambix-meta.pd


ambix_info_la_SOURCES		= ambix_info.c
ambix_read__la_SOURCES		= ambix_read~.c
ambix_readX__la_SOURCES		= ambix_read~.c
ambix_write__la_SOURCES		= ambix_write~.c

noinst_HEADERS			= winhacks.h

if WINDOWS
ambix_info_la_SOURCES		+= winhacks.c 
ambix_read__la_SOURCES		+= winhacks.c 
ambix_readX__la_SOURCES		+= winhacks.c 
ambix_write__la_SOURCES		+= winhacks.c 
endif WINDOWS


# convenience symlinks
.PHONY: convenience-link clean-convenience-link

convenience-link: $(rteambix_LTLIBRARIES)
	  @for lib in $^; do \
	   for soname in `echo | $(EGREP) "^dlname=" $$lib | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
	    echo "$$soname: creating convenience link from $(abs_builddir)/.libs to $(builddir)"; \
	    rm -f $$soname ; \
	    test -e $(abs_builddir)/.libs/$$soname && \
	    $(LN_S) $(abs_builddir)/.libs/$$soname $$soname || true; \
	   done; \
	  done

clean-convenience-link:
	  @for lib in $(rteambix_LTLIBRARIES); do \
	   for soname in `echo | $(EGREP) "^dlname=" $$lib | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
	    echo "$$soname: cleaning convenience links"; \
	    test -L $$soname && rm -f $$soname || true; \
	  done; done

all-local:: convenience-link

clean-local:: clean-convenience-link

