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

#ifndef MAINTAINR_SERVICE_GNOMEAPPS_H
#define MAINTAINR_SERVICE_GNOMEAPPS_H

#include "common.h"
#include "maintainr-service.h"

#define MAINTAINR_SERVICE_GNOMEAPPS_TYPE		(maintainr_service_gnomeapps_get_type ())
#define MAINTAINR_SERVICE_GNOMEAPPS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj),	\
							 MAINTAINR_SERVICE_GNOMEAPPS_TYPE,	\
							 MaintainrServiceGnomeapps))
#define MAINTAINR_SERVICE_GNOMEAPPS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass),	\
							 MAINTAINR_SERVICE_GNOMEAPPS_TYPE,	\
							 MaintainrServiceGnomeappsClass))
#define IS_MAINTAINR_SERVICE_GNOMEAPPS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj),	\
							 MAINTAINR_SERVICE_GNOMEAPPS_TYPE))
#define IS_MAINTAINR_SERVICE_GNOMEAPPS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass),	\
							 MAINTAINR_SERVICE_GNOMEAPPS_TYPE))
#define MAINTAINR_SERVICE_GNOMEAPPS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj),	\
							 MAINTAINR_SERVICE_GNOMEAPPS_TYPE,	\
							 MaintainrServiceGnomeappsClass))

typedef struct _MaintainrServiceGnomeapps		MaintainrServiceGnomeapps;
typedef struct _MaintainrServiceGnomeappsClass		MaintainrServiceGnomeappsClass;
typedef struct _MaintainrServiceGnomeappsPrivate	MaintainrServiceGnomeappsPrivate;

struct _MaintainrServiceGnomeapps {
	MaintainrService			parent;
	MaintainrServiceGnomeappsPrivate	*priv;
};

struct _MaintainrServiceGnomeappsClass {
	MaintainrServiceClass		parent_class;
};

GType				maintainr_service_gnomeapps_get_type ();

MaintainrServiceGnomeapps*	maintainr_service_gnomeapps_new ();

#endif
