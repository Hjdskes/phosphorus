-include $(top_srcdir)/common.mk
-include $(top_srcdir)/git.mk

bin_PROGRAMS = \
	phosphorus

phosphorus_SOURCES = \
	main.c

phosphorus_CPPFLAGS = \
	-DLOCALE_DIR=\"$(localedir)\"

phosphorus_CFLAGS = \
	$(PHOSPHORUS_BASE_CFLAGS) \
	$(AM_CFLAGS)

phosphorus_LDADD = \
	libphosphorus.la \
	plugin/libphosphorus-plugin.la

phosphorus_LDFLAGS = \
	-export-dynamic -no-undefined

pkglib_LTLIBRARIES = \
	libphosphorus.la

libphosphorus_la_SOURCES = \
	$(libphosphorus_c_files) \
	$(libphosphorus_NOINST_H_FILES) \
	$(libphosphorus_INST_H_FILES)

libphosphorus_la_CPPFLAGS = \
	-Iplugin \
	-DDATA_DIR=\"$(datadir)\" \
	-DLIB_DIR=\"$(libdir)\"

libphosphorus_la_CFLAGS = \
	$(PHOSPHORUS_BASE_CFLAGS) \
	$(PHOSPHORUS_PLUGIN_CFLAGS) \
	$(AM_CFLAGS)

libphosphorus_la_LIBADD = \
	$(PHOSPHORUS_BASE_LIBS) \
	$(PHOSPHORUS_PLUGIN_LIBS)

libphosphorus_la_LDFLAGS = \
	-avoid-version -export-dynamic -no-undefined -export-symbols-regex "^ph_.*"

# FIXME: make dist fails here:
#        No rule to make target '../data/phosphorus-resources.c', needed by 'distdir'.
libphosphorus_c_files = \
	../data/phosphorus-resources.c \
	ph-application.c \
	ph-plugin-manager.c \
	ph-preferences-dialog.c \
	ph-thumbview.c \
	ph-window.c \
	util.c

libphosphorus_NOINST_H_FILES = \
	ph-plugin-manager.h \
	ph-preferences-dialog.h \
	ph-thumbview.h \
	ph-window.h \
	util.h

libphosphorus_INST_H_FILES = \
	ph-application.h

include plugin/Makefile.am

