/* PhApplication
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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libpeas/peas.h>

#include <ph-plugin.h>

#include "ph-application.h"
#include "ph-preferences-dialog.h"
#include "ph-thumbview.h"
#include "ph-window.h"
#include "util.h"

#define SCHEMA "org.unia.phosphorus"
#define KEY_DIRECTORIES "directories"
#define KEY_RECURSE "recursion"

struct _PhApplicationPrivate {
	GSettings *settings;
	PeasEngine *engine;
	PeasExtensionSet *extensions;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhApplication, ph_application, GTK_TYPE_APPLICATION);

static void
ph_application_action_preferences (GSimpleAction *action,
				   GVariant      *parameter,
				   gpointer       user_data)
{
	GtkApplication *application = GTK_APPLICATION (user_data);
	PhWindow *window;

	window = PH_WINDOW (gtk_application_get_active_window (application));
	ph_preferences_dialog_show (window);
}

static void
ph_application_action_about (GSimpleAction *action,
			     GVariant      *parameter,
			     gpointer       user_data)
{
	GtkWindow *window;

	window = gtk_application_get_active_window (GTK_APPLICATION (user_data));
	g_return_if_fail (PH_IS_WINDOW (window));

	ph_window_show_about_dialog (PH_WINDOW (window));
}

static void
ph_application_action_quit (GSimpleAction *action,
			    GVariant      *parameter,
			    gpointer       user_data)
{
	GList *windows;

	windows = gtk_application_get_windows (GTK_APPLICATION (user_data));

	g_list_foreach (windows, (GFunc) ph_window_close, NULL);
}

static GActionEntry app_entries[] = {
	{ "preferences", ph_application_action_preferences, NULL, NULL, NULL },
	{ "about", ph_application_action_about, NULL, NULL, NULL },
	{ "quit",  ph_application_action_quit,  NULL, NULL, NULL },
};

static void
ph_application_init_accelerators (GtkApplication *application)
{
	/* Taken from eog, which in turn has this based on a simular
	 * construct in Evince (src/ev-application.c).
	 * Setting multiple accelerators at once for an action
	 * is not very straight forward in a static way.
	 *
	 * This gchar* array simulates an action<->accels mapping.
	 * Enter the action name followed by the accelerator strings
	 * and terminate the entry with a NULL-string.*/
	static const gchar *const accelmap[] = {
		"win.apply", "<Ctrl>a", NULL,
		NULL /* Terminating NULL */
	};

	const gchar *const *it;
	for (it = accelmap; it[0]; it += g_strv_length ((gchar **) it) + 1) {
		gtk_application_set_accels_for_action (application, it[0], &it[1]);
	}
}

static void
ph_application_startup (GApplication *application)
{
	PhApplicationPrivate *priv;

	G_APPLICATION_CLASS (ph_application_parent_class)->startup (application);

	priv = ph_application_get_instance_private (PH_APPLICATION (application));

	// TODO: update live when new directory is added
	priv->settings = g_settings_new (SCHEMA);

	g_set_application_name (_("Wallpaper browser"));

	g_action_map_add_action_entries (G_ACTION_MAP (application),
					 app_entries, G_N_ELEMENTS (app_entries),
					 application);

	ph_application_init_accelerators (GTK_APPLICATION (application));
}

static void
ph_application_activate (GApplication *application)
{
	PhApplicationPrivate *priv;
	PhWindow *window;
	PhRecurseType recurse;
	gchar **directories;

	priv = ph_application_get_instance_private (PH_APPLICATION (application));

	directories = g_settings_get_strv (priv->settings, KEY_DIRECTORIES);
	recurse = g_settings_get_enum (priv->settings, KEY_RECURSE);
	window = ph_window_new (PH_APPLICATION (application));
	ph_window_scan_directories (window, recurse, directories);
	g_strfreev (directories);

	gtk_window_present_with_time (GTK_WINDOW (window), GDK_CURRENT_TIME);
}

static void
ph_application_dispose (GObject *object)
{
	PhApplicationPrivate *priv;

	priv = ph_application_get_instance_private (PH_APPLICATION (object));

	g_clear_object (&priv->settings);
	g_clear_object (&priv->engine);
	g_clear_object (&priv->extensions);

	G_OBJECT_CLASS (ph_application_parent_class)->dispose (object);
}

static void
ph_application_class_init (PhApplicationClass *ph_application_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (ph_application_class);
	GApplicationClass *application_class = G_APPLICATION_CLASS (ph_application_class);

	object_class->dispose = ph_application_dispose;

	application_class->startup = ph_application_startup;
	application_class->activate = ph_application_activate;
}

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
init_peas (PhApplication *application)
{
	PhApplicationPrivate *priv;

	priv = ph_application_get_instance_private (application);

	priv->engine = peas_engine_get_default ();
	/* Require our own typelib, see gedit-plugin-engine.c. */
	require_phosphorus_typelib ();
	/* FIXME: Does this interfere with other PeasEngines? See enable_loader
	 * documentation. */
	peas_engine_enable_loader (priv->engine, "python3");
	/* Add Phosphorus and user directory plugins. */
	add_plugin_dirs (priv->engine);
	peas_engine_rescan_plugins (priv->engine);

	priv->extensions = peas_extension_set_new (priv->engine, PH_TYPE_PLUGIN, NULL);

	/* Load all plugins that were found. FIXME: disable apply button when no plugins are found. */
	const GList *plugins = peas_engine_get_plugin_list (priv->engine);
	for (const GList *plugin = plugins; plugin != NULL; plugin = plugin->next) {
		PeasPluginInfo *info = plugin->data;
		if (!peas_engine_load_plugin (priv->engine, info)) {
			const gchar *name = peas_plugin_info_get_name (info);
			g_printerr (_("Could not load plugin %s\n"), name);
		}
	}
}

static void
ph_application_init (PhApplication *application)
{
	init_peas (application);
}

PhApplication *
ph_application_new (void)
{
	GObject *application;

	application = g_object_new (PH_TYPE_APPLICATION,
				    "application-id", "org.unia.phosphorus",
				    "flags", G_APPLICATION_FLAGS_NONE,
				    NULL);

	return PH_APPLICATION (application);
}

// FIXME: perhaps PhWindow should have the filepath of the currently selected image as a property,
// with a signal whenever that property changes. PhApplication could then instead connect to this
// signal. This signal can then be ignored when no plugins are registered.
void
ph_application_proxy_plugin (PhApplication *application, const gchar *filepath)
{
	PhApplicationPrivate *priv;
	PeasPluginInfo *info;
	PeasExtension *extension;
	const GList *list;

	g_return_if_fail (application != NULL);
	g_return_if_fail (filepath != NULL);

	priv = ph_application_get_instance_private (application);
	list = peas_engine_get_plugin_list (priv->engine);
	/* FIXME: If apply is disabled when no plugins, this can go. Currently here to prevent a
	 * segault only. */
	if (list == NULL) {
		g_printerr (_("No plugins found. Can't apply wallpaper\n"));
		return;
	}

	/* Apply the first plugin. FIXME: in the future, we should make sure only one plugin is
	 * active at once. */
	info = PEAS_PLUGIN_INFO (list->data);
	extension = peas_extension_set_get_extension (priv->extensions, info);
	ph_plugin_set_background (PH_PLUGIN (extension), filepath);
}

