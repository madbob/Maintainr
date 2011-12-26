/*  Copyright (C) 2010/2011 Roberto Guido <bob4job@gmail.com>
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

#ifndef MAINTAINR_TODO_H
#define MAINTAINR_TODO_H

#include "common.h"

#define MAINTAINR_TODO_TYPE		(maintainr_todo_get_type ())
#define MAINTAINR_TODO(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
					 MAINTAINR_TODO_TYPE,			\
					 MaintainrTodo))
#define MAINTAINR_TODO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass),	\
					 MAINTAINR_TODO_TYPE,			\
					 MaintainrTodoClass))
#define IS_MAINTAINR_TODO(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
					 MAINTAINR_TODO_TYPE))
#define IS_MAINTAINR_TODO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
					 MAINTAINR_TODO_TYPE))
#define MAINTAINR_TODO_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
					 MAINTAINR_TODO_TYPE,			\
					 MaintainrTodoClass))

typedef struct _MaintainrTodo		MaintainrTodo;
typedef struct _MaintainrTodoClass	MaintainrTodoClass;
typedef struct _MaintainrTodoPrivate	MaintainrTodoPrivate;

struct _MaintainrTodo {
	GObject			parent;
	MaintainrTodoPrivate	*priv;
};

struct _MaintainrTodoClass {
	GObjectClass		parent_class;
};

GType		maintainr_todo_get_type ();

MaintainrTodo*	maintainr_todo_new ();
void		maintainr_todo_set_done (MaintainrTodo *todo, gboolean done);
gboolean	maintainr_todo_get_done (MaintainrTodo *todo);
void		maintainr_todo_set_string (MaintainrTodo *todo, gchar *string);
const gchar*	maintainr_todo_get_string (MaintainrTodo *todo);

#endif
