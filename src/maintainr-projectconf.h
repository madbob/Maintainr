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

#ifndef MAINTAINR_PROJECTCONF_H
#define MAINTAINR_PROJECTCONF_H

#include "common.h"
#include "maintainr-service.h"

#define MAINTAINR_PROJECTCONF_TYPE		(maintainr_projectconfig_get_type ())
#define MAINTAINR_PROJECTCONF(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
						 MAINTAINR_PROJECTCONF_TYPE,		\
						 MaintainrProjectconf))
#define MAINTAINR_PROJECTCONF_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass),	\
						 MAINTAINR_PROJECTCONF_TYPE,		\
						 MaintainrProjectconfClass))
#define IS_MAINTAINR_PROJECTCONF(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
						 MAINTAINR_PROJECTCONF_TYPE))
#define IS_MAINTAINR_PROJECTCONF_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
						 MAINTAINR_PROJECTCONF_TYPE))
#define MAINTAINR_PROJECTCONF_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
						 MAINTAINR_PROJECTCONF_TYPE,		\
						 MaintainrProjectconfClass))

typedef struct _MaintainrProjectconf		MaintainrProjectconf;
typedef struct _MaintainrProjectconfClass	MaintainrProjectconfClass;
typedef struct _MaintainrProjectconfPrivate	MaintainrProjectconfPrivate;

struct _MaintainrProjectconf {
	GObject				parent;
	MaintainrProjectconfPrivate	*priv;
};

struct _MaintainrProjectconfClass {
	GObjectClass		parent_class;
};

typedef enum {
	PROJECT_PRIORITY_HIGH,
	PROJECT_PRIORITY_MEDIUM,
	PROJECT_PRIORITY_LOW,
	PROJECT_PRIORITY_END
} PROJECT_PRIORITY;

GType			maintainr_projectconfig_get_type ();

MaintainrProjectconf*	maintainr_projectconf_new ();
void			maintainr_projectconf_read (MaintainrProjectconf *conf, xmlNode *node);
gchar*			maintainr_projectconf_write (MaintainrProjectconf *conf);
const gchar*		maintainr_projectconf_get_name (MaintainrProjectconf *conf);
void			maintainr_projectconf_set_name (MaintainrProjectconf *conf, gchar *name);
PROJECT_PRIORITY	maintainr_projectconf_get_priority (MaintainrProjectconf *conf);
void			maintainr_projectconf_set_priority (MaintainrProjectconf *conf, PROJECT_PRIORITY priority);
int			maintainr_projectconf_get_rank (MaintainrProjectconf *conf);
void			maintainr_projectconf_set_rank (MaintainrProjectconf *conf, int rank);
GList*			maintainr_projectconf_get_todos (MaintainrProjectconf *conf);
void			maintainr_projectconf_set_todos (MaintainrProjectconf *conf, GList *todos);
void			maintainr_projectconf_sort_todos (MaintainrProjectconf *conf, GList *todos);
GList*			maintainr_projectconf_get_services (MaintainrProjectconf *conf);
time_t			maintainr_projectconf_get_top_since (MaintainrProjectconf *conf);
int			maintainr_projectconf_get_top_since_days (MaintainrProjectconf *conf);
void			maintainr_projectconf_set_top_now (MaintainrProjectconf *conf);

#endif
