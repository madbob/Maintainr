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

#include "maintainr-projectconf.h"
#include "maintainr-todo.h"
#include "maintainr-service-identica.h"
#include "maintainr-service-gnomeapps.h"
#include "maintainr-service-alerts.h"

#define MAINTAINR_PROJECTCONF_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_PROJECTCONF_TYPE, MaintainrProjectconfPrivate))

struct _MaintainrProjectconfPrivate {
	gchar *name;
	int priority;
	int rank;
	GList *todos;
	GList *services;
};

G_DEFINE_TYPE (MaintainrProjectconf, maintainr_projectconfig, G_TYPE_OBJECT);

static void maintainr_projectconfig_finalize (GObject *obj)
{
	GList *iter;
	MaintainrProjectconf *conf;

	conf = MAINTAINR_PROJECTCONF (obj);

	if (conf->priv->name != NULL)
		g_free (conf->priv->name);

	if (conf->priv->services != NULL) {
		for (iter = conf->priv->services; iter; iter = iter->next)
			g_object_unref (iter->data);
		g_list_free (conf->priv->services);
	}

	if (conf->priv->todos != NULL) {
		for (iter = conf->priv->todos; iter; iter = iter->next)
			g_object_unref (iter->data);
		g_list_free (conf->priv->todos);
	}
}

static void maintainr_projectconfig_class_init (MaintainrProjectconfClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_projectconfig_finalize;

	g_type_class_add_private (klass, sizeof (MaintainrProjectconfPrivate));
}

static void maintainr_projectconfig_init (MaintainrProjectconf *item)
{
	item->priv = MAINTAINR_PROJECTCONF_GET_PRIVATE (item);
	item->priv->priority = 1;

	item->priv->services = g_list_prepend (item->priv->services, maintainr_service_alerts_new ());
	item->priv->services = g_list_prepend (item->priv->services, maintainr_service_gnomeapps_new ());
	item->priv->services = g_list_prepend (item->priv->services, maintainr_service_identica_new ());
}

MaintainrProjectconf* maintainr_projectconf_new ()
{
	return g_object_new (MAINTAINR_PROJECTCONF_TYPE, NULL);
}

static void parse_todos (MaintainrProjectconf *conf, xmlNode *node)
{
	gchar *str;
	xmlNode *iter;
	xmlNode *subiter;
	MaintainrTodo *t;

	for (iter = node->children; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "todo") == 0) {
			t = maintainr_todo_new ();

			for (subiter = iter->children; subiter; subiter = subiter->next) {
				if (strcmp ((gchar*) subiter->name, "done") == 0) {
					str = (gchar*) xmlNodeGetContent (subiter);
					maintainr_todo_set_done (t, strcmp (str, "true") == 0);
					xmlFree (str);
				}
				else if (strcmp ((gchar*) subiter->name, "content") == 0) {
					str = (gchar*) xmlNodeGetContent (subiter);
					maintainr_todo_set_string (t, str);
					xmlFree (str);
				}
			}

			conf->priv->todos = g_list_prepend (conf->priv->todos, t);
		}
	}

	if (conf->priv->todos != NULL)
		conf->priv->todos = g_list_reverse (conf->priv->todos);
}

void maintainr_projectconf_read (MaintainrProjectconf *conf, xmlNode *node)
{
	gchar *str;
	gchar *active;
	GList *serv;
	xmlNode *iter;

	for (iter = node; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "project-name") == 0) {
			conf->priv->name = (gchar*) xmlNodeGetContent (iter);
		}
		else if (strcmp ((gchar*) iter->name, "priority") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->priority = strtoll (str, NULL, 10);
		}
		else if (strcmp ((gchar*) iter->name, "rank") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->rank = strtoll (str, NULL, 10);
		}
		else if (strcmp ((gchar*) iter->name, "todos") == 0) {
			parse_todos (conf, iter);
		}
		else if (strcmp ((gchar*) iter->name, "service") == 0) {
			str = (gchar*) xmlGetProp (iter, (xmlChar*) "type");
			if (str == NULL)
				continue;

			for (serv = conf->priv->services; serv; serv = serv->next) {
				if (strcmp (maintainr_service_get_name (serv->data), str) == 0) {
					active = (gchar*) xmlGetProp (iter, (xmlChar*) "active");
					if (active != NULL) {
						maintainr_service_set_active (serv->data, strcmp (active, "true") == 0);
						xmlFree (active);
					}
					else {
						maintainr_service_set_active (serv->data, FALSE);
					}

					maintainr_service_read_config (serv->data, iter->children);
					break;
				}
			}

			xmlFree (str);
		}
	}
}

gchar* maintainr_projectconf_write (MaintainrProjectconf *conf)
{
	gchar *str;
	const gchar *name;
	GList *iter;
	GString *ret;

	ret = g_string_new ("<project>\n");

	g_string_append_printf (ret, "\t<project-name>%s</project-name>\n", conf->priv->name);
	g_string_append_printf (ret, "\t<priority>%d</priority>\n", conf->priv->priority);
	g_string_append_printf (ret, "\t<rank>%d</rank>\n", conf->priv->rank);

	for (iter = conf->priv->services; iter; iter = iter->next) {
		str = maintainr_service_write_config (iter->data);
		if (str == NULL)
			continue;

		name = maintainr_service_get_name (iter->data);
		g_string_append_printf (ret, "\t<service type=\"%s\" active=\"%s\">%s</service>\n",
					name, maintainr_service_get_active (iter->data) ? "true" : "false", str);
		g_free (str);
	}

	g_string_append_printf (ret, "\t<todos>\n");

	for (iter = conf->priv->todos; iter; iter = iter->next)
		g_string_append_printf (ret,
					"\t\t<todo><done>%s</done><content>%s</content></todo>\n",
					maintainr_todo_get_done (iter->data) ? "true" : "false",
					maintainr_todo_get_string (iter->data));

	g_string_append_printf (ret, "\t</todos>\n</project>\n");
	return g_string_free (ret, FALSE);
}

const gchar* maintainr_projectconf_get_name (MaintainrProjectconf *conf)
{
	return (const gchar*) conf->priv->name;
}

void maintainr_projectconf_set_name (MaintainrProjectconf *conf, gchar *name)
{
	if (conf->priv->name != NULL)
		g_free (conf->priv->name);
	conf->priv->name = g_strdup (name);
}

int maintainr_projectconf_get_priority (MaintainrProjectconf *conf)
{
	return conf->priv->priority;
}

void maintainr_projectconf_set_priority (MaintainrProjectconf *conf, int priority)
{
	conf->priv->priority = priority;
}

int maintainr_projectconf_get_rank (MaintainrProjectconf *conf)
{
	return conf->priv->rank;
}

void maintainr_projectconf_set_rank (MaintainrProjectconf *conf, int rank)
{
	conf->priv->rank = rank;
}

GList* maintainr_projectconf_get_todos (MaintainrProjectconf *conf)
{
	return conf->priv->todos;
}

void maintainr_projectconf_set_todos (MaintainrProjectconf *conf, GList *todos)
{
	GList *iter;

	if (conf->priv->todos != NULL) {
		for (iter = conf->priv->todos; iter; iter = iter->next)
			g_object_unref (iter->data);
		g_list_free (conf->priv->todos);
	}

	conf->priv->todos = todos;
}

void maintainr_projectconf_sort_todos (MaintainrProjectconf *conf, GList *todos)
{
	conf->priv->todos = todos;
}

GList* maintainr_projectconf_get_services (MaintainrProjectconf *conf)
{
	return conf->priv->services;
}
