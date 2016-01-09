-include $(top_srcdir)/common.mk
-include $(top_srcdir)/git.mk

# FIXME: Gedit/EOG have this, but do not actually install this file...
pkglib_LTLIBRARIES = \
	libphosphorus-plugin.la

headerdir = $(includedir)/phosphorus-$(PHOSPHORUS_API_VERSION)/phosphorus
header_DATA = $(INST_H_FILES)

libphosphorus_plugin_la_SOURCES = \
	ph-plugin.c \
	ph-plugin.h

libphosphorus_plugin_la_CFLAGS = \
	$(PHOSPHORUS_PLUGIN_CFLAGS) \
	$(AM_CFLAGS)

libphosphorus_plugin_la_LIBADD = \
	$(PHOSPHORUS_PLUGIN_LIBS)

libphosphorus_plugin_la_LDFLAGS = \
	-avoid-version -export-dynamic -no-undefined -export-symbols-regex "^ph_.*"

INST_H_FILES = \
	ph-plugin.h

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS = Phosphorus-1.0.gir
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all
INTROSPECTION_COMPILER_ARGS =

Phosphorus-1.0.gir: libphosphorus-plugin.la
Phosphorus_1_0_gir_NAMESPACE = Phosphorus
Phosphorus_1_0_gir_VERSION = 1.0
Phosphorus_1_0_gir_LIBS = libphosphorus-plugin.la
Phosphorus_1_0_gir_FILES = $(libphosphorus_plugin_la_SOURCES)
Phosphorus_1_0_gir_CFLAGS = $(AM_CFLAGS) $(libphosphorus_plugin_la_CFLAGS)
Phosphorus_1_0_gir_INCLUDES = GObject-2.0
Phosphorus_1_0_gir_SCANNERFLAGS = --warn-all --identifier-prefix=Ph $(foreach header,$(INST_H_FILES),--c-include "$(header)")

girdir = $(datadir)/phosphorus/gir-1.0/
gir_DATA = $(INTROSPECTION_GIRS)

typelibsdir = $(libdir)/phosphorus/girepository-1.0/
typelibs_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES = \
	$(gir_DATA) \
	$(typelibs_DATA)

DISTCLEANFILES = \
	$(pkgconfig_DATA)

