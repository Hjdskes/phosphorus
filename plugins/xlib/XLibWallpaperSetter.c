/* XLibWallpaperSetter
 *
 * Copyright (C) 2016 Jente Hidskes
 *
 * Author: Jente Hidskes <hjdskes@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libpeas/peas.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <ph-plugin.h>

#include "XLibWallpaperSetter.h"

static void ph_plugin_interface_init (PhPluginInterface *interface);

struct _XLibWallpaperSetterPrivate {
	Display *display;
};

G_DEFINE_DYNAMIC_TYPE_EXTENDED (XLibWallpaperSetter,
				xlib_wallpaper_setter,
				G_TYPE_OBJECT,
				0,
				G_ADD_PRIVATE_DYNAMIC (XLibWallpaperSetter)
				G_IMPLEMENT_INTERFACE_DYNAMIC (PH_TYPE_PLUGIN,
							       ph_plugin_interface_init))

static void
die (const gchar *msg, ...)
{
	va_list args;

	va_start (args, msg);
	vfprintf (stderr, msg, args);
	va_end (args);

	exit (EXIT_FAILURE);
}

static GdkPixbuf *
open_pixbuf (const gchar *filepath)
{
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	pixbuf = gdk_pixbuf_new_from_file (filepath, &error);
	if (!pixbuf) {
		g_printerr (_("Could not open pixbuf %s\n"), error->message);
		g_clear_error (&error);
		die (NULL);
	}
	return pixbuf;
}

static GdkPixbuf *
scale_pixbuf (GdkPixbuf *pixbuf, int width, int height)
{
	GdkPixbuf *scaled;

	scaled = gdk_pixbuf_scale_simple (pixbuf, width, height, GDK_INTERP_BILINEAR);
	if (!scaled) {
		return pixbuf;
	}
	g_object_unref (pixbuf);
	return scaled;
}

static void
pixbuf_onto_pixmap (GdkPixbuf *pixbuf, Display *display, int screen, Pixmap *pixmap, int width, int height)
{
	cairo_t *context;
	cairo_surface_t *target;
	Visual *visual;

	visual = DefaultVisual (display, screen);

	target = cairo_xlib_surface_create (display, *pixmap, visual, width, height);
	context = cairo_create (target);
	gdk_cairo_set_source_pixbuf (context, pixbuf, 0, 0);
	cairo_paint (context);

	cairo_destroy (context);
	cairo_surface_destroy (target);
}

/* Taken from hsetroot. */
static gboolean
set_root_atoms (Display *display, Window root, Pixmap pixmap)
{
	int format;
	unsigned long length, after;
	Atom atom_root, atom_eroot, type;
	unsigned char *data_root, *data_eroot;

	atom_root = XInternAtom (display, "_XROOTMAP_ID", True);
	atom_eroot = XInternAtom (display, "ESETROOT_PMAP_ID", True);

	/* Doing this to clean up after old background. */
	if (atom_root != None && atom_eroot != None) {
		XGetWindowProperty (display, root,
				    atom_root, 0L, 1L, False, AnyPropertyType,
				    &type, &format, &length, &after, &data_root);
		if (type == XA_PIXMAP) {
			XGetWindowProperty (display, root,
					    atom_eroot, 0L, 1L, False, AnyPropertyType,
					    &type, &format, &length, &after, &data_eroot);
			if (data_root && data_eroot && type == XA_PIXMAP &&
					*((Pixmap *) data_root) == *((Pixmap *) data_eroot))
			{
				XKillClient (display, *((Pixmap *) data_root));
			}
		}
	}

	atom_root = XInternAtom (display, "_XROOTPMAP_ID", False);
	atom_eroot = XInternAtom (display, "ESETROOT_PMAP_ID", False);

	if (atom_root == None || atom_eroot == None) {
		return FALSE;
	}

	/* Setting new background atoms. */
	XChangeProperty (display, root,
			 atom_root, XA_PIXMAP, 32, PropModeReplace,
			 (unsigned char *) &pixmap, 1);
	XChangeProperty (display, root,
			 atom_eroot, XA_PIXMAP, 32, PropModeReplace,
			 (unsigned char *) &pixmap, 1);

	return TRUE;
}

// FIXME: launching compton afterwards shows:
// error 9 (BadDrawable) request 14 minor 0 serial 352 ("BadDrawable (invalid Pixmap or Window parameter)")
// Using for example Nitrogen after, it crashes.
static void
xlib_wallpaper_setter_set_background (PhPlugin *plugin, const gchar *filepath)
{
	XLibWallpaperSetterPrivate *priv;
	GdkPixbuf *pixbuf;
	int screen;
	Window root;
	Pixmap pixmap;
	unsigned int width, height, depth;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (plugin));

	screen = DefaultScreen (priv->display);
	root = RootWindow (priv->display, screen);
	width = DisplayWidth (priv->display, screen);
	height = DisplayHeight (priv->display, screen);
	depth = DefaultDepth (priv->display, screen);

	pixbuf = open_pixbuf (filepath);
	pixbuf = scale_pixbuf (pixbuf, width, height);
	pixmap = XCreatePixmap (priv->display, root, width, height, depth);
	pixbuf_onto_pixmap (pixbuf, priv->display, screen, &pixmap, width, height);

	if (!set_root_atoms (priv->display, root, pixmap)) {
		die (_("Could not create atoms\n"));
	}

	XKillClient (priv->display, AllTemporary);
	XSetCloseDownMode (priv->display, RetainTemporary);

	XSetWindowBackgroundPixmap (priv->display, root, pixmap);
	XClearWindow (priv->display, root);

	XFlush (priv->display);

end:
	XFreePixmap (priv->display, pixmap);
	g_object_unref (pixbuf);
}

static void
xlib_wallpaper_setter_dispose (GObject *object)
{
	XLibWallpaperSetterPrivate *priv;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (object));

	if (priv->display) {
		XCloseDisplay (priv->display);
		priv->display = NULL;
	}

	G_OBJECT_CLASS (xlib_wallpaper_setter_parent_class)->dispose (object);
}

static void
xlib_wallpaper_setter_class_finalize (XLibWallpaperSetterClass *klass)
{
}

static void
xlib_wallpaper_setter_class_init (XLibWallpaperSetterClass *xlib_wallpaper_setter_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (xlib_wallpaper_setter_class);

	object_class->dispose = xlib_wallpaper_setter_dispose;
}

static void
xlib_wallpaper_setter_init (XLibWallpaperSetter *xlib_wallpaper_setter)
{
	XLibWallpaperSetterPrivate *priv;

	priv = xlib_wallpaper_setter_get_instance_private (xlib_wallpaper_setter);

	/* FIXME: should reuse Phosphorus' connection (if it runs on X11, but currently no
	 * Wayland implementation exists). */
	priv->display = XOpenDisplay (NULL);
	if (!(priv->display)) {
		die (_("Could not connect to the X server\n"));
	}
}

static void
ph_plugin_interface_init (PhPluginInterface *interface)
{
	interface->set_background = xlib_wallpaper_setter_set_background;
}

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
	xlib_wallpaper_setter_register_type (G_TYPE_MODULE (module));

	peas_object_module_register_extension_type (module,
						    PH_TYPE_PLUGIN,
						    XLIB_WALLPAPER_SETTER_TYPE);
}

