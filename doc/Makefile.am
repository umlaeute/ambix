EXTRA_DIST=libambix.doxy \
	libambix.h

html_DATA=apiref/*

apiref/*: all-local

all-local:
	doxygen libambix.doxy

#install-html:
#	test -z "$(DESTDIR)$(docdir)/apiref" || $(mkdir_p) "$(DESTDIR)$(docdir)/apiref"
#	$(INSTALL_DATA) apiref/* $(DESTDIR)$(docdir)/apiref

clean-local:
	-rm -rf apiref

uninstall-local:
	-rm -rf "$(DESTDIR)$(docdir)/apiref"
