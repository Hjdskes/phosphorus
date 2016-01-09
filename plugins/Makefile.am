plugindir = $(PHOSPHORUS_PLUGINS_LIBS_DIR)
PLUGIN_LIBTOOL_FLAGS=-module -avoid-version

plugin_in_files               =
dist_plugin_in_files          =
plugin_LTLIBRARIES            =
plugin_gsettings_SCHEMAS      =
dist_plugin_gsettings_SCHEMAS =

include gnome/Makefile.am
include xlib/Makefile.am

# GSettings schemas
@GSETTINGS_RULES@
gsettings_SCHEMAS = $(plugin_gsettings_SCHEMAS)

%.plugin: %.plugin.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po)
	$(AM_V_GEN) \
	$(MKDIR_P) "$(dir $@)"; \
	$(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

GENERATED_PLUGIN_FILES = $(plugin_in_files:.plugin.in=.plugin)

plugin_DATA = $(GENERATED_PLUGIN_FILES)

DISTCLEANFILES = \
	$(GENERATED_PLUGIN_FILES)

CLEANFILES = \
	$(GENERATED_PLUGIN_FILES)

EXTRA_DIST = \
	$(plugin_in_files) \
	$(dist_plugin_in_files) \
	$(plugin_gsettings_SCHEMAS:%.gschema.xml=%.gschema.xml.in) \
	$(dist_plugin_gsettings_SCHEMAS:%.gschema.xml=%.gschema.xml.in)

-include $(top_srcdir)/git.mk

