/* PhPlugin
 *
 * Copyright (C) 2015-2016 Jente Hidskes
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
#include <glib/gi18n.h>

#include "ph-plugin.h"

/**
 * SECTION:ph-plugin
 * @short_description: Interface for Phosphorus plugins
 *
 * #PhPlugin is an interface which all plugins to Phosphorus must implement.
 * It defines only one method, which is called by Phosphorus when the apply
 * button is clicked by the user.
 */
G_DEFINE_INTERFACE (PhPlugin, ph_plugin, G_TYPE_OBJECT);

static void
ph_plugin_default_init (PhPluginInterface *interface)
{
}

// FIXME: (not nullable) is not recognized. Documentation is outdated?
/**
 * ph_plugin_set_background:
 * @plugin: (skip): The #PhPlugin whose #ph_plugin_set_background to call.
 * @filepath: (in) (not nullable): The path to the selected image.
 *
 * This method is called by Phosphorus when the apply button is clicked by the user.
 * Plugins are free in deciding how to apply the wallpaper, but they *must* use the
 * provided file.
 */
void
ph_plugin_set_background (PhPlugin *plugin, const gchar *filepath)
{
	PhPluginInterface *interface = NULL;

	g_return_if_fail (PH_IS_PLUGIN (plugin));
	g_return_if_fail (filepath != NULL);

	interface = PH_PLUGIN_GET_IFACE (plugin);
	if (interface && interface->set_background) {
		interface->set_background (plugin, filepath);
	} else {
		g_printerr (_("Plugin does not define set_background!\n"));
	}
}

