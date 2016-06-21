EXTRA_DIST=libambix.doxy \
	libambix.incl

html_DATA=
if HAVE_DOXYGEN
html_DATA+=apiref/*
endif

DOXYFILE=$(srcdir)/libambix.doxy

apiref/*: all-local

all-local:
	$(DOXYGEN) $(DOXYFILE)

clean-local:
	-rm -rf apiref

uninstall-local:
	-rm -rf "$(DESTDIR)$(docdir)/apiref"
