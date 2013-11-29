/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * phosphorus.c
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

/* TODO:
 * create/use existing cache for thumbs
 * support multi-monitors / Xinerama
 * support more cmdline parameters a là Nitrogen
 */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>

#include "phosphorus.h"
#include "background.h"
#include "ui.h"

xconnection xcon = {0};
configuration cfg = {
	NULL,
	NULL,
	0,
	0,
	{ .0, .0, .0, 1.0 }
};

static gchar *hex_value (GdkRGBA *color) {
	int red = color->red * 255;
	int green = color->green * 255;
	int blue = color->blue * 255;
	return g_strdup_printf ("#%.2X%.2X%.2X", red, green, blue);
}

guint32 int_value (GdkRGBA *color) {
	guint32 ret = 0x00000000;
	ret |= ((unsigned int)(color->red * 255) << 24);
	ret |= ((unsigned int)(color->green * 255) << 16);
	ret |= ((unsigned int)(color->blue * 255) << 8);
	ret |= 255; /* alpha should always be full */

	return ret;
}

static GSList *load_wallpapers_in_dir (const char *wp_dir, GSList *wallpapers) {
	GDir *dir;

	if ((dir = g_dir_open (wp_dir, 0, NULL))) {
		const char *name;
		while ((name = g_dir_read_name (dir))) {
			const char *path = g_build_filename (wp_dir, name, NULL);
			if (g_file_test (path, G_FILE_TEST_IS_DIR) == TRUE)
				wallpapers = load_wallpapers_in_dir (path, wallpapers);
			else {
				/* test if we already have this in list */
				if (!g_slist_find_custom (wallpapers, name, (GCompareFunc) strcmp)) {
					wallpapers = g_slist_prepend (wallpapers, g_strdup (path));
					g_free ((gpointer) path);
				}
			}
		}
		g_dir_close (dir);
	}
	return wallpapers;
}

static void load_wallpapers (GtkListStore *store) {
	char **d;
	GSList *wallpapers = NULL;
	GtkTreeIter sel_it = {0};

	/* load directories */
	for (d = cfg.dirs; *d != NULL; ++d) {
		wallpapers = load_wallpapers_in_dir (*d, wallpapers);
	}

	GtkTreeIter it;
	GError *err = NULL;
	wallpapers = g_slist_sort (wallpapers, (GCompareFunc) strcmp);
	for (GSList *l = wallpapers; l; l = l->next) {
		GdkPixbuf *wp;
		wp = gdk_pixbuf_new_from_file ((char *)l->data, &err);
		if (err) {
			g_fprintf (stderr, "Error: %s\n", err->message);
			g_clear_error (&err);
		} else {
			if ((gdk_pixbuf_get_width (wp) > 80) || (gdk_pixbuf_get_height (wp) > 60)) {
				GdkPixbuf *scaled;
				int width = gdk_pixbuf_get_width (wp);
				int height = gdk_pixbuf_get_height (wp);
				float ratio = (float)width / (float)height;
				int new_width, new_height;

				if (abs (80 - width) > abs (60 - height)) {
					new_height = 60;
					new_width = new_height * ratio;
				} else {
					new_width = 80;
					new_height = new_width / ratio;
				}
				scaled = gdk_pixbuf_scale_simple (wp, new_width, new_height, GDK_INTERP_BILINEAR);
				g_object_unref (wp);
				wp = scaled;
			}
			gtk_list_store_insert_with_values (store, &it, -1, 0, wp, 1, (char *)l->data, -1);
			g_object_unref (wp);
		}
		/* if this wallpaper is the one currently in use ... */
		if (!sel_it.user_data) {
			if (strcmp ((char *)l->data, cfg.set_wp) == 0)
				sel_it = it;
		}
	}
	g_slist_free (wallpapers);

	/* ... then select that wallpaper */
	if (sel_it.user_data) {
		GtkTreeModel *model;
		GtkTreePath *path;

		model = gtk_icon_view_get_model (GTK_ICON_VIEW (icon_view));
		path = gtk_tree_model_get_path (model, &sel_it);
		gtk_icon_view_select_path (GTK_ICON_VIEW (icon_view), path);
		gtk_icon_view_scroll_to_path (GTK_ICON_VIEW (icon_view), path, FALSE, 0, 0);
		gtk_tree_path_free (path);
	}
}

static int save_config (const char *path) {
	char **d;
	GString *content;

	content = g_string_sized_new (512);
	g_string_append_printf (content, "[Wallpaper]\nset_wp = %s\ncolor = %s\nwp_mode = %d\n\n"
			"[Config]\ndirs = ", cfg.set_wp, hex_value (&cfg.bg_color), cfg.wp_mode);
	for (d = cfg.dirs; *d != NULL; ++d) {
		g_string_append_printf (content, "%s;", *d);
	}
	if (!g_file_set_contents (path, content->str, content->len, NULL)) {
		g_fprintf (stderr, "Error: failed to write to config file %s", path);
		return -1;
	}

	return 0;
}

static int load_config (const char *path) {
	GKeyFile *config;
	GError *err = NULL;

	config = g_key_file_new ();
	if (!g_key_file_load_from_file (config, path, G_KEY_FILE_NONE, &err)) {
		if (err) {
			g_fprintf (stderr, "Error: %s.\n", err->message);
			g_clear_error (&err);
		}
		return -1;
	} else {
		char *color;

		cfg.set_wp = g_key_file_get_string (config, "Wallpaper", "set_wp", &err);
		cfg.wp_mode = g_key_file_get_integer (config, "Wallpaper", "wp_mode", &err);
		color = g_key_file_get_string (config, "Wallpaper", "color", &err);
		cfg.dirs = g_key_file_get_string_list (config, "Config", "dirs", NULL, &err);

		g_key_file_free (config);
		if (err) {
			g_fprintf (stderr, "Error: %s.\n", err->message);
			g_clear_error (&err);
		}
		if (!gdk_rgba_parse (&cfg.bg_color, color))
			{ cfg.bg_color.red = 0.0; cfg.bg_color.green = 0.0; cfg.bg_color.blue = 0.0;
					cfg.bg_color.alpha = 1.0; }
		g_free ((gpointer) color);
	}

	return 0;
}

static int get_options (int argc, char **argv) {
	GOptionContext *option_context;
	GError *err = NULL;
	gboolean restore_background = FALSE;
	gboolean display_version = FALSE;

	GOptionEntry option_entries[] = {
		{ "version",  'v', 0, G_OPTION_ARG_NONE, &display_version, "Display version and exit", 
				NULL },
		{ "restore",  'r', 0, G_OPTION_ARG_NONE, &restore_background, "Restore set wallpaper and exit", NULL },
		{ NULL },
	};

	option_context = g_option_context_new (NULL);
	g_option_context_add_main_entries (option_context, option_entries, NULL);
	g_option_context_add_group (option_context, gtk_get_option_group (TRUE));

	if (g_option_context_parse (option_context, &argc, &argv, &err) == FALSE) {
		g_fprintf (stderr, "Error: can not parse command line arguments: %s\n", err->message);
		g_clear_error (&err);
		return -1;
	}
	g_option_context_free (option_context);

	if (display_version == TRUE) {
		g_fprintf (stdout, "Phosphorus: a simple and lightweight background setter. Version %s.\n",
				VERSION_STRING);
		return 1;
	}

	if (restore_background == TRUE) {
		const char *config_dir;
		const char *config_file;

		config_dir = g_get_user_config_dir ();
		config_file = g_build_filename (config_dir, "phosphorus/config.cfg", NULL);
		load_config (config_file);
		g_free ((gpointer) config_file);
		int res = set_background ();
		g_strfreev (cfg.dirs);
		XCloseDisplay (xcon.dpy);
		if (res == -1) {
			g_fprintf (stderr, "Applying background failed.\n");
			return -1;
		} else
			return 1;
	}

	return 0;
}

int main (int argc, char **argv) {
	int option = 0;
	GtkWidget *window;
	GtkListStore *wp_store;
	GThread *load_wp_thread;
	const char *config_dir;
	const char *config_file;

	/*bindtextdomain (GETTEXT_PACKAGE, LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);*/

	if (!(xcon.dpy = XOpenDisplay (NULL))) {
		g_fprintf (stderr, "Error: can not open X display to set wallpaper.\n");
		return -1;
	}
	xcon.screen_num = DefaultScreen (xcon.dpy);
	xcon.root = RootWindow (xcon.dpy, xcon.screen_num);
	gdk_pixbuf_xlib_init (xcon.dpy, xcon.screen_num);

	if ((option = get_options (argc, argv) != 0))
		return option < 0 ? -1 : 0;

	config_dir = g_get_user_config_dir ();
	config_file = g_build_filename (config_dir, "phosphorus/config.cfg", NULL);
	load_config (config_file);

	gtk_init (&argc, &argv);

	wp_store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	window = create_window (wp_store);
	gtk_widget_show_all (window);

	load_wp_thread = g_thread_new ("load wallpapers", (GThreadFunc) load_wallpapers, (gpointer) wp_store);
	g_thread_unref (load_wp_thread);
	gtk_main ();

	if (cfg.config_changed)
		save_config (config_file);

	g_free ((gpointer) config_file);
	g_strfreev (cfg.dirs);
	XCloseDisplay (xcon.dpy);
	return 0;
}
