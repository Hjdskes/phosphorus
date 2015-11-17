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

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _PhThumbview        PhThumbview;
typedef struct _PhThumbviewClass   PhThumbviewClass;
typedef struct _PhThumbviewPrivate PhThumbviewPrivate;

#define PH_TYPE_THUMBVIEW            (ph_thumbview_get_type ())
#define PH_THUMBVIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_THUMBVIEW, PhThumbview))
#define PH_THUMBVIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_THUMBVIEW, PhThumbviewClass))
#define PH_IS_THUMBVIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_THUMBVIEW))
#define PH_IS_THUMBVIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_THUMBVIEW))
#define PH_THUMBVIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_THUMBVIEW, PhThumbviewClass))

struct _PhThumbview {
	GtkScrolledWindow base_instance;
};

struct _PhThumbviewClass {
	GtkScrolledWindowClass parent_class;
};

GType        ph_thumbview_get_type (void);

PhThumbview *ph_thumbview_new (void);

G_END_DECLS

