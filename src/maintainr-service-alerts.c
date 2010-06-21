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

#include "maintainr-service-alerts.h"
#include <libgrss.h>

#define MAINTAINR_SERVICE_ALERTS_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_SERVICE_ALERTS_TYPE, MaintainrServiceAlertsPrivate))

struct _MaintainrServiceAlertsPrivate {
	gchar *url;
	GtkWidget *url_entry;
	FeedsPool *pool;

	GtkWidget *view;

	GtkWidget *config_panel;
	GtkWidget *action_panel;
};

G_DEFINE_TYPE (MaintainrServiceAlerts, maintainr_service_alerts, MAINTAINR_SERVICE_TYPE);

static void alerts_fetched (FeedsPool *pool, FeedChannel *channel, GList *items, MaintainrServiceAlerts *alerts)
{
	const gchar *id;
	gchar *existing_id;
	gboolean has;
	GList *items_iter;
	GtkTreeModel *model;
	GtkTreeIter iter;
	FeedItem *it;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (alerts->priv->view));

	for (items_iter = items; items_iter; items_iter = items_iter->next) {
		it = items_iter->data;
		id = feed_item_get_id (it);
		has = FALSE;

		if (gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
			do {
				gtk_tree_model_get (model, &iter, 0, &existing_id, -1);

				if (strcmp (existing_id, id) == 0)
					has = TRUE;

				g_free (existing_id);

			} while (has == FALSE && gtk_tree_model_iter_next (model, &iter) == TRUE);
		}

		if (has == FALSE) {
			gtk_list_store_insert_with_values (GTK_LIST_STORE (model), NULL, 0,
							   0, id, 1, feed_item_get_title (it), 2, feed_item_get_source (it), -1);
		}
	}
}

static void run_scheduler (MaintainrServiceAlerts *item)
{
	GList *channel;
	FeedChannel *c;

	if (item->priv->url == NULL || strlen (item->priv->url) == 0) {
		feeds_pool_switch (item->priv->pool, FALSE);
	}
	else {
		feeds_pool_switch (item->priv->pool, FALSE);

		channel = feeds_pool_get_listened (item->priv->pool);

		if (channel == NULL) {
			c = feed_channel_new ();
			channel = g_list_prepend (channel, c);
			feeds_pool_listen (item->priv->pool, channel);
		}
		else {
			c = channel->data;
		}

		feed_channel_set_source (c, item->priv->url);
		feeds_pool_switch (item->priv->pool, TRUE);
	}
}

static void open_link_in_browser (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, MaintainrServiceAlerts *item)
{
	gchar *url;
	GtkTreeModel *model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model (tree_view);

	if (gtk_tree_model_get_iter (model, &iter, path)) {
		gtk_tree_model_get (model, &iter, 2, &url, -1);
		gtk_show_uri (NULL, url, GDK_CURRENT_TIME, NULL);
		g_free (url);
	}
}

static const gchar* service_get_name (MaintainrService *service)
{
	return "alerts";
}

static void service_read_config (MaintainrService *service, xmlNode *node)
{
	xmlNode *iter;
	MaintainrServiceAlerts *self;

	self = MAINTAINR_SERVICE_ALERTS (service);

	for (iter = node; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "url") == 0) {
			self->priv->url = (gchar*) xmlNodeGetContent (iter);
			run_scheduler (self);
		}
	}
}

static gchar* service_write_config (MaintainrService *service)
{
	MaintainrServiceAlerts *self;

	self = MAINTAINR_SERVICE_ALERTS (service);
	return g_strdup_printf ("<url>%s</url>", self->priv->url);
}

static void service_config_saved (MaintainrService *service)
{
	MaintainrServiceAlerts *self;

	self = MAINTAINR_SERVICE_ALERTS (service);

	if (self->priv->url != NULL)
		g_free (self->priv->url);
	self->priv->url = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->url_entry)));

	run_scheduler (self);
}

static GtkWidget* service_config_panel (MaintainrService *service)
{
	MaintainrServiceAlerts *self;

	self = MAINTAINR_SERVICE_ALERTS (service);

	if (self->priv->config_panel == NULL) {
		self->priv->config_panel = gtk_table_new (1, 2, FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (self->priv->config_panel), 10);

		self->priv->url_entry = gtk_entry_new ();
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("URL"), 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->url_entry, 1, 2, 0, 1);

		if (self->priv->url != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->url_entry), self->priv->url);
	}

	return self->priv->config_panel;
}

static GtkWidget* service_action_panel (MaintainrService *service)
{
	GtkWidget *scroll;
	GtkListStore *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GtkTreeSelection *selection;
	MaintainrServiceAlerts *self;

	self = MAINTAINR_SERVICE_ALERTS (service);

	if (self->priv->action_panel == NULL) {
		self->priv->action_panel = gtk_vbox_new (FALSE, 0);

		scroll = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), scroll, TRUE, TRUE, 0);

		model = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		self->priv->view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->priv->view), FALSE);
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scroll), self->priv->view);

		renderer = gtk_cell_renderer_text_new ();
		col = gtk_tree_view_column_new_with_attributes ("String", renderer, "text", 1, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->view), col);

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

		g_signal_connect (self->priv->view, "row-activated", G_CALLBACK (open_link_in_browser), self);
	}

	return self->priv->action_panel;
}

static GtkWidget* service_action_buttons (MaintainrService *service)
{
	return NULL;
}

static void maintainr_service_alerts_finalize (GObject *obj)
{
	MaintainrServiceAlerts *conf;

	conf = MAINTAINR_SERVICE_ALERTS (obj);

	if (conf->priv->url != NULL)
		g_free (conf->priv->url);
}

static void maintainr_service_alerts_class_init (MaintainrServiceAlertsClass *klass)
{
	GObjectClass *gobject_class;
	MaintainrServiceClass *service_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_service_alerts_finalize;

	service_class = MAINTAINR_SERVICE_CLASS (klass);
	service_class->get_name = service_get_name;
	service_class->read_config = service_read_config;
	service_class->write_config = service_write_config;
	service_class->config_saved = service_config_saved;
	service_class->config_panel = service_config_panel;
	service_class->action_panel = service_action_panel;
	service_class->action_buttons = service_action_buttons;

	g_type_class_add_private (klass, sizeof (MaintainrServiceAlertsPrivate));
}

static void maintainr_service_alerts_init (MaintainrServiceAlerts *item)
{
	item->priv = MAINTAINR_SERVICE_ALERTS_GET_PRIVATE (item);

	item->priv->pool = feeds_pool_new ();
	g_signal_connect (item->priv->pool, "feed-ready", G_CALLBACK (alerts_fetched), item);
}

MaintainrServiceAlerts* maintainr_service_alerts_new ()
{
	return g_object_new (MAINTAINR_SERVICE_ALERTS_TYPE, NULL);
}
