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

#include "maintainr-service-identica.h"
#include <twitter-glib/twitter-glib.h>

#define MAINTAINR_SERVICE_IDENTICA_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), MAINTAINR_SERVICE_IDENTICA_TYPE, MaintainrServiceIdenticaPrivate))

struct _MaintainrServiceIdenticaPrivate {
	gchar *username;
	gchar *password;
	GtkWidget *username_entry;
	GtkWidget *password_entry;

	GtkWidget *message;
	TwitterClient *client;

	GtkWidget *config_panel;
	GtkWidget *action_panel;
};

G_DEFINE_TYPE (MaintainrServiceIdentica, maintainr_service_identica, MAINTAINR_SERVICE_TYPE);

static void send_status (GtkButton *button, MaintainrServiceIdentica *item)
{
	gchar *text;
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;

	if (item->priv->client != NULL && maintainr_service_get_active (MAINTAINR_SERVICE (item))) {
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (item->priv->message));

		gtk_text_buffer_get_start_iter (buffer, &start);
		gtk_text_buffer_get_end_iter (buffer, &end);
		text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);

		if (text != NULL && strlen (text) > 0) {
			twitter_client_add_status (item->priv->client, text);
			gtk_text_buffer_set_text (buffer, "", -1);
		}
	}

	g_signal_emit_by_name (MAINTAINR_SERVICE (item), "require-main-screen", NULL);
}

static const gchar* service_get_name (MaintainrService *service)
{
	return "identi.ca";
}

static void service_read_config (MaintainrService *service, xmlNode *node)
{
	xmlNode *iter;
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);

	for (iter = node; iter; iter = iter->next) {
		if (strcmp ((gchar*) iter->name, "username") == 0)
			self->priv->username = (gchar*) xmlNodeGetContent (iter);
		else if (strcmp ((gchar*) iter->name, "password") == 0)
			self->priv->password = (gchar*) xmlNodeGetContent (iter);
	}

	if (self->priv->username != NULL && self->priv->password != NULL)
		twitter_client_set_user (self->priv->client, self->priv->username, self->priv->password);
}

static gchar* service_write_config (MaintainrService *service)
{
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);
	return g_strdup_printf ("<username>%s</username><password>%s</password>", self->priv->username, self->priv->password);
}

static void service_config_saved (MaintainrService *service)
{
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);

	if (self->priv->username != NULL)
		g_free (self->priv->username);
	self->priv->username = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->username_entry)));

	if (self->priv->password != NULL)
		g_free (self->priv->password);
	self->priv->password = g_strdup (gtk_entry_get_text (GTK_ENTRY (self->priv->password_entry)));

	twitter_client_set_user (self->priv->client, self->priv->username, self->priv->password);
}

static GtkWidget* service_config_panel (MaintainrService *service)
{
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);

	if (self->priv->config_panel == NULL) {
		self->priv->config_panel = gtk_table_new (2, 2, FALSE);
		gtk_container_set_border_width (GTK_CONTAINER (self->priv->config_panel), 10);

		self->priv->username_entry = gtk_entry_new ();
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Username"), 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->username_entry, 1, 2, 0, 1);

		self->priv->password_entry = gtk_entry_new ();
		gtk_entry_set_visibility (GTK_ENTRY (self->priv->password_entry), FALSE);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), gtk_label_new ("Password"), 0, 1, 1, 2);
		gtk_table_attach_defaults (GTK_TABLE (self->priv->config_panel), self->priv->password_entry, 1, 2, 1, 2);

		if (self->priv->username != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->username_entry), self->priv->username);
		if (self->priv->password != NULL)
			gtk_entry_set_text (GTK_ENTRY (self->priv->password_entry), self->priv->password);
	}

	return self->priv->config_panel;
}

static GtkWidget* service_action_panel (MaintainrService *service)
{
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);

	if (self->priv->action_panel == NULL) {
		self->priv->action_panel = gtk_vbox_new (FALSE, 0);

		self->priv->message = gtk_text_view_new ();
		gtk_box_pack_start (GTK_BOX (self->priv->action_panel), self->priv->message, TRUE, TRUE, 0);

		/**
			TODO	Provide inline URL shorter
		*/
	}

	return self->priv->action_panel;
}

static GtkWidget* service_action_buttons (MaintainrService *service)
{
	GtkWidget *button;
	MaintainrServiceIdentica *self;

	self = MAINTAINR_SERVICE_IDENTICA (service);

	button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text (button, "Send this status change and back to the main screen");
	g_signal_connect (button, "clicked", G_CALLBACK (send_status), self);

	return button;
}

static void maintainr_service_identica_finalize (GObject *obj)
{
	MaintainrServiceIdentica *conf;

	conf = MAINTAINR_SERVICE_IDENTICA (obj);

	if (conf->priv->username != NULL)
		g_free (conf->priv->username);
	if (conf->priv->password != NULL)
		g_free (conf->priv->password);
}

static void maintainr_service_identica_class_init (MaintainrServiceIdenticaClass *klass)
{
	GObjectClass *gobject_class;
	MaintainrServiceClass *service_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = maintainr_service_identica_finalize;

	service_class = MAINTAINR_SERVICE_CLASS (klass);
	service_class->get_name = service_get_name;
	service_class->read_config = service_read_config;
	service_class->write_config = service_write_config;
	service_class->config_saved = service_config_saved;
	service_class->config_panel = service_config_panel;
	service_class->action_panel = service_action_panel;
	service_class->action_buttons = service_action_buttons;

	g_type_class_add_private (klass, sizeof (MaintainrServiceIdenticaPrivate));
}

static void maintainr_service_identica_init (MaintainrServiceIdentica *item)
{
	item->priv = MAINTAINR_SERVICE_IDENTICA_GET_PRIVATE (item);
	item->priv->client = g_object_new (TWITTER_TYPE_CLIENT, "provider", TWITTER_IDENTI_CA, "user-agent", "Maintainr", NULL);
}

MaintainrServiceIdentica* maintainr_service_identica_new ()
{
	return g_object_new (MAINTAINR_SERVICE_IDENTICA_TYPE, NULL);
}
