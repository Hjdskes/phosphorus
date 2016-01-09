SUBDIRS = plugin

-include $(top_srcdir)/common.mk
-include $(top_srcdir)/git.mk

bin_PROGRAMS = \
	phosphorus

# FIXME: make dist fails here:
#        No rule to make target '../data/phosphorus-resources.c', needed by 'distdir'.
phosphorus_SOURCES = \
	../data/phosphorus-resources.c \
	ph-application.c \
	ph-application.h \
	ph-thumbview.c \
	ph-thumbview.h \
	ph-window.c \
	ph-window.h \
	util.c \
	util.h \
	main.c

phosphorus_CPPFLAGS = \
	-Iplugin \
	-DLOCALE_DIR=\"$(localedir)\" \
	-DDATA_DIR=\"$(datadir)\" \
	-DLIB_DIR=\"$(libdir)\"

phosphorus_CFLAGS = \
	$(PHOSPHORUS_BASE_CFLAGS) \
	$(PHOSPHORUS_PLUGIN_CFLAGS) \
	$(AM_CFLAGS)

phosphorus_LDADD = \
	plugin/libphosphorus-plugin.la \
	$(PHOSPHORUS_BASE_LIBS) \
	$(PHOSPHORUS_PLUGIN_LIBS)

phosphorus_LDFLAGS = \
	-export-dynamic -no-undefined

