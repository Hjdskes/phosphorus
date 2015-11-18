/* PhBackendGnome
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

#include <gio/gio.h>
#include <glib/gi18n.h>

#include "ph-backend.h"
#include "ph-backend-gnome.h"

#define SCHEMA "org.gnome.desktop.background"
#define KEY_BACKGROUND "picture-uri"
// TODO: implement support for these in Phosphorus
#define KEY_PRIMARY_COLOR "primary-color"
#define KEY_SECONDARY_COLOR "secondary-color"
#define KEY_COLOR_SHADING_TYPE "color-shading-type"
#define KEY_BACKGROUND_STYLE "picture-options"

static void ph_backend_interface_init (PhBackendInterface *interface);

struct _PhBackendGnomePrivate {
	GSettings *settings;
};

G_DEFINE_TYPE_WITH_CODE (PhBackendGnome, ph_backend_gnome, G_TYPE_OBJECT,
			 G_ADD_PRIVATE (PhBackendGnome)
			 G_IMPLEMENT_INTERFACE (PH_TYPE_BACKEND, ph_backend_interface_init));

static void
ph_backend_gnome_set_background (PhBackend *backend, const gchar *filepath)
{
	PhBackendGnomePrivate *priv;
	gboolean success;

	priv = ph_backend_gnome_get_instance_private (PH_BACKEND_GNOME (backend));

	success = g_settings_set_string (priv->settings, KEY_BACKGROUND, filepath);
	if (!success) {
		g_printerr (_("The key %s is not writable\n"), KEY_BACKGROUND);
	}
}

static void
ph_backend_interface_init (PhBackendInterface *interface)
{
	interface->set_background = ph_backend_gnome_set_background;
}

static void
ph_backend_gnome_dispose (GObject *object)
{
	PhBackendGnomePrivate *priv;

	priv = ph_backend_gnome_get_instance_private (PH_BACKEND_GNOME (object));

	if (priv->settings) {
		g_object_unref (priv->settings);
		priv->settings = NULL;
	}

	G_OBJECT_CLASS (ph_backend_gnome_parent_class)->dispose (object);
}

static void
ph_backend_gnome_class_init (PhBackendGnomeClass *ph_backend_gnome_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ph_backend_gnome_class);

	object_class->dispose = ph_backend_gnome_dispose;
}

static void
ph_backend_gnome_init (PhBackendGnome *backend)
{
	PhBackendGnomePrivate *priv;

	priv = ph_backend_gnome_get_instance_private (backend);

	priv->settings = g_settings_new (SCHEMA);
}

PhBackend *
ph_backend_gnome_new ()
{
	return PH_BACKEND (g_object_new (PH_TYPE_BACKEND_GNOME, NULL));
}

