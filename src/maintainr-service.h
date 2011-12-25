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

#ifndef MAINTAINR_SERVICE_H
#define MAINTAINR_SERVICE_H

#include "common.h"

#define MAINTAINR_SERVICE_TYPE			(maintainr_service_get_type ())
#define MAINTAINR_SERVICE(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
						 MAINTAINR_SERVICE_TYPE,		\
						 MaintainrService))
#define MAINTAINR_SERVICE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass),	\
						 MAINTAINR_SERVICE_TYPE,		\
						 MaintainrServiceClass))
#define IS_MAINTAINR_SERVICE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
						 MAINTAINR_SERVICE_TYPE))
#define IS_MAINTAINR_SERVICE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
						 MAINTAINR_SERVICE_TYPE))
#define MAINTAINR_SERVICE_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
						 MAINTAINR_SERVICE_TYPE,		\
						 MaintainrServiceClass))

typedef struct _MaintainrService		MaintainrService;
typedef struct _MaintainrServiceClass		MaintainrServiceClass;
typedef struct _MaintainrServicePrivate		MaintainrServicePrivate;

struct _MaintainrService {
	GObject			parent;
	MaintainrServicePrivate	*priv;
};

struct _MaintainrServiceClass {
	GObjectClass		parent_class;

	const gchar* (*get_name) (MaintainrService *service);
	gboolean (*is_active) (MaintainrService *service);
	void (*set_active) (MaintainrService *service, gboolean active);
	void (*read_config) (MaintainrService *service, xmlNode *node);
	gchar* (*write_config) (MaintainrService *service);
	void (*config_saved) (MaintainrService *service);
	GtkWidget* (*config_panel) (MaintainrService *service);
	GtkWidget* (*action_panel) (MaintainrService *service);
	GList* (*action_buttons) (MaintainrService *service);
};

GType		maintainr_service_get_type ();

const gchar*	maintainr_service_get_name (MaintainrService *service);
gboolean	maintainr_service_get_active (MaintainrService *service);
void		maintainr_service_set_active (MaintainrService *service, gboolean active);
void		maintainr_service_read_config (MaintainrService *service, xmlNode *node);
gchar*		maintainr_service_write_config (MaintainrService *service);
void		maintainr_service_config_saved (MaintainrService *service);
GtkWidget*	maintainr_service_config_panel (MaintainrService *service);
GtkWidget*	maintainr_service_action_panel (MaintainrService *service);
GList*		maintainr_service_action_buttons (MaintainrService *service);

#endif
