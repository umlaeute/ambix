AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = file1.caf.org
noinst_DATA=$(EXTRA_DIST:%.org=%)

% :: %.org
	cp $< $@

clean-local:
	rm -rf $(noinst_DATA)
