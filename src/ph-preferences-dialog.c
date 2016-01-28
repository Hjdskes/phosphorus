/* PhPreferencesDialog
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
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <libpeas-gtk/peas-gtk.h>

#include "ph-dir.h"
#include "ph-preferences-dialog.h"
#include "gsettings.h"

enum {
	PROP_0,
	PROP_STORE,
};

enum {
	CLOSE,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

struct _PhPreferencesDialog {
	GtkWindow parent_instance;

	GListStore *dir_store;

	GSettings *settings;

	GtkWidget *dir_list;
	GtkWidget *recurse_switch;
	GtkWidget *plugin_manager;
};

G_DEFINE_TYPE (PhPreferencesDialog, ph_preferences_dialog, GTK_TYPE_WINDOW)

// FIXME: make pretty box with remove button 'n all
static GtkWidget *
widget_create_func (gpointer item, UNUSED gpointer user_data)
{
	PhDir *dir = PH_DIR (item);
	GtkWidget *label;
	gchar *directory;

	directory = ph_dir_get_path (dir);
	label = gtk_label_new (directory);
	g_free (directory);

	return label;
}

// TODO: set placeholder in list box when no directories are added.
static void
setup_phosphorus_page (PhPreferencesDialog *dialog)
{
	gtk_widget_show_all (dialog->dir_list);

	g_settings_bind (dialog->settings, KEY_RECURSE,
			 dialog->recurse_switch, "active",
			 G_SETTINGS_BIND_DEFAULT);
}

static void
setup_plugin_page (PhPreferencesDialog *dialog)
{
	gtk_widget_show_all (dialog->plugin_manager);
}

static void
ph_preferences_dialog_set_property (GObject      *object,
				    guint         prop_id,
				    const GValue *value,
				    GParamSpec   *pspec)
{
	PhPreferencesDialog *dialog = PH_PREFERENCES_DIALOG (object);

	switch (prop_id) {
		case PROP_STORE:
			dialog->dir_store = G_LIST_STORE (g_value_dup_object (value));
			gtk_list_box_bind_model (GTK_LIST_BOX (dialog->dir_list),
						 G_LIST_MODEL (dialog->dir_store),
						 (GtkListBoxCreateWidgetFunc) widget_create_func,
						 NULL, NULL);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_preferences_dialog_get_property (GObject    *object,
				    guint       prop_id,
				    GValue     *value,
				    GParamSpec *pspec)
{
	PhPreferencesDialog *dialog = PH_PREFERENCES_DIALOG (object);

	switch (prop_id) {
		case PROP_STORE:
			g_value_set_object (value, dialog->dir_store);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ph_preferences_dialog_dispose (GObject *object)
{
	PhPreferencesDialog *dialog = PH_PREFERENCES_DIALOG (object);

	g_clear_object (&dialog->settings);
	g_clear_object (&dialog->dir_store);

	G_OBJECT_CLASS (ph_preferences_dialog_parent_class)->dispose (object);
}

static void
ph_preferences_dialog_close (PhPreferencesDialog *dialog)
{
	gtk_window_close (GTK_WINDOW (dialog));
}

static void
ph_preferences_dialog_class_init (PhPreferencesDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkBindingSet *binding_set;

	object_class->set_property = ph_preferences_dialog_set_property;
	object_class->get_property = ph_preferences_dialog_get_property;
	object_class->dispose = ph_preferences_dialog_dispose;

	g_object_class_install_property (object_class, PROP_STORE,
					 g_param_spec_object ("directory-store",
							      "DirectoryStore",
							      "The directory store",
							      G_TYPE_LIST_STORE,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	signals[CLOSE] = g_signal_new_class_handler ("close",
						     G_TYPE_FROM_CLASS (klass),
						     G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
						     G_CALLBACK (ph_preferences_dialog_close),
						     NULL, NULL, NULL,
						     G_TYPE_NONE,
						     0);

	binding_set = gtk_binding_set_by_class (klass);
	gtk_binding_entry_add_signal (binding_set, GDK_KEY_Escape, 0, "close", 0);

	gtk_widget_class_set_template_from_resource (widget_class,
						     "/org/unia/phosphorus/preferences.ui");
	gtk_widget_class_bind_template_child (widget_class, PhPreferencesDialog, dir_list);
	gtk_widget_class_bind_template_child (widget_class, PhPreferencesDialog, recurse_switch);
	gtk_widget_class_bind_template_child (widget_class, PhPreferencesDialog, plugin_manager);
}

static void
ph_preferences_dialog_init (PhPreferencesDialog *dialog)
{
	dialog->settings = g_settings_new (SCHEMA);

	gtk_widget_init_template (GTK_WIDGET (dialog));

	setup_phosphorus_page (dialog);
	setup_plugin_page (dialog);
}

PhPreferencesDialog *
ph_preferences_dialog_new (GListStore *dir_store)
{
	g_return_val_if_fail (G_LIST_STORE (dir_store), NULL);

	return g_object_new (PH_PREFERENCES_DIALOG_TYPE,
			     "directory-store", dir_store,
			     NULL);
}

