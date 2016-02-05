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

#define THUMB_WIDTH 120
#define THUMB_HEIGHT 80

/* Synced with the list store defined in data/thumbview.ui. */
enum {
	COLUMN_THUMB,
	COLUMN_PATH,
	COLUMN_ROOT,
	COLUMN_NAME,
};

enum {
	SIGNAL_ACTIVATED,
	SIGNAL_SELECTION_CHANGED,
	SIGNAL_LAST
};

static guint signals[SIGNAL_LAST];

struct _PhThumbviewPrivate {
	GtkIconView *iconview;
	GtkListStore *store;

	/* GdkPixbuf's supported formats. */
	GSList *supported_formats;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhThumbview, ph_thumbview, GTK_TYPE_SCROLLED_WINDOW)

static void
ph_thumbview_add_image (PhThumbview *thumbview, const gchar *file, const gchar *root)
{
	PhThumbviewPrivate *priv;
	GdkPixbuf *thumb = NULL;
	GError *error = NULL;
	GtkTreeIter iter;
	gchar *basename;

	priv = ph_thumbview_get_instance_private (thumbview);

	thumb = gdk_pixbuf_new_from_file_at_size (file, THUMB_WIDTH, THUMB_HEIGHT, &error);
	if (!thumb) {
		g_printerr (_("Could not load image \"%s\": %s\n"), file, error->message);
		g_clear_error (&error);
		return;
	}

	basename = g_path_get_basename (file);
	gtk_list_store_append (priv->store, &iter);
	gtk_list_store_set (priv->store, &iter,
			    COLUMN_THUMB, thumb,
			    COLUMN_PATH, file,
			    COLUMN_ROOT, root,
			    COLUMN_NAME, basename,
			    -1);
	g_free (basename);
	g_object_unref (thumb);
}

static void
ph_thumbview_add_directory_recursive (PhThumbview *thumbview, const gchar *path, const gchar *root)
{
	PhThumbviewPrivate *priv;
	GDir *directory = NULL;
	gchar *filepath;
	const gchar *file;
	GError *error = NULL;

	priv = ph_thumbview_get_instance_private (thumbview);

	directory = g_dir_open (path, 0, &error);
	if (!directory) {
		g_printerr ("%s\n", error->message);
		g_clear_error (&error);
		return;
	}

	while ((file = g_dir_read_name (directory))) {
		filepath = g_build_filename (path, file, NULL);

		if (g_file_test (filepath, G_FILE_TEST_IS_DIR)) {
			ph_thumbview_add_directory_recursive (thumbview, filepath, root);
		} else if (ph_file_is_image (priv->supported_formats, filepath)) {
			ph_thumbview_add_image (thumbview, filepath, root);
		}

		g_free (filepath);
	}

	g_dir_close (directory);
}

static void
ph_thumbview_item_activated (UNUSED GtkIconView *iconview, GtkTreePath *path, gpointer user_data)
{
	PhThumbview *thumbview = PH_THUMBVIEW (user_data);
	PhThumbviewPrivate *priv;
	GtkTreeIter iter;
	gchar *filepath;

	priv = ph_thumbview_get_instance_private (thumbview);

	if (!gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->store), &iter, path)) {
		g_printerr (_("Invalid path, won't emit signal\n"));
		return;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, COLUMN_PATH, &filepath, -1);
	g_signal_emit (thumbview, signals[SIGNAL_ACTIVATED], 0, filepath);
	g_free (filepath);
}

static void
ph_thumbview_selection_changed (UNUSED GtkIconView *iconview, gpointer user_data)
{
	g_signal_emit (PH_THUMBVIEW (user_data), signals[SIGNAL_SELECTION_CHANGED], 0);
}

static void
ph_thumbview_dispose (GObject *object)
{
	PhThumbviewPrivate *priv;

	priv = ph_thumbview_get_instance_private (PH_THUMBVIEW (object));

	if (priv->supported_formats) {
		g_slist_free (priv->supported_formats);
		priv->supported_formats = NULL;
	}

	G_OBJECT_CLASS (ph_thumbview_parent_class)->dispose (object);
}

static void
ph_thumbview_class_init (PhThumbviewClass *ph_thumbview_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ph_thumbview_class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (ph_thumbview_class);

	object_class->dispose = ph_thumbview_dispose;

	signals[SIGNAL_ACTIVATED] =
		g_signal_new ("activated", // FIXME: wrap in I_()?
			      PH_TYPE_THUMBVIEW,
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__STRING,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRING);

	signals[SIGNAL_SELECTION_CHANGED] =
		g_signal_new ("selection-changed", // FIXME: wrap in I_()?
			      PH_TYPE_THUMBVIEW,
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);

	gtk_widget_class_set_template_from_resource (widget_class,
			"/org/unia/phosphorus/thumbview.ui");

	gtk_widget_class_bind_template_child_private (widget_class, PhThumbview, iconview);
	gtk_widget_class_bind_template_child_private (widget_class, PhThumbview, store);

	gtk_widget_class_bind_template_callback (widget_class, ph_thumbview_item_activated);
	gtk_widget_class_bind_template_callback (widget_class, ph_thumbview_selection_changed);
}

static void
ph_thumbview_init (PhThumbview *thumbview)
{
	PhThumbviewPrivate *priv;

	priv = ph_thumbview_get_instance_private (thumbview);

	/* Hold GdkPixbuf's supported formats in memory. This list will be traversed for every file
	 * found inside a directory being scanned. It is therefore better to hold it in memory than
	 * to query and free it for every file being scanned. Perhaps, to reduce memory usage, the
	 * list should be freed after startup due to it being unlikely that many new directories
	 * will be added after (the first) startup. However, doing that now is a premature
	 * optimization. */
	priv->supported_formats = gdk_pixbuf_get_formats ();

	gtk_widget_init_template (GTK_WIDGET (thumbview));
}

PhThumbview *
ph_thumbview_new ()
{
	return g_object_new (PH_TYPE_THUMBVIEW, NULL);
}

/**
 * Loading images happens as follows: a directory that is retrieved from the settings is scanned by
 * this method. If another directory is found and the recurse setting is true, then the recursive
 * method is called. If the recurse setting is false, the directory is ignored. When an image is
 * found, it is added together with its root directory (see below).
 *
 * The recursive method exists for two reasons:
 * 1. As a fast track, we can now skip checking whether or not to recurse and if the arguments are
 *    valid.
 * 2. When thumbnails are added, the directory from which they were loaded is stored with them. In
 *    case of recursion, this directory is the "root" from which recursion originated. This makes it
 *    very easy to correctly remove thumbnails when a directory has been removed; the root directory
 *    is compared with the removed directory and if it matches, the thumbnail is removed. This
 *    circumvents quite a few nasty corner-cases we would otherwise have to deal with.
 */
void
ph_thumbview_add_directory (PhThumbview *thumbview, gboolean recurse, const gchar *path)
{
	PhThumbviewPrivate *priv;
	GDir *directory = NULL;
	gchar *filepath;
	const gchar *file;
	GError *error = NULL;

	g_return_if_fail (PH_IS_THUMBVIEW (thumbview));
	g_return_if_fail (path != NULL);

	priv = ph_thumbview_get_instance_private (thumbview);

	directory = g_dir_open (path, 0, &error);
	if (!directory) {
		g_printerr ("%s\n", error->message);
		g_clear_error (&error);
		return;
	}

	/* Checking errno as suggested for g_dir_read_name does not make much sense in this case:
	 * calling g_dir_read_name again will likely attempt to open the same file in case NULL
	 * was returned because of an error. There is (AFAIK) no way to tell GDir to advance to
	 * the next file, skipping the problem. Therefore, reading files would halt regardless of
	 * whether it was the last file or a problematic file. */
	while ((file = g_dir_read_name (directory))) {
		filepath = g_build_filename (path, file, NULL);

		if (g_file_test (filepath, G_FILE_TEST_IS_DIR)) {
			if (recurse) {
				ph_thumbview_add_directory_recursive (thumbview, filepath, path);
			} else {
				g_free (filepath);
				continue;
			}
		} else if (ph_file_is_image (priv->supported_formats, filepath)) {
			ph_thumbview_add_image (thumbview, filepath, path);
		}

		g_free (filepath);
	}

	g_dir_close (directory);
}

void
ph_thumbview_remove_directory (PhThumbview *thumbview, const gchar *path)
{
	PhThumbviewPrivate *priv;
	GtkTreeIter iter;
	gchar *root;

	g_return_if_fail (PH_IS_THUMBVIEW (thumbview));
	g_return_if_fail (path != NULL);

	priv = ph_thumbview_get_instance_private (thumbview);

	if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->store), &iter)) {
		g_printerr (_("The model is empty; no images to remove\n"));
		return;
	}

	/* Naive way to go about it, but it appears to be fast enough and it is very simple.
	 * Apparantly one extra iteration is done with an invalid iter in case the last row is an
	 * image that has to be removed, but this does not seem to give any issues. */
	for (;;) {
		gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter, COLUMN_ROOT, &root, -1);
		if (g_strcmp0 (root, path) == 0) {
			if (!gtk_list_store_remove (priv->store, &iter)) {
				break;
			}
		} else {
			if (!gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->store), &iter)) {
				break;
			}
		}
		g_free (root);
	}

	/* Must free the root present when the loop was broken out of. */
	g_free (root);
}

void
ph_thumbview_activate (PhThumbview *thumbview)
{
	PhThumbviewPrivate *priv;
	GList *selected_items;

	g_return_if_fail (PH_IS_THUMBVIEW (thumbview));

	priv = ph_thumbview_get_instance_private (thumbview);

	selected_items = gtk_icon_view_get_selected_items (priv->iconview);
	/* Phosphorus has single selection mode, so no use getting more than the first item. */
	gtk_icon_view_item_activated (priv->iconview, selected_items->data);
	g_list_free_full (selected_items, (GDestroyNotify) gtk_tree_path_free);
}

