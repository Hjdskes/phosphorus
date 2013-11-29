/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * phosphorus.h
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

#ifndef _PHOSPHORUS_H_
#define _PHOSPHORUS_H_

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>

#define VERSION_STRING "0.1"

G_BEGIN_DECLS

enum {
	WP_AUTO = 0,
	WP_SCALED,
	WP_CENTERED,
	WP_TILED,
	WP_ZOOMED,
	WP_ZOOMED_FILL
};

typedef struct _xconnection	xconnection;
struct _xconnection {
	Display *dpy;
	Window   root;
	int      screen_num;
};

typedef struct _configuration configuration;
struct _configuration {
	GSList      *dirs;
	const char  *set_wp;
	unsigned int wp_mode;
	unsigned int config_changed;
	GdkRGBA      bg_color;
};

GtkWidget *icon_view;
extern xconnection xcon;
extern configuration cfg;

guint32 int_value (GdkRGBA *color);

G_END_DECLS

#endif
