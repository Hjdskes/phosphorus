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
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <libpeas/peas.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <ph-application.h>
#include <plugin/ph-plugin.h>

#include "XLibWallpaperSetter.h"

enum {
	PROP_0,
	PROP_APPLICATION,
};

static void ph_plugin_interface_init (PhPluginInterface *interface);

struct _XLibWallpaperSetterPrivate {
	PhApplication *application;
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
warn (const gchar *msg, ...)
{
	va_list args;

	va_start (args, msg);
	vfprintf (stderr, msg, args);
	va_end (args);
}

static void
die (const gchar *msg, ...)
{
	warn (msg);
	exit (EXIT_FAILURE);
}

static GdkPixbuf *
open_pixbuf (const gchar *filepath)
{
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	pixbuf = gdk_pixbuf_new_from_file (filepath, &error);
	if (!pixbuf) {
		warn (_("Could not open pixbuf %s\n"), error->message);
		g_clear_error (&error);
		exit (EXIT_FAILURE);
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
	unsigned int width, height;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (plugin));

	screen = DefaultScreen (priv->display);
	root = RootWindow (priv->display, screen);
	width = DisplayWidth (priv->display, screen);
	height = DisplayHeight (priv->display, screen);

	gdk_pixbuf_xlib_init (priv->display, screen);
	pixbuf = open_pixbuf (filepath);
	pixbuf = scale_pixbuf (pixbuf, width, height);
	gdk_pixbuf_xlib_render_pixmap_and_mask (pixbuf, &pixmap, NULL, 0);

	if (!set_root_atoms (priv->display, root, pixmap)) {
		warn (_("Could not create atoms\n"));
		goto end;
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
xlib_wallpaper_setter_load (UNUSED PhPlugin *plugin)
{
	g_print("load\n");
}

static void
xlib_wallpaper_setter_unload (UNUSED PhPlugin *plugin)
{
	g_print("unload\n");
}

static void
xlib_wallpaper_setter_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
	XLibWallpaperSetterPrivate *priv;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (object));

	switch (prop_id) {
		case PROP_APPLICATION:
			priv->application = PH_APPLICATION (g_value_dup_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
xlib_wallpaper_setter_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
	XLibWallpaperSetterPrivate *priv;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (object));;

	switch (prop_id) {
		case PROP_APPLICATION:
			g_value_set_object (value, priv->application);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
xlib_wallpaper_setter_dispose (GObject *object)
{
	XLibWallpaperSetterPrivate *priv;

	priv = xlib_wallpaper_setter_get_instance_private (XLIB_WALLPAPER_SETTER (object));

	g_clear_object (&priv->application);

	if (priv->display) {
		XCloseDisplay (priv->display);
		priv->display = NULL;
	}

	G_OBJECT_CLASS (xlib_wallpaper_setter_parent_class)->dispose (object);
}

static void
xlib_wallpaper_setter_class_finalize (UNUSED XLibWallpaperSetterClass *klass)
{
}

static void
xlib_wallpaper_setter_class_init (XLibWallpaperSetterClass *xlib_wallpaper_setter_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (xlib_wallpaper_setter_class);

	object_class->set_property = xlib_wallpaper_setter_set_property;
	object_class->get_property = xlib_wallpaper_setter_get_property;
	object_class->dispose = xlib_wallpaper_setter_dispose;

	g_object_class_override_property (object_class, PROP_APPLICATION, "application");
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
	interface->load = xlib_wallpaper_setter_load;
	interface->unload = xlib_wallpaper_setter_unload;
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

