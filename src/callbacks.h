/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * callbacks.h
 * Copyright (C) 2013 Jente Hidskes <jthidskes@outlook.com>
 *
 * Phosphorus is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Phosphorus is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

G_BEGIN_DECLS

void on_apply_button_clicked (GtkButton *button, gpointer user_data);
void on_combo_changed (GtkComboBox *combo, gpointer user_data);
void on_item_activated (GtkIconView *view, GtkTreePath *path, gpointer user_data);
void on_color_button_clicked (GtkColorButton *button, gpointer user_data);

G_END_DECLS

#endif
