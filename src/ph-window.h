/* PhWindow
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

#include "ph-application.h"

G_BEGIN_DECLS

typedef struct _PhWindow        PhWindow;
typedef struct _PhWindowClass   PhWindowClass;
typedef struct _PhWindowPrivate PhWindowPrivate;

#define PH_TYPE_WINDOW            (ph_window_get_type ())
#define PH_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PH_TYPE_WINDOW, PhWindow))
#define PH_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  PH_TYPE_WINDOW, PhWindowClass))
#define PH_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PH_TYPE_WINDOW))
#define PH_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  PH_TYPE_WINDOW))
#define PH_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  PH_TYPE_WINDOW, PhWindowClass))

struct _PhWindow {
	GtkApplicationWindow base_instance;
};

struct _PhWindowClass {
	GtkApplicationWindowClass parent_class;
};

GType     ph_window_get_type (void);

PhWindow *ph_window_new (PhApplication *application);

void      ph_window_show_about_dialog (PhWindow *window);

void      ph_window_close (PhWindow *window);

void      ph_window_scan_directories (PhWindow *window, gchar * const *directories);

G_END_DECLS

