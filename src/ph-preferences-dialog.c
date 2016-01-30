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

#include "ph-preferences-dialog.h"
#include "gsettings.h"

/* Modeled after Gedit's preferences dialog:
 *
 * ph-preferences-dialog is a singleton since we don't
 * want two dialogs showing an inconsistent state of the
 * preferences. When ph_preferences_dialog_show is called
 * and there is already a dialog open, it is reparented
 * and shown.
 */

enum {
	CLOSE,
	LAST_SIGNAL,
};

static guint signals[LAST_SIGNAL];

struct _PhPreferencesDialog {
	GtkWindow parent;

	GSettings *settings;

	GtkWidget *recurse_switch;
	GtkWidget *plugin_manager;
};

G_DEFINE_TYPE (PhPreferencesDialog, ph_preferences_dialog, GTK_TYPE_WINDOW)

static void
setup_phosphorus_page (PhPreferencesDialog *dialog)
{
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
ph_preferences_dialog_dispose (GObject *object)
{
	PhPreferencesDialog *dialog = PH_PREFERENCES_DIALOG (object);

	g_clear_object (&dialog->settings);

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

	object_class->dispose = ph_preferences_dialog_dispose;

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

void
ph_preferences_dialog_show (GtkWindow *window)
{
	static GtkWindow *preferences_dialog = NULL;

	if (preferences_dialog == NULL) {
		preferences_dialog = GTK_WINDOW (g_object_new (PH_PREFERENCES_DIALOG_TYPE,
							       "application",
							       g_application_get_default (),
							       NULL));
		g_signal_connect (preferences_dialog, "destroy", G_CALLBACK (gtk_widget_destroyed),
				  &preferences_dialog);
	}

	if (window != gtk_window_get_transient_for (preferences_dialog)) {
		gtk_window_set_transient_for (preferences_dialog, window);
	}

	gtk_window_present (preferences_dialog);
}

