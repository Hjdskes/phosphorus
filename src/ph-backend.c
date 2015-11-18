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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib-object.h>

#include "ph-backend.h"

G_DEFINE_INTERFACE (PhBackend, ph_backend, G_TYPE_OBJECT);

static void
ph_backend_default_init (PhBackendInterface *interface)
{
}

void
ph_backend_set_background (PhBackend *backend, const gchar *filepath)
{
	g_return_if_fail (PH_IS_BACKEND (backend));
	g_return_if_fail (filepath != NULL);

	PH_BACKEND_GET_IFACE (backend)->set_background (backend, filepath);
}

