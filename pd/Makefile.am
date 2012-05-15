AUTOMAKE_OPTIONS = foreign

rte_LTLIBRARIES=

AM_CFLAGS =
AM_LDFLAGS= -module -avoid-version -shared -shrext .@RTE_EXTENSION@

if WINDOWS
AM_LDFLAGS+= -no-undefined
endif


# RTE
AM_CFLAGS += @RTE_CFLAGS@
AM_LDFLAGS+= @RTE_LIBS@

AM_LDFLAGS+=$(top_builddir)/libambix/src/libambix.la

rte_LTLIBRARIES+= \
        ambix_read~.la \
        ambix_readX~.la

INCLUDES = -I$(top_srcdir)/libambix

ambix_read__la_SOURCES		= ambix_read~.c
ambix_readX__la_SOURCES		= ambix_read~.c




# convenience symlinks
.PHONY: convenience-link clean-convenience-link

convenience-link: $(rte_LTLIBRARIES)
	  @for lib in $^; do \
	   for soname in `echo | $(EGREP) "^dlname=" $$lib | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
	    echo "$$soname: creating convenience link from $(abs_builddir)/.libs to $(builddir)"; \
	    rm -f $$soname ; \
	    test -e $(abs_builddir)/.libs/$$soname && \
	    $(LN_S) $(abs_builddir)/.libs/$$soname $$soname || true; \
	   done; \
	  done

clean-convenience-link:
	  @for lib in $(rte_LTLIBRARIES); do \
	   for soname in `echo | $(EGREP) "^dlname=" $$lib | $(SED) -e "s|^dlname='\(.*\)'|\1|"`; do  \
	    echo "$$soname: cleaning convenience links"; \
	    test -L $$soname && rm -f $$soname || true; \
	  done; done

all-local:: convenience-link

clean-local:: clean-convenience-link


foo:: $(rte_LTLIBRARIES)
