/* PhPluginManager
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

#include <girepository.h>
#include <glib/gi18n.h>
#include <libpeas/peas.h>

#include <plugin/ph-plugin.h>

#include "ph-application.h"
#include "ph-plugin-manager.h"
#include "gsettings.h"
#include "util.h"

enum {
	PROP_0,
	PROP_APPLICATION,
};

struct _PhPluginManager {
	GObject parent_instance;

	PhApplication *application;
	GSettings *plugin_settings;
	PeasEngine *engine;
	PeasExtensionSet *extensions;
};

G_DEFINE_TYPE (PhPluginManager, ph_plugin_manager, G_TYPE_OBJECT)

static void
require_phosphorus_typelib (void)
{
	gchar *typelib_dir;
	GError *error = NULL;

	typelib_dir = g_build_filename (LIB_DIR, "phosphorus", "girepository-1.0", NULL);

	if (!g_irepository_require_private (g_irepository_get_default (),
					    typelib_dir, "Phosphorus", "1.0", 0, &error)) {
		g_warning (_("Could not load Phosphorus repository: %s"), error->message);
		g_clear_error (&error);
	}
	g_free (typelib_dir);
}

static void
add_plugin_dirs (PeasEngine *engine)
{
	gchar *user_dir;
	gchar *plugin_dir;
	gchar *plugin_data_dir;

	ph_get_plugin_dirs (&plugin_dir, &plugin_data_dir);
	peas_engine_add_search_path (engine, plugin_dir, plugin_data_dir);

	ph_get_user_dir (&user_dir);
	peas_engine_add_search_path (engine, user_dir, NULL);

	g_free (user_dir);
	g_free (plugin_dir);
	g_free (plugin_data_dir);
}

static void
set_background (UNUSED PeasExtensionSet *extensions,
		UNUSED PeasPluginInfo   *info,
		PeasExtension           *extension,
		gpointer                 user_data)
{
	const gchar *filepath = (const gchar *) user_data;

	ph_plugin_set_background (PH_PLUGIN (extension), filepath);
}

static void
extension_added (UNUSED PeasExtensionSet *extensions,
		 UNUSED PeasPluginInfo   *info,
		 PeasExtension           *extension,
		 UNUSED gpointer          user_data)
{
	ph_plugin_load (PH_PLUGIN (extension));
}

static void
extension_removed (UNUSED PeasExtensionSet *extensions,
		   UNUSED PeasPluginInfo   *info,
		   PeasExtension           *extension,
		   UNUSED gpointer          user_data)
{
	ph_plugin_unload (PH_PLUGIN (extension));
}

static void
ph_plugin_manager_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
	PhPluginManager *manager = PH_PLUGIN_MANAGER (object);

	switch (prop_id) {
		case PROP_APPLICATION:
			manager->application = PH_APPLICATION (g_value_dup_object (value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_plugin_manager_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
	PhPluginManager *manager = PH_PLUGIN_MANAGER (object);

	switch (prop_id) {
		case PROP_APPLICATION:
			g_value_set_object (value, manager->application);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_plugin_manager_dispose (GObject *object)
{
	PhPluginManager *manager = PH_PLUGIN_MANAGER (object);

	g_clear_object (&manager->plugin_settings);
	g_clear_object (&manager->application);
	g_clear_object (&manager->engine);
	g_clear_object (&manager->extensions);

	G_OBJECT_CLASS (ph_plugin_manager_parent_class)->dispose (object);
}

static void
ph_plugin_manager_class_init (PhPluginManagerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = ph_plugin_manager_set_property;
	object_class->get_property = ph_plugin_manager_get_property;
	object_class->dispose = ph_plugin_manager_dispose;

	g_object_class_install_property (object_class, PROP_APPLICATION,
					 g_param_spec_object ("application",
							      "Application",
							      "The Phosphorus application",
							      PH_TYPE_APPLICATION,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));
}

static void
ph_plugin_manager_init (PhPluginManager *manager)
{
	manager->plugin_settings = g_settings_new (SCHEMA);
	manager->engine = peas_engine_get_default ();

	/* Require our own typelib, see gedit-plugin-manager.c. */
	require_phosphorus_typelib ();
	/* Add Phosphorus and user directory plugins. */
	add_plugin_dirs (manager->engine);
	peas_engine_enable_loader (manager->engine, "python3");
	peas_engine_rescan_plugins (manager->engine);

	manager->extensions = peas_extension_set_new (manager->engine, PH_TYPE_PLUGIN,
						      "application", manager->application,
						      NULL);

	g_signal_connect (manager->extensions, "extension-added", G_CALLBACK (extension_added),
			  manager);
	g_signal_connect (manager->extensions, "extension-removed", G_CALLBACK (extension_removed),
			  manager);

	peas_extension_set_foreach (manager->extensions,
				    (PeasExtensionSetForeachFunc) extension_added, manager);

	/* FIXME: disable apply button when no plugins are found. */

	g_settings_bind (manager->plugin_settings, "active-plugins",
	                 manager->engine, "loaded-plugins",
	                 G_SETTINGS_BIND_DEFAULT);
}

PhPluginManager *
ph_plugin_manager_get_default (PhApplication *application)
{
	static PhPluginManager *default_manager = NULL;

	if (!default_manager) {
		default_manager = PH_PLUGIN_MANAGER (g_object_new (PH_PLUGIN_MANAGER_TYPE,
								   "application", application,
								   NULL));
		g_object_add_weak_pointer (G_OBJECT (default_manager), (gpointer) &default_manager);
	}

	return default_manager;
}

void
ph_plugin_manager_proxy_plugins (PhPluginManager *manager, const gchar *filepath)
{
	g_return_if_fail (PH_IS_PLUGIN_MANAGER (manager));
	g_return_if_fail (filepath != NULL);

	peas_extension_set_foreach (manager->extensions,
				    (PeasExtensionSetForeachFunc) set_background,
				    (gpointer) filepath);
}

