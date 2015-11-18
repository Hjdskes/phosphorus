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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _PhBackendGnome        PhBackendGnome;
typedef struct _PhBackendGnomeClass   PhBackendGnomeClass;
typedef struct _PhBackendGnomePrivate PhBackendGnomePrivate;

#define PH_TYPE_BACKEND_GNOME            (ph_backend_gnome_get_type ())
#define PH_BACKEND_GNOME(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_BACKEND_GNOME, PhBackendGnome))
#define PH_BACKEND_GNOME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_BACKEND_GNOME, PhBackendGnomeClass))
#define PH_IS_BACKEND_GNOME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_BACKEND_GNOME))
#define PH_IS_BACKEND_GNOME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_BACKEND_GNOME))
#define PH_BACKEND_GNOME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_BACKEND_GNOME, PhBackendGnomeClass))

struct _PhBackendGnome {
	GObject base_instance;
};

struct _PhBackendGnomeClass {
	GObjectClass parent_class;
};

GType      ph_backend_gnome_get_type (void);

PhBackend *ph_backend_gnome_new (void);

G_END_DECLS

