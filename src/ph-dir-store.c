/* PhDirStore
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

#include <gio/gio.h>
#include <glib/gi18n.h>

#include "ph-dir-store.h"
#include "gsettings.h"

enum {
	PROP_0,
	PROP_DIRECTORIES,
};

enum {
	SIGNAL_DIRS_ADDED,
	SIGNAL_DIRS_REMOVED,
	SIGNAL_LAST,
};

static guint signals[SIGNAL_LAST];

struct _PhDirStorePrivate {
	GSettings *settings;
	gchar **current_directories;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhDirStore, ph_dir_store, G_TYPE_OBJECT)

/**
 * Naive but simple implementation that appears to be fast enough.
 */
static void
on_directories_changed (GSettings *settings, gchar *key, gpointer user_data)
{
	PhDirStore *dir_store = PH_DIR_STORE (user_data);
	PhDirStorePrivate *priv;
	gchar **new_directories;
	guint n_new_directories;
	guint n_current_directories;
	gchar **added_directories = NULL;
	gchar **removed_directories = NULL;

	priv = ph_dir_store_get_instance_private (dir_store);

	new_directories = g_settings_get_strv (settings, key);

	/* There are no current directories; every new directory is an added one. */
	if (!priv->current_directories || !*priv->current_directories) {
		g_signal_emit (dir_store, signals[SIGNAL_DIRS_ADDED], 0, new_directories);
		goto end;
	} else if (!new_directories || !*new_directories) {
		/* There are no new directories, but there are current ones: all are removed. */
		g_signal_emit (dir_store, signals[SIGNAL_DIRS_REMOVED], 0, priv->current_directories);
		goto end;
	}

	n_new_directories = g_strv_length (new_directories);
	n_current_directories = g_strv_length (priv->current_directories);

	added_directories = g_new0 (gchar *, n_new_directories + 1);
	removed_directories = g_new0 (gchar *, n_current_directories + 1);

	for (guint i = 0, j = 0; i < n_new_directories; i++) {
		/* Currently loaded directories does not contain a string that is in the new
		 * directories, therefore it must be added. */
		if (!g_strv_contains ((const gchar * const *) priv->current_directories, new_directories[i])) {
			added_directories[j++] = new_directories[i];
		}
	}
	for (guint i = 0, j = 0; i < n_current_directories; i++) {
		/* New directories does not contain a string that is in the currently loaded
		 * directories, therefore it must be removed. */
		if (!g_strv_contains ((const gchar * const *) new_directories, priv->current_directories[i])) {
			removed_directories[j++] = priv->current_directories[i];
		}
	}

	if (g_strv_length (removed_directories) > 0) {
		g_signal_emit (dir_store, signals[SIGNAL_DIRS_REMOVED], 0, removed_directories);
	}
	if (g_strv_length (added_directories) > 0) {
		g_signal_emit (dir_store, signals[SIGNAL_DIRS_ADDED], 0, added_directories);
	}

end:
	/* Use g_free instead of g_strfreev because shallow copies are made. */
	g_free (added_directories);
	g_free (removed_directories);
	g_strfreev (priv->current_directories);
	priv->current_directories = new_directories;
}

static void
ph_dir_store_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	PhDirStorePrivate *priv;

	priv = ph_dir_store_get_instance_private (PH_DIR_STORE (object));

	switch (prop_id) {
		case PROP_DIRECTORIES:
			g_settings_set_strv (priv->settings, KEY_DIRECTORIES,
					     (const gchar * const *) g_value_get_boxed (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_dir_store_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	PhDirStorePrivate *priv;

	priv = ph_dir_store_get_instance_private (PH_DIR_STORE (object));

	switch (prop_id) {
		case PROP_DIRECTORIES:
			g_value_set_boxed (value, priv->current_directories);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_dir_store_dispose (GObject *object)
{
	PhDirStorePrivate *priv;

	priv = ph_dir_store_get_instance_private (PH_DIR_STORE (object));

	if (priv->current_directories) {
		g_strfreev (priv->current_directories);
		priv->current_directories = NULL;
	}

	G_OBJECT_CLASS (ph_dir_store_parent_class)->dispose (object);
}

static void
ph_dir_store_class_init (PhDirStoreClass *ph_dir_store_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ph_dir_store_class);

	object_class->set_property = ph_dir_store_set_property;
	object_class->get_property = ph_dir_store_get_property;
	object_class->dispose = ph_dir_store_dispose;

	g_object_class_install_property (object_class, PROP_DIRECTORIES,
			g_param_spec_boxed ("directories",
					    "Directories",
					    "The loaded directories",
					    G_TYPE_STRV,
					    G_PARAM_READWRITE |
					    G_PARAM_STATIC_STRINGS));

	signals[SIGNAL_DIRS_ADDED] =
		g_signal_new ("directories-added", // FIXME: wrap in I_()?
			      PH_TYPE_DIR_STORE,
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRV);

	signals[SIGNAL_DIRS_REMOVED] =
		g_signal_new ("directories-removed", // FIXME: wrap in I_()?
			      PH_TYPE_DIR_STORE,
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOXED,
			      G_TYPE_NONE, 1,
			      G_TYPE_STRV);
}

static void
ph_dir_store_init (PhDirStore *dir_store)
{
	PhDirStorePrivate *priv;

	priv = ph_dir_store_get_instance_private (dir_store);

	priv->settings = g_settings_new (SCHEMA);
	priv->current_directories = NULL;

	g_signal_connect (priv->settings, "changed::"KEY_DIRECTORIES,
			  G_CALLBACK (on_directories_changed), dir_store);
	on_directories_changed (priv->settings, KEY_DIRECTORIES, dir_store);
}

PhDirStore *
ph_dir_store_new ()
{
	return g_object_new (PH_TYPE_DIR_STORE, NULL);
}

void
ph_dir_store_activate_added (PhDirStore *dir_store)
{
	PhDirStorePrivate *priv;

	g_return_if_fail (PH_IS_DIR_STORE (dir_store));

	priv = ph_dir_store_get_instance_private (dir_store);

	g_signal_emit (dir_store, signals[SIGNAL_DIRS_ADDED], 0, priv->current_directories);
}

