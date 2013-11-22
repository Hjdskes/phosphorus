/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Phosphorus.c
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
 * graphical preferences to go with config?
 * polish UI with images a là Nitrogen
 * support multi-monitors / Xinerama
 * support more cmdline parameters a là Nitrogen
 */

#include <math.h>
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
	WP_AUTO = 0,
	WP_SCALED,
	WP_CENTERED,
	WP_TILED,
	WP_ZOOMED,
	WP_ZOOMED_FILL
};

struct xconnection {
	Display *dpy;
	Window   root;
	int      screen_num;
} xcon;

struct config {
	char       **dirs;
	const char  *set_wp;
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
		gdk_pixbuf_copy_area (base, 0, 0, win_w, ((count + 1) * src_h) > win_h ? src_h -
				(((count + 1) * src_h) - win_h) : src_h, out, 0, count * src_h);
	}

	g_object_unref (base);
	return out;
}

static GdkPixbuf *pixbuf_make_zoom (GdkPixbuf *base, int win_w, int win_h) {
	GdkPixbuf *out, *temp;
	int x = 0, y = 0, res_x, res_y;

	/* depends on bigger side */
	unsigned int src_w = gdk_pixbuf_get_width (base);
	unsigned int src_h = gdk_pixbuf_get_height (base);

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

static int set_background (void) {
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
	}
	g_object_unref (pix);

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
	char **d;
	GSList *wallpapers = NULL;
	GtkTreeIter sel_it = {0};

	/* load directories */
	for (d = cfg.dirs; *d != NULL; ++d) {
		wallpapers = load_wallpapers_in_dir (*d, wallpapers);
	}

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
				scaled = gdk_pixbuf_scale_simple (wp, new_width, new_height, GDK_INTERP_BILINEAR);
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
	g_string_append_printf (content, "[Wallpaper]\nset_wp = %s\ncolor = %s\nwp_mode = %d\n\n"
			"[Config]\ndirs = %s\n", cfg.set_wp, hex_value (&cfg.bg_color), cfg.wp_mode, *cfg.dirs);
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

		cfg.dirs = g_key_file_get_string_list (config, "Config", "dirs", NULL, &errr);
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
	}

	return 0;
}

static void on_about_button_clicked (GtkButton *button, gpointer user_data) {
	GtkWindow *parent = (GtkWindow *)user_data;
	GtkWidget *about_dialog;
	gchar *license_trans;

	const gchar *authors[] = { "Jente Hidskes", NULL };
	const gchar *license[] = {
		N_("Phosphorus is free software: you can redistribute it and/or modify "
		   "it under the terms of the GNU General Public License as published by "
		   "the Free Software Foundation, either version 3 of the License, or "
		   "(at your option) any later version."),
		N_("Phosphorus is distributed in the hope that it will be useful "
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

	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (about_dialog), "Phosphorus");
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (about_dialog), "0.1");
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG (about_dialog), _("A simple wallpaper setter")),
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (about_dialog), "Copyright \xc2\xa9 2013 Jente Hidskes");
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG (about_dialog), license_trans);
	gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (about_dialog), TRUE);
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (about_dialog), authors);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (about_dialog), _("translator-credits"));
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (about_dialog), _("Website"));
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (about_dialog), "http://unia.github.io/phosphorus");
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
	gtk_window_set_title (GTK_WINDOW (window), "Phosphorus");
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
	//gtk_icon_view_set_tooltip_column (GTK_ICON_VIEW (icon_view), 1); //FIXME: can cause crash
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
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Automatic"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Scaled"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Centered"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Tiled"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Zoomed"), -1);
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, _("Zoomed Fill"), -1);
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
	config_file = g_build_filename (config_dir, "phosphorus/config.cfg", NULL);
	load_config (config_file);

	gtk_init (&argc, &argv);

	wp_store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);

	window = create_window (wp_store);
	load_wallpapers (wp_store);
	g_strfreev (cfg.dirs);
	gtk_widget_show_all (window);
	gtk_main ();

	if (cfg.config_changed)
		save_config (config_file);

	g_free ((gpointer) config_file);
	XCloseDisplay (xcon.dpy);
	return 0;
}
