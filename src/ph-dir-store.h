/* PhDirStore
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

typedef struct _PhDirStore        PhDirStore;
typedef struct _PhDirStoreClass   PhDirStoreClass;
typedef struct _PhDirStorePrivate PhDirStorePrivate;

#define PH_TYPE_DIR_STORE            (ph_dir_store_get_type ())
#define PH_DIR_STORE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_DIR_STORE, PhDirStore))
#define PH_DIR_STORE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_DIR_STORE, PhDirStoreClass))
#define PH_IS_DIR_STORE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_DIR_STORE))
#define PH_IS_DIR_STORE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_DIR_STORE))
#define PH_DIR_STORE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_DIR_STORE, PhDirStoreClass))

struct _PhDirStore {
	GObject base_instance;
};

struct _PhDirStoreClass {
	GObjectClass parent_class;
};

GType       ph_dir_store_get_type (void);

PhDirStore *ph_dir_store_new (void);

void        ph_dir_store_activate_added (PhDirStore *dir_store);

G_END_DECLS

