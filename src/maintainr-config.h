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

#ifndef MAINTAINR_CONFIG_H
#define MAINTAINR_CONFIG_H

#include "common.h"
#include "maintainr-projectconf.h"

#define MAINTAINR_CONFIG_TYPE			(maintainr_config_get_type ())
#define MAINTAINR_CONFIG(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
						 MAINTAINR_CONFIG_TYPE,			\
						 MaintainrConfig))
#define MAINTAINR_CONFIG_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass),	\
						 MAINTAINR_CONFIG_TYPE,			\
						 MaintainrConfigClass))
#define IS_MAINTAINR_CONFIG(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
						 MAINTAINR_CONFIG_TYPE))
#define IS_MAINTAINR_CONFIG_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
						 MAINTAINR_CONFIG_TYPE))
#define MAINTAINR_CONFIG_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
						 MAINTAINR_CONFIG_TYPE,			\
						 MaintainrConfigClass))

typedef struct _MaintainrConfig		MaintainrConfig;
typedef struct _MaintainrConfigClass	MaintainrConfigClass;
typedef struct _MaintainrConfigPrivate	MaintainrConfigPrivate;

struct _MaintainrConfig {
	GObject			parent;
	MaintainrConfigPrivate	*priv;
};

struct _MaintainrConfigClass {
	GObjectClass		parent_class;
};

GType			maintainr_config_get_type ();

MaintainrConfig*	maintainr_config_read_configuration ();
void			maintainr_config_save (MaintainrConfig *conf);

GList*			maintainr_config_get_projects (MaintainrConfig *conf);
void			maintainr_config_add_project (MaintainrConfig *conf, MaintainrProjectconf *project);
void			maintainr_config_delete_project (MaintainrConfig *conf, MaintainrProjectconf *project);
void			maintainr_config_sort_projects (MaintainrConfig *conf);
void			maintainr_config_step_projects (MaintainrConfig *conf);
void			maintainr_config_force_top (MaintainrConfig *conf, MaintainrProjectconf *project);

#endif
