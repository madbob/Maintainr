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

#include "maintainr-config.h"

#define MAINTAINR_CONFIG_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_CONFIG_TYPE, MaintainrConfigPrivate))

struct _MaintainrConfigPrivate {
	GList *projects;
};

G_DEFINE_TYPE (MaintainrConfig, maintainr_config, G_TYPE_OBJECT);

static void maintainr_config_finalize (GObject *obj)
{
	GList *iter;
	MaintainrConfig *conf;

	conf = MAINTAINR_CONFIG (obj);

	if (conf->priv->projects != NULL) {
		for (iter = conf->priv->projects; iter; iter = iter->next)
			g_object_unref (iter->data);
		g_list_free (conf->priv->projects);
	}
}

static void maintainr_config_class_init (MaintainrConfigClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_config_finalize;

	g_type_class_add_private (klass, sizeof (MaintainrConfigPrivate));
}

static void maintainr_config_init (MaintainrConfig *item)
{
	item->priv = MAINTAINR_CONFIG_GET_PRIVATE (item);
}

static gchar* conf_file_path ()
{
	return g_build_filename (g_get_user_config_dir (), "Maintainr", "conf.xml", NULL);
}

MaintainrConfig* maintainr_config_read_configuration ()
{
	gchar *path;
	xmlDocPtr doc;
	xmlNode *root;
	xmlNode *node;
	MaintainrConfig *ret;
	MaintainrProjectconf *proj;

	ret = g_object_new (MAINTAINR_CONFIG_TYPE, NULL);
	path = conf_file_path ();

	if (access (path, F_OK) == 0) {
		doc = xmlReadFile (path, NULL, XML_PARSE_NOBLANKS);
		g_free (path);

		root = xmlDocGetRootElement (doc);

		if (strcmp ((gchar*) root->name, "maintainr") == 0) {
			for (node = root->children; node; node = node->next) {
				if (strcmp ((gchar*) node->name, "project") == 0) {
					proj = maintainr_projectconf_new ();
					maintainr_projectconf_read (proj, node->children);
					ret->priv->projects = g_list_prepend (ret->priv->projects, proj);
				}
			}
		}

		xmlFreeDoc (doc);
		maintainr_config_sort_projects (ret);
	}

	return ret;
}

void maintainr_config_save (MaintainrConfig *conf)
{
	gchar *path;
	gchar *data;
	FILE *file;
	GList *iter;

	path = conf_file_path ();

	if (access (path, F_OK) != 0) {
		data = g_dirname (path);
		g_mkdir_with_parents (data, 0744);
		g_free (data);
	}

	file = fopen (path, "w");
	if (file != NULL) {
		fprintf (file, "<maintainr>\n\n");

		for (iter = conf->priv->projects; iter; iter = iter->next) {
			data = maintainr_projectconf_write (iter->data);
			fprintf (file, "%s\n", data);
			g_free (data);
		}

		fprintf (file, "\n</maintainr>\n");
		fclose (file);
	}
	else {
		g_warning ("Unable to save configuration on file %s: %s", path, strerror (errno));
	}

	g_free (path);
}

GList* maintainr_config_get_projects (MaintainrConfig *conf)
{
	return conf->priv->projects;
}

void maintainr_config_add_project (MaintainrConfig *conf, MaintainrProjectconf *project)
{
	GList *iter;

	for (iter = conf->priv->projects; iter; iter = iter->next)
		if (iter->data == project)
			return;

	conf->priv->projects = g_list_prepend (conf->priv->projects, project);
	maintainr_config_force_top (conf, project);
}

void maintainr_config_delete_project (MaintainrConfig *conf, MaintainrProjectconf *project)
{
	gboolean force;
	GList *iter;

	for (iter = conf->priv->projects; iter; iter = iter->next)
		if (iter->data == project) {
			force = FALSE;
			if (iter == conf->priv->projects)
				force = TRUE;

			g_object_unref (iter->data);
			conf->priv->projects = g_list_delete_link (conf->priv->projects, iter);

			/*
				This is to be sure to update number of days
				on top for the new top project
			*/
			if (force == TRUE)
				maintainr_projectconf_set_top_now (conf->priv->projects->data);

			return;
		}
}

static int cmp_project_rank (gconstpointer a, gconstpointer b)
{
	return maintainr_projectconf_get_rank ((MaintainrProjectconf*) a) - maintainr_projectconf_get_rank ((MaintainrProjectconf*) b);
}

void maintainr_config_sort_projects (MaintainrConfig *conf)
{
	conf->priv->projects = g_list_sort (conf->priv->projects, cmp_project_rank);
	conf->priv->projects = g_list_reverse (conf->priv->projects);
}

void maintainr_config_step_projects (MaintainrConfig *conf)
{
	int i;
	int min_rank;
	PROJECT_PRIORITY priority;
	MaintainrProjectconf *next_project;
	MaintainrProjectconf *iter_project;
	GList *iter;

	if (conf->priv->projects == NULL)
		return;

	next_project = NULL;
	min_rank = G_MAXINT;
	priority = maintainr_projectconf_get_priority (conf->priv->projects->data);

	for (i = 0; i < PROJECT_PRIORITY_END; i++) {
		priority = (priority + 1) % PROJECT_PRIORITY_END;

		for (iter = conf->priv->projects; iter; iter = iter->next) {
			iter_project = iter->data;

			if (maintainr_projectconf_get_priority (iter_project) == priority) {
				if (maintainr_projectconf_get_rank (iter_project) < min_rank) {
					min_rank = maintainr_projectconf_get_rank (iter_project);
					next_project = iter_project;
				}
			}
		}

		if (next_project != NULL) {
			maintainr_config_force_top (conf, next_project);
			break;
		}
	}
}

static void low_all_ranks (MaintainrConfig *conf)
{
	int min_rank;
	GList *iter;

	min_rank = G_MAXINT;

	/*
		This is to avoid arbitrary grown of the "rank" value, which anyway is a simple
		integer with some limit... Here is provided a way to decrease homogeneusly all
		ranks
	*/
	for (iter = conf->priv->projects; iter; iter = iter->next) {
		if (maintainr_projectconf_get_rank (iter->data) < min_rank)
			min_rank = maintainr_projectconf_get_rank (iter->data);
	}

	if (min_rank != 0)
		for (iter = conf->priv->projects; iter; iter = iter->next)
			maintainr_projectconf_set_rank (iter->data, maintainr_projectconf_get_rank (iter->data) - min_rank);
}

void maintainr_config_force_top (MaintainrConfig *conf, MaintainrProjectconf *project)
{
	int new_rank;

	if (conf->priv->projects != NULL)
		new_rank = maintainr_projectconf_get_rank (conf->priv->projects->data) + 1;
	else
		new_rank = 1;

	maintainr_projectconf_set_rank (project, new_rank);

	if (new_rank > g_list_length (conf->priv->projects) + 1)
		low_all_ranks (conf);

	maintainr_projectconf_set_top_now (project);
	maintainr_config_sort_projects (conf);
}
