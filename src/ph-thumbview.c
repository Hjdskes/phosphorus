/* PhThumbview
 *
 * Copyright (C) 2015 Jente Hidskes
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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "ph-thumbview.h"
#include "util.h"

#define THUMB_WIDTH 80
#define THUMB_HEIGHT 60

/* Synced with the list store defined in data/thumbview.ui. */
enum {
	COLUMN_THUMB,
	COLUMN_PATH,
	COLUMN_NAME,
};

enum {
	SIGNAL_ACTIVATED,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST];

struct _PhThumbviewPrivate {
	GtkListStore *store;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhThumbview, ph_thumbview, GTK_TYPE_SCROLLED_WINDOW);

static void
ph_thumbview_add_image (PhThumbview *thumbview, const gchar *file)
{
	PhThumbviewPrivate *priv;
	GdkPixbuf *thumb = NULL;
	GError *error = NULL;
	GtkTreeIter iter;
	gchar *basename;

	priv = ph_thumbview_get_instance_private (thumbview);

	thumb = gdk_pixbuf_new_from_file_at_size (file, THUMB_WIDTH, THUMB_HEIGHT, &error);
	if (!thumb) {
		g_printerr ("Could not load image \"%s\": %s\n", file, error->message);
		return;
	}

	basename = g_path_get_basename (file);
	gtk_list_store_append (priv->store, &iter);
	gtk_list_store_set (priv->store, &iter,
			    COLUMN_THUMB, thumb,
			    COLUMN_PATH, file,
			    COLUMN_NAME, basename,
			    -1);
	g_free (basename);
	g_object_unref (thumb);
}

static void
ph_thumbview_item_activated (GtkIconView *iconview, GtkTreePath *path, gpointer user_data)
{
	PhThumbview *thumbview = PH_THUMBVIEW (user_data);
	PhThumbviewPrivate *priv;
	GtkTreeIter iter;
	gchar *filepath;

	priv = ph_thumbview_get_instance_private (thumbview);

	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->store), &iter, path)) {
		g_printerr ("Invalid path, won't emit signal\n");
		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, COLUMN_PATH, &filepath, -1);
	g_signal_emit (thumbview, signals[SIGNAL_ACTIVATED], 0, filepath);
	g_free (filepath);
}

static void
ph_thumbview_class_init (PhThumbviewClass *ph_thumbview_class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (ph_thumbview_class);

	signals[SIGNAL_ACTIVATED] =
		g_signal_new ("activated", // FIXME: wrap in I_()?
			      PH_TYPE_THUMBVIEW,
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);

	gtk_widget_class_set_template_from_resource (widget_class,
			"/org/unia/phosphorus/thumbview.ui");

	gtk_widget_class_bind_template_child_private (widget_class, PhThumbview, store);

	gtk_widget_class_bind_template_callback (widget_class, ph_thumbview_item_activated);
}

static void
ph_thumbview_init (PhThumbview *thumbview)
{
	gtk_widget_init_template (GTK_WIDGET (thumbview));
}

PhThumbview *
ph_thumbview_new ()
{
	return g_object_new (PH_TYPE_THUMBVIEW, NULL);
}

void
ph_thumbview_add_directory (PhThumbview *thumbview, PhRecurseType recurse, const gchar *path)
{
	GDir *directory = NULL;
	gchar *filepath;
	const gchar *file;
	GError *error = NULL;

	g_return_if_fail (thumbview != NULL);
	g_return_if_fail (path != NULL);

	directory = g_dir_open (path, 0, &error);
	if (!directory) {
		g_printerr ("%s\n", error->message);
		g_clear_error (&error);
		return;
	}

	/* FIXME: Check errno with g_file_error_from_errno () to distinguish actual errors
	 * from reaching the end of the directory? */
	while ((file = g_dir_read_name (directory))) {
		filepath = g_build_filename (path, file, NULL);

		if (g_file_test (filepath, G_FILE_TEST_IS_DIR) && recurse == RECURSE) {
			ph_thumbview_add_directory (thumbview, recurse, filepath);
		} else if (ph_file_is_image (file)) {
			ph_thumbview_add_image (thumbview, filepath);
		}

		g_free (filepath);
	}

	g_dir_close (directory);
}

