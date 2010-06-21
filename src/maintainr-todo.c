/*  Copyright (C) 2010 Roberto Guido <madbob@users.barberaware.org>
 *
 *  This file is part of Maintainr
 *
 *  Maintainr is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "maintainr-todo.h"

#define MAINTAINR_TODO_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_TODO_TYPE, MaintainrTodoPrivate))

struct _MaintainrTodoPrivate {
	gboolean done;
	gchar *string;
};

G_DEFINE_TYPE (MaintainrTodo, maintainr_todo, G_TYPE_OBJECT);

static void maintainr_todo_finalize (GObject *obj)
{
	MaintainrTodo *conf;

	conf = MAINTAINR_TODO (obj);

	if (conf->priv->string != NULL)
		g_free (conf->priv->string);
}

static void maintainr_todo_class_init (MaintainrTodoClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_todo_finalize;

	g_type_class_add_private (klass, sizeof (MaintainrTodoPrivate));
}

static void maintainr_todo_init (MaintainrTodo *item)
{
	item->priv = MAINTAINR_TODO_GET_PRIVATE (item);
}

MaintainrTodo* maintainr_todo_new ()
{
	return g_object_new (MAINTAINR_TODO_TYPE, NULL);
}

void maintainr_todo_set_done (MaintainrTodo *todo, gboolean done)
{
	todo->priv->done = done;
}

gboolean maintainr_todo_get_done (MaintainrTodo *todo)
{
	return todo->priv->done;
}

void maintainr_todo_set_string (MaintainrTodo *todo, gchar *string)
{
	todo->priv->string = g_strdup (string);
}

const gchar* maintainr_todo_get_string (MaintainrTodo *todo)
{
	return (const gchar*) todo->priv->string;
}
