/* PhDir
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

#include <glib-object.h>

#include "ph-dir.h"

enum {
	PROP_0,
	PROP_PATH,
};

struct _PhDirPrivate {
	const gchar *path;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhDir, ph_dir, G_TYPE_OBJECT);

static void
ph_dir_set_property (GObject      *object,
		     guint         prop_id,
		     const GValue *value,
		     GParamSpec   *pspec)
{
	PhDirPrivate *priv;

	priv = ph_dir_get_instance_private (PH_DIR (object));

	switch (prop_id) {
		case PROP_PATH:
			priv->path = g_value_dup_string (value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_dir_get_property (GObject    *object,
		     guint       prop_id,
		     GValue     *value,
		     GParamSpec *pspec)
{
	PhDirPrivate *priv;

	priv = ph_dir_get_instance_private (PH_DIR (object));

	switch (prop_id) {
		case PROP_PATH:
			g_value_set_string (value, priv->path);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_dir_dispose (GObject *object)
{
	PhDirPrivate *priv;

	priv = ph_dir_get_instance_private (PH_DIR (object));

	g_free (&priv->path);

	G_OBJECT_CLASS (ph_dir_parent_class)->dispose (object);
}

static void
ph_dir_class_init (PhDirClass *ph_dir_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ph_dir_class);

	object_class->set_property = ph_dir_set_property;
	object_class->get_property = ph_dir_get_property;
	object_class->dispose = ph_dir_dispose;

	g_object_class_install_property (object_class, PROP_PATH,
					 g_param_spec_string ("path",
							      "Path",
							      "The path of this PhDir",
							       NULL,
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT_ONLY |
							       G_PARAM_STATIC_STRINGS));
}

static void
ph_dir_init (UNUSED PhDir *dir)
{
}

PhDir *
ph_dir_new (const gchar *path)
{
	g_return_val_if_fail (path != NULL, NULL);

	return g_object_new (PH_TYPE_DIR,
			     "path", path,
			     NULL);
}

gchar *
ph_dir_get_path (PhDir *dir)
{
	PhDirPrivate *priv;

	g_return_val_if_fail (PH_IS_DIR (dir), NULL);

	priv = ph_dir_get_instance_private (dir);

	return g_strdup (priv->path);
}

