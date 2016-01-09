/* XLibWallpaperSetter
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

#pragma once

#include <glib-object.h>

#define XLIB_WALLPAPER_SETTER_TYPE            (xlib_wallpaper_setter_get_type ())
#define XLIB_WALLPAPER_SETTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XLIB_WALLPAPER_SETTER_TYPE, XLibWallpaperSetter))
#define XLIB_WALLPAPER_SETTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), XLIB_WALLPAPER_SETTER_TYPE, XLibWallpaperSetterClass))
#define XLIB_IS_WALLPAPER_SETTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XLIB_WALLPAPER_SETTER_TYPE))
#define XLIB_IS_WALLPAPER_SETTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XLIB_WALLPAPER_SETTER_TYPE))
#define XLIB_CHECK_UPDATE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), XLIB_WALLPAPER_SETTER_TYPE, XLibWallpaperSetterClass))

typedef struct _XLibWallpaperSetter	   XLibWallpaperSetter;
typedef struct _XLibWallpaperSetterClass   XLibWallpaperSetterClass;
typedef struct _XLibWallpaperSetterPrivate XLibWallpaperSetterPrivate;

struct _XLibWallpaperSetter {
	GObject parent;
};

struct _XLibWallpaperSetterClass {
	GObjectClass parent_class;
};

GType                xlib_wallpaper_setter_get_type (void);

G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

