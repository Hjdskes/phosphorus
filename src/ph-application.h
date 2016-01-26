/* PhApplication
 *
 * Copyright (C) 2015-2016 Jente Hidskes
 *
 * Author: Jente Hidskes <hjdskes@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _PhApplication        PhApplication;
typedef struct _PhApplicationClass   PhApplicationClass;
typedef struct _PhApplicationPrivate PhApplicationPrivate;

#define PH_TYPE_APPLICATION            (ph_application_get_type ())
#define PH_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_APPLICATION, PhApplication))
#define PH_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_APPLICATION, PhApplicationClass))
#define PH_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_APPLICATION))
#define PH_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_APPLICATION))
#define PH_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_APPLICATION, PhApplicationClass))

struct _PhApplication {
	GtkApplication base_instance;
};

struct _PhApplicationClass {
	GtkApplicationClass parent_class;
};

GType          ph_application_get_type (void);

PhApplication *ph_application_new (gboolean restore_wallpaper);

G_END_DECLS

