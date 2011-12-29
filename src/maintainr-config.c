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

#include "maintainr-config.h"

#define MAX_BACKUP_COPIES			5

#define MAINTAINR_CONFIG_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_CONFIG_TYPE, MaintainrConfigPrivate))

struct _MaintainrConfigPrivate {
	int window_x;
	int window_y;
	int window_width;
	int window_height;

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

	item->priv->window_x = 0;
	item->priv->window_y = 0;
	item->priv->window_width = 300;
	item->priv->window_height = 500;
}

static gchar* conf_file_path ()
{
	return g_build_filename (g_get_user_config_dir (), "Maintainr", "conf.xml", NULL);
}

static void remove_old_copies (gchar *original_path)
{
	int i;
	gchar *pattern;
	glob_t globbuf;

	pattern = g_strdup_printf ("%s.*", original_path);
	globbuf.gl_offs = 0;
	glob (pattern, 0, NULL, &globbuf);

	if (globbuf.gl_pathc > MAX_BACKUP_COPIES) {
		for (i = 0; i < (globbuf.gl_pathc - MAX_BACKUP_COPIES); i++)
			remove (globbuf.gl_pathv [i]);
	}

	globfree (&globbuf);
}

static gpointer manage_backups (gpointer user_data)
{
	gchar date [100];
	gchar *original_path;
	gchar *original_contents;
	gchar *copy_path;
	int original_fd;
	int copy_fd;
	time_t now_t;
	struct tm now_tm;
	gboolean fail;
	GIOChannel *original_stream;
	GIOChannel *copy_stream;
	GError *error;

	original_path = user_data;
	remove_old_copies (original_path);

	original_fd = open (original_path, O_RDONLY);
	if (original_fd == -1) {
		g_warning ("Unable to open original file: %s", strerror (errno));
		g_free (original_path);
		return NULL;
	}

	original_stream = g_io_channel_unix_new (original_fd);
	error = NULL;
	fail = FALSE;

	if (g_io_channel_read_to_end (original_stream, &original_contents, NULL, &error) == G_IO_STATUS_ERROR) {
		g_warning ("Unable to read original file: %s", error->message);
		g_error_free (error);
		fail = TRUE;
	}

	g_io_channel_unref (original_stream);

	if (fail == TRUE) {
		g_free (original_path);
		return NULL;
	}

	now_t = time (NULL);
	localtime_r (&now_t, &now_tm);
	strftime (date, 100, "%Y%m%d%H%M%S", &now_tm);
	copy_path = g_strdup_printf ("%s.%s.bkp", original_path, date);
	g_free (original_path);

	copy_fd = open (copy_path, O_WRONLY | O_CREAT, 0644);
	if (copy_fd == -1) {
		g_warning ("Unable to open copy file: %s", strerror (errno));
		g_free (copy_path);
		g_free (original_contents);
		return NULL;
	}

	copy_stream = g_io_channel_unix_new (copy_fd);
	g_io_channel_write_chars (copy_stream, original_contents, -1, NULL, NULL);
	g_io_channel_unref (copy_stream);
	g_free (copy_path);
	g_free (original_contents);

	return NULL;
}

void maintainr_config_handle_backup ()
{
	gchar *path;

	path = conf_file_path ();
	if (access (path, F_OK) == 0)
		g_thread_create (manage_backups, path, FALSE, NULL);
}

void read_application_configuration (MaintainrConfig *conf, xmlNode *node)
{
	gchar *str;
	xmlNode *iter;

	for (iter = node; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "windowx") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->window_x = strtoll (str, NULL, 10);
		}
		else if (strcmp ((gchar*) iter->name, "windowy") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->window_y = strtoll (str, NULL, 10);
		}
		else if (strcmp ((gchar*) iter->name, "windowwidth") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->window_width = strtoll (str, NULL, 10);
		}
		else if (strcmp ((gchar*) iter->name, "windowheight") == 0) {
			str = (gchar*) xmlNodeGetContent (iter);
			conf->priv->window_height = strtoll (str, NULL, 10);
		}
	}
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
				else if (strcmp ((gchar*) node->name, "application") == 0) {
					read_application_configuration (ret, node->children);
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

		fprintf (file, "<application>\n");
		fprintf (file, "<windowx>%d</windowx>\n", conf->priv->window_x);
		fprintf (file, "<windowy>%d</windowy>\n", conf->priv->window_y);
		fprintf (file, "<windowwidth>%d</windowwidth>\n", conf->priv->window_width);
		fprintf (file, "<windowheight>%d</windowheight>\n", conf->priv->window_height);
		fprintf (file, "</application>\n");

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

void maintainr_config_get_window_properties (MaintainrConfig *conf, int *width, int *height, int *x, int *y)
{
	*x = conf->priv->window_x;
	*y = conf->priv->window_y;
	*width = conf->priv->window_width;
	*height = conf->priv->window_height;
}

void maintainr_config_set_window_properties (MaintainrConfig *conf, int width, int height, int x, int y)
{
	conf->priv->window_x = x;
	conf->priv->window_y = y;
	conf->priv->window_width = width;
	conf->priv->window_height = height;
}

GList* maintainr_config_get_projects (MaintainrConfig *conf)
{
	return conf->priv->projects;
}

int maintainr_config_get_projects_num (MaintainrConfig *conf)
{
	return g_list_length (conf->priv->projects);
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
			if (force == TRUE) {
				if (conf->priv->projects != NULL)
					maintainr_projectconf_set_top_now (conf->priv->projects->data);
				else
					maintainr_projectconf_set_top_now (NULL);
			}

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
