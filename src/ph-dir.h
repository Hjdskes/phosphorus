/* PhDir
 *
 * Copyright (C) 2016 Jente Hidskes
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

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _PhDir        PhDir;
typedef struct _PhDirClass   PhDirClass;
typedef struct _PhDirPrivate PhDirPrivate;

#define PH_TYPE_DIR            (ph_dir_get_type ())
#define PH_DIR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_DIR, PhDir))
#define PH_DIR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_DIR, PhDirClass))
#define PH_IS_DIR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_DIR))
#define PH_IS_DIR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_DIR))
#define PH_DIR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_DIR, PhDirClass))

struct _PhDir {
	GObject base_instance;
};

struct _PhDirClass {
	GObjectClass parent_class;
};

GType  ph_dir_get_type (void);

PhDir *ph_dir_new (const gchar *path);

gchar *ph_dir_get_path (PhDir *dir);

G_END_DECLS

