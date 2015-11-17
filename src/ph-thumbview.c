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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "ph-thumbview.h"

struct _PhThumbviewPrivate {
	GtkWidget *placeholder;
};

G_DEFINE_TYPE_WITH_PRIVATE (PhThumbview, ph_thumbview, GTK_TYPE_SCROLLED_WINDOW);

static void
ph_thumbview_class_init (PhThumbviewClass *ph_thumbview_class)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (ph_thumbview_class);

	gtk_widget_class_set_template_from_resource (widget_class,
			"/org/unia/phosphorus/thumbview.ui");

	//gtk_widget_class_bind_template_child_private (widget_class, PhThumbview, );
}

static void
ph_thumbview_init (PhThumbview *thumbview)
{
	//PhThumbviewPrivate *priv;

	//priv = ph_thumbview_get_instance_private (thumbview);

	gtk_widget_init_template (GTK_WIDGET (thumbview));
}

PhThumbview *
ph_thumbview_new ()
{
	return g_object_new (PH_TYPE_THUMBVIEW, NULL);
}

