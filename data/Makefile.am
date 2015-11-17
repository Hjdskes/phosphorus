@INTLTOOL_DESKTOP_RULE@
DESKTOP_IN_FILES = phosphorus.desktop.in.in
DESKTOP_FILES = $(DESKTOP_IN_FILES:.desktop.in.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST = \
	$(DESKTOP_IN_FILES) \
	phosphorus.gresource.xml

resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(top_srcdir)/data --generate-dependencies $(top_srcdir)/data/phosphorus.gresource.xml)

BUILT_SOURCES = \
	phosphorus-resources.h \
	phosphorus-resources.c

phosphorus-resources.h: $(top_srcdir)/data/phosphorus.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name phosphorus $<
phosphorus-resources.c: $(top_srcdir)/data/phosphorus.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name phosphorus $<

DISTCLEANFILES = \
	$(DESKTOP_FILES)
