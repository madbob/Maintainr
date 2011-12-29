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

#include "maintainr-service-gnomeapps.h"
#include <ogd.h>

#define MAINTAINR_SERVICE_GNOMEAPPS_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_SERVICE_GNOMEAPPS_TYPE, MaintainrServiceGnomeappsPrivate))

struct _MaintainrServiceGnomeappsPrivate {
	OGDProvider *provider;
	gchar *contentid;
	gchar *username;
	gchar *password;

	GtkWidget *active;
	GtkWidget *id_entry;
	GtkWidget *username_entry;
	GtkWidget *password_entry;

	int num_fans;
	int num_comments;
	int num_downloads;

	GtkWidget *config_panel;
	GtkWidget *action_panel;
};

G_DEFINE_TYPE (MaintainrServiceGnomeapps, maintainr_service_gnomeapps, MAINTAINR_SERVICE_TYPE);

static const gchar* service_get_name (MaintainrService *service)
{
	return "gtk-apps.org";
}

static void service_read_config (MaintainrService *service, xmlNode *node)
{
	xmlNode *iter;
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	for (iter = node; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "contentid") == 0)
			self->priv->contentid = (gchar*) xmlNodeGetContent (iter);
		else if (strcmp ((gchar*) iter->name, "username") == 0)
			self->priv->username = (gchar*) xmlNodeGetContent (iter);
		else if (strcmp ((gchar*) iter->name, "password") == 0)
			self->priv->password = (gchar*) xmlNodeGetContent (iter);
	}
}

static gchar* service_write_config (MaintainrService *service)
{
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);
	return g_strdup_printf ("<contentid>%s</contentid><username>%s</username><password>%s</password>",
				self->priv->contentid ? self->priv->contentid : "",
				self->priv->username ? self->priv->username : "",
				self->priv->password ? self->priv->password : "");
}

static void empty_action_panel (MaintainrServiceGnomeapps *self)
{
	GList *children;
	GList *iter;

	children = gtk_container_get_children (GTK_CONTAINER (self->priv->action_panel));

	for (iter = children; iter != NULL; iter = iter->next)
		gtk_container_remove (GTK_CONTAINER (self->priv->action_panel), GTK_WIDGET (iter->data));
	g_list_free (children);
}

static void update_visible_data (MaintainrService *service)
{
	gchar *string;
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (self->priv->num_fans != -1 && self->priv->num_comments != -1) {
		empty_action_panel (self);

		string = g_strdup_printf ("Fans: %d", self->priv->num_fans);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), gtk_label_new (string), FALSE, TRUE, 0);
		g_free (string);

		string = g_strdup_printf ("Comments: %d", self->priv->num_comments);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), gtk_label_new (string), FALSE, TRUE, 0);
		g_free (string);

		string = g_strdup_printf ("Downloads: %d", self->priv->num_downloads);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), gtk_label_new (string), FALSE, TRUE, 0);
		g_free (string);

		gtk_widget_show_all (GTK_WIDGET (self->priv->action_panel));
	}
}

static void set_fans (GList *list, gpointer userdata)
{
	GList *iter;
	MaintainrService *service;
	MaintainrServiceGnomeapps *self;

	service = (MaintainrService*) userdata;
	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (list == NULL) {
		self->priv->num_fans = 0;
	}
	else {
		self->priv->num_fans = g_list_length (list);

		for (iter = list; iter != NULL; iter = iter->next)
			g_object_unref (OGD_OBJECT (iter->data));
		g_list_free (list);
	}

	update_visible_data (service);
}

static void set_comments (GList *list, gpointer userdata)
{
	GList *iter;
	MaintainrService *service;
	MaintainrServiceGnomeapps *self;

	service = (MaintainrService*) userdata;
	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (list == NULL) {
		self->priv->num_comments = 0;
	}
	else {
		self->priv->num_comments = g_list_length (list);

		for (iter = list; iter != NULL; iter = iter->next)
			g_object_unref (OGD_OBJECT (iter->data));
		g_list_free (list);
	}

	update_visible_data (service);
}

static void on_ogd_contents (OGDObject *obj, MaintainrService *service)
{
	OGDContent *content;
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (obj == NULL) {
		empty_action_panel (self);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), gtk_label_new ("Unable to retrieve the content on gtk-apps.org"), TRUE, TRUE, 0);
		gtk_widget_show_all (GTK_WIDGET (self->priv->action_panel));
		return;
	}

	content = OGD_CONTENT (obj);

	self->priv->num_fans = self->priv->num_comments = -1;
	self->priv->num_downloads = ogd_content_get_num_downloads (content);
	ogd_content_get_fans_async (content, set_fans, service);
	ogd_content_get_comments_async (content, set_comments, service);
}

static void fill_action_panel (MaintainrService *service)
{
	MaintainrServiceGnomeapps *self;

	if (maintainr_service_get_active (service) == TRUE) {
		self = MAINTAINR_SERVICE_GNOMEAPPS (service);

		empty_action_panel (self);
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), gtk_label_new ("Waiting for contents..."), TRUE, TRUE, 0);

		ogd_provider_auth_user_and_pwd (self->priv->provider, self->priv->username, self->priv->password);
		ogd_content_new_by_id_async (self->priv->provider, self->priv->contentid, (OGDAsyncCallback) on_ogd_contents, service);
	}
}

static void service_config_saved (MaintainrService *service)
{
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (self->priv->contentid != NULL)
		g_free (self->priv->contentid);
	self->priv->contentid = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->id_entry)));

	if (self->priv->username != NULL)
		g_free (self->priv->username);
	self->priv->username = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->username_entry)));

	if (self->priv->password != NULL)
		g_free (self->priv->password);
	self->priv->password = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry)));

	fill_action_panel (service);
}

static void toggle_active_service (GtkToggleButton *button, MaintainrService *service)
{
	maintainr_service_set_active (service, gtk_toggle_button_get_active (button));
}

static GtkWidget* service_config_panel (MaintainrService *service)
{
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (self->priv->config_panel == NULL) {
		self->priv->config_panel = gtk_table_new (4, 2, FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (self->priv->config_panel), 10);

		self->priv->active = gtk_check_button_new ();
		g_signal_connect (G_OBJECT (self->priv->active), "toggled", G_CALLBACK (toggle_active_service), service);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Enable"), 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->active, 1, 2, 0, 1);

		self->priv->id_entry = gtk_entry_new ();
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Content ID"), 0, 1, 1, 2);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->id_entry, 1, 2, 1, 2);

		self->priv->username_entry = gtk_entry_new ();
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Username"), 0, 1, 2, 3);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->username_entry, 1, 2, 2, 3);

		self->priv->password_entry = gtk_entry_new ();
		gtk_entry_set_visibility (GTK_ENTRY (self->priv->password_entry), FALSE);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Password"), 0, 1, 3, 4);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->password_entry, 1, 2, 3, 4);

		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (self->priv->active), maintainr_service_get_active (service));

		if (self->priv->contentid != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->id_entry), self->priv->contentid);
		if (self->priv->username != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->username_entry), self->priv->username);
		if (self->priv->password != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->password_entry), self->priv->password);
	}

	return self->priv->config_panel;
}

static GtkWidget* service_action_panel (MaintainrService *service)
{
	MaintainrServiceGnomeapps *self;

	self = MAINTAINR_SERVICE_GNOMEAPPS (service);

	if (self->priv->action_panel == NULL) {
		self->priv->action_panel = gtk_vbox_new (FALSE, 0);
		fill_action_panel (service);
	}

	return self->priv->action_panel;
}

static GList* service_action_buttons (MaintainrService *service)
{
	return NULL;
}

static void maintainr_service_gnomeapps_finalize (GObject *obj)
{
	MaintainrServiceGnomeapps *conf;

	conf = MAINTAINR_SERVICE_GNOMEAPPS (obj);

	if (conf->priv->contentid != NULL)
		g_free (conf->priv->contentid);
	if (conf->priv->username != NULL)
		g_free (conf->priv->username);
	if (conf->priv->password != NULL)
		g_free (conf->priv->password);
}

static void maintainr_service_gnomeapps_class_init (MaintainrServiceGnomeappsClass *klass)
{
	GObjectClass *gobject_class;
	MaintainrServiceClass *service_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_service_gnomeapps_finalize;

	service_class = MAINTAINR_SERVICE_CLASS (klass);
	service_class->get_name = service_get_name;
	service_class->read_config = service_read_config;
	service_class->write_config = service_write_config;
	service_class->config_saved = service_config_saved;
	service_class->config_panel = service_config_panel;
	service_class->action_panel = service_action_panel;
	service_class->action_buttons = service_action_buttons;

	g_type_class_add_private (klass, sizeof (MaintainrServiceGnomeappsPrivate));
}

static void maintainr_service_gnomeapps_init (MaintainrServiceGnomeapps *item)
{
	item->priv = MAINTAINR_SERVICE_GNOMEAPPS_GET_PRIVATE (item);

	item->priv->provider = ogd_provider_new ("api.gtk-apps.org");
}

MaintainrServiceGnomeapps* maintainr_service_gnomeapps_new ()
{
	return g_object_new (MAINTAINR_SERVICE_GNOMEAPPS_TYPE, NULL);
}
