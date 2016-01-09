# Desktop file
@INTLTOOL_DESKTOP_RULE@
desktop_in_files = phosphorus.desktop.in.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)

# GSettings schemas
@GSETTINGS_RULES@
gsettings_in_files = org.unia.phosphorus.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_in_files:.xml.in=.xml)

# Pkgconfig file
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = phosphorus-plugin.pc

# Resource files
GRESDEPS = $1 $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(dir $1) $1)
GRESGEN = $(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target="$@" --sourcedir="$(dir $<)" --generate-source "$<"

phosphorus_resource_deps = $(call GRESDEPS,phosphorus.gresource.xml)
phosphorus-resources.c: $(phosphorus_resource_deps)
	$(GRESGEN)

BUILT_SOURCES = phosphorus-resources.c

EXTRA_DIST = \
	$(desktop_in_files) \
	$(gsettings_in_files) \
	phosphorus.gresource.xml \
	$(phosphorus_resource_deps) \
	phosphorus-plugin.pc.in

CLEANFILES = \
	$(gsettings_SCHEMAS) \
	$(BUILT_SOURCES)

DISTCLEANFILES = \
	$(desktop_DATA) \
	$(gsettings_SCHEMAS) \
	$(BUILT_SOURCES)

MAINTAINERCLEANFILES = \
	$(gsettings_in_files:.xml.in=.valid)

-include $(top_srcdir)/git.mk

