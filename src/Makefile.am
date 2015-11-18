-include $(top_srcdir)/common.mk

bin_PROGRAMS = \
	phosphorus

phosphorus_SOURCES = \
	../data/phosphorus-resources.c \
	ph-application.c \
	ph-backend.c \
	ph-backend-gnome.c \
	ph-thumbview.c \
	ph-window.c \
	util.c \
	main.c

phosphorus_CFLAGS = \
	-DLOCALE_DIR=\"$(localedir)\" \
	$(PHOSPHORUS_CFLAGS) \
	$(AM_CFLAGS)

phosphorus_LDADD = \
	$(PHOSPHORUS_LIBS)

phosphorus_LDFLAGS = \
	-export-dynamic -no-undefined

