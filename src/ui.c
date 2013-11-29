/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * ui.c
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>

#include "phosphorus.h"
#include "callbacks.h"

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

GtkWidget *prefs_dialog_open (GtkWindow *parent) {
	GtkWidget *prefs_dialog, *content_area, *dirs_label, *scroll, *tree, *button_box, *add_button, *remove_button;
	GtkListStore *liststore;
	GtkTreeSelection *selection;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeIter iter;
	char **d;

	prefs_dialog = gtk_dialog_new_with_buttons (_("Preferences"), parent, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			_("Cancel"), GTK_RESPONSE_REJECT, _("Ok"), GTK_RESPONSE_ACCEPT, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (prefs_dialog), 0);
	gtk_window_set_default_size (GTK_WINDOW (prefs_dialog), 300, 300);
	gtk_dialog_set_default_response (GTK_DIALOG (prefs_dialog), GTK_RESPONSE_REJECT);
	gtk_dialog_set_response_sensitive (GTK_DIALOG (prefs_dialog), GTK_RESPONSE_REJECT, TRUE);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (prefs_dialog));

	dirs_label = gtk_label_new (_("Directories"));
	gtk_label_set_justify (GTK_LABEL (dirs_label), GTK_JUSTIFY_CENTER);
	gtk_label_set_line_wrap (GTK_LABEL (dirs_label), TRUE);

	scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll), GTK_SHADOW_IN);
	gtk_widget_set_vexpand (scroll, TRUE);

	liststore = gtk_list_store_new (1, G_TYPE_STRING);
	tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore));
	g_object_unref (liststore);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Directory"), renderer, "text", 0, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

	for (d = cfg.dirs; *d != NULL; ++d) {
		gtk_list_store_insert_with_values (liststore, &iter, -1, 0, *d, -1);
	}

	button_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_set_border_width (GTK_CONTAINER (button_box), 5);
	add_button = gtk_button_new_with_label (_("Add"));
	remove_button = gtk_button_new_with_label (_("Remove"));

	g_signal_connect (add_button, "clicked", G_CALLBACK (on_prefs_dlg_add_btn_clicked), parent);
	g_signal_connect (remove_button, "clicked", G_CALLBACK (on_prefs_dlg_rmv_btn_clicked), selection);

	gtk_container_add (GTK_CONTAINER (content_area), dirs_label);
	gtk_container_add (GTK_CONTAINER (scroll), tree);
	gtk_box_pack_start (GTK_BOX (button_box), add_button, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (button_box), remove_button, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (content_area), scroll);
	gtk_container_add (GTK_CONTAINER (content_area), button_box);

	gtk_widget_show_all (prefs_dialog);
	return prefs_dialog;
}

GtkWidget *create_window (GtkListStore *store) {
	GtkWidget *window, *box_all;
	GtkWidget *box_buttons, *button_about, *button_prefs, *button_exit, *button_apply;
	GtkWidget *button_color, *combo_mode;
	GtkWidget *scroll;
	GtkListStore *wp_modes;
	GtkCellRenderer *renderer;
	GtkIconTheme *icon_theme;
	GdkPixbuf *generic_icon, *icon;
	GError *err = NULL;

	/* Main window */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Phosphorus");
	gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
	gtk_window_set_default_size (GTK_WINDOW (window), 450, 500);
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
	button_prefs = gtk_button_new_with_label (_("Preferences"));
	button_apply = gtk_button_new_with_label (_("Apply"));
	button_exit = gtk_button_new_with_label (_("Exit"));

	/* Color button */
	button_color = gtk_color_button_new_with_rgba (&cfg.bg_color);
	gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (button_color), FALSE);
	g_signal_connect (button_color, "color-set", G_CALLBACK (on_color_button_clicked), NULL);

	/* Combo box */
	wp_modes = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	icon_theme = gtk_icon_theme_get_default ();
	generic_icon = gtk_icon_theme_load_icon (icon_theme, "image-x-generic", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (err) {
		gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, 16, 16);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Automatic"), -1);
	icon = gtk_icon_theme_load_icon (icon_theme, "wallpaper-scaled", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (!err)
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, icon, 1, _("Scaled"), -1);
	else {
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Scaled"), -1);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	icon = gtk_icon_theme_load_icon (icon_theme, "wallpaper-centered", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (!err)
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, icon, 1, _("Centered"), -1);
	else {
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Centered"), -1);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	icon = gtk_icon_theme_load_icon (icon_theme, "wallpaper-tiled", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (!err)
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, icon, 1, _("Tiled"), -1);
	else {
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Tiled"), -1);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	icon = gtk_icon_theme_load_icon (icon_theme, "wallpaper-zoomed", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (!err)
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, icon, 1, _("Zoomed"), -1);
	else {
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Zoomed"), -1);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	icon = gtk_icon_theme_load_icon (icon_theme, "wallpaper-zoomed", 16, GTK_ICON_LOOKUP_NO_SVG, &err);
	if (!err)
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, icon, 1, _("Zoomed Fill"), -1);
	else {
		gtk_list_store_insert_with_values (wp_modes, NULL, -1, 0, generic_icon, 1, _("Zoomed Fill"), -1);
		g_fprintf (stderr, "%s\n", err->message);
		g_clear_error (&err);
	}
	combo_mode = gtk_combo_box_new_with_model (GTK_TREE_MODEL (wp_modes));
	g_object_unref (wp_modes);
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_mode), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_mode), renderer, "pixbuf", 0, NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_mode), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_mode), renderer, "text", 1, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo_mode), cfg.wp_mode);
	g_signal_connect (combo_mode, "changed", G_CALLBACK (on_combo_changed), NULL);

	g_signal_connect (button_about, "clicked", G_CALLBACK (on_about_button_clicked), window);
	g_signal_connect (button_prefs, "clicked", G_CALLBACK (on_prefs_button_clicked), window);
	g_signal_connect (button_apply, "clicked", G_CALLBACK (on_apply_button_clicked), NULL);
	g_signal_connect (button_exit, "clicked", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect (icon_view, "item-activated", G_CALLBACK (on_item_activated), NULL);

	gtk_box_pack_start (GTK_BOX (box_buttons), button_about, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box_buttons), button_prefs, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box_buttons), combo_mode, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (box_buttons), button_color, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (box_buttons), button_apply, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (box_buttons), button_exit, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (box_all), box_buttons);

	return window;
}
