/* PhBackend
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

#define PH_TYPE_BACKEND ph_backend_get_type ()

G_DECLARE_INTERFACE (PhBackend, ph_backend, PH, BACKEND, GObject)

struct _PhBackendInterface {
	GTypeInterface interface;

	void (*set_background) (PhBackend *backend, const gchar *filepath);
};

void ph_backend_set_background (PhBackend *backend, const gchar *filepath);

G_END_DECLS

