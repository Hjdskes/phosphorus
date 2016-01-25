if ENABLE_XLIB

plugin_LTLIBRARIES += xlib/libXLibWallpaperSetter.la

xlib_libXLibWallpaperSetter_la_SOURCES = \
	xlib/XLibWallpaperSetter.c \
	xlib/XLibWallpaperSetter.h

xlib_libXLibWallpaperSetter_la_LDFLAGS = \
	$(PLUGIN_LIBTOOL_FLAGS)

xlib_libXLibWallpaperSetter_la_LIBADD =	\
	$(top_builddir)/src/plugin/libphosphorus-plugin.la \
	$(XLIB_PLUGIN_LIBS) \
	$(PHOSPHORUS_PLUGIN_LIBS)

xlib_libXLibWallpaperSetter_la_CPPFLAGS = \
	-I$(top_builddir)/src/

xlib_libXLibWallpaperSetter_la_CFLAGS = \
	$(XLIB_PLUGIN_CFLAGS) \
	$(PHOSPHORUS_PLUGIN_CFLAGS) \
	$(AM_CFLAGS)

plugin_in_files += xlib/XLibWallpaperSetter.plugin.in

else

dist_plugin_in_files += xlib/XLibWallpaperSetter.plugin.in

endif

