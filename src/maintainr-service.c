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

#include "maintainr-service.h"

#define MAINTAINR_SERVICE_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_SERVICE_TYPE, MaintainrServicePrivate))

struct _MaintainrServicePrivate {
	gboolean active;
};

enum {
	MAIN_SCREEN,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL];

G_DEFINE_ABSTRACT_TYPE (MaintainrService, maintainr_service, G_TYPE_OBJECT)

static void maintainr_service_class_init (MaintainrServiceClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (MaintainrServicePrivate));

	signals [MAIN_SCREEN] = g_signal_newv ("require-main-screen",
					G_TYPE_FROM_CLASS (gobject_class), G_SIGNAL_RUN_LAST,
					NULL, NULL, NULL,
					g_cclosure_marshal_VOID__VOID,
					G_TYPE_NONE, 0, NULL);
}

static void maintainr_service_init (MaintainrService *item)
{
	item->priv = MAINTAINR_SERVICE_GET_PRIVATE (item);
}

const gchar* maintainr_service_get_name (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->get_name != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->get_name (service);
	else
		return "Unnamed";
}

gboolean maintainr_service_get_active (MaintainrService *service)
{
	return service->priv->active;
}

void maintainr_service_set_active (MaintainrService *service, gboolean active)
{
	service->priv->active = active;
}

void maintainr_service_read_config (MaintainrService *service, xmlNode *node)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->read_config != NULL)
		MAINTAINR_SERVICE_GET_CLASS (service)->read_config (service, node);
}

gchar* maintainr_service_write_config (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->write_config != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->write_config (service);
	else
		return NULL;
}

void maintainr_service_config_saved (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->config_saved != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->config_saved (service);
}

GtkWidget* maintainr_service_config_panel (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->config_panel != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->config_panel (service);
	else
		return NULL;
}

GtkWidget* maintainr_service_action_panel (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->action_panel != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->action_panel (service);
	else
		return NULL;
}

GList* maintainr_service_action_buttons (MaintainrService *service)
{
	if (MAINTAINR_SERVICE_GET_CLASS (service)->action_buttons != NULL)
		return MAINTAINR_SERVICE_GET_CLASS (service)->action_buttons (service);
	else
		return NULL;
}
