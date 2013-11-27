/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * background.c
 * Copyright (C) 2013 Jente Hidskes <jthidskes@outlook.com>
 *
 * Phosphorus is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Phosphorus is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>

#include "phosphorus.h"

static int get_best_mode (GdkPixbuf *base, int win_w, int win_h) {
	int mode = WP_ZOOMED;
	float ratio = ((float)gdk_pixbuf_get_width (base)) / ((float)gdk_pixbuf_get_height (base));

	float f2t = 1.333f;
	float f2f = 1.25f;

	if (fabsf (ratio - f2t) < 0.001)
		mode = WP_SCALED;
	else if (fabsf (ratio - f2f) < 0.001)
		mode = WP_SCALED;
	else if (ratio == 1.0 && gdk_pixbuf_get_width (base) <= 640)
		mode = WP_TILED;
	else if (gdk_pixbuf_get_width (base) <= win_w && gdk_pixbuf_get_height (base) <= win_h)
		mode = WP_CENTERED;

	return mode;
}

static GdkPixbuf *pixbuf_make_scaled (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out;

	out = gdk_pixbuf_scale_simple (base, win_w, win_h, GDK_INTERP_BILINEAR);
	g_object_unref (base);

	return out;
}

static GdkPixbuf *pixbuf_make_centered (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out;
	int dest_x = (win_w - gdk_pixbuf_get_width (base)) >> 1;
	int dest_y = (win_h - gdk_pixbuf_get_height (base)) >> 1;
	int src_x = 0;
	int src_y = 0;
	int cpy_w = gdk_pixbuf_get_width (base);
	int cpy_h = gdk_pixbuf_get_height (base);

	out = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (base), gdk_pixbuf_get_has_alpha (base),
			gdk_pixbuf_get_bits_per_sample (base), win_w, win_h);
	gdk_pixbuf_fill (out, int_value(&cfg.bg_color));

	if (gdk_pixbuf_get_width (base) > win_w) {
		src_x = (gdk_pixbuf_get_width (base) - win_w) >> 1;
		dest_x = 0;
		cpy_w = win_w;
	}
	if (gdk_pixbuf_get_height (base) > win_h) {
		src_y = (gdk_pixbuf_get_height (base) - win_h) >> 1;
		dest_y = 0;
		cpy_h = win_h;
	}

	gdk_pixbuf_copy_area (base, src_x, src_y, cpy_w, cpy_h, out, dest_x, dest_y);
	g_object_unref (base);

	return out;
}

static GdkPixbuf *pixbuf_make_tiled (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out;
	int count = 0;
	int src_w = gdk_pixbuf_get_width (base);
	int src_h = gdk_pixbuf_get_height (base);

	/* copy and resize (mainly just resize) */
	out = gdk_pixbuf_scale_simple (base, win_w, win_h, GDK_INTERP_NEAREST);

	/* copy across horizontally first */
	int iterations = (int)ceil ((double)win_w / (double)src_w);
	for (count = 0; count < iterations; count++) {
		gdk_pixbuf_copy_area (base, 0, 0, ((count + 1) * src_w) > win_w ? src_w -
				(((count + 1) * src_w) - win_w) : src_w, src_h, out, count * src_w, 0);
	}

	/* now vertically */
	iterations = (int)ceil ((double)win_h / (double)src_h);
	/* start at 1 because the first real (0) iteration is already done from before
	   (it's the source of our copy!) */
	for (count = 1; count < iterations; count++) {
		gdk_pixbuf_copy_area (base, 0, 0, src_w, ((count + 1) * src_h) > win_h ? src_h -
				(((count + 1) * src_h) - win_h) : src_h, out, 0, count * src_h);
	}

	g_object_unref (base);
	return out;
}

static GdkPixbuf *pixbuf_make_zoom (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out, *temp;
	int x = 0, y = 0, res_x, res_y;

	/* depends on bigger side */
	int src_w = gdk_pixbuf_get_width (base);
	int src_h = gdk_pixbuf_get_height (base);

	/* the second term (after the &&) is needed to ensure that the new height
	   does not exceed the root window height */
	if (src_w > src_h && ((float)src_w / (float)src_h) > ((float)win_w / (float)win_h)) {
		res_x = win_w;
		res_y = (int)(((float)(gdk_pixbuf_get_height (base) * res_x)) / (float)gdk_pixbuf_get_width (base));
		x = 0;
		y = (win_h - res_y) >> 1;
	} else {
		res_y = win_h;
		res_x = (int)(((float)(gdk_pixbuf_get_width (base) * res_y)) / (float)gdk_pixbuf_get_height (base));
		y = 0;
		x = (win_w - res_x) >> 1;
	}

	/* fix to make sure we can make it */
	if (res_x > win_w)
		res_x = win_w;
	if (res_y > win_h)
		res_y = win_h;
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	temp = gdk_pixbuf_scale_simple (base, res_x, res_y, GDK_INTERP_BILINEAR);
	out = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (base), gdk_pixbuf_get_has_alpha (base),
			gdk_pixbuf_get_bits_per_sample (base), win_w, win_h);
	gdk_pixbuf_fill (out, int_value(&cfg.bg_color));

	gdk_pixbuf_copy_area (temp, 0, 0, gdk_pixbuf_get_width (temp), gdk_pixbuf_get_height (temp),
			out, x, y);
	g_object_unref (base);
	g_object_unref (temp);

	return out;
}

static GdkPixbuf *pixbuf_make_zoom_fill (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out, *temp;
	int x, y, w, h;

	/* depends on bigger side */
	int src_w = gdk_pixbuf_get_width (base);
	int src_h = gdk_pixbuf_get_height (base);

	/* what if we expand it to fit the screen width? */
	x = 0;
	w = win_w;
	h = win_w * src_h / src_w;
	y = (h - win_h) / 2;

	if (!(h >= win_h)) {
		/* the image isn't tall enough that way!
		   expand it to fit the screen height */
		y = 0;
		w = win_h * src_w / src_h;
		h = win_h;
		x = (w - win_w) / 2;
	}

	temp = gdk_pixbuf_scale_simple (base, w, h, GDK_INTERP_BILINEAR);
	out = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (base), gdk_pixbuf_get_has_alpha (base),
			gdk_pixbuf_get_bits_per_sample (base), win_w, win_h);
	gdk_pixbuf_fill (out, int_value(&cfg.bg_color));

	gdk_pixbuf_copy_area (temp, x, y, win_w, win_h, out, 0, 0);
	g_object_unref (base);
	g_object_unref (temp);

	return out;
}

int set_background (void) {
	GC gc;
	Pixmap xpixmap;
	GdkPixbuf *pix, *outpix;
	Atom prop_root = None, prop_esetroot = None;
	GError *err = NULL;
	int win_w, win_h;

	pix = gdk_pixbuf_new_from_file (cfg.set_wp, &err);
	if (err) {
		g_fprintf (stderr, "Error: %s\n", err->message);
		g_clear_error (&err);
		return -1;
	}

	gc = XCreateGC (xcon.dpy, xpixmap, 0, NULL);
	win_w = DisplayWidth (xcon.dpy, xcon.screen_num);
	win_h = DisplayHeight (xcon.dpy, xcon.screen_num);
	XFillRectangle (xcon.dpy, xpixmap, gc, 0, 0, win_w, win_h);

	if (cfg.wp_mode == WP_AUTO)
		cfg.wp_mode = get_best_mode (pix, win_w, win_h);

	switch (cfg.wp_mode) {
		case WP_SCALED:
			outpix = pixbuf_make_scaled (pix, win_w, win_h);
			break;
		case WP_CENTERED:
			outpix = pixbuf_make_centered (pix, win_w, win_h);
			break;
		case WP_TILED: //FIXME: does not work correctly
			outpix = pixbuf_make_tiled (pix, win_w, win_h);
			break;
		case WP_ZOOMED:
			outpix = pixbuf_make_zoom (pix, win_w, win_h);
			break;
		case WP_ZOOMED_FILL:
			outpix = pixbuf_make_zoom_fill (pix, win_w, win_h);
			break;
		default:
			g_fprintf (stderr, "Error: unknown wallpaper mode.\n");
			return -1;
	}

	gdk_pixbuf_xlib_render_pixmap_and_mask (outpix, &xpixmap, NULL, 1);
	g_object_unref (outpix);

	prop_root = XInternAtom (xcon.dpy, "_XROOTPMAP_ID", False);
	prop_esetroot = XInternAtom (xcon.dpy, "ESETROOT_PMAP_ID", False);
	if (prop_root == None || prop_esetroot == None) {
		g_fprintf (stderr, "Error:  creation of pixmap property failed.\n");
		return -1;
	}

	XGrabServer (xcon.dpy);
	XChangeProperty (xcon.dpy, xcon.root, prop_root, XA_PIXMAP, 32, PropModeReplace,
			(unsigned char *)&xpixmap, 1);
	XChangeProperty (xcon.dpy, xcon.root, prop_esetroot, XA_PIXMAP, 32, PropModeReplace,
			(unsigned char *)&xpixmap, 1);

	XSetWindowBackgroundPixmap (xcon.dpy, xcon.root, xpixmap);
	XClearWindow (xcon.dpy, xcon.root);

	XUngrabServer (xcon.dpy);
	XFlush (xcon.dpy);
	XFreePixmap (xcon.dpy, xpixmap);
	XFreeGC (xcon.dpy, gc);

	return 0;
}
