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

#ifndef MAINTAINR_SERVICE_ALERTS_H
#define MAINTAINR_SERVICE_ALERTS_H

#include "common.h"
#include "maintainr-service.h"

#define MAINTAINR_SERVICE_ALERTS_TYPE			(maintainr_service_alerts_get_type ())
#define MAINTAINR_SERVICE_ALERTS(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
							 MAINTAINR_SERVICE_ALERTS_TYPE,		\
							 MaintainrServiceAlerts))
#define MAINTAINR_SERVICE_ALERTS_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass),	\
							 MAINTAINR_SERVICE_ALERTS_TYPE,		\
							 MaintainrServiceAlertsClass))
#define IS_MAINTAINR_SERVICE_ALERTS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
							 MAINTAINR_SERVICE_ALERTS_TYPE))
#define IS_MAINTAINR_SERVICE_ALERTS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
							 MAINTAINR_SERVICE_ALERTS_TYPE))
#define MAINTAINR_SERVICE_ALERTS_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
							 MAINTAINR_SERVICE_ALERTS_TYPE,		\
							 MaintainrServiceAlertsClass))

typedef struct _MaintainrServiceAlerts	MaintainrServiceAlerts;
typedef struct _MaintainrServiceAlertsClass	MaintainrServiceAlertsClass;
typedef struct _MaintainrServiceAlertsPrivate	MaintainrServiceAlertsPrivate;

struct _MaintainrServiceAlerts {
	MaintainrService		parent;
	MaintainrServiceAlertsPrivate	*priv;
};

struct _MaintainrServiceAlertsClass {
	MaintainrServiceClass		parent_class;
};

GType				maintainr_service_alerts_get_type ();

MaintainrServiceAlerts*		maintainr_service_alerts_new ();

#endif
