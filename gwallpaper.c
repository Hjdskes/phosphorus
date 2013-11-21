/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gwallpaper.c
 * Copyright (C) 2013 Jente Hidskes <jthidskes@outlook.com>
 *
 * Gwallpaper is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gwallpaper is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* TODO:
 * fix coordinates of wallpapers
 * create/use existing cache for thumbs
 * graphical preferences to go with config?
 * polish UI with images a là Nitrogen
 * support multi-monitors / Xinerama
 * support more cmdline parameters a là Nitrogen
 */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>

#define VERSION_STRING "0.1"

enum {
	WP_COLOR = 0,
	WP_TILE,
	WP_SCALED,
	WP_FIT,
	WP_CENTER
};

struct xconnection {
	Display *dpy;
	Window   root;
	int      screen_num;
} xcon;

struct config {
	const char  *set_wp;
	const char  *dir;
	unsigned int wp_mode;
	unsigned int config_changed;
	GdkRGBA      bg_color;
} cfg = {
	NULL,
	NULL,
	0,
	0,
	{ .0, .0, .0, 1.0 }
};

GtkWidget *icon_view;

gchar *hex_value (GdkRGBA *color) {
	int red = color->red * 255;
	int green = color->green * 255;
	int blue = color->blue * 255;
#ifdef DEBUG
	g_fprintf (stdout, "color to string: %s\n", gdk_rgba_to_string (&cfg.bg_color));
	g_fprintf (stdout, "color: #%.2X%.2X%.2X\n", red, green, blue);
#endif
	return g_strdup_printf ("#%.2X%.2X%.2X", red, green, blue);
}

static int set_background (void) {
	Pixmap xpixmap;
	GC gc;
	XGCValues gcv;
	XColor xcolor;
	GdkPixbuf *pix, *scaled;
	Atom prop_root = None, prop_esetroot = None, type;
	GError *err = NULL;
	unsigned long length, after;
	unsigned char *data_root, *data_esetroot;
	int format, x = 0, y = 0, src_w = 0, src_h = 0, dest_w = 0, dest_h = 0;

#ifdef DEBUG
	g_fprintf (stdout, "set_wp: %s\n", cfg.set_wp);
#endif

	pix = gdk_pixbuf_new_from_file (cfg.set_wp, &err);
	if (err) {
		g_fprintf (stderr, "Error: %s\n", err->message);
		g_clear_error (&err);
		return -1;
	}

	src_w = gdk_pixbuf_get_width (pix);
	src_h = gdk_pixbuf_get_height (pix);
	if (cfg.wp_mode == WP_TILE) {
		dest_w = src_w;
		dest_h = src_h;
	} else {
		dest_w = DisplayWidth (xcon.dpy, xcon.screen_num);
		dest_h = DisplayHeight (xcon.dpy, xcon.screen_num);
	}

#ifdef DEBUG
	g_fprintf (stdout, "scr_w: %d\nsrc_h: %d\ndest_w: %d\ndest_h: %d\n\n",
			src_w, src_h, dest_w, dest_h);
#endif

	switch (cfg.wp_mode) {
		case WP_COLOR:
			g_fprintf (stderr, "Error: not implemented yet.\n");
			return 0;
		case WP_TILE: /* do nothing here */
#ifdef DEBUG
			g_fprintf (stdout, "wp_mode: tile\n\n");
#endif
			break;
		case WP_SCALED:
#ifdef DEBUG
			g_fprintf (stdout, "wp_mode: scaled\n\n");
#endif
			if (dest_w == src_w && dest_h == src_h)
				scaled = (GdkPixbuf *)g_object_ref (pix);
			else
				scaled = gdk_pixbuf_scale_simple (pix, dest_w, dest_h, GDK_INTERP_BILINEAR);
			g_object_unref (pix);
			pix = scaled;
			break;
		case WP_FIT:
#ifdef DEBUG
			g_fprintf (stdout, "wp_mode: fit\n\n");
#endif
			if (dest_w != src_w || dest_h != src_h) {
				double w_ratio = (float)dest_w / src_w;
				double h_ratio = (float)dest_h / src_h;
				double ratio = MIN (w_ratio, h_ratio);
				if (ratio != 1.0) {
					src_w *= ratio;
					src_h *= ratio;
					scaled = gdk_pixbuf_scale_simple (pix, src_w, src_h, GDK_INTERP_BILINEAR);
					g_object_unref (pix);
					pix = scaled;
				}
			}
			/* continue to execute code in case WP_CENTER */
		case WP_CENTER:
#ifdef DEBUG
			g_fprintf (stdout, "wp_mode: center\n\n");
#endif
			x = (dest_w - src_w) / 2;
			y = (dest_h - src_h) / 2;
			break;
	}

#ifdef DEBUG
	g_fprintf (stdout, "scr_w: %d\nsrc_h: %d\ndest_w: %d\ndest_h: %d\n\n",
			src_w, src_h, dest_w, dest_h);
#endif

	char *bg_color_string = hex_value (&cfg.bg_color);
	if (bg_color_string && XParseColor (xcon.dpy, DefaultColormap (xcon.dpy, xcon.screen_num),
			bg_color_string, &xcolor) && XAllocColor (xcon.dpy, DefaultColormap (xcon.dpy,
			xcon.screen_num), &xcolor))
		gcv.foreground = gcv.background = xcolor.pixel;
	else
		gcv.foreground = gcv.background = BlackPixel (xcon.dpy, xcon.screen_num);
	g_free ((gpointer) bg_color_string);

	gc = XCreateGC (xcon.dpy, xpixmap, (GCForeground | GCBackground), &gcv);
	XFillRectangle (xcon.dpy, xpixmap, gc, x, y, dest_w, dest_h);

	gdk_pixbuf_xlib_render_pixmap_and_mask (pix, &xpixmap, NULL, 1);

	prop_root = XInternAtom (xcon.dpy, "_XROOTPMAP_ID", True);
	prop_esetroot = XInternAtom (xcon.dpy, "ESETROOT_PMAP_ID", True);

	if (prop_root != None && prop_esetroot != None) {
		XGetWindowProperty (xcon.dpy, xcon.root, prop_root, 0L, 1L, False, AnyPropertyType, &type,
				&format, &length, &after, &data_root);
		if (type == XA_PIXMAP) {
			XGetWindowProperty (xcon.dpy, xcon.root, prop_esetroot, 0L, 1L, False, AnyPropertyType,
					&type, &format, &length, &after, &data_esetroot);
			if (data_root && data_esetroot)
				if (type == XA_PIXMAP && *((Pixmap *) data_root) == *((Pixmap *) data_esetroot))
					XKillClient (xcon.dpy, *((Pixmap *) data_root));
		}
	}

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
	if (pix || cfg.wp_mode != WP_COLOR)
		g_object_unref (pix);
	if (cfg.wp_mode == WP_SCALED || cfg.wp_mode == WP_FIT)
		g_object_unref (scaled);

	return 0;
}

static GSList *load_wallpapers_in_dir (const char *wp_dir, GSList *wallpapers) {
	GDir *dir;

	if ((dir = g_dir_open (wp_dir, 0, NULL))) {
		const char *name;
		while ((name = g_dir_read_name (dir))) {
			const char *path = g_build_filename (wp_dir, name, NULL); //FIXME: simplify?
			if (g_file_test (path, G_FILE_TEST_IS_DIR) == TRUE)
				wallpapers = load_wallpapers_in_dir (path, wallpapers);
			else {
				/* test if we already have this in list */
				if (!g_slist_find_custom (wallpapers, path, (GCompareFunc) strcmp)) {
					wallpapers = g_slist_prepend (wallpapers, g_strdup (path));
					g_free ((gpointer) path);
				}
			}
		}
		g_dir_close (dir);
	}
	return wallpapers;
}

static void load_wallpapers (GtkListStore *store) { //FIXME: speed this up -> threads?
	GSList *wallpapers = NULL;
	GtkTreeIter sel_it = {0};

	/* load user dir */ //FIXME: multiple directories
	wallpapers = load_wallpapers_in_dir (cfg.dir, wallpapers);

	wallpapers = g_slist_sort (wallpapers, (GCompareFunc) strcmp);
	for (GSList *l = wallpapers; l; l = l->next) {
		GtkTreeIter it;
		GdkPixbuf *wp;
		GError *err = NULL;
		char *name = (char *)l->data;
		wp = gdk_pixbuf_new_from_file (name, &err);
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
				scaled = gdk_pixbuf_scale_simple (wp, new_width, new_height,
						GDK_INTERP_BILINEAR);
				g_object_unref (wp);
				wp = scaled;
			}
			gtk_list_store_insert_with_values (store, &it, -1, 0, wp, 1, name, -1);
			g_object_unref (wp);
		}
		/* if this wallpaper is the one currently in use ... */
		if (!sel_it.user_data) {
			if (strcmp (name, cfg.set_wp) == 0)
				sel_it = it;
		}
		g_free (name);
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

static void on_apply_button_clicked (GtkButton *button, gpointer user_data) {
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	cfg.config_changed = 1;
	model = gtk_icon_view_get_model (GTK_ICON_VIEW (icon_view));
	path = gtk_tree_model_get_path (model, &iter); //FIXME: fails
	if (!gtk_tree_model_get_iter (model, &iter, path)) {
		g_fprintf (stderr, "Error: can not determine activated wallpaper (from apply button).\n");
		return;
	}
	gtk_tree_path_free (path);
	gtk_tree_model_get (model, &iter, 1, &cfg.set_wp, -1);

	set_background ();
}

static void on_combo_changed (GtkComboBox *combo, gpointer user_data) {
	cfg.config_changed = 1;
	cfg.wp_mode = gtk_combo_box_get_active (combo);
}

static void on_item_activated (GtkIconView *view, GtkTreePath *path, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeIter iter;

	cfg.config_changed = 1;
	model = gtk_icon_view_get_model (GTK_ICON_VIEW (icon_view));
	if (!gtk_tree_model_get_iter (model, &iter, path)) {
		g_fprintf (stderr, "Error: can not determine activated wallpaper (from item activated).\n");
		return;
	}
	gtk_tree_model_get (model, &iter, 1, &cfg.set_wp, -1);

	set_background ();
}

static void on_color_button_clicked (GtkColorButton *button, gpointer user_data) {
	cfg.config_changed = 1;
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (button), &cfg.bg_color);
}

int save_config (const char *path) {
	GString *content;

	content = g_string_sized_new (512);
	char *color = hex_value (&cfg.bg_color);
	g_string_append_printf (content, "[Wallpaper]\nset_wp = %s\ncolor = %s\nwp_mode = %d\n\n"
			"[Config]\ndirs = %s\n", cfg.set_wp, color, cfg.wp_mode, cfg.dir);
	g_free ((gpointer) color);
	if (!g_file_set_contents (path, content->str, content->len, NULL))
		g_fprintf (stderr, "Error: failed to write to config file %s", path);
	return 0;
}

int load_config (const char *path) {
	GKeyFile *config;
	GError *err = NULL;

	config = g_key_file_new ();
	if (g_key_file_load_from_file (config, path, G_KEY_FILE_NONE, &err) == FALSE) {
		if (err) {
			g_fprintf (stderr, "Error: %s.\n", err->message);
			g_clear_error (&err);
		}
		return -1;
	} else {
		GError *errr = NULL;
		char *color;

		cfg.dir = g_key_file_get_string (config, "Config", "dirs", &errr);
		cfg.set_wp = g_key_file_get_string (config, "Wallpaper", "set_wp", &errr);
		cfg.wp_mode = g_key_file_get_integer (config, "Wallpaper", "wp_mode", &errr);
		color = g_key_file_get_string (config, "Wallpaper", "color", &errr);

		g_key_file_free (config);
		if (errr) {
			g_fprintf (stderr, "Error: %s.\n", errr->message);
			g_clear_error (&errr);
		}
		if (!gdk_rgba_parse (&cfg.bg_color, color))
			{ cfg.bg_color.red = 0.0; cfg.bg_color.green = 0.0; cfg.bg_color.blue = 0.0; cfg.bg_color.alpha = 1.0; }
		g_free ((gpointer) color);
#ifdef DEBUG
		g_fprintf (stdout, "dir:     %s\nset_wp:  %s\nwp_mode: %d\n\n", cfg.dir, cfg.set_wp, cfg.wp_mode);
#endif
	}

	return 0;
}

static void on_about_button_clicked (GtkButton *button, gpointer user_data) {
	GtkWindow *parent = (GtkWindow *)user_data;
	GtkWidget *about_dialog;
	gchar *license_trans;

	const gchar *authors[] = { "Jente Hidskes", NULL };
	const gchar *license[] = {
		N_("Gwallpaper is free software: you can redistribute it and/or modify "
		   "it under the terms of the GNU General Public License as published by "
		   "the Free Software Foundation, either version 3 of the License, or "
		   "(at your option) any later version."),
		N_("Gwallpaper is distributed in the hope that it will be useful "
		   "but WITHOUT ANY WARRANTY; without even the implied warranty of "
		   "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		   "GNU General Public License for more details."),
		N_("You should have received a copy of the GNU General Public License "
		   "along with this program. If not, see http://www.gnu.org/licenses/.")
	};
	license_trans = g_strjoin ("\n\n", _(license[0]), _(license[1]), _(license[2]), NULL);

	about_dialog = gtk_about_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW (about_dialog), parent);
	gtk_window_set_modal (GTK_WINDOW (about_dialog), TRUE);

	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (about_dialog), "Gwallpaper");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about_dialog), "0.1");
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (about_dialog), _("A simple wallpaper setter")),
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (about_dialog), "Copyright \xc2\xa9 2013 Jente Hidskes");
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (about_dialog), license_trans);
	gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (about_dialog), TRUE);
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (about_dialog), authors);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (about_dialog), _("translator-credits"));
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (about_dialog), _("Website"));
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (about_dialog), "http://unia.github.io/gwallpaper");
	gtk_about_dialog_set_logo_icon_name (GTK_ABOUT_DIALOG (about_dialog), "preferences-wallpaper");

	g_signal_connect (GTK_DIALOG (about_dialog), "response", G_CALLBACK (gtk_widget_destroy), about_dialog);

	gtk_widget_show (about_dialog);

	g_free (license_trans);
}

static GtkWidget *create_window (GtkListStore *store) {
	GtkWidget *window, *box_all;
	GtkWidget *box_buttons, *button_about, *button_exit, *button_apply;
	GtkWidget *button_color, *combo_mode;
	GtkWidget *scroll;
	GtkListStore *wp_modes;
	GtkCellRenderer *renderer;

	/* Main window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Gwallpaper");
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	box_all = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	gtk_container_set_border_width (GTK_CONTAINER (box_all), 5);
	gtk_container_add (GTK_CONTAINER (window), box_all);

	/* Wallpaper view */
	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_IN);
	gtk_widget_set_vexpand (scroll, TRUE);
	gtk_container_add (GTK_CONTAINER (box_all), scroll);

	icon_view = gtk_icon_view_new_with_model (GTK_TREE_MODEL (store));
	g_object_unref (store);
	gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view), 0);
	gtk_icon_view_set_tooltip_column (GTK_ICON_VIEW (icon_view), 1);
	gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view), GTK_SELECTION_SINGLE);
	gtk_icon_view_set_margin (GTK_ICON_VIEW (icon_view), 0);
	gtk_icon_view_set_column_spacing (GTK_ICON_VIEW (icon_view), 1);
	gtk_icon_view_set_row_spacing (GTK_ICON_VIEW (icon_view), 1);
	gtk_container_add (GTK_CONTAINER (scroll), icon_view);

	/* Main window buttons */
	box_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
	button_about = gtk_button_new_with_label (_("About"));
	button_apply = gtk_button_new_with_label (_("Apply"));
	button_exit = gtk_button_new_with_label (_("Exit"));

	/* Color button */
	button_color = gtk_color_button_new_with_rgba (&cfg.bg_color);
	gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (button_color), FALSE);
	g_signal_connect (button_color, "color-set", G_CALLBACK (on_color_button_clicked), NULL);

	/* Combo box */
	wp_modes = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Color"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Tiled"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Scaled"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Fit"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Centered"), -1);
	combo_mode = gtk_combo_box_new_with_model (GTK_TREE_MODEL (wp_modes));
	g_object_unref (wp_modes);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_mode), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_mode), renderer, "text", 0, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo_mode), cfg.wp_mode);
	g_signal_connect (combo_mode, "changed", G_CALLBACK (on_combo_changed), NULL);

	g_signal_connect (button_about, "clicked", G_CALLBACK (on_about_button_clicked), window);
	g_signal_connect (button_apply, "clicked", G_CALLBACK (on_apply_button_clicked), NULL);
	g_signal_connect (button_exit, "clicked", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect (icon_view, "item-activated", G_CALLBACK (on_item_activated), NULL);

	gtk_box_pack_start (GTK_BOX (box_buttons), button_about, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box_buttons), combo_mode, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box_buttons), button_color, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (box_buttons), button_apply, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (box_buttons), button_exit, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (box_all), box_buttons);

	return window;
}

static int get_options (int argc, char **argv) {
	GOptionContext *option_context;
	GError *err = NULL;
	gboolean restore_background = FALSE;
	gboolean display_version = FALSE;

	GOptionEntry option_entries[] = {
		{ "version",  'v', 0, G_OPTION_ARG_NONE, &display_version, "Display version and exit", 
				NULL },
		{ "restore",  'r', 0, G_OPTION_ARG_NONE, &restore_background, "Use symbolic icons", NULL },
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
		g_fprintf (stdout, "Gwallpaper: a simple and lightweight background setter. Version %s.\n",
				VERSION_STRING);
		return 1;
	}

	if (restore_background == TRUE) {
		const char *config_dir;
		const char *config_file;

		config_dir = g_get_user_config_dir ();
		config_file = g_build_filename (config_dir, "gwallpaper/config.cfg", NULL);
		load_config (config_file);
		g_free ((gpointer) config_file);
		set_background ();
		return 1;
	}

	return 0;
}

int main (int argc, char **argv) {
	int option = 0;
	GtkWidget *window;
	GtkListStore *wp_store;
	const char *config_dir;
	const char *config_file;

#ifdef ENABLE_NLS //FIXME: implement gettext support
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

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
	config_file = g_build_filename (config_dir, "gwallpaper/config.cfg", NULL);
	load_config (config_file);

	gtk_init (&argc, &argv);

	wp_store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	window = create_window (wp_store);
	load_wallpapers (wp_store);
	gtk_widget_show_all (window);
	gtk_main ();

	if (cfg.config_changed)
		save_config (config_file);

	g_free ((gpointer) config_file);
	XCloseDisplay (xcon.dpy);
	return 0;
}
