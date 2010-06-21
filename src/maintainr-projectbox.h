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

#ifndef MAINTAINR_PROJECTBOX_H
#define MAINTAINR_PROJECTBOX_H

#include "common.h"
#include "maintainr-projectconf.h"

#define MAINTAINR_PROJECTBOX_TYPE		(maintainr_projectbox_get_type ())
#define MAINTAINR_PROJECTBOX(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
						 MAINTAINR_PROJECTBOX_TYPE,		\
						 MaintainrProjectbox))
#define MAINTAINR_PROJECTBOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass),	\
						 MAINTAINR_PROJECTBOX_TYPE,		\
						 MaintainrProjectboxClass))
#define IS_MAINTAINR_PROJECTBOX(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
						 MAINTAINR_PROJECTBOX_TYPE))
#define IS_MAINTAINR_PROJECTBOX_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
						 MAINTAINR_PROJECTBOX_TYPE))
#define MAINTAINR_PROJECTBOX_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
						 MAINTAINR_PROJECTBOX_TYPE,		\
						 MaintainrProjectboxClass))

typedef struct _MaintainrProjectbox		MaintainrProjectbox;
typedef struct _MaintainrProjectboxClass	MaintainrProjectboxClass;
typedef struct _MaintainrProjectboxPrivate	MaintainrProjectboxPrivate;

struct _MaintainrProjectbox {
	GtkNotebook			parent;
	MaintainrProjectboxPrivate	*priv;
};

struct _MaintainrProjectboxClass {
	GtkNotebookClass		parent_class;
};

GType			maintainr_projectbox_get_type ();

GtkWidget*		maintainr_projectbox_new ();
void			maintainr_projectbox_set_editing_mode (MaintainrProjectbox *box);
void			maintainr_projectbox_set_conf (MaintainrProjectbox *box, MaintainrProjectconf *conf);
MaintainrProjectconf*	maintainr_projectbox_get_conf (MaintainrProjectbox *box);

#endif
